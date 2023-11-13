// **************************************************************
//     Filename: wavTrigger.cpp
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, jamie@robertsonics.com
//
// September 2023: Additional track serial query status added and integration into the GPStar Proton Pack and Neutrona Wand codebase. - Michael Rajotte, michael.rajotte@gpstartechnologies.com
// **************************************************************

#include "wavTrigger.h"


// **************************************************************
void wavTrigger::start(void) {

uint8_t txbuf[5];

	versionRcvd = false;
	sysinfoRcvd = false;
	WTSerial.begin(57600);
	flush();

  // Request version string
  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_VERSION;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);

	// Request system info
	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x05;
	txbuf[3] = CMD_GET_SYS_INFO;
	txbuf[4] = EOM;
	WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::flush(void) {
  int i;

	rxCount = 0;
	rxLen = 0;
	rxMsgReady = false;

  for (i = 0; i < MAX_NUM_VOICES; i++) {
    voiceTable[i] = 0xffff;
  }

	while(WTSerial.available())
		WTSerial.read();
}


// **************************************************************
void wavTrigger::update(void) {
  uint8_t dat;

  int i;
  uint8_t voice;
  uint16_t track;

	rxMsgReady = false;
	while (WTSerial.available() > 0) {
		dat = WTSerial.read();
		if ((rxCount == 0) && (dat == SOM1)) {
			rxCount++;
		}
		else if (rxCount == 1) {
			if (dat == SOM2)
				rxCount++;
			else {
				rxCount = 0;
				//Serial.print("Bad msg 1\n");
			}
		}
		else if (rxCount == 2) {
			if (dat <= MAX_MESSAGE_LEN) {
				rxCount++;
				rxLen = dat - 1;
			}
			else {
				rxCount = 0;
				//Serial.print("Bad msg 2\n");
			}
		}
		else if ((rxCount > 2) && (rxCount < rxLen)) {
			rxMessage[rxCount - 3] = dat;
			rxCount++;
		}
		else if (rxCount == rxLen) {
			if (dat == EOM)
				rxMsgReady = true;
			else {
				rxCount = 0;
				//Serial.print("Bad msg 3\n");
			}
		}
		else {
			rxCount = 0;
			//Serial.print("Bad msg 4\n");
		}

		if (rxMsgReady) {
			switch (rxMessage[0]) {
        case RSP_TRACK_REPORT_EX:
          /*
          rxMessage
          0 = RSP_TRACK_REPORT_EX -> 133
          1 = track # lsb
          2 = track # msb
          3 = state -> 1 = playing, 0 = not playing
          */

          track = rxMessage[2];
          track = (track << 8) + rxMessage[1];

          currentMusicTrack = track;

          if(rxMessage[3] == 0) {
            currentMusicStatus = false;
          }
          else {
            currentMusicStatus = true;
          }

          resetTrackCounter(false);
        break;

        case RSP_TRACK_REPORT:
          track = rxMessage[2];
          track = (track << 8) + rxMessage[1] + 1;
          voice = rxMessage[3];
          if (voice < MAX_NUM_VOICES) {
            if (rxMessage[4] == 0) {
              if (track == voiceTable[voice])
                voiceTable[voice] = 0xffff;
            }
            else
              voiceTable[voice] = track;
          }
          // ==========================
          //Serial.print("Track ");
          //Serial.print(track);
          //if (rxMessage[4] == 0)
            //Serial.print(" off\n");
          //else
            //Serial.print(" on\n");
          // ==========================
        break;


        case RSP_VERSION_STRING:
          for (i = 0; i < (VERSION_STRING_LEN - 1); i++)
            version[i] = rxMessage[i + 1];
          version[VERSION_STRING_LEN - 1] = 0;
          versionRcvd = true;
          // ==========================
          //Serial.write(version);
          //Serial.write("\n");
          // ==========================
        break;

				case RSP_SYSTEM_INFO:
					numVoices = rxMessage[1];
					numTracks = rxMessage[3];
					numTracks = (numTracks << 8) + rxMessage[2];
					sysinfoRcvd = true;
					// ==========================
					///\Serial.print("Sys info received\n");
					// ==========================
				break;

			}
			rxCount = 0;
			rxLen = 0;
			rxMsgReady = false;

		} // if (rxMsgReady)

	} // while (WTSerial.available() > 0)
}

bool wavTrigger::currentMusicTrackStatus(int trk) {
  if(trk == currentMusicTrack) {
    if(currentMusicStatus == true) {

      return true;
    }
  }

  return false;
}

bool wavTrigger::trackCounterReset() {
  return trackCounter;
}

void wavTrigger::resetTrackCounter(bool bReset) {
  trackCounter = bReset;
}

// **************************************************************
void wavTrigger::trackPlayingStatus(int trk) {

uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_GET_TRACK_STATUS;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = EOM;
  WTSerial.write(txbuf, 7);
}

// **************************************************************
bool wavTrigger::isTrackPlaying(int trk) {

int i;
bool fResult = false;

  update();
  for (i = 0; i < MAX_NUM_VOICES; i++) {
    if (voiceTable[i] == (uint16_t)trk)
      fResult = true;
  }
  return fResult;
}

// **************************************************************
void wavTrigger::masterGain(int gain) {

uint8_t txbuf[7];
unsigned short vol;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x07;
	txbuf[3] = CMD_MASTER_VOLUME;
	vol = (unsigned short)gain;
	txbuf[4] = (uint8_t)vol;
	txbuf[5] = (uint8_t)(vol >> 8);
	txbuf[6] = EOM;
	WTSerial.write(txbuf, 7);
}

// **************************************************************
void wavTrigger::setAmpPwr(bool enable) {

uint8_t txbuf[6];

    txbuf[0] = SOM1;
    txbuf[1] = SOM2;
    txbuf[2] = 0x06;
    txbuf[3] = CMD_AMP_POWER;
    txbuf[4] = enable;
    txbuf[5] = EOM;
    WTSerial.write(txbuf, 6);
}

// **************************************************************
void wavTrigger::setReporting(bool enable) {

uint8_t txbuf[6];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x06;
	txbuf[3] = CMD_SET_REPORTING;
	txbuf[4] = enable;
	txbuf[5] = EOM;
	WTSerial.write(txbuf, 6);
}

// **************************************************************
bool wavTrigger::getVersion(char *pDst) {

int i;

  update();
  if (!versionRcvd) {
    return false;
  }
  for (i = 0; i < (VERSION_STRING_LEN - 1); i++) {
    pDst[i] = version[i];
  }
  return true;
}

// **************************************************************
int wavTrigger::getNumTracks(void) {

	update();
	return numTracks;
}

// **************************************************************
void wavTrigger::trackPlaySolo(int trk) {

	trackControl(trk, TRK_PLAY_SOLO);
}

// **************************************************************
void wavTrigger::trackPlaySolo(int trk, bool lock) {

	trackControl(trk, TRK_PLAY_SOLO, lock);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk) {

	trackControl(trk, TRK_PLAY_POLY);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk, bool lock) {

	trackControl(trk, TRK_PLAY_POLY, lock);
}

// **************************************************************
void wavTrigger::trackLoad(int trk) {

	trackControl(trk, TRK_LOAD);
}

// **************************************************************
void wavTrigger::trackLoad(int trk, bool lock) {

	trackControl(trk, TRK_LOAD, lock);
}

// **************************************************************
void wavTrigger::trackStop(int trk) {

	trackControl(trk, TRK_STOP);
}

// **************************************************************
void wavTrigger::trackPause(int trk) {

	trackControl(trk, TRK_PAUSE);
}

// **************************************************************
void wavTrigger::trackResume(int trk) {

	trackControl(trk, TRK_RESUME);
}

// **************************************************************
void wavTrigger::trackLoop(int trk, bool enable) {

	if (enable)
		trackControl(trk, TRK_LOOP_ON);
	else
		trackControl(trk, TRK_LOOP_OFF);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code) {

uint8_t txbuf[8];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x08;
	txbuf[3] = CMD_TRACK_CONTROL;
	txbuf[4] = (uint8_t)code;
	txbuf[5] = (uint8_t)trk;
	txbuf[6] = (uint8_t)(trk >> 8);
	txbuf[7] = EOM;
	WTSerial.write(txbuf, 8);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code, bool lock) {

uint8_t txbuf[9];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x09;
	txbuf[3] = CMD_TRACK_CONTROL_EX;
	txbuf[4] = (uint8_t)code;
	txbuf[5] = (uint8_t)trk;
	txbuf[6] = (uint8_t)(trk >> 8);
	txbuf[7] = lock;
	txbuf[8] = EOM;
	WTSerial.write(txbuf, 9);
}

// **************************************************************
void wavTrigger::stopAllTracks(void) {

uint8_t txbuf[5];

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x05;
	txbuf[3] = CMD_STOP_ALL;
	txbuf[4] = EOM;
	WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::resumeAllInSync(void) {

uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::trackGain(int trk, int gain) {

uint8_t txbuf[9];
unsigned short vol;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x09;
	txbuf[3] = CMD_TRACK_VOLUME;
	txbuf[4] = (uint8_t)trk;
	txbuf[5] = (uint8_t)(trk >> 8);
	vol = (unsigned short)gain;
	txbuf[6] = (uint8_t)vol;
	txbuf[7] = (uint8_t)(vol >> 8);
	txbuf[8] = EOM;
	WTSerial.write(txbuf, 9);
}

// **************************************************************
void wavTrigger::trackFade(int trk, int gain, int time, bool stopFlag) {

uint8_t txbuf[12];
unsigned short vol;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x0c;
	txbuf[3] = CMD_TRACK_FADE;
	txbuf[4] = (uint8_t)trk;
	txbuf[5] = (uint8_t)(trk >> 8);
	vol = (unsigned short)gain;
	txbuf[6] = (uint8_t)vol;
	txbuf[7] = (uint8_t)(vol >> 8);
	txbuf[8] = (uint8_t)time;
	txbuf[9] = (uint8_t)(time >> 8);
	txbuf[10] = stopFlag;
	txbuf[11] = EOM;
	WTSerial.write(txbuf, 12);
}

// **************************************************************
void wavTrigger::samplerateOffset(int offset) {

uint8_t txbuf[7];
unsigned short off;

	txbuf[0] = SOM1;
	txbuf[1] = SOM2;
	txbuf[2] = 0x07;
	txbuf[3] = CMD_SAMPLERATE_OFFSET;
	off = (unsigned short)offset;
	txbuf[4] = (uint8_t)off;
	txbuf[5] = (uint8_t)(off >> 8);
	txbuf[6] = EOM;
	WTSerial.write(txbuf, 7);
}

// **************************************************************
void wavTrigger::setTriggerBank(int bank) {

uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_TRIGGER_BANK;
  txbuf[4] = (uint8_t)bank;
  txbuf[5] = EOM;
  WTSerial.write(txbuf, 6);
}
