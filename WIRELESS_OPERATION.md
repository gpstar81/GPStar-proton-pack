# Wireless Operation

This guide will cover the web interface available via the Attenuator or Wireless Adapter devices to control your Proton Pack and Neutrona Wand. At present the same software will be utilized for both solution.

## Firmware Flashing

Please see the [ATTENUATOR_FLASHING](ATTENUATOR_FLASHING) guide for details on compiling and/or uploading software to your Wireless Adapter controller.

## Web UI

When using the ESP32 controller for either the Attenuator or Wireless Adapter, a web-based user interface is available to view the state of your Proton Pack and Neutrona Wand, and to manage specific actions. The available sections are described below.

### Equipment Status

The equipment status will reflect the status of your Proton Pack and Neutrona Wand and will update in real-time as you interact with the device.

**Note:** If you see a "&mdash;" (dash) beside these values it can indicate a potential communication issue. Simply refresh the page and/or check your WiFi connection to the device.

![](images/WebUI-Equipment.jpg)

### Audio Controls

This section allows full control of the master (overall) volume and to mute/unmute all devices. For playback of music you can advance forward or backwards in the music queue, or select a specific track for playback via the selection field (switching immediately if already playing, otherwise will be the track started via the Start/Stop button). Lastly, you can change the effects volume as needed via the dedicated buttons.

**Note:** Only the track numbers are known to the WavTrigger device, and track names are not available for display.

![](images/WebUI-Audio.jpg)

### Pack Controls

Controls will be made available on a per-action or per-state basis. Shown here, the pack and wand are both in an Idle state while in the "Original" mode which allows the pack to be turned on/off remotely. The options to remotely vent or to "Attenuate" are only enabled when the devices are in a specific state.

![](images/WebUI-Controls.jpg)

### Administration

These links allow you to update the WiFi password to one of your choice, or to update the software (respectively).

![](images/WebUI-Admin.jpg)

## Web API

The following URL's will serve the pages as shown above:

	/ - Index Page
	/password - WiFi Password Update Page
	/style.css - Common Stylesheet

For real-time updates, a special url exists at `/ws` to support [WebSockets](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API). When connected, the ESP32 device will "push" any relevant information direct to clients. Note that this data may be in the form of a JSON object or a plain string, so check the contents of the data carefully before usage.

The following URL's are available for managing actions within your devices:

	GET /status - Obtain the current equipment status
	GET /pack/on - Turn the pack on (subject to system state)
	GET /pack/off - Turn the pack onf (subject to system state)
	GET /pack/cancel - Cancel the overheat (attenuate) the pack
	GET /pack/vent - Manual vent (subject to system state)
	GET /volume/toggle - Mute or Unmute all devices
	GET /volume/master/up - Increase master volume
	GET /volume/master/down - Decrease master volume
	GET /volume/effects/up - Increase effects volume
	GET /volume/effects/down - Decrease effects volume
	GET /music/toggle - Start or Stop music playback
	GET /music/next - Move to next track
	GET /music/select?track=### - Select a specific music track (must start at 500)
	GET /music/prev - Move to previous track