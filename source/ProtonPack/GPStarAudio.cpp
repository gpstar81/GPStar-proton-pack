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

#include "GPStarAudio.h"
#include <string.h>

static cbMillis get_millis = NULL;
static cbSerialReceiveChar serial_receive = NULL;
static cbSerialSend serial_send = NULL;
static void* cb_param = NULL;
static uint8_t initialized = 0;
static uint16_t out_crc16;
static uint8_t little_endian = 1;

static uint32_t rx_timeout = 0;
static uint8_t rx_enabled = 0;
static uint8_t rx_state;
static uint32_t rx_offset;
static uint32_t rx_packet_timeout = 0;
static uint16_t rx_crc;
static uint16_t rx_calc_crc;
static uint16_t rx_len;

#define SWAP16(x) (((x & 0xFF) << 8) | ((x >> 8) & 0xFF))

static uint16_t gpstarCRC16(void* data, uint32_t len, uint16_t partial) {
	uint16_t crc = partial;
	uint32_t i;
	uint8_t j, c;
	uint8_t* ptr = (uint8_t*) data;

	for(i = 0; i < len; i++) {
		c = ((crc>>8) & 0x00FF);
		c ^= *ptr++;
		crc = ((uint16_t) c << 8) | (crc & 0x00FF);
		for(j = 0; j < 8; j++) {
			if(crc & 0x8000) {
				crc <<= 1;
				crc ^= 0x1021;
			}
      else {
				crc <<= 1;
			}
		}
	}

	return crc;
}

static void resetRx() {
	rx_timeout = 0;
	rx_enabled = 0;
	rx_state = 0;
	rx_offset = 0;
	rx_packet_timeout = 0;
	rx_crc = 0;
	rx_calc_crc = 0;
	rx_len = 0;
}

static void gpstarStartOutput() {
  uint8_t hdr;
  out_crc16 = 0;
  uint8_t c;

  // Clean RX buffer
  while (serial_receive(&c, cb_param));

  hdr = SERIAL_HDR1;
  out_crc16 = gpstarCRC16(&hdr, 1, 0);
  serial_send(&hdr, 1, cb_param);

  hdr = SERIAL_HDR2;
  out_crc16 = gpstarCRC16(&hdr, 1, out_crc16);
  serial_send(&hdr, 1, cb_param);
}

static void gpstarOutput(uint8_t* data, uint32_t len) {
  out_crc16 = gpstarCRC16(data, len, out_crc16);
  serial_send(data, len, cb_param);
}

static void gpstarEndOutput() {
  if(!little_endian) {
    out_crc16 = SWAP16(out_crc16);
  }

  serial_send((uint8_t*) &out_crc16, 2, cb_param);
}

static void gpstarSendCommand(uint8_t cmd, uint8_t* data, uint16_t len) {
  uint16_t txlen = len;

  gpstarStartOutput();
  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    txlen = SWAP16(txlen);
  }

  gpstarOutput((uint8_t*) &txlen, 2);

  if(data && len) {
    gpstarOutput(data, len);
  }

  gpstarEndOutput();
}

void gpstarInit(cbMillis cbTicks, cbSerialReceiveChar cbReceive, cbSerialSend cbSend, void* param) {
  uint16_t test = 0x01;
  uint8_t* test_ptr = (uint8_t*) &test;

  if(initialized) {
      return;
  }

  get_millis = cbTicks;
  serial_receive = cbReceive;
  serial_send = cbSend;
  cb_param = param;

  little_endian = *test_ptr == 1;

  resetRx();

  initialized = 1;
}

static uint8_t gpstarPullData(uint8_t* cmd, uint8_t* data, uint16_t* len) {
  uint8_t c;
	uint8_t error;
  uint32_t timeout = 250;

  if(!initialized || !cmd) {
    return 0;
  }

  resetRx();

  rx_packet_timeout = get_millis();

	while((get_millis() - rx_packet_timeout) < timeout) {
    if(!serial_receive(&c, cb_param)) {
    // Check for 50ms timeout between received chars.
    // The count is reset at every received char.
      if(rx_timeout != 0 && (get_millis() - rx_timeout > 50))
        resetRx();
        continue;
    }

    // Reset char receiver timeout
    rx_timeout = get_millis();

    switch (rx_state) {
      case 0:
        if(c == SERIAL_HDR1) {
          rx_calc_crc = gpstarCRC16(&c, 1, 0);
          rx_state++;
        }
      break;

      case 1:
        if(c == SERIAL_HDR2) {
          rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
          rx_state++;
        }
        else {
          resetRx();
        }
      break;

      case 2:
        *cmd = c;
        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
        rx_state++;
      break;

      case 3:
        if(little_endian) {
          rx_len = c;
        }
        else {
          rx_len = c << 8;
        }

        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
        rx_state++;
      break;

      case 4:
        if(little_endian) {
          rx_len |= c << 8;
        }
        else {
          rx_len |= c;
        }

        if(!rx_len) {
          rx_state = 6;
        }
        else {
          if(!len || *len < rx_len || !data) {
            // Check if it is an error code. Use the 'error' variable if the user did not provide any data.
            if (*cmd == GPSTAR_AUDIO_CMD_ERROR && !data && rx_len == 1) {
              data = &error;
            }
            else {
              return GPSTAR_AUDIO_ERROR_NOT_ENOUGH_BUFFER;
            }
          }

          if(rx_len > GPSTAR_MAX_PACKET_DATA_SIZE) {
            return GPSTAR_AUDIO_ERROR_INVALID_LENGTH;
          }

          rx_state++;
        }

        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
      break;

      case 5:
        if(rx_offset == GPSTAR_MAX_PACKET_DATA_SIZE)  {
          // Overflow
          return GPSTAR_AUDIO_ERROR_INVALID_LENGTH;
        }

        data[rx_offset++] = c;
        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);

        if(rx_offset == rx_len) {
          rx_state++;
        }
      break;

      case 6:
        if(little_endian) {
          rx_crc = c;
        }
        else {
          rx_crc = c << 8;
        }

        rx_state++;
      break;

      case 7:
        if(little_endian) {
          rx_crc |= c << 8;
        }
        else {
          rx_crc |= c;
        }

        if(rx_crc == rx_calc_crc) {
          if(len) {
            *len = rx_len;
          }

          if(*cmd == GPSTAR_AUDIO_CMD_ERROR) {
            if(rx_len == 1 && data) {
              return data[0];
            }

            return GPSTAR_AUDIO_ERROR_ON_RX;
          }

          return GPSTAR_AUDIO_ERROR_NONE;
        }

      return GPSTAR_AUDIO_ERROR_CRC16_MISMATCH;
    }

	}

	return GPSTAR_AUDIO_ERROR_RX_TIMEOUT;
}

uint8_t gpstarPullPacket(gpstarPacket* packet, uint32_t timeout) {
  uint8_t c;

  if(!initialized) {
    return 0;
  }

  rx_packet_timeout = get_millis();

	while((get_millis() - rx_packet_timeout) < timeout) {
    if(!serial_receive(&c, cb_param)) {
      // Check for 50ms timeout between received chars. The count is reset at every received char.
      if(rx_timeout != 0 && (get_millis() - rx_timeout > 50)) {
        resetRx();
      }

      continue;
    }

    // Reset char receiver timeout
    rx_timeout = get_millis();

    switch (rx_state) {
      case 0:
        if (c == SERIAL_HDR1) {
          rx_calc_crc = gpstarCRC16(&c, 1, 0);
          rx_state++;
        }
      break;

      case 1:
        if(c == SERIAL_HDR2) {
          rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
          rx_state++;
        }
      break;

      case 2:
        packet->cmd = c;
        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
        rx_state++;
      break;

      case 3:
        if(little_endian) {
          packet->data_len = c;
        }
        else {
          packet->data_len = c << 8;

          rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
          rx_state++;
        }
      break;

      case 4:
        if(little_endian) {
          packet->data_len |= c << 8;
        }
        else {
          packet->data_len |= c;
        }

        if(!packet->data_len) {
          rx_state = 6;
        }
        else {
          rx_state++;
        }

        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);
      break;

      case 5:
        if(rx_offset == GPSTAR_MAX_PACKET_DATA_SIZE) {
          // Overflow
          resetRx();
          break;
        }

        packet->data[rx_offset++] = c;
        rx_calc_crc = gpstarCRC16(&c, 1, rx_calc_crc);

        if(rx_offset == packet->data_len) {
          rx_state++;
        }
      break;

      case 6:
        if(little_endian) {
          rx_crc = c;
        }
        else {
          rx_crc = c << 8;
        }

        rx_state++;
      break;

      case 7:
        if(little_endian) {
          rx_crc |= c << 8;
        }
        else {
          rx_crc |= c;
        }

        c = (rx_crc == rx_calc_crc) ? 1 : 0;
        resetRx();

        return c;
    }

  }

	return 0;
}

void gpstarPushPacket(gpstarPacket* packet) {
  uint16_t len;

  if(!initialized) {
    return;
  }

  gpstarStartOutput();

  gpstarOutput(&packet->cmd, 1);

  if(little_endian) {
    len = packet->data_len;
  }
  else {
    len = SWAP16(packet->data_len);
  }

  gpstarOutput((uint8_t*) &len, 2);

	if(packet->data_len) {
    gpstarOutput(packet->data, packet->data_len);
  }

  gpstarEndOutput();
}

void gpstarSendErrorCode(uint8_t code) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_ERROR;
  uint16_t len = 1;

  gpstarStartOutput();

  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    len = SWAP16(len);
  }

  gpstarOutput((uint8_t*) &len, 2);

  gpstarOutput(&code, 1);

  gpstarEndOutput();
}

uint8_t gpstarHello() {
  uint8_t cmd = GPSTAR_AUDIO_CMD_HELLO;
  uint8_t res;

  gpstarSendCommand(cmd, NULL, 0);

  res = gpstarPullData(&cmd, NULL, 0);

  if(res != GPSTAR_AUDIO_ERROR_NONE) {
    return res;
  }

  if(cmd != GPSTAR_AUDIO_CMD_HELLO) {
    return GPSTAR_AUDIO_ERROR_ON_RX;
  }

  return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarStopAll() {
	uint8_t cmd = GPSTAR_AUDIO_CMD_STOP_ALL;

	gpstarSendCommand(cmd, NULL, 0);

	return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarPauseAll() {
  uint8_t cmd = GPSTAR_AUDIO_CMD_PAUSE_ALL;

	gpstarSendCommand(cmd, NULL, 0);

	return GPSTAR_AUDIO_ERROR_NONE;
}

bool gpstarIsTrackPlaying(uint16_t i_track) {
  uint8_t cmd = GPSTAR_AUDIO_CMD_IS_TRACK_PLAYING;
	uint8_t res;
  uint8_t i_is_playing = 0;
  uint16_t len = 2;

  if(!little_endian) {
    i_track = SWAP16(i_track);
  }

	gpstarSendCommand(cmd, (uint8_t*) &i_track, 2);

  res = gpstarPullData(&cmd, (uint8_t*) &i_is_playing, &len);

  if(res != GPSTAR_AUDIO_ERROR_NONE) {
    return false;
  }

  if(cmd != GPSTAR_AUDIO_CMD_IS_TRACK_PLAYING) {
    return false;
  }

  if(i_is_playing == 1) {
    return true;
  }
  else {
    return false;
  }
}

uint16_t gpstarGetTrackCount() {
  uint8_t cmd = GPSTAR_AUDIO_CMD_GET_TRACK_COUNT;
  uint8_t res;
  uint16_t trackCount = 0;
  uint16_t len = 2;

  gpstarSendCommand(cmd, NULL, 0);

  res = gpstarPullData(&cmd, (uint8_t*) &trackCount, &len);

  if(res != GPSTAR_AUDIO_ERROR_NONE) {
    return res;
  }

  if(cmd != GPSTAR_AUDIO_CMD_GET_TRACK_COUNT) {
    return GPSTAR_AUDIO_ERROR_ON_RX;
  }

	if(!little_endian) {
    trackCount = SWAP16(trackCount);
  }

  return trackCount;
}

uint8_t gpstarSetVolume(int16_t volume) {
  // 0 is default volume. Range is from -60 to +25.
  uint8_t cmd = GPSTAR_AUDIO_CMD_SET_VOLUME;

  Serial.println(volume);
  
  if(!little_endian) {
    volume = SWAP16(volume);
  }

	gpstarSendCommand(cmd, (uint8_t*) &volume, 2);

	return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarPauseTrack(uint16_t track) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_PAUSE_TRACK;
  uint16_t len = 2;

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);

  gpstarStartOutput();
  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    len = SWAP16(len);
  }

  gpstarOutput((uint8_t*) &len, 2);
  gpstarOutput(&track1, 1);
  gpstarOutput(&track2, 1);
  gpstarEndOutput();

  return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarResumeTrack(uint16_t track) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_RESUME_TRACK;
  uint16_t len = 2;

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);

  gpstarStartOutput();
  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    len = SWAP16(len);
  }

  gpstarOutput((uint8_t*) &len, 2);
  gpstarOutput(&track1, 1);
  gpstarOutput(&track2, 1);
  gpstarEndOutput();

  return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarStopTrack(uint16_t track) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_STOP_TRACK;
  uint16_t len = 2;

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);

  gpstarStartOutput();
  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    len = SWAP16(len);
  }

  gpstarOutput((uint8_t*) &len, 2);
  gpstarOutput(&track1, 1);
  gpstarOutput(&track2, 1);
  gpstarEndOutput();

  return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarSetTrackLoop(uint16_t track, bool b_loop) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_SET_TRACK_LOOP;
  uint16_t len = 3;

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);

  uint8_t i_loop = 0;

  if(b_loop == true) {
    i_loop = 1;
  }

  gpstarStartOutput();
  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    len = SWAP16(len);
  }

  gpstarOutput((uint8_t*) &len, 2);
  gpstarOutput(&track1, 1);
  gpstarOutput(&track2, 1);
  gpstarOutput(&i_loop, 1);
  gpstarEndOutput();

  return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarPlayTrack(uint16_t track, uint8_t mode) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_PLAY_GPSTAR_FILE;
  uint16_t len = 3;

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);

  if (mode != GPSTAR_PLAY_NORMAL && mode != GPSTAR_PLAY_LOOP) {
    return GPSTAR_AUDIO_ERROR_PARAM;
  }

  gpstarStartOutput();
  gpstarOutput(&cmd, 1);

  if(!little_endian) {
    len = SWAP16(len);
  }

  gpstarOutput((uint8_t*) &len, 2);
  gpstarOutput(&track1, 1);
  gpstarOutput(&track2, 1);
  gpstarOutput(&mode, 1);
  gpstarEndOutput();

  return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarTrackVolume(uint16_t track, float volume) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_SET_GPSTAR_TRACK_VOLUME;
  uint8_t data[4]; // From 3 before
  uint16_t vol;

	if(volume > 5 || volume < 0) {
		return GPSTAR_AUDIO_ERROR_PARAM;
  }

  vol = (uint16_t) (volume * 100.0f);

  if(!little_endian) {
    vol = SWAP16(vol);
  }

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);

  data[0] = track1;
  data[1] = track2;
  memcpy(&data[2], (uint8_t*) &vol, 2);

  gpstarSendCommand(cmd, data, 4);

	return GPSTAR_AUDIO_ERROR_NONE;
}

uint8_t gpstarTrackFade(uint16_t track, float volume_target, uint16_t duration) {
	uint8_t cmd = GPSTAR_AUDIO_CMD_SET_GPSTAR_FADE_TRACK;
  uint8_t data[5];

	if(volume_target > 5 || volume_target < 0) {
		return GPSTAR_AUDIO_ERROR_PARAM;
  }

  uint8_t volume = volume_target * 10;

  uint8_t track1 = (uint8_t)track;
  uint8_t track2 = (uint8_t)(track >> 8);
  uint8_t duration1 = (uint8_t)duration;
  uint8_t duration2 = (uint8_t)(duration >> 8);

  data[0] = track1;
  data[1] = track2;
  data[2] = duration1;
  data[3] = duration2;
  data[4] = volume;

  gpstarSendCommand(cmd, data, 5);

	return GPSTAR_AUDIO_ERROR_NONE;
}