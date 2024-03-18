/*  *************************************************************************
 *   GPStar Audio.
 *   Serial communication for GPStar Audio.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *
 *   Serial Protocol library and documentation by Ivan Meleca.
 *   Copyright (c) 2021
 *
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

#include <Arduino.h>
#ifndef __GPSTAR_AUDIO_H__
#define __GPSTAR_AUDIO_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GPSTAR_MAX_PACKET_DATA_SIZE	512
#define GPSTAR_MAX_CHANNELS 14

#define SERIAL_HDR1	                                0x7F
#define SERIAL_HDR2	                                0xAA

#define GPSTAR_AUDIO_CMD_HELLO				              0x01
#define GPSTAR_AUDIO_CMD_STOP_ALL			              0x05
#define GPSTAR_AUDIO_CMD_PAUSE_ALL			            0x08
#define GPSTAR_AUDIO_CMD_RESUME_ALL			            0x0A
#define GPSTAR_AUDIO_CMD_ERROR				              0xFF

#define GPSTAR_AUDIO_CMD_PLAY_GPSTAR_FILE			      0x13
#define GPSTAR_AUDIO_CMD_SET_GPSTAR_TRACK_VOLUME    0x14
#define GPSTAR_AUDIO_CMD_SET_GPSTAR_FADE_TRACK      0x15
#define GPSTAR_AUDIO_CMD_GET_TRACK_COUNT            0x16
#define GPSTAR_AUDIO_CMD_SET_VOLUME                 0x17
#define GPSTAR_AUDIO_CMD_PAUSE_TRACK                0x18
#define GPSTAR_AUDIO_CMD_RESUME_TRACK               0x19
#define GPSTAR_AUDIO_CMD_STOP_TRACK                 0x20
#define GPSTAR_AUDIO_CMD_IS_TRACK_PLAYING           0x21
#define GPSTAR_AUDIO_CMD_SET_TRACK_LOOP             0x22

#define GPSTAR_AUDIO_ERROR_NONE					            0x00
#define GPSTAR_AUDIO_ERROR_NOT_ENOUGH_BUFFER        0x01
#define GPSTAR_AUDIO_ERROR_INVALID_LENGTH		        0x02
#define GPSTAR_AUDIO_ERROR_INVALID_FILE_LENGTH    	0x03
#define GPSTAR_AUDIO_ERROR_INVALID_CHANNEL		      0x04
#define GPSTAR_AUDIO_ERROR_INVALID_MODE			        0x05
#define GPSTAR_AUDIO_ERROR_INTERNAL				          0x06
#define GPSTAR_AUDIO_ERROR_PLAYING				          0x07
#define GPSTAR_AUDIO_ERROR_CRC16_MISMATCH           0x08

#define GPSTAR_AUDIO_ERROR_GPSTAR_FINDING_PLAYER    0x20

#define GPSTAR_AUDIO_ERROR_NOT_PAUSED			          0xFB
#define GPSTAR_AUDIO_ERROR_NOT_PLAYING			        0xFC
#define GPSTAR_AUDIO_ERROR_ON_RX					          0xFD
#define GPSTAR_AUDIO_ERROR_RX_TIMEOUT			          0xFE
#define GPSTAR_AUDIO_ERROR_PARAM					          0xFF

#define GPSTAR_PLAY_NORMAL                          0
#define GPSTAR_PLAY_LOOP                            1

#define GPSTAR_AUDIO_STATUS_STOPPED                 0
#define GPSTAR_AUDIO_STATUS_PLAYING                 1
#define GPSTAR_AUDIO_STATUS_PAUSED                  2

typedef uint32_t (*cbMillis)();
typedef uint8_t (*cbSerialReceiveChar)(uint8_t*, void*);
typedef void (*cbSerialSend)(uint8_t*, size_t, void*);

typedef struct _serialProtocolPacket {
	uint8_t cmd;
	uint16_t data_len;
	uint8_t data[GPSTAR_MAX_PACKET_DATA_SIZE];
} gpstarPacket;

void gpstarInit(cbMillis cbTicks, cbSerialReceiveChar cbReceive, cbSerialSend cbSend, void* param);
uint8_t gpstarHello();
uint8_t gpstarStopAll();
uint8_t gpstarPauseAll();
uint8_t gpstarResumeAll();
uint8_t gpstarPauseTrack(uint16_t i_track);
uint8_t gpstarResumeTrack(uint16_t i_track);
uint8_t gpstarStopTrack(uint16_t i_track);
uint8_t gpstarSetVolume(int16_t volume);
uint8_t gpstarPlayTrack(uint16_t i_track, uint8_t mode);
uint8_t gpstarTrackVolume(uint16_t i_track, float volume);
uint8_t gpstarTrackFade(uint16_t i_track, float volume_target, uint16_t duration);
uint8_t gpstarPullPacket(gpstarPacket* packet, uint32_t timeout);
uint8_t gpstarSetTrackLoop(uint16_t i_track, bool b_loop);
uint16_t gpstarGetTrackCount();

bool gpstarIsTrackPlaying(uint16_t i_track);

void gpstarPushPacket(gpstarPacket* packet);
void gpstarSendErrorCode(uint8_t code);

class GPStarAudio
{
public:
	GPStarAudio(Stream& serial = Serial) :
		serial(serial), last_error(GPSTAR_AUDIO_ERROR_NONE) {}

	~GPStarAudio() {

	}

	static uint8_t serialReceive(uint8_t* c, void* param) {
		GPStarAudio* p = (GPStarAudio*) param;

		if (p->serial.available()) {
			*c = p->serial.read();
			return 1;
		}

		return 0;
	}

	static void serialSend(uint8_t* buf, size_t len, void* param) {
		GPStarAudio* p = (GPStarAudio*) param;
		p->serial.write(buf, len);
	}

	void begin() {
		gpstarInit(millis, serialReceive, serialSend, this);
	}

	bool hello() {
		last_error = gpstarHello();

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

	bool stopAll() {
		last_error = gpstarStopAll();

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

	bool pauseAll() {
		last_error = gpstarPauseAll();

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

	bool resumeAll() {
		last_error = gpstarResumeAll();

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

  bool isTrackPlaying(uint16_t i_track) {
    return gpstarIsTrackPlaying(i_track);
  }

	uint16_t getTrackCount() {
		return gpstarGetTrackCount();
	}

  bool playTrack(uint16_t i_track, uint8_t mode) {
		last_error = gpstarPlayTrack(i_track, mode);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

  // Set the master volume of the system.
	bool setVolume(uint16_t volume) {
		last_error = gpstarSetVolume(volume);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

  // Set the volume of a individual track.
	bool trackVolume(uint16_t i_track, float volume) {
		last_error = gpstarTrackVolume(i_track, volume);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

  bool trackFade(uint16_t i_track, float volume_target, uint16_t i_duration) {
		last_error = gpstarTrackFade(i_track, volume_target, i_duration);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

	bool pauseTrack(uint16_t i_track) {
		last_error = gpstarPauseTrack(i_track);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

	bool resumeTrack(uint16_t i_track) {
		last_error = gpstarResumeTrack(i_track);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

	bool stopTrack(uint16_t i_track) {
		last_error = gpstarStopTrack(i_track);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
	}

    bool onSetLoop(uint16_t i_track, bool b_loop) {
		last_error = gpstarSetTrackLoop(i_track, b_loop);

		return (last_error == GPSTAR_AUDIO_ERROR_NONE);
    }

	inline uint8_t getLastError() { 
    return last_error; 
  }

private:
	Stream& serial;
	uint8_t last_error;
};

#ifdef __cplusplus
}
#endif

#endif // __GPSTAR_AUDIO_H__