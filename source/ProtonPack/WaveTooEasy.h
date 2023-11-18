//
// WaveTooEasy: Library for Arduino
//
// For WaveTooEasy board
// https://www.artekit.eu/products/devboards/wavetooeasy/
//
// Copyright (c) 2021 Artekit Labs
// https://www.artekit.eu
//
// Released under MIT license
//

#include <Arduino.h>
#include "WaveTooEasy_protocol.h"

class Artekit_WaveTooEasy
{
public:
	Artekit_WaveTooEasy(Stream& serial = Serial) :
		serial(serial), last_error(ERROR_NONE) {}

	~Artekit_WaveTooEasy()
	{

	}

	static uint8_t serialReceive(uint8_t* c, void* param)
	{
		Artekit_WaveTooEasy* p = (Artekit_WaveTooEasy*) param;

		if (p->serial.available())
		{
			*c = p->serial.read();
			return 1;
		}

		return 0;
	}

	static void serialSend(uint8_t* buf, size_t len, void* param)
	{
		Artekit_WaveTooEasy* p = (Artekit_WaveTooEasy*) param;
		p->serial.write(buf, len);
	}

	void begin()
	{
		wteInit(millis, serialReceive, serialSend, this);
	}

	bool hello()
	{
		last_error = wteHello();
		return (last_error == ERROR_NONE);
	}

	bool getVersion(uint8_t* major, uint8_t* minor, uint8_t* fix)
	{
		last_error = wteGetVersion(major, minor, fix);
		return (last_error == ERROR_NONE);
	}

	bool playFile(char* file, uint8_t channel, uint8_t mode)
	{
		last_error = wtePlayFile(file, channel, mode);
		return (last_error == ERROR_NONE);
	}

	bool playChannel(uint8_t channel, uint8_t mode)
	{
		last_error = playChannel(channel, mode);
		return (last_error == ERROR_NONE);
	}

	bool stopChannel(uint8_t channel)
	{
		last_error = wteStopChannel(channel);
		return (last_error == ERROR_NONE);
	}

	bool stopAll()
	{
		last_error = wteStopAll();
		return (last_error == ERROR_NONE);
	}

	bool pauseChannel(uint8_t channel)
	{
		last_error = wtePauseChannel(channel);
		return (last_error == ERROR_NONE);
	}

	bool pauseAll()
	{
		last_error = wtePauseAll();
		return (last_error == ERROR_NONE);
	}

	bool resumeChannel(uint8_t channel)
	{
		last_error = wteResumeChannel(channel);
		return (last_error == ERROR_NONE);
	}

	bool resumeAll()
	{
		last_error = wteResumeAll();
		return (last_error == ERROR_NONE);
	}

	bool getAllChannelsStatus(WTE_CHANNELS_STATUS* channels)
	{
		last_error = wteGetAllChannelsStatus(channels);
		return (last_error == ERROR_NONE);
	}

	bool getChannelStatus(uint8_t channel, uint8_t* status)
	{
		last_error = wteGetChannelStatus(channel, status);
		return (last_error == ERROR_NONE);
	}

	bool getChannelVolume(uint8_t channel, float* volume)
	{
		last_error = wteGetChannelVolume(channel, volume);
		return (last_error == ERROR_NONE);
	}

	bool setChannelVolume(uint8_t channel, float volume)
	{
		last_error = wteSetChannelVolume(channel, volume);
		return (last_error == ERROR_NONE);
	}

	bool setSpeakersVolume(float volume)
	{
		last_error = wteSetSpeakersVolume(volume);
		return (last_error == ERROR_NONE);
	}

	bool setHeadphoneVolume(float volume)
	{
		last_error = wteSetHeadphoneVolume(volume);
		return (last_error == ERROR_NONE);
	}

    bool getSpeakersVolume(float* volume)
	{
		last_error = wteGetSpeakersVolume(volume);
		return (last_error == ERROR_NONE);
	}

	bool getHeadphoneVolume(float* volume)
	{
		last_error = wteGetHeadphoneVolume(volume);
		return (last_error == ERROR_NONE);
	}
	
	inline uint8_t getLastError() { return last_error; }

private:
	Stream& serial;
	uint8_t last_error;
};
