/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
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

// ESP - Suppress warning about SPI hardware pins
// Define this before including <FastLED.h>
#define FASTLED_INTERNAL

// PROGMEM macro
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <ht16k33.h>
#include <Wire.h>
#include <SerialTransfer.h>

// Local Files
#include "Configuration.h"
#include "Communication.h"
#include "Header.h"
#include "Bargraph.h"
#include "Colours.h"
#include "Serial.h"
#include "Wireless.h"
#include "System.h"

void setup() {
  // Enable Serial connection(s) and communication with GPStar Proton Pack PCB.
  // ESP - Serial Console for messages and Device Comms via Serial2
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  packComs.begin(Serial2, false);
  pinMode(BUILT_IN_LED, OUTPUT);

  // Assume the Super Hero arming mode with Afterlife (default for Haslab).
  SYSTEM_MODE = MODE_SUPER_HERO;
  RED_SWITCH_MODE = SWITCH_OFF;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;

  // Boot into proton mode (default for pack and wand).
  STREAM_MODE = PROTON;

  // Set a default animation for the radiation indicator.
  RAD_LENS_IDLE = AMBER_PULSE;

  // ESP - Get Special Device Preferences
  preferences.begin("device", true); // Access namespace in read-only mode.

  // Return stored values if available, otherwise use a default value.
  b_invert_leds = preferences.getBool("invert_led", false);
  b_enable_buzzer = preferences.getBool("use_buzzer", true);
  b_enable_vibration = preferences.getBool("use_vibration", true);
  b_overheat_feedback = preferences.getBool("use_overheat", true);
  b_firing_feedback = preferences.getBool("fire_feedback", false);

  switch(preferences.getShort("radiation_idle", 0)) {
    case 0:
      RAD_LENS_IDLE = AMBER_PULSE;
    break;
    case 1:
      RAD_LENS_IDLE = ORANGE_FADE;
    break;
    case 2:
      RAD_LENS_IDLE = RED_FADE;
    break;
  }

  switch(preferences.getShort("display_type", 0)) {
    case 0:
      DISPLAY_TYPE = STATUS_TEXT;
    break;
    case 1:
      DISPLAY_TYPE = STATUS_GRAPHIC;
    break;
    case 2:
    default:
      DISPLAY_TYPE = STATUS_BOTH;
    break;
  }

  s_track_listing = preferences.getString("track_list", "");
  preferences.end();

  // CPU Frequency MHz: 80, 160, 240 [Default]
  // Lower frequency means less power consumption.
  setCpuFrequencyMhz(240);
  Serial.print(F("CPU Freq (MHz): "));
  Serial.println(getCpuFrequencyMhz());

  if(!b_wait_for_pack) {
    // If not waiting for the pack set power level to 5.
    POWER_LEVEL = LEVEL_5;
  }
  else {
    // When waiting for the pack set power level to 1.
    POWER_LEVEL = LEVEL_1;
  }

  // Begin at menu level one. This affects the behavior of the rotary dial.
  MENU_LEVEL = MENU_1;

  // RGB LEDs for effects (upper/lower) and user status (top).
  FastLED.addLeds<NEOPIXEL, DEVICE_LED_PIN>(device_leds, DEVICE_NUM_LEDS);

  // Set all LEDs as off (black) until the device is ready.
  device_leds[0] = getHueAsRGB(0, C_BLACK);
  device_leds[1] = getHueAsRGB(1, C_BLACK);
  device_leds[2] = getHueAsRGB(2, C_BLACK);

  // Debounce the toggle switches and encoder pushbutton.
  switch_left.setDebounceTime(switch_debounce_time);
  switch_right.setDebounceTime(switch_debounce_time);
  encoder_center.setDebounceTime(switch_debounce_time);

  // Rotary encoder on the top of the Attenuator.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(r_encoderA), readEncoder, CHANGE);

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  // Feedback devices (piezo buzzer and vibration motor)
  pinMode(BUZZER_PIN, OUTPUT);
  // ESP32 - Note: "ledcAttachChannel" is a combined method for the arduino-esp32 v3.x board library
  ledcAttachChannel(VIBRATION_PIN, 5000, 8, 5); // Uses 5 kHz frequency, 8-bit resolution, Channel 5
  //setToneChannel(0); // Forces Tone to use Channel 0, implemented in ESP32 3.1.0

  // Turn off any user feedback.
  noTone(BUZZER_PIN);
  vibrateOff();

  // Get initial switch/button states.
  switchLoops();

  // Delay before configuring WiFi and web access.
  delay(100);

  // ESP - Setup WiFi and WebServer
  if(startWiFi()) {
    // Start the local web server.
    startWebServer();

    // Begin timer for remote client events.
    ms_cleanup.start(i_websocketCleanup);
  }

  // Initialize critical timers.
  ms_fast_led.start(0);
  if(b_wait_for_pack) {
    ms_packsync.start(0);
  }
}

void loop() {
  // Call this on each loop in case the user changed their preference.
  if(b_invert_leds) {
    // Flip the identification of the LEDs.
    i_device_led[0] = 2; // Top
    i_device_led[1] = 1; // Upper
    i_device_led[2] = 0; // Lower
  }
  else {
    // Use the expected order for the LEDs.
    // aka. Defaults for the Arduino Nano and ESP32.
    i_device_led[0] = 0; // Top
    i_device_led[1] = 1; // Upper
    i_device_led[2] = 2; // Lower
  }

  // ESP - Manage cleanup for old WebSocket clients.
  if(ms_cleanup.remaining() < 1) {
    // Clean up oldest WebSocket connections.
    ws.cleanupClients();

    // Restart timer for next cleanup action.
    ms_cleanup.start(i_websocketCleanup);
  }

  // Handle device reboot after an OTA update.
  ElegantOTA.loop();

  // Update the current count of AP clients.
  i_ap_client_count = WiFi.softAPgetStationNum();

  if(b_wait_for_pack) {
    if(ms_packsync.justFinished()) {
      // Tell the pack we are trying to sync.
      attenuatorSerialSend(A_SYNC_START);

      digitalWrite(BUILT_IN_LED, LOW);

      // Pause and try again in a moment.
      ms_packsync.start(i_sync_initial_delay);
    }

    checkPack();

    if(!b_wait_for_pack) {
      // Indicate that we are no longer waiting on the pack.
      digitalWrite(BUILT_IN_LED, HIGH);
    }
  }
  else {
    // When not waiting for the pack go directly to the main loop.
    mainLoop();
  }
}