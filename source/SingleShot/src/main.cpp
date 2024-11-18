/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Dustin Grau <dustin.grau@gmail.com>
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

// Required for PlatformIO
#include <Arduino.h>

// Defines the microcontroller as part of a GPStar PCB
#if defined(__AVR_ATmega2560__)
  #define GPSTAR_NEUTRONA_DEVICE_PCB
#endif

// Set to 1 to enable built-in debug messages
#define DEBUG 0

// Debug macros
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// PROGMEM macro
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <CRC32.h>
#include <EEPROM.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <avdweb_Switch.h>
#include <ht16k33.h>
#include <Wire.h>

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Header.h"
#include "Colours.h"
#include "Bargraph.h"
#include "Cyclotron.h"
#include "Audio.h"
#include "Preferences.h"
#include "System.h"
#include "Actions.h"

/*
 * Delay for fastled to update the addressable LEDs.
 * 0.03 ms to update 1 LED. So 1.47 ms should be okay? Let's bump it up to 3 just in case.
 */
#define FAST_LED_UPDATE_MS 3
uint8_t i_fast_led_delay = FAST_LED_UPDATE_MS; // Default delay via standard definition
millisDelay ms_fast_led; // Timer for all updates to addressable LEDs across the device

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  // Setup the audio device for this controller.
  setupAudioDevice();

  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR1B = (TCCR1B & B11111000) | B00000100; // for PWM frequency of 122.55 Hz

  // System LEDs
  FastLED.addLeds<NEOPIXEL, SYSTEM_LED_PIN>(system_leds, CYCLOTRON_LED_COUNT + BARREL_LED_COUNT);

  // Setup default system settings.
  VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
  VIBRATION_MODE = VIBRATION_MODE_EEPROM;
  DEVICE_MENU_LEVEL = MENU_LEVEL_1;
  MENU_OPTION_LEVEL = OPTION_5;
  POWER_LEVEL = LEVEL_1;

  // Set callback events for these toggles, which need to count the activations for EEPROM menu entry.
  switch_vent.setPushedCallback(&ventSwitched); // For the LED EEPROM Menu
  switch_device.setPushedCallback(&deviceSwitched); // For the Config EEPROM Menu

  // Rotary encoder on the top of the device.
  encoder.initialize();

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  // Initialize all non-addressable LEDs
  led_SloBlo.initialize();
  led_Clippard.initialize();
  led_TopWhite.initialize();
  led_Vent.initialize();
  led_Hat1.initialize();
  led_Hat2.initialize();
  led_Tip.initialize();

  pinMode(vibration, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.

  // Device status.
  DEVICE_STATUS = MODE_OFF;
  DEVICE_ACTION_STATUS = ACTION_IDLE;

  // We bootup the device in the classic proton mode.
  STREAM_MODE = PROTON;

  // Load any saved settings stored in the EEPROM memory of the GPStar Single-Shot Blaster.
  if(b_eeprom) {
    readEEPROM();
  }

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  // Check music timer for bench test mode only.
  ms_check_music.start(i_music_check_delay);

  // Reset our master volume manually.
  updateMasterVolume(true);

  // Make sure lights are off, including the bargraph.
  allLightsOff();

  // Execute the System POST (Power On Self Test)
  systemPOST();

  // Initialize the fastLED state update timer.
  ms_fast_led.start(i_fast_led_delay);
}

void animate() {
  // Update bargraph with latest state and pattern changes.
  if(ms_firing_pulse.isRunning()) {
    // Increase the speed for updates while this timer is still running.
    bargraphUpdate(POWER_LEVEL - 1);
  }
  else {
    // Otherwise run with the standard timing.
    bargraphUpdate();
  }

  // Keep the cyclotron spinning as necessary.
  checkCyclotron();

  // Update all addressable LEDs to reflect any changes.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

void inputCheck() {
  updateAudio(); // Update the state of the available sound board.

  checkMusic(); // Perform music control here as this is a standalone device.

  switchLoops(); // Standard polling for switch/button changes via user inputs.

  // Get the current state of any input devices (toggles, buttons, and switches).
  checkRotaryEncoder();
  checkMenuVibration();

  // Handle button press events based on current device state and menu level (for config/EEPROM purposes).
  checkDeviceAction();

  // Perform updates/actions based on timer events.
  checkGeneralTimers();
}

void loop() {
  // Check for user input
  inputCheck();

  // Animate all LEDs
  animate();
}
