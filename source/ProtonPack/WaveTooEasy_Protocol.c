//
// WaveTooEasy: Generic C library
//
// For WaveTooEasy board
// https://www.artekit.eu/products/devboards/wavetooeasy/
//
// Copyright (c) 2021 Artekit Labs
// https://www.artekit.eu
//
// Released under MIT license
//


#include "WaveTooEasy_protocol.h"
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

static uint16_t wteCRC16(void* data, uint32_t len, uint16_t partial)
{
	uint16_t crc = partial;
	uint32_t i;
	uint8_t j, c;
	uint8_t* ptr = (uint8_t*) data;

	for(i = 0; i < len; i++)
	{
		c = ((crc>>8) & 0x00FF);
		c ^= *ptr++;
		crc = ((uint16_t) c << 8) | (crc & 0x00FF);
		for (j = 0; j < 8; j++) {
			if (crc & 0x8000) {
				crc <<= 1;
				crc ^= 0x1021;
			} else {
				crc <<= 1;
			}
		}
	}

	return crc;
}

static void resetRx()
{
	rx_timeout = 0;
	rx_enabled = 0;
	rx_state = 0;
	rx_offset = 0;
	rx_packet_timeout = 0;
	rx_crc = 0;
	rx_calc_crc = 0;
	rx_len = 0;
}

static void wteStartOutput()
{
    uint8_t hdr;
    out_crc16 = 0;
    uint8_t c;

    // Clean RX buffer
    while (serial_receive(&c, cb_param));

    hdr = SERIAL_HDR1;
    out_crc16 = wteCRC16(&hdr, 1, 0);
    serial_send(&hdr, 1, cb_param);

    hdr = SERIAL_HDR2;
    out_crc16 = wteCRC16(&hdr, 1, out_crc16);
    serial_send(&hdr, 1, cb_param);
}

static void wteOutput(uint8_t* data, uint32_t len)
{
    out_crc16 = wteCRC16(data, len, out_crc16);
    serial_send(data, len, cb_param);
}

static void wteEndOutput()
{
    if (!little_endian)
        out_crc16 = SWAP16(out_crc16);

    serial_send((uint8_t*) &out_crc16, 2, cb_param);
}

static void wteSendCommand(uint8_t cmd, uint8_t* data, uint16_t len)
{
    uint16_t txlen = len;

    wteStartOutput();
    wteOutput(&cmd, 1);

    if (!little_endian)
        txlen = SWAP16(txlen);

    wteOutput((uint8_t*) &txlen, 2);

    if (data && len)
        wteOutput(data, len);

    wteEndOutput();
}

void wteInit(cbMillis cbTicks, cbSerialReceiveChar cbReceive, cbSerialSend cbSend, void* param)
{
    uint16_t test = 0x01;
    uint8_t* test_ptr = (uint8_t*) &test;

    if (initialized)
        return;

    get_millis = cbTicks;
    serial_receive = cbReceive;
    serial_send = cbSend;
    cb_param = param;

    little_endian = *test_ptr == 1;

    resetRx();

    initialized = 1;
}

// Alternative, internal blocking version that doesn't require allocating
// a whole packet. If CMD_ERROR is received, return the error code (the one in
// 'data'). Otherwise return either timeout or ERROR_NONE
static uint8_t wtePullData(uint8_t* cmd, uint8_t* data, uint16_t* len)
{
    uint8_t c;
	uint8_t error;
    uint32_t timeout = 250;

    if (!initialized || !cmd)
        return 0;

    resetRx();

    rx_packet_timeout = get_millis();

	while ((get_millis() - rx_packet_timeout) < timeout)
	{
        if (!serial_receive(&c, cb_param))
        {
    		// Check for 50ms timeout between received chars.
    		// The count is reset at every received char.
        	if (rx_timeout != 0 && (get_millis() - rx_timeout > 50))
        		resetRx();
            continue;
        }

        // Reset char receiver timeout
        rx_timeout = get_millis();

        switch (rx_state)
        {
        	case 0:
        		if (c == SERIAL_HDR1)
        		{
        			rx_calc_crc = wteCRC16(&c, 1, 0);
        			rx_state++;
        		}
        		break;

        	case 1:
        		if (c == SERIAL_HDR2)
        		{
        			rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
        			rx_state++;
				}
				else {
					resetRx();
				}
        		break;

        	case 2:
        		*cmd = c;
        		rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
        		rx_state++;
        		break;

        	case 3:
                if (little_endian)
       			    rx_len = c;
                else
                    rx_len = c << 8;

       			rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
       			rx_state++;
        		break;

        	case 4:
                if (little_endian)
        		    rx_len |= c << 8;
                else
                    rx_len |= c;

        		if (!rx_len)
        		{
        			rx_state = 6;
        		} else {
					if (!len || *len < rx_len || !data)
					{
						// Check if it is an error code.
						// Use the 'error' variable if the user didn't
						// provide 'data'.
						if (*cmd == CMD_ERROR && !data && rx_len == 1)
						{
							data = &error;
						} else {
							return ERROR_NOT_ENOUGH_BUFFER;
						}
					}

        			if (rx_len > WTE_MAX_PACKET_DATA_SIZE)
        				return ERROR_INVALID_LENGTH;

        			rx_state++;
        		}

        		rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
        		break;

        	case 5:
       			if (rx_offset == WTE_MAX_PACKET_DATA_SIZE)
       				// Overflow
       				return ERROR_INVALID_LENGTH;

       			data[rx_offset++] = c;
       			rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);

       			if (rx_offset == rx_len)
       				rx_state++;
        		break;

        	case 6:
                if (little_endian)
                    rx_crc = c;
                else
                    rx_crc = c << 8;

        		rx_state++;
        		break;

        	case 7:
                if (little_endian)
        		    rx_crc |= c << 8;
                else
                    rx_crc |= c;

        		if (rx_crc == rx_calc_crc)
        		{
        			if (len)
        				*len = rx_len;

				if (*cmd == CMD_ERROR)
				{
					if (rx_len == 1 && data)
						return data[0];

					return ERROR_ON_RX;
				}

        			return ERROR_NONE;
        		}
				return ERROR_CRC16_MISMATCH;
        }

	}

	return ERROR_RX_TIMEOUT;
}

uint8_t wtePullPacket(wtePacket* packet, uint32_t timeout)
{
    uint8_t c;

    if (!initialized)
        return 0;

    rx_packet_timeout = get_millis();

	while ((get_millis() - rx_packet_timeout) < timeout)
	{
        if (!serial_receive(&c, cb_param))
        {
    			// Check for 50ms timeout between received chars.
    			// The count is reset at every received char.
        		if (rx_timeout != 0 && (get_millis() - rx_timeout > 50))
        			resetRx();

        		continue;
        }

        // Reset char receiver timeout
        rx_timeout = get_millis();

        switch (rx_state)
        {
			case 0:
				if (c == SERIAL_HDR1)
				{
					rx_calc_crc = wteCRC16(&c, 1, 0);
					rx_state++;
				}
				break;

			case 1:
				if (c == SERIAL_HDR2)
				{
					rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
					rx_state++;
				}
				break;

			case 2:
				packet->cmd = c;
				rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
				rx_state++;
				break;

			case 3:
				if (little_endian)
					packet->data_len = c;
				else
					packet->data_len = c << 8;

				rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
				rx_state++;
				break;

			case 4:
				if (little_endian)
					packet->data_len |= c << 8;
				else
					packet->data_len |= c;

				if (!packet->data_len)
					rx_state = 6;
				else
					rx_state++;

				rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);
				break;

			case 5:
				if (rx_offset == WTE_MAX_PACKET_DATA_SIZE)
				{
					// Overflow
					resetRx();
					break;
				}

				packet->data[rx_offset++] = c;
				rx_calc_crc = wteCRC16(&c, 1, rx_calc_crc);

				if (rx_offset == packet->data_len)
					rx_state++;
				break;

			case 6:
				if (little_endian)
					rx_crc = c;
				else
					rx_crc = c << 8;

				rx_state++;
				break;

			case 7:
				if (little_endian)
					rx_crc |= c << 8;
				else
					rx_crc |= c;

				c = (rx_crc == rx_calc_crc) ? 1 : 0;
				resetRx();
				return c;
        }

	}

	return 0;
}

void wtePushPacket(wtePacket* packet)
{
    uint16_t len;
    if (!initialized)
        return;

    wteStartOutput();

    wteOutput(&packet->cmd, 1);

    if (little_endian)
        len = packet->data_len;
    else
        len = SWAP16(packet->data_len);

    wteOutput((uint8_t*) &len, 2);

	if (packet->data_len)
		wteOutput(packet->data, packet->data_len);

    wteEndOutput();
}

void wteSendErrorCode(uint8_t code)
{
	uint8_t cmd = CMD_ERROR;
    uint16_t len = 1;

    wteStartOutput();

    wteOutput(&cmd, 1);

    if (!little_endian)
        len = SWAP16(len);

    wteOutput((uint8_t*) &len, 2);

    wteOutput(&code, 1);

    wteEndOutput();
}

uint8_t wteHello()
{
    uint8_t cmd = CMD_HELLO;
    uint8_t res;

    wteSendCommand(cmd, NULL, 0);

    res = wtePullData(&cmd, NULL, 0);
    if (res != ERROR_NONE)
        return res;

    if (cmd != CMD_HELLO)
        return ERROR_ON_RX;

    return ERROR_NONE;
}

uint8_t wteGetVersion(uint8_t* major, uint8_t* minor, uint8_t* fix)
{
    uint8_t cmd = CMD_VERSION;
	uint8_t data[3];
	uint16_t len = 3;
	uint8_t res;

	if (!major || !minor || !fix)
		return ERROR_PARAM;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, data, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_VERSION || len != 3)
		return ERROR_ON_RX;

	*major = data[0];
	*minor = data[1];
	*fix = data[2];

	return ERROR_NONE;
}
uint8_t wtePlayFile(char* file, uint8_t channel, uint8_t mode)
{
	uint8_t cmd = CMD_PLAY_FILE;
	uint16_t len;
    uint16_t filelen;
	uint8_t data;
	uint8_t res;

	if (!file)
		return ERROR_PARAM;

	if (!channel || channel > WTE_MAX_CHANNELS)
		return ERROR_PARAM;

	if (mode != PLAY_MODE_NORMAL && mode != PLAY_MODE_LOOP)
		return ERROR_PARAM;

	filelen = (uint16_t) strlen(file);
	if (filelen > 254)
		return ERROR_PARAM;

    len = filelen + 2;
    if (!little_endian)
        len = SWAP16(len);

    wteStartOutput();
    wteOutput(&cmd, 1);
    wteOutput((uint8_t*) &len, 2);
    wteOutput(&channel, 1);
    wteOutput(&mode, 1);
    wteOutput((uint8_t*) file, filelen);
    wteEndOutput();

	len = 1;
	res = wtePullData(&cmd, &data, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd == CMD_PLAY_FILE && len == 1)
		return ERROR_NONE;

	return ERROR_ON_RX;
}

uint8_t wtePlayChannel(uint8_t channel, uint8_t mode)
{
    uint8_t cmd = CMD_PLAY_CHANNEL;
    uint16_t len = 2;
    uint8_t data;
    uint8_t res;

	if (channel == 0 || channel > WTE_MAX_CHANNELS)
		return ERROR_PARAM;

    if (mode != PLAY_MODE_NORMAL && mode != PLAY_MODE_LOOP)
		return ERROR_PARAM;

    wteStartOutput();
    wteOutput(&cmd, 1);

    if (!little_endian)
        len = SWAP16(len);

    wteOutput((uint8_t*) &len, 2);
    wteOutput(&channel, 1);
    wteOutput(&mode, 1);
    wteEndOutput();

	len = 1;
	res = wtePullData(&cmd, &data, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_PLAY_CHANNEL || len != 1)
		return ERROR_ON_RX;
    return ERROR_NONE;
}

uint8_t wteStopChannel(uint8_t channel)
{
    uint8_t cmd = CMD_STOP;
	uint16_t len = 1;
    uint8_t data;
	uint8_t res;

	if (channel == 0 || channel > WTE_MAX_CHANNELS)
		return ERROR_PARAM;

	wteSendCommand(cmd, &channel, 1);

	res = wtePullData(&cmd, &data, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_STOP || len != 1 || data != channel)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteStopAll()
{
	uint8_t cmd = CMD_STOP_ALL;
	uint8_t res;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, NULL, 0);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_STOP_ALL)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wtePauseChannel(uint8_t channel)
{
    uint8_t cmd = CMD_PAUSE;
	uint16_t len = 1;
	uint8_t data;
	uint8_t res;

	if (channel == 0 || channel > WTE_MAX_CHANNELS)
		return ERROR_PARAM;

	wteSendCommand(cmd, &channel, 1);

	res = wtePullData(&cmd, &data, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_PAUSE || len != 1)
		return ERROR_ON_RX;

	if (!data)
		return ERROR_NOT_PLAYING;

	return ERROR_NONE;
}

uint8_t wtePauseAll()
{
    uint8_t cmd = CMD_PAUSE_ALL;
	uint8_t res;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, NULL, 0);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_PAUSE_ALL)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteResumeChannel(uint8_t channel)
{
	uint8_t cmd = CMD_RESUME;
	uint16_t len = 1;
	uint8_t data;
	uint8_t res;

	if (channel == 0 || channel > WTE_MAX_CHANNELS)
		return ERROR_PARAM;

	wteSendCommand(cmd, &channel, 1);

	res = wtePullData(&cmd, &data, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_RESUME || len != 1)
		return ERROR_ON_RX;

	if (!data)
		return ERROR_NOT_PAUSED;

	return ERROR_NONE;
}

uint8_t wteResumeAll()
{
	uint8_t cmd = CMD_RESUME_ALL;
	uint8_t res;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, NULL, 0);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_RESUME_ALL)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteGetAllChannelsStatus(WTE_CHANNELS_STATUS* channels)
{
    uint8_t cmd = CMD_CHANNELS_STATUS;
	uint8_t res;
	uint16_t len = WTE_MAX_CHANNELS;

	if (!channels)
		return ERROR_PARAM;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, (uint8_t*) channels, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_CHANNELS_STATUS || len != WTE_MAX_CHANNELS)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteGetChannelStatus(uint8_t channel, uint8_t* status)
{
	uint8_t cmd = CMD_CHANNEL_STATUS;
	uint16_t len = 1;
	uint8_t res;

	if (channel == 0 || channel > WTE_MAX_CHANNELS || !status)
		return ERROR_PARAM;

	wteSendCommand(cmd, &channel, 1);

	res = wtePullData(&cmd, status, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_CHANNEL_STATUS || len != 1)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteGetChannelVolume(uint8_t channel, float* volume)
{
	uint8_t cmd = CMD_GET_CHANNEL_VOL;
	uint16_t len = 2;
	uint8_t res;
    uint16_t vol;

	if (channel == 0 || channel > WTE_MAX_CHANNELS || !volume)
		return ERROR_PARAM;

	wteSendCommand(cmd, &channel, 1);

	res = wtePullData(&cmd, (uint8_t*) &vol, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_GET_CHANNEL_VOL || len != 2)
		return ERROR_ON_RX;

    if (!little_endian)
        vol = SWAP16(vol);

    *volume = vol / 100.0f;

	return ERROR_NONE;
}

uint8_t wteSetChannelVolume(uint8_t channel, float volume)
{
	uint8_t cmd = CMD_SET_CHANNEL_VOL;
	uint8_t res;
    uint8_t data[3];
    uint16_t vol;

	if (channel == 0 || channel > WTE_MAX_CHANNELS)
		return ERROR_PARAM;

	if (volume > 5 || volume < 0)
		return ERROR_PARAM;

    vol = (uint16_t) (volume * 100.0f);

    if (!little_endian)
        vol = SWAP16(vol);

    data[0] = channel;
    memcpy(&data[1], (uint8_t*) &vol, 2);

    wteSendCommand(cmd, data, 3);

	res = wtePullData(&cmd, NULL, NULL);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_SET_CHANNEL_VOL)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteSetSpeakersVolume(float volume)
{
    uint8_t cmd = CMD_SET_SPEAKERS_VOL;
	uint8_t res;
    int16_t vol = (int16_t) (volume * 10.0f);

    if (!little_endian)
    		vol = SWAP16(vol);

	wteSendCommand(cmd, (uint8_t*) &vol, 2);

	res = wtePullData(&cmd, NULL, NULL);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_SET_SPEAKERS_VOL)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteSetHeadphoneVolume(float volume)
{
    uint8_t cmd = CMD_SET_HEADPHONE_VOL;
	uint8_t res;
    int16_t vol = (int16_t)(volume * 10.0f);

    if (!little_endian)
    		vol = SWAP16(vol);

	wteSendCommand(cmd, (uint8_t*) &vol, 2);

	res = wtePullData(&cmd, NULL, NULL);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_SET_HEADPHONE_VOL)
		return ERROR_ON_RX;

	return ERROR_NONE;
}

uint8_t wteGetSpeakersVolume(float* volume)
{
    uint8_t cmd = CMD_GET_SPEAKERS_VOL;
	uint8_t res;
	int16_t vol;
	uint16_t len = 2;

	if (!volume)
		return ERROR_PARAM;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, (uint8_t*) &vol, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_GET_SPEAKERS_VOL || len != 2)
		return ERROR_ON_RX;

	if (!little_endian)
		vol = SWAP16(vol);

	*volume = vol / 10.0f;
	return ERROR_NONE;
}

uint8_t wteGetHeadphoneVolume(float* volume)
{
    uint8_t cmd = CMD_GET_HEADPHONE_VOL;
	uint8_t res;
	int16_t vol;
	uint16_t len = 2;

	if (!volume)
		return ERROR_PARAM;

	wteSendCommand(cmd, NULL, 0);

	res = wtePullData(&cmd, (uint8_t*) &vol, &len);
	if (res != ERROR_NONE)
		return res;

	if (cmd != CMD_GET_HEADPHONE_VOL || len != 2)
		return ERROR_ON_RX;

	if (!little_endian)
		vol = SWAP16(vol);

	*volume = vol / 10.0f;
	return ERROR_NONE;
}
