/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#if defined(__XTENSA__)
  // ESP - Suppress warning about SPI hardware pins
  #define FASTLED_INTERNAL
#endif

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
#include "Colours.h"
#include "Bargraph.h"
#if defined(__XTENSA__)
  // ESP - Include WiFi/Bluetooth
  #include "Wireless.h"
#endif

void setup() {
  // Enable Serial connection(s) and communication with gpstar Proton Pack PCB.
  #if defined(__XTENSA__)
    // ESP32 - Serial Console and Device Comms via Serial2
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    packComs.begin(Serial2);
  #else
    // Nano - Utilizes the only Serial connection
    Serial.begin(9600);
    packComs.begin(Serial);
  #endif

  // Bootup into proton mode (default for pack and wand).
  FIRING_MODE = PROTON;

  if(!b_wait_for_pack) {
    // If not waiting for the pack set power level to 5.
    POWER_LEVEL = LEVEL_5;
  }
  else {
    // When waiting for the pack set power level to 1.
    POWER_LEVEL = LEVEL_1;
  }

  // Default to 1984 for power level animation when pack is not connected, otherwise 2021.
  if(!b_wait_for_pack) {
    YEAR_MODE = YEAR_1984;
  }
  else {
    YEAR_MODE = YEAR_2021;
  }

  // Begin at menu level one. This affects the behavior of the rotary dial.
  MENU_LEVEL = MENU_1;

  // RGB LEDs for effects (upper/lower) and user status (top).
  FastLED.addLeds<NEOPIXEL, ATTENUATOR_LED_PIN>(attenuator_leds, ATTENUATOR_NUM_LEDS);

  // Change top indicator to green when device is on and ready.
  attenuator_leds[TOP_LED] = getHueAsRGB(TOP_LED, C_GREEN, 128);

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
  #if defined(__XTENSA__)
    // ESP32
    ledcSetup(0, 5000, 8);
    ledcAttachPin(VIBRATION_PIN, 0);
  #else
    // Nano
    pinMode(VIBRATION_PIN, OUTPUT);
  #endif

  // Turn off any user feedback.
  noTone(BUZZER_PIN);
  useVibration(i_min_power, 0);

  #if defined(__XTENSA__)
    // ESP32 - Setup WiFi and WebServer
    bool b_ap_started = startAccessPoint();
    Serial.println(b_ap_started ? "Ready" : "Failed");

    if(b_ap_started) {
      delay(10); // Allow a small delay before config.

      // Do the AP network configuration.
      configureNetwork();

      // Start the local web server.
      startWebServer();

      // Begin timer for remote client events.
      ms_cleanup.start(i_websocketCleanup);
    }
  #endif

  // Initialize critical timers.
  ms_fast_led.start(1);
}

void loop() {
  #if defined(__XTENSA__)
    // ESP32 - Manage the WebSocket clients.
    if (ms_cleanup.remaining() < 1) {
      // Clean up any websocket clients (oldest connections).
      ws.cleanupClients();
      ms_cleanup.start(i_websocketCleanup);
    }
  #endif

  if(b_wait_for_pack) {
    // Handshake with the pack. Telling the pack that we are here.
    attenuatorSerialSend(A_HANDSHAKE);

    // Synchronise some settings with the pack.
    checkPack();

    if(b_comms_open) {
      // Move into the main loop if we got data from the pack.
      b_wait_for_pack = false;
      mainLoop();
    }
    else {
      // Pause and try again in a moment.
      delay(10);
    }
  }
  else {
    mainLoop();
  }
}

void debug(String message) {
  // Write a debug message to the serial console.
  #if defined(__XTENSA__)
    // ESP32
    Serial.println(message);
  #else
    // Nano
    if(!b_wait_for_pack) {
      // Can only use Serial output if pack not connected.
      Serial.println(message);
    }
  #endif
}

void mainLoop() {
  // Monitor for interactions by user.
  checkPack();
  switchLoops();
  checkRotaryPress();
  checkRotaryEncoder();

  /*
   * Left Toggle
   *
   * Paired:
   * When paired with the gpstar Proton Pack controller, will turn the
   * pack on or off. When the pack is on the bargraph will automatically
   * enable and display an animation which matches the Neutrona Wand
   * bargraph (whether stock 5-LED version or 28-segment by Frutto).
   *
   * Standalone:
   * When not paired with the gpstar Proton Pack controller, will turn
   * on the bargraph which will display some pre-set pattern.
   */

  // Turns the pack on or off (when paired) via left toggle.
  if(switch_left.isPressed() || switch_left.isReleased()) {
    if(switch_left.getState() == LOW) {
      attenuatorSerialSend(A_TURN_PACK_ON);
      b_pack_on = true;
    }
    else {
      attenuatorSerialSend(A_TURN_PACK_OFF);
      b_pack_on = false;
    }
  }

  // Turn on the bargraph when certain conditions are met.
  // This supports pack connection or standalone operation.
  if(b_pack_on) {
    if(BARGRAPH_STATE == BG_OFF && !(b_overheating || b_pack_alarm)) {
      bargraphReset(); // Enable bargraph for use (resets variables and turns it on).
      BARGRAPH_PATTERN = BG_POWER_RAMP; // Bargraph idling loop.
    }
  }
  else {
    if(switch_left.getState() == HIGH && !b_wait_for_pack){
      // Clear all bargraph elements and turn off the device.
      bargraphOff();
    }
  }

  /*
   * Right Toggle
   *
   * The right toggle activates the LEDs on the device manually.
   *
   * When paired with the gpstar Proton pack controller, the LEDs
   * will change colors based on user interactions.
   */
  if(switch_right.getState() == LOW) {
    // If in pre-overheat warning, overheat, or alarm modes...
    if((b_firing && i_speed_multiplier > 1) || b_overheating || b_pack_alarm) {
      if(ms_blink_leds.justFinished()) {
        ms_blink_leds.start(i_blink_leds / i_speed_multiplier);
      }

      if(ms_blink_leds.isRunning()) {
        if(b_firing && i_speed_multiplier > 1 && !b_overheating) {
          // Switch to a modified firing pattern for the pre-overheat warning.
          BARGRAPH_PATTERN = BG_INNER_PULSE;
        }

        if(ms_blink_leds.remaining() < (i_blink_leds / i_speed_multiplier) / 2) {
          // Only blink the lower LED as we will use a fade effect for the upper LED.
          attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
          useVibration(i_min_power, 0); // Stop vibration.
          buzzOff(); // Stop buzzer tone.
        }
        else {
          controlLEDs(); // Turn LEDs on using appropriate color scheme.
          useVibration(i_max_power, i_vibrate_max_time); // Provide physical feedback.
          buzzOn(523); // Tone as note C4
        }
      }
    }
    else {
      controlLEDs(); // Turn LEDs on using appropriate color scheme.
    }
  }
  else {
    // Turn off the LEDs by setting to black.
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
    attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
  }

  // Turn off buzzer if timer finished.
  if(b_buzzer_on && ms_buzzer.justFinished()) {
    buzzOff();
  }

  // Turn off vibration if timer finished.
  if(b_vibrate_on && ms_vibrate.justFinished()) {
    useVibration(i_min_power, 0);
  }

  // Update bargraph elements, leveraging cyclotron speed modifier.
  // In reality this multiplier is a divisor to the standard delay.
  bargraphUpdate(i_speed_multiplier);

  // Update the device LEDs and restart the timer.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

void buzzOn(unsigned int i_freq) {
  tone(BUZZER_PIN, i_freq);
  ms_buzzer.start(i_buzzer_max_time);
  b_buzzer_on = true;
}

void buzzOff() {
  noTone(BUZZER_PIN);
  ms_buzzer.stop();
  b_buzzer_on = false;
}

void useVibration(uint8_t i_power_level, unsigned int i_duration) {
  // Power should be specified as 0-255
  #if defined(__XTENSA__)
    // ESP32
    ledcWrite(0, i_power_level);
  #else
    // Nano
    analogWrite(VIBRATION_PIN, i_power_level);
  #endif
  b_vibrate_on = (i_power_level > 0);

  if(b_vibrate_on) {
    // Set timer for shorter of given duration or max runtime.
    ms_vibrate.start(min(i_duration, i_vibrate_max_time));
  }
}

void controlLEDs() {
  // Set upper LED based on alarm or overheating state, when connected.
  // Otherwise, use the standard pattern/color for illumination.
  if(b_pack_alarm || b_overheating) {
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_RED_FADE);
  }
  else {
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_AMBER_PULSE);
  }

  // Set lower LED based on the current firing mode.
  uint8_t i_scheme;
  switch(FIRING_MODE) {
    case SLIME:
      i_scheme = C_GREEN;
    break;

    case STASIS:
      i_scheme = C_LIGHT_BLUE;
    break;

    case MESON:
      i_scheme = C_ORANGE;
    break;

    case SPECTRAL:
      i_scheme = C_RAINBOW;
    break;

    case HOLIDAY:
      i_scheme = C_REDGREEN;
    break;

    case SPECTRAL_CUSTOM:
      i_scheme = C_SPECTRAL_CUSTOM;
    break;

    case SETTINGS:
      i_scheme = C_WHITE;
    break;

    case PROTON:
    case VENTING:
    default:
      i_scheme = C_RED;
    break;
  }

  // Update the lower LED based on the scheme determined above.
  attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, i_scheme);
}

/*
  * Rotary Dial Center Press
  *
  * Performs action based on a short or long press of this button.
  *  Short: Action
  *  Long: Navigation
  */
void checkRotaryPress() {
  // Reset on each loop as we need to detect for change.
  CENTER_STATE = NO_ACTION;

  // Determine whether the rotary dial (center button) got a short or long press.
  if(encoder_center.isPressed()) {
    // Start all timers when the rotary dial is pressed.
    ms_center_double_tap.start(i_center_double_tap_delay);
    ms_center_long_press.start(i_center_long_press_delay);
    b_center_pressed = true;
  }

  if(b_center_pressed) {
    if(encoder_center.isReleased() && i_press_count >= 1) {
      // If released and we already counted 1 press, this is a "double tap".
      CENTER_STATE = DOUBLE_PRESS;
      b_center_pressed = false;
      i_press_count = 0;
      ms_center_double_tap.stop();
    }
    else if(encoder_center.isReleased() && ms_center_double_tap.remaining() > 0) {
      // If released and the double-tap timer is still running, then ONLY increment count.
      i_press_count++;
    }
    else if(ms_center_double_tap.remaining() < 1 && i_press_count == 1) {
      // If the double-tap counter ran out with only a single press, this was a "short" press.
      CENTER_STATE = SHORT_PRESS;
      b_center_pressed = false;
      i_press_count = 0;
      ms_center_double_tap.stop();
      ms_center_long_press.stop();
    }
    else if(ms_center_long_press.remaining() < 1) {
      // Consider a long-press event if the timer is run out before released.
      CENTER_STATE = LONG_PRESS;
      b_center_pressed = false;
      i_press_count = 0;
    }
  }

  switch(CENTER_STATE) {
    case SHORT_PRESS:
      // Perform action for short press based on current menu level.
      switch(MENU_LEVEL) {
        case MENU_1:
          // A short, single press should start/stop the music.
          attenuatorSerialSend(A_MUSIC_START_STOP);
          useVibration(i_max_power, i_vibrate_min_time); // Give a quick nudge.
        break;

        case MENU_2:
          // A short, single press should advance to the next track.
          attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
          useVibration(i_max_power, i_vibrate_min_time); // Give a quick nudge.
        break;
      }
    break;

    case DOUBLE_PRESS:
      // Perform action for double tap based on current menu level.
      switch(MENU_LEVEL) {
        case MENU_1:
          // A double press should mute the pack and wand.
          attenuatorSerialSend(A_TOGGLE_MUTE);
          useVibration(i_max_power, i_vibrate_min_time); // Give a quick nudge.
        break;

        case MENU_2:
          // A double press should move back to the previous track.
          attenuatorSerialSend(A_MUSIC_PREV_TRACK);
          useVibration(i_max_power, i_vibrate_min_time); // Give a quick nudge.
        break;
      }
    break;

    case LONG_PRESS:
      // Toggle between the menu levels on a long press.
      // Also provides audio cues as to which menu is in use.
      switch(MENU_LEVEL) {
        case MENU_1:
          MENU_LEVEL = MENU_2; // Change menu level.
          Serial.println("Changed to Menu 2");
          useVibration(i_max_power, i_vibrate_min_time); // Give a quick nudge.
          buzzOn(784); // Tone as note G4
        break;
        case MENU_2:
          MENU_LEVEL = MENU_1; // Change menu level.
          Serial.println("Changed to Menu 1");
          useVibration(i_max_power, i_vibrate_max_time); // Give a long nudge.
          buzzOn(440); // Tone as note A4
        break;
      }
    break;

    default:
      // eg. NO_ACTION - No-op
    break;
  }
}

void readEncoder() {
  // Determines if encoder was turned CW or CCW.
  if(digitalRead(r_encoderA) == digitalRead(r_encoderB)) {
    i_encoder_pos++;
  }
  else {
    i_encoder_pos--;
  }

  i_val_rotary = i_encoder_pos / 2.5;
}

/*
  * Rotary Dial Rotation
  *
  * Performs action based turning the dial.
  */
void checkRotaryEncoder() {
  // Take action if rotary encoder value was turned CW.
  if(i_val_rotary > i_last_val_rotary) {
    if(!ms_rotary_debounce.isRunning()) {
      if(b_firing && i_speed_multiplier > 1) {
        // Tell the pack to cancel the current overheat warning.
        // Only do so after 5 turns of the dial.
        i_rotary_count++;
        if(i_rotary_count % 5 == 0) {
          attenuatorSerialSend(A_WARNING_CANCELLED);
          i_rotary_count = 0;
        }
      }
      else {
        // Perform action based on the current menu level.
        switch(MENU_LEVEL) {
          case MENU_1:
            // Tell pack to increase overall volume.
            attenuatorSerialSend(A_VOLUME_INCREASE);
          break;

          case MENU_2:
            // Tell pack to increase effects volume.
            attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
          break;
        }
      }

      ms_rotary_debounce.start(rotary_debounce_time);
    }
  }

  // Take action if rotary encoder value was turned CCW.
  if(i_val_rotary < i_last_val_rotary) {
    if(!ms_rotary_debounce.isRunning()) {
      if(b_firing && i_speed_multiplier > 1) {
        // Tell the pack to cancel the current overheat warning.
        // Only do so after 5 turns of the dial.
        i_rotary_count++;
        if(i_rotary_count % 5 == 0) {
          attenuatorSerialSend(A_WARNING_CANCELLED);
          i_rotary_count = 0;
        }
      }
      else {
        // Perform action based on the current menu level.
        switch(MENU_LEVEL) {
          case MENU_1:
            // Tell pack to decrease overall volume.
            attenuatorSerialSend(A_VOLUME_DECREASE);
          break;

          case MENU_2:
            // Tell pack to decrease effects volume.
            attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
          break;
        }
      }

      ms_rotary_debounce.start(rotary_debounce_time);
    }
  }

  // Remember the last rotary value.
  i_last_val_rotary = i_val_rotary;

  if(ms_rotary_debounce.justFinished()) {
    ms_rotary_debounce.stop();
  }
}

void switchLoops() {
  // Perform debounce and get button/switch states.
  switch_left.loop();
  switch_right.loop();
  encoder_center.loop();
}

void attenuatorSerialSend(int i_message) {
  sendStruct.i = i_message;
  sendStruct.s = A_COM_START;
  sendStruct.e = A_COM_END;

  packComs.sendDatum(sendStruct);
}

void checkPack() {
  // Pack communication to the Attenuator device.
  if(packComs.available()) {
    packComs.rxObj(comStruct);

    if(!packComs.currentPacketID()) {
      if(comStruct.i > 0 && comStruct.s == A_COM_START && comStruct.e == A_COM_END) {
        b_comms_open = true;

        // Use the passed communication flag to set the proper state for this device.
        switch(comStruct.i) {
          case A_SYNC_START:
            debug("Sync Start");
            b_wait_for_pack = true;
            i_speed_multiplier = 1;
          break;

          case A_SYNC_END:
            debug("Sync End");
            b_wait_for_pack = false;
          break;

          case A_PACK_ON:
            debug("Pack On");

            // Pack is on (directly).
            b_pack_on = true;

            BARGRAPH_PATTERN = BG_POWER_RAMP;
          break;

          case A_WAND_ON:
            debug("Wand On");

            // Pack is on (via wand).
            b_pack_on = true;

            BARGRAPH_PATTERN = BG_POWER_RAMP;
          break;

          case A_PACK_OFF:
          case A_WAND_OFF:
            debug("Pack Off");

            // Pack is off (directly or via the wand).
            b_pack_on = false;

            if(BARGRAPH_STATE != BG_OFF) {
              // If not already off, illuminate fully before ramp down.
              bargraphFull();
            }
            BARGRAPH_PATTERN = BG_RAMP_DOWN;
          break;

          case A_PACK_CONNECTED:
            // The Proton Pack is connected.
            debug("Pack Connected");
          break;

          case A_HANDSHAKE:
            //debug("Handshake");

            // The pack is asking us if we are still here. Respond back.
            attenuatorSerialSend(A_HANDSHAKE);
          break;

          case A_YEAR_1984:
            if(YEAR_MODE != YEAR_1984) {
              debug("Mode 1984");
              YEAR_MODE = YEAR_1984;
            }
          break;

          case A_YEAR_1989:
            if(YEAR_MODE != YEAR_1989) {
              debug("Mode 1989");
              YEAR_MODE = YEAR_1989;
            }
          break;

          case A_YEAR_AFTERLIFE:
            if(YEAR_MODE != YEAR_2021) {
              debug("Mode 2021");
              YEAR_MODE = YEAR_2021;
            }
          break;

          case A_PROTON_MODE:
            debug("Proton");
            FIRING_MODE = PROTON;
          break;

          case A_SLIME_MODE:
            debug("Slime");
            FIRING_MODE = SLIME;
          break;

          case A_STASIS_MODE:
            debug("Stasis");
            FIRING_MODE = STASIS;
          break;

          case A_MESON_MODE:
            debug("Meson");
            FIRING_MODE = MESON;
          break;

          case A_SPECTRAL_CUSTOM_MODE:
            debug("Spectral Custom");
            FIRING_MODE = SPECTRAL_CUSTOM;

            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_COLOUR_DATA:
            debug("Spectral Color Data");
            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_MODE:
            debug("Spectral");
            FIRING_MODE = SPECTRAL;
          break;

          case A_HOLIDAY_MODE:
            debug("Spectral Holiday");
            FIRING_MODE = HOLIDAY;
          break;

          case A_VENTING_MODE:
            debug("Venting");
            FIRING_MODE = VENTING;
          break;

          case A_SETTINGS_MODE:
            debug("Settings");
            FIRING_MODE = SETTINGS;
          break;

          case A_POWER_LEVEL_1:
            debug("Power Level 1");
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_1;
          break;

          case A_POWER_LEVEL_2:
            debug("Power Level 2");
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_2;
          break;

          case A_POWER_LEVEL_3:
            debug("Power Level 3");
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_3;
          break;

          case A_POWER_LEVEL_4:
            debug("Power Level 4");
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_4;
          break;

          case A_POWER_LEVEL_5:
            debug("Power Level 5");
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_5;
          break;

          case A_ALARM_ON:
            debug("Alarm On");

            // Alarm is on.
            b_pack_alarm = true;

            bargraphFull();
            BARGRAPH_PATTERN = BG_RAMP_DOWN;

            if(b_pack_on) {
              ms_blink_leds.start(i_blink_leds);
            }
          break;

          case A_ALARM_OFF:
            debug("Alarm Off");

            // Alarm is off.
            b_pack_alarm = false;

            if(b_pack_on) {
              ms_blink_leds.stop();

              bargraphClear();
              BARGRAPH_PATTERN = BG_POWER_RAMP;

              useVibration(i_min_power, 0); // Stop vibration.
            }
          break;

          case A_OVERHEATING:
            debug("Overheating");

            // Pack is overheating.
            b_overheating = true;
            ms_blink_leds.start(i_blink_leds);

            bargraphFull();
            BARGRAPH_PATTERN = BG_RAMP_DOWN;
          break;

          case A_OVERHEATING_FINISHED:
            debug("Vented");

            // Venting process completed.
            b_overheating = false;
            ms_blink_leds.stop();

            bargraphClear();
            BARGRAPH_PATTERN = BG_POWER_RAMP;

            useVibration(i_min_power, 0); // Stop vibration.
          break;

          case A_FIRING:
            debug("Firing");

            b_firing = true; // Implies the wand is powered on.
            b_pack_on = true; // Implies the pack is powered on.
            ms_blink_leds.start(i_blink_leds / i_speed_multiplier);

            bargraphClear();
            BARGRAPH_PATTERN = BG_OUTER_INNER;
          break;

          case A_FIRING_STOPPED:
            debug("Idle");

            b_firing = false;
            ms_blink_leds.stop();

            i_speed_multiplier = 1; // Return to normal speed.

            if(b_pack_alarm) {
              // Ramp down if the pack alarm happens while firing.
              bargraphFull();
              BARGRAPH_PATTERN = BG_RAMP_DOWN;
            }
            else {
              // We ramp the bargraph back up after finishing firing.
              bargraphClear();
              BARGRAPH_PATTERN = BG_POWER_RAMP;
            }
          break;

          case A_CYCLOTRON_INCREASE_SPEED:
            debug("Cyclotron Speed Increasing...");

            i_speed_multiplier++;

            debug(String(i_speed_multiplier));
          break;

          case A_CYCLOTRON_NORMAL_SPEED:
            debug("Cyclotron Speed Reset");

            i_speed_multiplier = 1;

            if(b_firing) {
              // Use the "normal" pattern if still firing.
              bargraphClear();
              BARGRAPH_PATTERN = BG_OUTER_INNER;
            }
            else {
              // Otherwise go to the standard power ramp.
              bargraphClear();
              BARGRAPH_PATTERN = BG_POWER_RAMP;
            }
          break;
        }

        #if defined(__XTENSA__)
          // ESP - Alert all WebSocket clients after an API call was received.
          // This excludes the handshake as this is received way too oftens.
          if (comStruct.i != A_HANDSHAKE) {
            notifyWSClients();
          }
        #endif
      }

      comStruct.i = 0;
      comStruct.s = 0;
    }
  }
}