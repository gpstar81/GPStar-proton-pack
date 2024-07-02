/**
 *   GPStarAudio.h
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *
 *   Based on the serial protocol library by Jamie Robertson.
 *   Compatible with both the GPStar Audio and Robertsonics WAV Trigger audio devices.
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

#pragma once

#if defined(__AVR_ATmega2560__)
  #define GPSTAR_PCB
#endif

// ==================================================================
// The following defines are used to control which serial class is
//  used. Uncomment only the one you wish to use. If all of them are
//  commented out, the library will use Hardware Serial
//#define __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
//#define __GPSTAR_AUDIO_USE_SERIAL1__
//#define __GPSTAR_AUDIO_USE_SERIAL2__
//#define __GPSTAR_AUDIO_USE_SERIAL3__
#ifdef GPSTAR_PCB
  #define __GPSTAR_AUDIO_USE_SERIAL3__
#else
  #define __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
#endif
// ==================================================================

#define CMD_GET_VERSION          1
#define CMD_GET_SYS_INFO         2
#define CMD_TRACK_CONTROL        3
#define CMD_STOP_ALL             4
#define CMD_MASTER_VOLUME        5
#define CMD_TRACK_VOLUME         8
#define CMD_AMP_POWER            9
#define CMD_TRACK_FADE          10
#define CMD_RESUME_ALL_SYNC     11
#define CMD_SAMPLERATE_OFFSET   12
#define CMD_TRACK_CONTROL_EX    13
#define CMD_SET_REPORTING       14
#define CMD_SET_TRIGGER_BANK    15
#define CMD_GET_TRACK_STATUS    16
#define CMD_GET_GPSTAR_HELLO    17
#define CMD_LED_ON              18
#define CMD_LED_OFF             19
#define CMD_SHORT_OVERLOAD_ON   20
#define CMD_SHORT_OVERLOAD_OFF  21
#define CMD_TRACK_FORCE_ON      22
#define CMD_TRACK_FORCE_OFF     23

#define TRK_PLAY_SOLO            0
#define TRK_PLAY_POLY            1
#define TRK_PAUSE                2
#define TRK_RESUME               3
#define TRK_STOP                 4
#define TRK_LOOP_ON              5
#define TRK_LOOP_OFF             6
#define TRK_LOAD                 7

#define RSP_VERSION_STRING     129
#define RSP_SYSTEM_INFO        130
#define RSP_STATUS             131
#define RSP_TRACK_REPORT       132
#define RSP_TRACK_REPORT_EX    133
#define RSP_GPSTAR_HELLO       134
#define MAX_MESSAGE_LEN         32
#define MAX_NUM_VOICES          14
#define VERSION_STRING_LEN      21

#define SOM1   0xf0
#define SOM2   0xaa
#define EOM    0x55


#ifdef __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
#include <AltSoftSerial.h>
#else
#include <HardwareSerial.h>
#ifdef __GPSTAR_AUDIO_USE_SERIAL1__
#define GPStarSerial Serial1
#define __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#endif
#ifdef __GPSTAR_AUDIO_USE_SERIAL2__
#define GPStarSerial Serial2
#define __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#endif
#ifdef __GPSTAR_AUDIO_USE_SERIAL3__
#define GPStarSerial Serial3
#define __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#endif
#ifndef __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#define GPStarSerial Serial
#endif
#endif

class gpstarAudio
{
public:
  gpstarAudio() {;}
  ~gpstarAudio() {;}
  void start(void);
  void update(void);
  void flush(void);
  void setReporting(bool enable);
  void setAmpPwr(bool enable);
  bool getVersion(char *pDst);
  uint16_t getNumTracks(void);
  bool isTrackPlaying(uint16_t trk);
  void masterGain(int16_t gain);
  void stopAllTracks(void);
  void resumeAllInSync(void);
  void trackPlaySolo(uint16_t trk);
  void trackPlaySolo(uint16_t trk, bool lock);
  void trackPlayPoly(uint16_t trk);
  void trackPlayPoly(uint16_t trk, bool lock);
  void trackLoad(uint16_t trk);
  void trackLoad(uint16_t trk, bool lock);
  void trackStop(uint16_t trk);
  void trackPause(uint16_t trk);
  void trackResume(uint16_t trk);
  void trackLoop(uint16_t trk, bool enable);
  void trackGain(uint16_t trk, int16_t gain);
  void trackFade(uint16_t trk, int16_t gain, uint16_t time, bool stopFlag);
  void samplerateOffset(uint16_t offset);
  void setTriggerBank(uint8_t bank);
  void trackPlayingStatus(uint16_t trk);
  bool currentMusicTrackStatus(uint16_t trk);
  bool trackCounterReset(void);
  void resetTrackCounter(bool bReset);
  void serialFlush(void);
  void hello(void);
  void gpstarLEDStatus(bool status);
  void gpstarShortTrackOverload(bool status);
  void gpstarTrackForce(bool status);
  bool wasSysInfoRcvd(void);
  bool gpstarAudioHello(void);

private:
  void trackControl(uint16_t trk, uint8_t code);
  void trackControl(uint16_t trk, uint8_t code, bool lock);

  #ifdef __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
    AltSoftSerial GPStarSerial;
  #endif

  uint16_t voiceTable[MAX_NUM_VOICES];
  uint8_t rxMessage[MAX_MESSAGE_LEN];
  char version[VERSION_STRING_LEN];
  uint16_t numTracks;
  uint8_t numVoices;
  uint8_t rxCount;
  uint8_t rxLen;
  bool rxMsgReady;
  bool versionRcvd;
  bool sysInfoRcvd;
  bool gpsInfoRcvd;
  uint16_t currentMusicTrack;
  bool currentMusicStatus;
  bool trackCounter;
};