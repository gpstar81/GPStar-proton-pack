# Web-Based API Reference

This guide will cover the web interface available via an Attenuator or standalone WiFi add-on which offers a visual, web-based UI capable of controlling operations of your Proton Pack and Neutrona Wand.

While this guide is primarily targeted to the UI from an **Attenuator** (or standalone WiFi add-on). For use of the GPStar Proton Pack II and GPStar Neutrona Wand II controllers you should view the dedicated [GPStar II Wireless Guide](WIRELESS_GPSTAR_II.md).

## Background

The web UI is built as a single-page application, using single HTML pages for the static interface elements and performing actions using an API layer. These API endpoints are available for use if you wish to build your own interface. They pass data in JSON format though the exact structure is not described here at this time.

## File Requests

The following URI's will serve the informational/maintenance pages as shown in the wireless operation guides. These are listed for you to observe how the API's may be called and how data is used for changing states within the UI:

	GET / - Standard Index/Landing Page
	GET /network - External WiFi Settings Page
	GET /password - WiFi Password Update Page
	GET /settings/device = Special Device Settings Page
	GET /settings/pack - Pack Settings Page
	GET /settings/wand - Wand Settings Page
	GET /settings/smoke - Smoke Settings Page
	GET /style.css - Common Stylesheet
	GET /common.js - Common JavaScript
	GET /index.js - Index Page JavaScript

## API Reference

The following URI's are API endpoints available for managing actions within your devices. You may use these to create your own UI or control your available hardware devices. For instance, you can monitor the `/status` endpoint for changes, or use the volume/music endpoints to create your own jukebox interface. All data should use the `application/json` content type for sending or receiving of data. Where applicable for body data to be sent to the device a footnote describes where to find a sample of the JSON payload.

	GET /status - Obtain all current equipment status (pack + wand)
	DELETE /restart - Perform a software restart of the Attenuator or Wireless controller

	PUT /pack/on - Turn the pack on (subject to system state)
	PUT /pack/off - Turn the pack onf (subject to system state)
	PUT /pack/attenuate - Cancel pack overheat via "attenuation"
	PUT /pack/vent - Perform manual vent (subject to system state)
	PUT /pack/cyclotron/clockwise - Spin cyclotron clockwise
	PUT /pack/cyclotron/counterclockwise - Spin cyclotron counterclockwise
	PUT /pack/smoke/on - Enable smoke
	PUT /pack/smoke/off - Disable smoke
	PUT /pack/stream/proton - Switch to Proton Stream
	PUT /pack/stream/stasis - Switch to Dark Matter Generator
	PUT /pack/stream/slime - Switch to Plasm System
	PUT /pack/stream/meson - Switch to Particle System
	PUT /pack/stream/spectral - Switch to Spectral Proton Stream
	PUT /pack/stream/holiday_halloween - Switch to Halloween Proton Stream
	PUT /pack/stream/holiday_christmas - Switch to Christmas Proton Stream
	PUT /pack/stream/spectral_custom - Switch to Custom Proton Stream
	PUT /pack/theme/1984 - Switch to GB1 theme
	PUT /pack/theme/1989 - Switch to GB2 theme
	PUT /pack/theme/2021 - Switch to GB:AL theme
	PUT /pack/theme/2024 - Switch to GB:FE theme
	PUT /pack/vibration/on - Enable vibration
	PUT /pack/vibration/off - Disable vibration

	PUT /volume/mute - Mute for all devices
	PUT /volume/unmute - Unmute for all devices
	PUT /volume/master/up - Increase system (master) volume
	PUT /volume/master/down - Decrease system (master) volume
	PUT /volume/effects/up - Increase effects volume
	PUT /volume/effects/down - Decrease effects volume
	PUT /volume/music/up - Increase music volume
	PUT /volume/music/down - Decrease music volume
	PUT /volume/music/loop - Toggle looping a single track

	PUT /music/startstop - Toggle music playback via start/stop
	PUT /music/pauseresume - Toggle music playback via resume/pause
	PUT /music/next - Move to next track
	PUT /music/prev - Move to previous track
	PUT /music/loop/all - Set looping at end of playlist
	PUT /music/loop/single - Set looping of a single track
	PUT /music/select?track=[INTEGER] - Select a specific music track (Min Value: 500)

	GET /wifi/settings - Returns the current external WiFi settings
	PUT /wifi/update - Save new/modified external WiFi settings
		Body: Send same JSON body as returned by /wifi/settings

	GET /config/device - Obtain the current special device settings
	PUT /config/device/save - Saves attenuator settings
		Body: Send same JSON body as returned by /config/pack

	GET /config/pack - Obtain the current pack equipment settings
	PUT /config/pack/save - Saves pack settings for evaluation
		Body: Send same JSON body as returned by /config/pack

	GET /config/wand - Obtain the current wand equipment settings
	PUT /config/wand/save - Saves wand settings for evaluation
		Body: Send same JSON body as returned by /config/wand

	GET /config/smoke - Obtain the current pack/wand smoke settings
	PUT /config/smoke/save - Saves smoke settings for evaluation
		Body: Send same JSON body as returned by /config/smoke

	WARNING: Only call these API's as necessary as these cause write cycles to the EEPROM!

	PUT /eeprom/all - Stores all current preferences to pack/wand EEPROMs (eg. smoke settings)
	PUT /eeprom/pack - Stores current pack preferences to pack EEPROM only
	PUT /eeprom/wand - Stores current wand preferences to wand EEPROM only

## WebSockets

All of the API endpoints previously highlighted work on a "poll" or "pull" mechanism. This means that in order to get information from endpoints such as `/status` you would need to constantly make a request at regular intervals to obtain data. This is inefficient and can put unnecessary load on the device when no interactions have occurred.

Instead, when real-time updates are required we can instead have the devices "push" information as it happens directly from user input or other events. The built-in web server offers a special URI endpoint `/ws` to support [WebSockets](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API). This is much more efficient as it 1) only sends data as the system changes states, and 2) is meant to report only the information necessary for interactions.

All devices support the WebSocket output, which primarily reports the current device status and is updated after specific system actions occur. Note that while this data will always be in the form of a JSON object, some events may send a plain string, so always check the contents of the text data carefully before usage to ensure you are working with the expected data format.