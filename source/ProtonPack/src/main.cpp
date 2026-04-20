/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Set to 1 to enable built-in debug messages via Serial device output.
// Use with DEBUG_SEND_TO_CONSOLE and other DEBUG_'s in Configuration.h
#define GPSTAR_DEBUG 0

// Debug macros
#if GPSTAR_DEBUG == 1
  #define debug(...) Serial.print(__VA_ARGS__)
  #define debugf(...) Serial.printf(__VA_ARGS__)
  #define debugln(...) Serial.println(__VA_ARGS__)
#else
  #define debug(...)
  #define debugf(...)
  #define debugln(...)
#endif

// PROGMEM macros
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <CRC32.h>
#include <digitalWriteFast.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <Ramp.h>
#include <SerialTransfer.h>
#include <Wire.h>
#ifdef ESP32
  #include <HDC1080.h>
  GuL::HDC1080 tempSensor(Wire1);
  #include <HardwareSerial.h>
#else
  #include <EEPROM.h>
#endif

// Forward declaration for use in all includes.
void sendDebug(const String& message);

// Shared Libraries
#include <DeviceState.h>
#include <Communication.h>
#ifdef ESP32
  #include <WirelessManager.h>
  #include <WebRouter.h>

  // Define the WirelessManager pointer globally (initialized to nullptr).
  // This matches the extern declaration in Wireless.h
  WirelessManager* wirelessMgr = nullptr;
#endif

// Global instance of DeviceState class for the Proton Pack.
DeviceState gpstarPack;

// References to global instances of all preference/sync structs.
extern PackPrefs packConfig;
extern WandPrefs wandConfig;
extern SmokePrefs smokeConfig;
extern WandSyncData wandSyncData;
extern AttenuatorSyncData attenuatorSyncData;

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Header.h"
#include "Colours.h"
#include "Audio.h"
#include "PowerMeter.h"
#ifdef ESP32
  #include "PreferencesESP.h"
#else
  #include "PreferencesATMega.h"
#endif
#include "System.h"
#include "Command.h"
#include "Serial.h"
#ifdef ESP32
  #include "Wireless.h"
  #include "Webhandler.h"
  #include "Webrouting.h"
#endif

// Writes a debug message to the serial console or sends to the WebSocket or Events stream.
void sendDebug(const String& message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET) and defined(ESP32)
    if(b_httpd_started) {
      ws.textAll(message); // Send a copy to the WebSocket.
    }
  #endif
  #if defined(DEBUG_SEND_TO_EVENTS) and defined(ESP32)
    sendDebugEvent(message.c_str()); // Send message to the events stream.
  #endif
}

void setup() {
#ifdef ESP32
  // Force RMT driver exclusively (requires FastLED 3.10.4 at a minimum, not yet released).
  // This avoids issues with WiFi/networking on ESP32 when using the default bit-banging method.
  //FastLED.setExclusiveDriver("RMT");
#endif

  // Power Cell, Cyclotron Lid, and N-Filter.
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, MAX_POWERCELL_LED_COUNT + OUTER_CYCLOTRON_LED_MAX + JEWEL_NFILTER_LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setMaxRefreshRate(0); // Disable FastLED's blocking 2.5ms delay.

  // Inner Cyclotron LEDs (Inner Panel + Cyclotron + Cavity).
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, INNER_CYCLOTRON_LED_PANEL_MAX + INNER_CYCLOTRON_CAKE_LED_MAX + INNER_CYCLOTRON_CAVITY_LED_MAX).setCorrection(TypicalLEDStrip);

  // Update all addressable LEDs to prevent stale LED states.
  FastLED.show();

#ifdef ESP32
  // Reduce CPU frequency to 160 MHz to save ~33% power compared to 240 MHz.
  // Do not set below 80 MHz as it will affect WiFi and other peripherals.
  setCpuFrequencyMhz(160);

  // This is required in order to make sure the board boots successfully.
  Serial.begin(115200);

#if GPSTAR_DEBUG == 1
  // When debugging is enabled, wait for Serial to be ready (max 3 seconds).
  unsigned long startMillis = millis();
  while (!Serial && millis() - startMillis < 3000) {
    delay(10);
  }
  Serial.flush(); // Ensure buffer is clear.
  Serial.setTxTimeoutMs(0); // Optional: reduce USB-CDC transmission delay.
  Serial.println(F("Serial is Ready")); // Should appear after Serial is ready.
#endif

  /* This loop changes GPIO39~GPIO42 to Function 1, which is GPIO.
   * PIN_FUNC_SELECT sets the IOMUX function register appropriately.
   * IO_MUX_GPIO0_REG is the register for GPIO0, which we then seek from.
   * PIN_FUNC_GPIO is a define for Function 1, which sets the pins to GPIO mode.
   */
  for(uint8_t gpio_pin = 39; gpio_pin < 43; gpio_pin++) {
    PIN_FUNC_SELECT(IO_MUX_GPIO0_REG + (gpio_pin * 4), PIN_FUNC_GPIO);
  }

  // Assign AttenuatorSerial to pins 11/10 for the Attenuator/Wireless communications.
  AttenuatorSerial.begin(9600, SERIAL_8N1, ATTENUATOR_RX_PIN, ATTENUATOR_TX_PIN);

  // Assign WandSerial to pins 44/43 for the Neutrona Wand communications.
  WandSerial.begin(9600, SERIAL_8N1, WAND_RX_PIN, WAND_TX_PIN);

  // Define the WirelessManager object only after NVS/Preferences are initialized.
  if(wirelessMgr == nullptr) {
    wirelessMgr = new WirelessManager(WirelessDeviceType::PROTON_PACK, "192.168.1.4");

    #if defined(RESET_AP_SETTINGS)
      // Reset the WiFi password to the expected default on every startup.
      wirelessMgr->resetWifiPassword();
      debugln(F("WARNING: Firmware forced a reset of the local WiFi password!"));
    #endif
  }
#else
  Serial.begin(9600); // Standard HW serial (USB) console.
  AttenuatorSerial.begin(9600); // Add-on Attenuator communication (19/18).
  WandSerial.begin(9600); // Communication to the Neutrona Wand (17/16).
#endif

  // Initialize the SerialTransfer objects by passing in the appropriate ports.
  attenuatorComs.begin(AttenuatorSerial, false, Serial, 100); // Attenuator/Wireless
  wandComs.begin(WandSerial, false); // Neutrona Wand

  // Setup the audio device for this controller.
  setupAudioDevice();

  // Setup the i2c bus using the Wire protocol.
#ifdef ESP32
  // ESP32-S3 requires manually specifying SDA and SCL pins first.
  Wire.begin(I2C_SDA, I2C_SCL, 400000UL);
  Wire1.begin(TEMP_SDA, TEMP_SCL, 400000UL);

  // Initialize the HDC1080 temp/humidity sensor.
  Wire1.beginTransmission(0x40);
  if(Wire1.endTransmission() == 0) {
    b_temp_sensor_detected = true;
    tempSensor.resetConfiguration();
    tempSensor.disableHeater();
    tempSensor.setHumidityResolution(GuL::HDC1080::HumidityMeasurementResolution::HUM_RES_14BIT);
    tempSensor.setTemperaturResolution(GuL::HDC1080::TemperatureMeasurementResolution::TEMP_RES_14BIT);
    tempSensor.setAcquisitionMode(GuL::HDC1080::AcquisitionModes::SINGLE_CHANNEL);
  }
#else
  Wire.begin();
  Wire.setClock(400000UL); // Sets the i2c bus to 400kHz
#endif

  // Initialize an optional power meter on the i2c bus.
  if(b_use_power_meter) {
    sendDebug(F("Init power meter..."));
    powerMeterInit();
  }

  // Rotary encoder for volume control.
  pinModeFast(ROTARY_ENCODER_A, INPUT_PULLUP);
  pinModeFast(ROTARY_ENCODER_B, INPUT_PULLUP);

  // Status indicator LED on the v1.5 GPStar Proton Pack Board.
  pinModeFast(PACK_STATUS_LED_PIN, OUTPUT);
  digitalWriteFast(PACK_STATUS_LED_PIN, LOW);

  // Configure the various switches on the pack.
  switch_power.setDebounceTime(50);
  switch_alarm.setDebounceTime(50);
  switch_mode.setDebounceTime(50);
  switch_vibration.setDebounceTime(50);
  switch_cyclotron_lid.setDebounceTime(50);
#ifndef ESP32
  switch_cyclotron_direction.setDebounceTime(50);
  switch_smoke.setDebounceTime(50);
#endif

// Change PWM frequency of pin 45 for the vibration motor, we do not want it high pitched.
#ifdef ESP32
  // Use of the register is not needed by ESP32, as it uses a different method for PWM.
  ledcAttachChannel(VIBRATION_PIN, 155, 8, 5); // Uses 155 Hz frequency, 8-bit resolution, channel 5
#else
  // For ATmega2560, we set the PWM frequency for pin 45 (TCCR5B) to 122.55 Hz.
  TCCR5B = (TCCR5B & B11111000) | B00000100;
  pinMode(VIBRATION_PIN, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.
#endif

  // Smoke motor for the N-Filter.
  pinModeFast(NFILTER_SMOKE_PIN, OUTPUT);

  // Fan pin for the N-Filter smoke.
  pinModeFast(NFILTER_FAN_PIN, OUTPUT);

  // Second smoke motor (booster tube)
  pinModeFast(BOOSTER_TUBE_SMOKE_PIN, OUTPUT);

  // A fan pin that goes off at the same time as the booster tube smoke pin.
  pinModeFast(BOOSTER_TUBE_FAN_PIN, OUTPUT);

  // Another optional N-Filter LED.
  pinModeFast(NFILTER_LED_PIN, OUTPUT);

#ifdef ESP32
  // Reserved for future expansion.
  // FastLED.addLeds<NEOPIXEL, EXPANSION1_LED_PIN>(tvg_leds, 64).setCorrection(TypicalLEDStrip);

  // Reserved for future expansion.
  // FastLED.addLeds<NEOPIXEL, EXPANSION2_LED_PIN>(expansion_leds, 64).setCorrection(TypicalLEDStrip);
#else
  // Cyclotron Switch Panel LEDs [Deprecated for the PackII board]
  pinModeFast(CYCLOTRON_SWITCH_LED_R1_PIN, OUTPUT);
  pinModeFast(CYCLOTRON_SWITCH_LED_R2_PIN, OUTPUT);
  pinModeFast(CYCLOTRON_SWITCH_LED_Y1_PIN, OUTPUT);
  pinModeFast(CYCLOTRON_SWITCH_LED_Y2_PIN, OUTPUT);
  pinModeFast(CYCLOTRON_SWITCH_LED_G1_PIN, OUTPUT);
  pinModeFast(CYCLOTRON_SWITCH_LED_G2_PIN, OUTPUT);
  pinModeFast(YEAR_TOGGLE_LED_PIN, OUTPUT);
  pinModeFast(VIBRATION_TOGGLE_LED_PIN, OUTPUT);
#endif

  // Configure the vibration state.
  if(switch_vibration.getState() == LOW) {
    b_vibration_switch_on = true;
  }
  else {
    b_vibration_switch_on = false;
  }

  // Configure the year mode, though this will be modified
  // as based on the user's stored preferences in EEPROM.
  if(switch_mode.getState() == LOW) {
    gpstarPack.setSystemTheme(SYSTEM_1984);
  }
  else {
    gpstarPack.setSystemTheme(SYSTEM_AFTERLIFE);
  }
  SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();

  // Load any saved settings stored in the EEPROM memory of the Proton Pack.
  if(b_eeprom) {
    readEEPROM();
  }

  // Reset the master volume. Important to keep this as we startup the system at the lowest volume.
  // Then the EEPROM reads any settings if required, then we reset the volume.
  updateMasterVolume(true);

  // Setup and configure the Inner Cyclotron LEDs.
  resetInnerCyclotronLEDs();
  updateProtonPackLEDCounts();

  // Check some LED brightness settings for various LEDs.
  // The datatype used should avoid checks for negative values.
  if(i_powercell_brightness > 100) {
    i_powercell_brightness = 100;
  }

  if(i_cyclotron_brightness > 100) {
    i_cyclotron_brightness = 100;
  }

  if(i_cyclotron_inner_brightness > 100) {
    i_cyclotron_inner_brightness = 100;
  }

  // Reset cyclotron ramps.
  resetRampSpeeds();

  // Perform initial pack reset.
  packOffReset();

  // Start some timers
  ms_fast_led.start(i_fast_led_delay);
  ms_check_music.start(i_music_check_delay);
  ms_attenuator_check.start(i_attenuator_disconnect_delay);
  ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);

  // Perform power-on sequence if demo light mode is not enabled per user preferences.
  if(!b_demo_light_mode) {
    // System Power On Self Test
    playEffect(S_POWER_ON);
    ms_delay_post.start(0);
  }
  else {
    if(gpstarPack.getSystemMode() == MODE_SUPER_HERO) {
      // Auto start the pack if it is in startup (demo) light mode.
      PACK_ACTION_STATE = ACTION_ACTIVATE;
    }

    ms_wand_check.start(i_wand_disconnect_delay / 2);
    b_pack_post_finish = true;
  }

#ifdef ESP32
  debugf("Setup complete, free heap: %u bytes\n", ESP.getFreeHeap());
#endif
}

void updateLEDs() {
  // Update all LED's when the FastLED timer has finished.
  if(ms_fast_led.justFinished()) {
    FastLED.show();

    // Restart the FastLED timer.
    ms_fast_led.start(i_fast_led_delay);

    if(b_powercell_updating) {
      b_powercell_updating = false;
    }
  }
}

// Loop logic dedicated to this device which handles all of the standard operations.
void mainLoop() {
  checkMusic();
  checkSwitches();
  checkRotaryEncoder();
  checkMenuVibration();

  // Check current voltage/amperage draw using available methods if enabled.
  if(b_use_power_meter) {
    // Only check if power meter if present and self-test has completed.
    checkPowerMeter();
  }

  switch(PACK_STATE) {
    case MODE_OFF:
      // Turn on the status indicator LED.
      digitalWriteFast(PACK_STATUS_LED_PIN, HIGH);

      if(vibrationSwitchedCount >= 5) {
        // Vibration switch was just toggled 5 times with ribbon cable off, so reset the pack wifi password.
        resetWifiCommand();
        vibrationSwitchedCount = 0;
      }

      if(PACK_ACTION_STATE == ACTION_IDLE && ms_delay_post.justFinished()) {
        // Brass Pack shutdown steam effect.
        playEffect(PROGMEM_READU16(sfx_smoke[random(5)]));
      }

      if(b_ramp_down && !b_overheating && !b_pack_alarm) {
        if(b_spectral_lights_on) {
          // If we enter the LED EEPROM menu while the pack is ramping off, stop it right away.
          packOffReset();
          spectralLightsOn();
        }
        else {
          cyclotronControl();
          cyclotronSwitchLEDLoop();
          powercellLoop();
        }
      }
      else {
        if(!b_spectral_lights_on) {
          if(ms_fadeout.justFinished()) {
            if(fadeOutCyclotron()) {
              ms_fadeout.start(i_fadeout_duration);
            }
            else {
              ms_fadeout.stop();
              b_fade_out = false;
            }
          }

          if(!b_reset_start_led && !ms_fadeout.isRunning()) {
            packOffReset();
          }
        }
      }
    break;

    case MODE_ON:
      // Turn off the status indicator LED.
      digitalWriteFast(PACK_STATUS_LED_PIN, LOW);

      if(b_spectral_lights_on) {
        spectralLightsOff();
      }

      if(b_pack_shutting_down) {
        b_pack_shutting_down = false;
      }

      if(b_ramp_down && !ms_mash_lockout.isRunning()) {
        b_ramp_down = false;
        b_ramp_down_start = false;
        b_inner_ramp_down = false;

        resetRampUp();
      }

      if(ribbonCableAttached() && !b_overheating) {
        if(b_pack_alarm) {
          if(gpstarPack.isTheme80s()) {
            // Reset the LEDs before resetting the alarm flag.
            if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
              resetCyclotronState();
            }

            ms_cyclotron.start(0);
          }
          else {
            ms_cyclotron.start(i_outer_current_ramp_speed);
          }

          ms_cyclotron_ring.start(i_inner_current_ramp_speed);

          ventLight(false);
          ventLightLEDW(false);

          b_pack_alarm = false;

          resetRampUp();

          stopEffect(S_PACK_RECOVERY);
          playEffect(S_PACK_RECOVERY);

          packStartup(false); // Start the pack using an abbreviated startup sequence.
        }
      }

      // Play a little bit of smoke and N-Filter vent lights while firing and other misc sound effects.
      if(b_wand_firing) {
        // Mix some impact sound effects.
        mixExtraFiringEffects();

        if(ms_smoke_on.justFinished()) {
          ms_smoke_on.stop();
          ms_smoke_timer.start(PROGMEM_READU16(i_smoke_timer[(uint8_t)gpstarPack.getPowerLevel() - 1]));
          b_vent_sounds_playing = false;
        }

        if(ms_smoke_timer.justFinished()) {
          if(!ms_smoke_on.isRunning()) {
            ms_smoke_on.start(PROGMEM_READU16(i_smoke_on_time[(uint8_t)gpstarPack.getPowerLevel() - 1]));
          }
        }

        if(ms_smoke_on.isRunning()) {
          // Turn on some smoke and play some vent sounds if smoke is enabled.
          if(b_smoke_enabled) {
            // Turn on some smoke.
            smokeNFilter(true);

            // Play some sounds with the smoke and vent lighting.
            if(!b_vent_sounds_playing) {
              playVentSounds();

              b_vent_sounds_playing = true;
            }

            fanNFilter(true);
          }

          // We are strobing the N-Filter jewel.
          if(ms_vent_light_off.justFinished()) {
            ms_vent_light_off.stop();
            ms_vent_light_on.start(i_vent_light_delay);

            ventLight(true);
          }
          else if(ms_vent_light_on.justFinished()) {
            ms_vent_light_on.stop();
            ms_vent_light_off.start(i_vent_light_delay);

            ventLight(false);
          }

          // The LED-W will not strobe during this venting.
          ventLightLEDW(true);
        }
        else {
          smokeNFilter(false);
          ventLight(false);
          ventLightLEDW(false);
          fanNFilter(false);
        }
      }

      if(b_venting) {
        packVenting();
      }

      cyclotronControl(); // Set timers for the cyclotron.

      if(ms_mash_lockout.justFinished()) {
        restartFromWandMash();
      }
      else if(b_wand_mash_lockout && ms_mash_lockout.isRunning()) {
        if((ms_mash_lockout.delay() / 1.5) > ms_mash_lockout.remaining()) {
          // Force incorrect Powercell LED to switch it off temporarily.
          i_powercell_led = i_powercell_num_leds + 1;
        }
      }

      if(ms_delay_post.justFinished()) {
        // This controls the idle SFX fadeout.
        fadeoutIdleSounds();
      }

      cyclotronSwitchLEDLoop(); // Update the cyclotron.

      if(b_overheating && b_overheat_lights_off) {
        powercellRampDown();
      }
      else {
        powercellLoop();
      }
    break;
  }

  switch(PACK_ACTION_STATE) {
    case ACTION_IDLE:
    default:
      // Do nothing.
    break;

    case ACTION_OFF:
      packShutdown();
    break;

    case ACTION_ACTIVATE:
      packStartup(true); // Start the pack using the full-length startup sequence.

      if(b_first_boot) {
        // Used in demo light mode to determine first boot.
        b_first_boot = false;
      }
    break;
  }
}

// The main loop of the program which manages all system operations which must occur on every loop.
void loop() {
  #ifdef ESP32
  if(b_initial_wifi_setup_finished) {
  #endif
  // Update the available audio device.
  updateAudio();

  // Check for any new serial commands were received from the Neutrona Wand.
  checkWand();

  // Check if the wand is considered to have been disconnected.
  wandDisconnectCheck();

  // Check if Attenuator is present.
  attenuatorHandShake();

  // Check if any new serial commands were received.
  checkAttenuator();

  if(b_pack_post_finish) {
    if(!b_demo_light_mode || !b_first_boot || (b_demo_light_mode && b_wand_connected) || (b_demo_light_mode && !b_wand_connected && !b_wand_syncing && ms_wand_check.remaining() < 1)) {
      // Handle any actions after POST event.
      mainLoop();

      // Update the LEDs.
      updateLEDs();
    }
  }
  else {
    // Run the POST sequence.
    systemPOST();
  }
#ifdef ESP32
  }
  // The ESP32 uses a dual-core CPU with the loop() executing in Core0 by default.
  // Using vTaskDelay even without core-pinning will allow other tasks to run on Core1.
  // Features such as networking, WiFi, and OTA updates can benefit from this delay.
  vTaskDelay(pdMS_TO_TICKS(1)); // Translate 1ms to ticks for a very brief delay.

  // Run checks on web-related tasks.
  webLoops();

  // Get the current temperature from the HDC1080 sensor.
  readTemperature();

  // Take action with Wifi based on user preference and presence of the Attenuator.
  switch(WIFI_USER_MODE) {
    case WIFI_DISABLED:
      shutdownWireless(); // Keep the WiFi off (function will only take action if WiFi is still on).
    break;

    case WIFI_ENABLED:
      // Force the WiFi to remain on, disregarding any Attenuator connection.
      if(!b_httpd_started && b_pack_post_finish) {
        // Begin by setting up WiFi as a prerequisite to all else.
        restartWireless();
      }
    break;

    case WIFI_DEFAULT:
    default:
      // Take action based solely on the presence of the Attenuator (Connected = WiFi Off, Disconnected = WiFi On).
      if(b_attenuator_connected) {
        // Turn off WiFi and the web server if the Attenuator is connected.
        shutdownWireless();
      }
      else if(!b_attenuator_connected && !b_attenuator_syncing && !b_httpd_started && b_pack_post_finish) {
        // Begin by setting up WiFi as a prerequisite to all else.
        restartWireless();
      }
    break;
  }

  if(!b_initial_wifi_setup_finished) {
    b_initial_wifi_setup_finished = true;
  }
#endif
}
