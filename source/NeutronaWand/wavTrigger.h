// **************************************************************
//     Filename: wavTrigger.h
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, info@robertsonics.com
//
//
// **************************************************************
//
// Revision History
//
// Date      Description
// --------  -----------
//
// 02/22/14  First version created.
//           LIMITATIONS: Hard-coded for AltSoftwareSerial Library.
//           Also only supports commands TO the WAV Trigger. Will
//           fix these things.
//
// 05/10/14  Tested with UNO. Added new functions for fades, cross-
//           fades and starting multiple tracks in sample sync.
//
// 04/26/15  Added support for sample-rate / pitch bend control,
//           and compile macro switches for hardware serial ports.
//
// 11/06/16  Support for v1.30 firmware features, including two-way
//           communication with track status reporting.
//
// 09/05/23  Additional track serial query status added and integration into the GPStar Proton Pack and Neutrona Wand codebase. - Michael Rajotte, michael.rajotte@gpstartechnologies.com

#ifndef WAVTRIGGER_H
#define WAVTRIGGER_H

#if defined(__AVR_ATmega2560__)
  #define GPSTAR_PCB
#endif

// ==================================================================
// The following defines are used to control which serial class is
//  used. Uncomment only the one you wish to use. If all of them are
//  commented out, the library will use Hardware Serial
//#define __WT_USE_ALTSOFTSERIAL__
//#define __WT_USE_SERIAL1__
//#define __WT_USE_SERIAL2__
//#define __WT_USE_SERIAL3__
#ifdef GPSTAR_PCB
  #define __WT_USE_SERIAL3__
#else
  #define __WT_USE_ALTSOFTSERIAL__
#endif
// ==================================================================

#define CMD_GET_VERSION					1
#define CMD_GET_SYS_INFO				2
#define CMD_TRACK_CONTROL				3
#define CMD_STOP_ALL					4
#define CMD_MASTER_VOLUME				5
#define CMD_TRACK_VOLUME				8
#define CMD_AMP_POWER					9
#define CMD_TRACK_FADE					10
#define CMD_RESUME_ALL_SYNC				11
#define CMD_SAMPLERATE_OFFSET			12
#define	CMD_TRACK_CONTROL_EX			13
#define	CMD_SET_REPORTING				14
#define CMD_SET_TRIGGER_BANK			15
#define CMD_GET_TRACK_STATUS			16

#define TRK_PLAY_SOLO					0
#define TRK_PLAY_POLY					1
#define TRK_PAUSE						2
#define TRK_RESUME						3
#define TRK_STOP						4
#define TRK_LOOP_ON						5
#define TRK_LOOP_OFF					6
#define TRK_LOAD						7

#define	RSP_VERSION_STRING				129
#define	RSP_SYSTEM_INFO					130
#define	RSP_STATUS						131
#define	RSP_TRACK_REPORT				132
#define RSP_TRACK_REPORT_EX				133
#define MAX_MESSAGE_LEN					32
#define MAX_NUM_VOICES					14
#define VERSION_STRING_LEN				21

#define SOM1	0xf0
#define SOM2	0xaa
#define EOM		0x55


#ifdef __WT_USE_ALTSOFTSERIAL__
#include <AltSoftSerial.h>
#else
#include <HardwareSerial.h>
#ifdef __WT_USE_SERIAL1__
#define WTSerial Serial1
#define __WT_SERIAL_ASSIGNED__
#endif
#ifdef __WT_USE_SERIAL2__
#define WTSerial Serial2
#define __WT_SERIAL_ASSIGNED__
#endif
#ifdef __WT_USE_SERIAL3__
#define WTSerial Serial3
#define __WT_SERIAL_ASSIGNED__
#endif
#ifndef __WT_SERIAL_ASSIGNED__
#define WTSerial Serial
#endif
#endif

class wavTrigger
{
public:
	wavTrigger() {;}
	~wavTrigger() {;}
	void start(void);
  void update(void);
  void flush(void);
	void setReporting(bool enable);
	void setAmpPwr(bool enable);
  #ifdef GPSTAR_PCB
	  bool getVersion(char *pDst);
  #endif

	int getNumTracks(void);

  #ifdef GPSTAR_PCB
	  bool isTrackPlaying(int trk);
  #endif

	void masterGain(int gain);
	void stopAllTracks(void);
	void resumeAllInSync(void);
	void trackPlaySolo(int trk);
	void trackPlaySolo(int trk, bool lock);
	void trackPlayPoly(int trk);
	void trackPlayPoly(int trk, bool lock);
	void trackLoad(int trk);
	void trackLoad(int trk, bool lock);
	void trackStop(int trk);
	void trackPause(int trk);
	void trackResume(int trk);
	void trackLoop(int trk, bool enable);
	void trackGain(int trk, int gain);
	void trackFade(int trk, int gain, int time, bool stopFlag);
  
  void samplerateOffset(int offset);
  
  #ifdef GPSTAR_PCB
	  void setTriggerBank(int bank);
  #endif

  #ifdef GPSTAR_PCB
    void trackPlayingStatus(int trk);
    bool currentMusicTrackStatus(int trk);
    bool trackCounterReset(void);
    void resetTrackCounter(bool bReset);
  #endif

private:
	void trackControl(int trk, int code);
	void trackControl(int trk, int code, bool lock);

  #ifdef __WT_USE_ALTSOFTSERIAL__
    AltSoftSerial WTSerial;
  #endif

  #ifdef GPSTAR_PCB 
    uint16_t voiceTable[MAX_NUM_VOICES];
  #endif

	uint8_t rxMessage[MAX_MESSAGE_LEN];
  
  #ifdef GPSTAR_PCB
	  char version[VERSION_STRING_LEN];
  #endif

	uint16_t numTracks;
	uint8_t numVoices;
	uint8_t rxCount;
	uint8_t rxLen;
	bool rxMsgReady;
	bool versionRcvd;
	bool sysinfoRcvd;

  #ifdef GPSTAR_PCB
    int currentMusicTrack;
    bool currentMusicStatus;
    bool trackCounter;
  #endif
};

#endif