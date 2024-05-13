/**
 *   GPStarAudio.cpp
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *
 *   Based on the serial protocol library by Jamie Robertson.
 *   Compatible with both the GPStar Audio and Robertonics WAV Trigger audio devices.
 *   Copyright (c) 2014

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "GPStarAudio.h"

void gpstarAudio::start(void) {
  uint8_t txbuf[5];

  versionRcvd = false;
  sysInfoRcvd = false;
  gpsInfoRcvd = false;
  GPStarSerial.begin(57600);
  flush();

  // Request version string from a WAV Trigger.
  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_VERSION;
  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);

  // Request system info from a WAV Trigger.
  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_SYS_INFO;
  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

void gpstarAudio::flush(void) {
  rxCount = 0;
  rxLen = 0;
  rxMsgReady = false;

  for(uint8_t i = 0; i < MAX_NUM_VOICES; i++) {
    voiceTable[i] = 0xffff;
  }

  while(GPStarSerial.available()) {
    GPStarSerial.read();
  }
}

void gpstarAudio::serialFlush(void) {
  GPStarSerial.flush();
}

void gpstarAudio::update(void) {
  uint8_t dat;
  uint8_t voice;
  uint16_t track;

  rxMsgReady = false;

  while(GPStarSerial.available() > 0) {
    dat = GPStarSerial.read();

    if((rxCount == 0) && (dat == SOM1)) {
      rxCount++;
    }
    else if(rxCount == 1) {
      if(dat == SOM2) {
        rxCount++;
      }
      else {
        rxCount = 0; // Bad serial data.
      }
    }
    else if(rxCount == 2) {
      if(dat == SOM1 || dat == SOM2 || dat == EOM) {
        rxCount = 0; // Bad serial data.
      }
      else if(dat <= MAX_MESSAGE_LEN) {
        rxCount++;
        rxLen = dat - 1;
      }
      else {
        rxCount = 0; // Bad serial data.
      }
    }
    else if((rxCount > 2) && (rxCount < rxLen)) {
      if(dat == SOM1 || dat == SOM2 || dat == EOM) {
        rxCount = 0; // Bad serial data.
      }
      else {
        rxMessage[rxCount - 3] = dat;
        rxCount++;
      }
    }
    else if(rxCount == rxLen) {
      if(dat == EOM) {
        rxMsgReady = true;
      }
      else {
        rxCount = 0; // Bad serial data.
      }
    }
    else {
      rxCount = 0; // Bad serial data.
    }

    if(rxMsgReady) {
      switch (rxMessage[0]) {
        case RSP_TRACK_REPORT_EX:
          track = rxMessage[2];
          track = (track << 8) + rxMessage[1];

          currentMusicTrack = track;

          // 0 = not playing. 1 = playing.
          if(rxMessage[3] == 0) {
            currentMusicStatus = false;
          }
          else {
            currentMusicStatus = true;
          }

          resetTrackCounter(false);
        break;

        case RSP_TRACK_REPORT:
          // Specific for WAV Triggers.
          track = rxMessage[2];
          track = (track << 8) + rxMessage[1] + 1;
          voice = rxMessage[3];
          if(voice < MAX_NUM_VOICES) {
            if(rxMessage[4] == 0) {
              if(track == voiceTable[voice])
                voiceTable[voice] = 0xffff;
            }
            else
              voiceTable[voice] = track;
          }
        break;

        case RSP_VERSION_STRING:
          // Specific for WAV Triggers.
          for(uint8_t i = 0; i < (VERSION_STRING_LEN - 1); i++) {
            version[i] = rxMessage[i + 1];
          }
          version[VERSION_STRING_LEN - 1] = 0;
          versionRcvd = true;
        break;

        case RSP_SYSTEM_INFO:
          // Specific for WAV Triggers.
          numVoices = rxMessage[1];
          numTracks = rxMessage[3];
          numTracks = (numTracks << 8) + rxMessage[2];
          sysInfoRcvd = true;
        break;

        case RSP_GPSTAR_HELLO:
          // Specific for GPStar Audio.
          numVoices = rxMessage[1];
          numTracks = rxMessage[3];
          numTracks = (numTracks << 8) + rxMessage[2];
          gpsInfoRcvd = true;
        break;
      }

      rxCount = 0;
      rxLen = 0;
      rxMsgReady = false;
    }
  }
}

bool gpstarAudio::currentMusicTrackStatus(uint16_t trk) {
  if(trk == currentMusicTrack) {
    if(currentMusicStatus == true) {
      return true;
    }
  }

  return false;
}

bool gpstarAudio::trackCounterReset() {
  return trackCounter;
}

void gpstarAudio::resetTrackCounter(bool bReset) {
  trackCounter = bReset;
}

void gpstarAudio::trackPlayingStatus(uint16_t trk) {
  uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_GET_TRACK_STATUS;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = EOM;
  GPStarSerial.write(txbuf, 7);
}

bool gpstarAudio::isTrackPlaying(uint16_t trk) {
  update();

  for(uint8_t i = 0; i < MAX_NUM_VOICES; i++) {
    if(voiceTable[i] == trk) {
      return true;
    }
  }

  return false;
}

void gpstarAudio::masterGain(int16_t gain) {
  uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_MASTER_VOLUME;
  txbuf[4] = (uint8_t)gain;
  txbuf[5] = (uint8_t)(gain >> 8);
  txbuf[6] = EOM;
  GPStarSerial.write(txbuf, 7);
}

void gpstarAudio::setAmpPwr(bool enable) {
  uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_AMP_POWER;
  txbuf[4] = enable;
  txbuf[5] = EOM;
  GPStarSerial.write(txbuf, 6);
}

void gpstarAudio::setReporting(bool enable) {
  uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_REPORTING;
  txbuf[4] = enable;
  txbuf[5] = EOM;
  GPStarSerial.write(txbuf, 6);
}

bool gpstarAudio::getVersion(char *pDst) {
  update();

  if(!versionRcvd) {
    return false;
  }

  for(uint8_t i = 0; i < (VERSION_STRING_LEN - 1); i++) {
    pDst[i] = version[i];
  }

  return true;
}

uint16_t gpstarAudio::getNumTracks(void) {
  update();

  return numTracks;
}

void gpstarAudio::trackPlaySolo(uint16_t trk) {
  trackControl(trk, TRK_PLAY_SOLO);
}

void gpstarAudio::trackPlaySolo(uint16_t trk, bool lock) {
  trackControl(trk, TRK_PLAY_SOLO, lock);
}

void gpstarAudio::trackPlayPoly(uint16_t trk) {
  trackControl(trk, TRK_PLAY_POLY);
}

void gpstarAudio::trackPlayPoly(uint16_t trk, bool lock) {
  trackControl(trk, TRK_PLAY_POLY, lock);
}

void gpstarAudio::trackLoad(uint16_t trk) {
  trackControl(trk, TRK_LOAD);
}

void gpstarAudio::trackLoad(uint16_t trk, bool lock) {
  trackControl(trk, TRK_LOAD, lock);
}

void gpstarAudio::trackStop(uint16_t trk) {
  trackControl(trk, TRK_STOP);
}

void gpstarAudio::trackPause(uint16_t trk) {
  trackControl(trk, TRK_PAUSE);
}

void gpstarAudio::trackResume(uint16_t trk) {
  trackControl(trk, TRK_RESUME);
}

void gpstarAudio::trackLoop(uint16_t trk, bool enable) {
  if(enable) {
    trackControl(trk, TRK_LOOP_ON);
  }
  else {
    trackControl(trk, TRK_LOOP_OFF);
  }
}

void gpstarAudio::trackControl(uint16_t trk, uint8_t code) {
  uint8_t txbuf[8];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x08;
  txbuf[3] = CMD_TRACK_CONTROL;
  txbuf[4] = code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = EOM;
  GPStarSerial.write(txbuf, 8);
}

void gpstarAudio::trackControl(uint16_t trk, uint8_t code, bool lock) {
  uint8_t txbuf[9];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_CONTROL_EX;
  txbuf[4] = code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = lock;
  txbuf[8] = EOM;
  GPStarSerial.write(txbuf, 9);
}

void gpstarAudio::stopAllTracks(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_STOP_ALL;
  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

void gpstarAudio::resumeAllInSync(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

void gpstarAudio::trackGain(uint16_t trk, int16_t gain) {
  uint8_t txbuf[9];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_VOLUME;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = (uint8_t)gain;
  txbuf[7] = (uint8_t)(gain >> 8);
  txbuf[8] = EOM;
  GPStarSerial.write(txbuf, 9);
}

void gpstarAudio::trackFade(uint16_t trk, int16_t gain, uint16_t time, bool stopFlag) {
  uint8_t txbuf[12];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = (uint8_t)gain;
  txbuf[7] = (uint8_t)(gain >> 8);
  txbuf[8] = (uint8_t)time;
  txbuf[9] = (uint8_t)(time >> 8);
  txbuf[10] = stopFlag;
  txbuf[11] = EOM;
  GPStarSerial.write(txbuf, 12);
}

void gpstarAudio::samplerateOffset(uint16_t offset) {
  uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_SAMPLERATE_OFFSET;
  txbuf[4] = (uint8_t)offset;
  txbuf[5] = (uint8_t)(offset >> 8);
  txbuf[6] = EOM;
  GPStarSerial.write(txbuf, 7);
}

void gpstarAudio::setTriggerBank(uint8_t bank) {
  uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_TRIGGER_BANK;
  txbuf[4] = bank;
  txbuf[5] = EOM;
  GPStarSerial.write(txbuf, 6);
}

// Turn on or off the LED on GPStar Audio. Default is on.
void gpstarAudio::gpstarLEDStatus(bool status) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;

  if(status == true) {
    txbuf[3] = CMD_LED_ON;
  }
  else {
    txbuf[3] = CMD_LED_OFF;
  }

  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

// Turn on track short overload or turn it off.
void gpstarAudio::gpstarShortTrackOverload(bool status) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;

  if(status == true) {
    txbuf[3] = CMD_SHORT_OVERLOAD_ON;
  }
  else {
    txbuf[3] = CMD_SHORT_OVERLOAD_OFF;
  }

  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

// Turn on track force or turn it off.
void gpstarAudio::gpstarTrackForce(bool status) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;

  if(status == true) {
    txbuf[3] = CMD_TRACK_FORCE_ON;
  }
  else {
    txbuf[3] = CMD_TRACK_FORCE_OFF;
  }

  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

void gpstarAudio::hello(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_GPSTAR_HELLO;
  txbuf[4] = EOM;
  GPStarSerial.write(txbuf, 5);
}

bool gpstarAudio::wasSysInfoRcvd(void) {
  update();

  return sysInfoRcvd;
}

bool gpstarAudio::gpstarAudioHello(void) {
  update();

  return gpsInfoRcvd;
}