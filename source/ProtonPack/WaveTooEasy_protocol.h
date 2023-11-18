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


#ifndef __WAVETOOEASY_PROTOCOL_H__
#define __WAVETOOEASY_PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WTE_MAX_PACKET_DATA_SIZE	512
#define WTE_MAX_CHANNELS			10

#define SERIAL_HDR1	                0x7F
#define SERIAL_HDR2	                0xAA

#define CMD_HELLO				    0x01
#define CMD_VERSION				    0x02
#define CMD_PLAY_FILE			    0x03
#define CMD_PLAY_CHANNEL		    0x04
#define CMD_STOP_ALL			    0x05
#define CMD_STOP				    0x06
#define CMD_PAUSE				    0x07
#define CMD_PAUSE_ALL			    0x08
#define CMD_RESUME				    0x09
#define CMD_RESUME_ALL			    0x0A
#define CMD_CHANNELS_STATUS		    0x0B
#define CMD_CHANNEL_STATUS		    0x0C
#define CMD_GET_CHANNEL_VOL		    0x0D
#define CMD_SET_CHANNEL_VOL		    0x0E
#define CMD_SET_SPEAKERS_VOL	    0x0F
#define CMD_SET_HEADPHONE_VOL	    0x10
#define CMD_GET_SPEAKERS_VOL	    0x11
#define CMD_GET_HEADPHONE_VOL	    0x12
#define CMD_ERROR				    0xFF

#define ERROR_NONE					0x00
#define ERROR_NOT_ENOUGH_BUFFER     0x01
#define ERROR_INVALID_LENGTH		0x02
#define ERROR_INVALID_FILE_LENGTH	0x03
#define ERROR_INVALID_CHANNEL		0x04
#define ERROR_INVALID_MODE			0x05
#define ERROR_INTERNAL				0x06
#define ERROR_PLAYING				0x07
#define ERROR_CRC16_MISMATCH        0x08

#define ERROR_NOT_PAUSED			0xFB
#define ERROR_NOT_PLAYING			0xFC
#define ERROR_ON_RX					0xFD
#define ERROR_RX_TIMEOUT			0xFE
#define ERROR_PARAM					0xFF

#define PLAY_MODE_NORMAL			0
#define PLAY_MODE_LOOP				1

#define STATUS_STOPPED              0
#define STATUS_PLAYING              1
#define STATUS_PAUSED               2

typedef uint32_t (*cbMillis)();
typedef uint8_t (*cbSerialReceiveChar)(uint8_t*, void*);
typedef void (*cbSerialSend)(uint8_t*, size_t, void*);

typedef struct _serialProtocolPacket
{
	uint8_t cmd;
	uint16_t data_len;
	uint8_t data[WTE_MAX_PACKET_DATA_SIZE];
} wtePacket;

typedef struct wteChannelsStatus
{
	uint8_t channel1;
	uint8_t channel2;
	uint8_t channel3;
	uint8_t channel4;
	uint8_t channel5;
	uint8_t channel6;
	uint8_t channel7;
	uint8_t channel8;
	uint8_t channel9;
	uint8_t channel10;
} WTE_CHANNELS_STATUS;

// Initialization
void wteInit(cbMillis cbTicks, cbSerialReceiveChar cbReceive, cbSerialSend cbSend, void* param);

// Commands
uint8_t wteHello();
uint8_t wteGetVersion(uint8_t* major, uint8_t* minor, uint8_t* fix);
uint8_t wtePlayFile(char* file, uint8_t channel, uint8_t mode);
uint8_t wtePlayChannel(uint8_t channel, uint8_t mode);
uint8_t wteStopChannel(uint8_t channel);
uint8_t wteStopAll();
uint8_t wtePauseChannel(uint8_t channel);
uint8_t wtePauseAll();
uint8_t wteResumeChannel(uint8_t channel);
uint8_t wteResumeAll();
uint8_t wteGetAllChannelsStatus(WTE_CHANNELS_STATUS* channels);
uint8_t wteGetChannelStatus(uint8_t channel, uint8_t* status);
uint8_t wteGetChannelVolume(uint8_t channel, float* volume);
uint8_t wteSetChannelVolume(uint8_t channel, float volume);
uint8_t wteSetSpeakersVolume(float volume);
uint8_t wteSetHeadphoneVolume(float volume);
uint8_t wteGetSpeakersVolume(float* volume);
uint8_t wteGetHeadphoneVolume(float* volume);

// Generic read/write
uint8_t wtePullPacket(wtePacket* packet, uint32_t timeout);
void wtePushPacket(wtePacket* packet);
void wteSendErrorCode(uint8_t code);

#ifdef __cplusplus
}
#endif

#endif // __WAVETOOEASY_PROTOCOL_H__
