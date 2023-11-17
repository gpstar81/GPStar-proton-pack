/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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
  // Define this before including <FastLED.h>
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
  // Enable Serial connection(s) and communication with GPStar Proton Pack PCB.
  #if defined(__XTENSA__)
    // ESP - Serial Console for messages and Device Comms via Serial2
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    packComs.begin(Serial2);
    pinMode(BUILT_IN_LED, OUTPUT);
  #else
    // Nano - Utilizes the only Serial connection
    Serial.begin(9600);
    packComs.begin(Serial);
  #endif

  // Assume the Super Hero arming mode with Afterlife (default for Haslab).
  ARMING_MODE = MODE_SUPERHERO;
  RED_SWITCH_MODE = SWITCH_OFF;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;

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

  // Begin at menu level one. This affects the behavior of the rotary dial.
  MENU_LEVEL = MENU_1;

  // RGB LEDs for effects (upper/lower) and user status (top).
  FastLED.addLeds<NEOPIXEL, DEVICE_LED_PIN>(device_leds, DEVICE_NUM_LEDS);

  // Change top indicator to red when device is on and ready.
  i_top_led_color = C_RED;
  device_leds[TOP_LED] = getHueAsRGB(TOP_LED, i_top_led_color, i_top_led_brightness);

  // Debounce the toggle switches and encoder pushbutton.
  switch_left.setDebounceTime(switch_debounce_time);
  switch_right.setDebounceTime(switch_debounce_time);
  encoder_center.setDebounceTime(switch_debounce_time);

  // Rotary encoder on the top of the Attenuator.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(r_encoderA), readEncoder, CHANGE);

  // Setup the bargraph after a brief delay.
  delay(100);
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
  vibrateOff();

  #if defined(__XTENSA__)
    // ESP - Setup WiFi and WebServer
    if(startWiFi()) {
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
    // ESP - Manage cleanup for old WebSocket clients.
    if(ms_cleanup.remaining() < 1) {
      // Clean up oldest WebSocket connections.
      ws.cleanupClients();
      ms_cleanup.start(i_websocketCleanup);
    }

    ElegantOTA.loop(); // Handle device reboot after OTA update.
  #endif

  if(b_wait_for_pack) {
    // Wait and synchronise some settings with the pack.
    checkPack();

    if(!b_wait_for_pack) {
      // Indicate that we are no longer waiting on the pack.
      #if defined(__XTENSA__)
        // ESP - Illuminate built-in LED.
        digitalWrite(BUILT_IN_LED, HIGH);
      #endif
    }
    else {
      // Pause and try again in a moment.
      delay(10);
    }
  }
  else {
    // When not waiting for the pack go directly to the main loop.
    mainLoop();
  }
}

void debug(String message) {
  // Writes a debug message to the serial console.
  #if defined(__XTENSA__)
    // ESP32
    #if defined(DEBUG_SEND_TO_CONSOLE)
      Serial.println(message); // Print to serial console.
    #endif
    #if defined(DEBUG_SEND_TO_WEBSOCKET)
      ws.textAll(message); // Send a copy to the WebSocket.
    #endif
  #else
    // Nano
    if(!b_wait_for_pack) {
      // Can only use Serial output if pack is not connected.
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.println(message);
      #endif
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
   * Left Toggle - Uses a pull-up resistor, so setting LOW indicates ON.
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
      b_left_toggle_on = true;
    }
    else {
      attenuatorSerialSend(A_TURN_PACK_OFF);
      b_pack_on = false;
      b_left_toggle_on = false;
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
      bargraphOff(); // Clear all bargraph elements and turn off the device.
    }
  }

  /*
   * Right Toggle - Uses a pull-up resistor, so setting LOW indicates ON.
   *
   * The right toggle activates the LEDs on the device manually.
   *
   * When paired with the gpstar Proton pack controller, the LEDs
   * will change colors based on user interactions.
   *
   * Note that audio and physical feedback will also be disabled
   * when this switch is in the off position.
   */
  if(switch_right.getState() == LOW) {
    b_right_toggle_on = true;

    // If in pre-overheat warning, overheat, or alarm modes...
    if((b_firing && i_speed_multiplier > 1) || b_overheating || b_pack_alarm) {
      // Sets a timer value proportional to the speed of the cyclotron.
      unsigned int i_blink_time = int(i_blink_leds / i_speed_multiplier);

      if(ms_blink_leds.justFinished()) {
        ms_blink_leds.start(i_blink_time);
      }

      if(ms_blink_leds.isRunning()) {
        if(b_firing && i_speed_multiplier > 1 && !b_overheating) {
          // Switch to a modified bargraph pattern for the pre-overheat (venting)
          // warning while the wand is still firing.
          BARGRAPH_PATTERN = BG_INNER_PULSE;
        }

        // Adjust feedback over 1/2 of the blink time allotted.
        if(ms_blink_leds.remaining() < (i_blink_time / 2)) {
          // Denote that certain LED's should be in the dark phase of blinking.
          b_blink_blank = true;
          vibrateOff(); // Stop vibration.
          buzzOff(); // Stop buzzer tone.
        }
        else {
          // Denote that certain LED's should be in the lit phase of blinking.
          b_blink_blank = false;
          useVibration(i_vibrate_min_time); // Provide physical feedback.
          buzzOn(523); // Tone as note C4
        }
      }
    }
    else {
      b_blink_blank = false;
    }
  }
  else {
    // Toggle is in the OFF position.
    b_right_toggle_on = false;
    b_blink_blank = false;
    // Turn off the LEDs by setting to black.
    if(device_leds[UPPER_LED] != CRGB::Black) {
      device_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
    }
    if(device_leds[LOWER_LED] != CRGB::Black) {
      device_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
    }
  }

  // Update LEDs using appropriate color scheme and environment vars.
  updateLEDs();

  // Turn off buzzer if timer finished.
  if(b_buzzer_on && ms_buzzer.justFinished()) {
    buzzOff();
  }

  // Turn off vibration if timer finished.
  if(b_vibrate_on && ms_vibrate.justFinished()) {
    vibrateOff();
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
  if(!b_buzzer_on) {
    tone(BUZZER_PIN, i_freq);
    ms_buzzer.start(i_buzzer_max_time);
  }
  b_buzzer_on = true;
}

void buzzOff() {
  noTone(BUZZER_PIN);
  ms_buzzer.stop();
  b_buzzer_on = false;
}

void useVibration(unsigned int i_duration) {
  if(!b_vibrate_on) {
    #if defined(__XTENSA__)
      // ESP32
      ledcWrite(0, i_max_power);
    #else
      // Nano
      analogWrite(VIBRATION_PIN, i_max_power);
    #endif

    // Set timer for shorter of given duration or max runtime.
    ms_vibrate.start(min(i_duration, i_vibrate_max_time));
  }
  b_vibrate_on = true;
}

void vibrateOff() {
  #if defined(__XTENSA__)
    // ESP32
    ledcWrite(0, i_min_power);
  #else
    // Nano
    analogWrite(VIBRATION_PIN, i_min_power);
  #endif
  ms_vibrate.stop();
  b_vibrate_on = false;
}

void updateLEDs() {
  #if defined(__XTENSA__)
    // ESP - Change top LED color based on wireless connections.
    if(i_ws_client_count > 0) {
      // Change to green when clients are connected remotely.
      i_top_led_color = C_GREEN;
    }
    else {
      // Return to red if no wireless clients are connected.
      i_top_led_color = C_RED;
    }
  #endif

  // Update the top LED based on certain system statuses.
  switch(MENU_LEVEL) {
    case MENU_1:
      // Keep indicator solid.
      ms_top_blink.stop(); // Stop the blink timer which won't be used at this menu level.
      b_top_led_off = false; // Denotes LED is not in an off (blinking) state, but solid.
      device_leds[TOP_LED] = getHueAsRGB(TOP_LED, i_top_led_color, i_top_led_brightness);
    break;

    case MENU_2:
      // Blink the LED when in this menu level.
      if(ms_top_blink.remaining() < 1) {
        ms_top_blink.start(i_top_blink_delay); // Restart the timer to change state.
        b_top_led_off = !b_top_led_off; // Whatever the last value, just flip it.
      }

      if(b_top_led_off) {
        // Not completely dark but very dim (1/10th of the normal brightness).
        device_leds[TOP_LED] = getHueAsRGB(TOP_LED, i_top_led_color, int(i_top_led_brightness / 10));
      }
      else {
        // Return to normal brightness for the current top LED color.
        device_leds[TOP_LED] = getHueAsRGB(TOP_LED, i_top_led_color, i_top_led_brightness);
      }
    break;
  }

  if(b_right_toggle_on) {
    // Set upper LED based on alarm or overheating state, when connected.
    // Otherwise, use the standard pattern/color for illumination.
    if(b_pack_alarm || b_overheating) {
      device_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_RED_FADE);
    }
    else {
      device_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_AMBER_PULSE);
    }
  }
  else {
    if(device_leds[UPPER_LED] != CRGB::Black) {
      device_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
    }
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
  if(!b_right_toggle_on || b_blink_blank) {
    // Turn off when right toggle is off or when mid-blink.
    if(device_leds[LOWER_LED] != CRGB::Black) {
      device_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
    }
  }
  else {
    device_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, i_scheme);
  }
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
          //attenuatorSerialSend(A_MUSIC_START_STOP);
          attenuatorSerialSend(A_MUSIC_PAUSE_RESUME);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          #if defined(__XTENSA__)
            debug("Music Start/Stop");
          #endif
        break;

        case MENU_2:
          // A short, single press should advance to the next track.
          attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          #if defined(__XTENSA__)
            debug("Next Track");
          #endif
        break;
      }
    break;

    case DOUBLE_PRESS:
      // Perform action for double tap based on current menu level.
      switch(MENU_LEVEL) {
        case MENU_1:
          // A double press should mute the pack and wand.
          attenuatorSerialSend(A_TOGGLE_MUTE);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          #if defined(__XTENSA__)
            debug("Toggle Mute");
          #endif
        break;

        case MENU_2:
          // A double press should move back to the previous track.
          attenuatorSerialSend(A_MUSIC_PREV_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          #if defined(__XTENSA__)
            debug("Previous Track");
          #endif
        break;
      }
    break;

    case LONG_PRESS:
      // Toggle between the menu levels on a long press.
      // Also provides audio cues as to which menu is in use.
      switch(MENU_LEVEL) {
        case MENU_1:
          MENU_LEVEL = MENU_2; // Change menu level.
          #if defined(__XTENSA__)
            debug("Changed to Menu 2");
          #endif
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          buzzOn(784); // Tone as note G4
        break;
        case MENU_2:
          MENU_LEVEL = MENU_1; // Change menu level.
          #if defined(__XTENSA__)
            debug("Changed to Menu 1");
          #endif
          useVibration(i_vibrate_max_time); // Give a long nudge.
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
    i_encoder_pos++; // Clockwise
  }
  else {
    i_encoder_pos--; // Counter-clockwise
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
        // Only do so after 5 turns of the dial (CW).
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
            #if defined(__XTENSA__)
              debug("Increase Master Volume");
            #endif
          break;

          case MENU_2:
            // Tell pack to increase effects volume.
            attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
            #if defined(__XTENSA__)
              debug("Increase Effects Volume");
            #endif
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
        // Only do so after 5 turns of the dial (CCW).
        i_rotary_count++;
        if(i_rotary_count % 5 == 0) {
          attenuatorSerialSend(A_WARNING_CANCELLED);
          #if defined(__XTENSA__)
            debug("Overheat Cancelled");
          #endif
          i_rotary_count = 0;
        }
      }
      else {
        // Perform action based on the current menu level.
        switch(MENU_LEVEL) {
          case MENU_1:
            // Tell pack to decrease overall volume.
            attenuatorSerialSend(A_VOLUME_DECREASE);
            #if defined(__XTENSA__)
              debug("Decrease Master Volume");
            #endif
          break;

          case MENU_2:
            // Tell pack to decrease effects volume.
            attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
            #if defined(__XTENSA__)
              debug("Decrease Effects Volume");
            #endif
          break;
        }
      }

      ms_rotary_debounce.start(rotary_debounce_time);
    }
  }

  // Remember the last rotary value for comparison later.
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

void attenuatorSerialSend(uint16_t i_message) {
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
        // Use the passed communication flag to set the proper state for this device.
        switch(comStruct.i) {
          case A_SYNC_START:
            #if defined(__XTENSA__)
              debug("Sync Start");
            #endif

            i_speed_multiplier = 1;
            b_a_sync_start = true;
          break;

          case A_SYNC_END:
            #if defined(__XTENSA__)
              debug("Sync End");
            #endif
            b_wait_for_pack = false;
            b_a_sync_start = false;
          break;

          case A_PACK_ON:
            #if defined(__XTENSA__)
              debug("Pack On");
            #endif

            // Pack is on (directly).
            b_pack_on = true;

            BARGRAPH_PATTERN = BG_POWER_RAMP;
          break;

          case A_WAND_ON:
            #if defined(__XTENSA__)
              debug("Wand On");
            #endif

            // Pack is on (via wand).
            b_pack_on = true;
            b_wand_on = true;

            BARGRAPH_PATTERN = BG_POWER_RAMP;
          break;

          case A_PACK_OFF:
            #if defined(__XTENSA__)
              debug("Pack Off");
            #endif

            // Pack is off (directly or via the wand).
            b_pack_on = false;

            if(BARGRAPH_STATE != BG_OFF) {
              // If not already off, illuminate fully before ramp down.
              bargraphFull();
            }
            BARGRAPH_PATTERN = BG_RAMP_DOWN;
          break;

          case A_WAND_OFF:
            #if defined(__XTENSA__)
              debug("Wand Off");
            #endif

            // Pack is off (directly or via the wand).
            b_pack_on = false;
            b_wand_on = false;

            if(BARGRAPH_STATE != BG_OFF) {
              // If not already off, illuminate fully before ramp down.
              bargraphFull();
            }
            BARGRAPH_PATTERN = BG_RAMP_DOWN;
          break;

          case A_MUSIC_TRACK_COUNT_SYNC:
          #if defined(__XTENSA__)
            debug("Music Track Sync");

            if(comStruct.d1 > 0) {
              i_music_track_count = comStruct.d1;
            }

            debug("Track Count: " + String(i_music_track_count));

            if(i_music_track_count > 0) {
              i_music_track_min = i_music_track_offset; // First music track possible (eg. 500)
              i_music_track_max = i_music_track_offset + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
            }
          #endif
          break;

          case A_PACK_CONNECTED:
            // The Proton Pack is connected.
            #if defined(__XTENSA__)
              debug("Pack Connected");
            #endif
          break;

          case A_HANDSHAKE:
            #if defined(__XTENSA__)
              // debug("Handshake");
            #endif

            if(b_wait_for_pack == true && b_a_sync_start != true) {
              b_a_sync_start = true;
              attenuatorSerialSend(A_SYNC_START);
            }
            else if(b_a_sync_start != true) {
              // The pack is asking us if we are still here. Respond back.
              attenuatorSerialSend(A_HANDSHAKE);
            }
          break;

          case A_MODE_SUPER_HERO:
            if(ARMING_MODE != MODE_SUPERHERO) {
              #if defined(__XTENSA__)
                debug("Super Hero Sequence");
              #endif
              ARMING_MODE = MODE_SUPERHERO;
            }
          break;

          case A_MODE_ORIGINAL:
            if(ARMING_MODE != MODE_ORIGINAL) {
              #if defined(__XTENSA__)
                debug("Original Sequence");
              #endif
              ARMING_MODE = MODE_ORIGINAL;
            }
          break;

          case A_MODE_ORIGINAL_RED_SWITCH_ON:
            // The proton pack red switch is on and has power (cyclotron not powered up yet).
            if(RED_SWITCH_MODE != SWITCH_ON) {
              #if defined(__XTENSA__)
                debug("Red Switch On");
              #endif
              RED_SWITCH_MODE = SWITCH_ON;
            }
          break;

          case A_MODE_ORIGINAL_RED_SWITCH_OFF:
            // The proton pack red switch is off. This will cause a total system shutdown.
            if(RED_SWITCH_MODE != SWITCH_OFF) {
              #if defined(__XTENSA__)
                debug("Red Switch Off");
              #endif
              RED_SWITCH_MODE = SWITCH_OFF;
            }
          break;

          case A_YEAR_1984:
            if(SYSTEM_YEAR != SYSTEM_1984) {
              #if defined(__XTENSA__)
                debug("Mode 1984");
              #endif
              SYSTEM_YEAR = SYSTEM_1984;
            }
          break;

          case A_YEAR_1989:
            if(SYSTEM_YEAR != SYSTEM_1989) {
              #if defined(__XTENSA__)
                debug("Mode 1989");
              #endif
              SYSTEM_YEAR = SYSTEM_1989;
            }
          break;

          case A_YEAR_AFTERLIFE:
            if(SYSTEM_YEAR != SYSTEM_AFTERLIFE) {
              #if defined(__XTENSA__)
                debug("Mode 2021");
              #endif
              SYSTEM_YEAR = SYSTEM_AFTERLIFE;
            }
          break;

/*
          case A_YEAR_FROZEN_EMPIRE:
            if(SYSTEM_YEAR != SYSTEM_FROZEN_EMPIRE) {
              #if defined(__XTENSA__)
                debug("Mode 2024");
              #endif
              SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
            }
          break;
*/

          case A_PROTON_MODE:
            #if defined(__XTENSA__)
              debug("Proton");
            #endif
            FIRING_MODE = PROTON;
          break;

          case A_SLIME_MODE:
            #if defined(__XTENSA__)
              debug("Slime");
            #endif
            FIRING_MODE = SLIME;
          break;

          case A_STASIS_MODE:
            #if defined(__XTENSA__)
              debug("Stasis");
            #endif
            FIRING_MODE = STASIS;
          break;

          case A_MESON_MODE:
            #if defined(__XTENSA__)
              debug("Meson");
            #endif
            FIRING_MODE = MESON;
          break;

          case A_SPECTRAL_CUSTOM_MODE:
            #if defined(__XTENSA__)
              debug("Spectral Custom");
            #endif
            FIRING_MODE = SPECTRAL_CUSTOM;

            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_COLOUR_DATA:
            #if defined(__XTENSA__)
              debug("Spectral Color Data");
            #endif
            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_MODE:
            #if defined(__XTENSA__)
              debug("Spectral");
            #endif
            FIRING_MODE = SPECTRAL;
          break;

          case A_HOLIDAY_MODE:
            #if defined(__XTENSA__)
              debug("Spectral Holiday");
            #endif
            FIRING_MODE = HOLIDAY;
          break;

          case A_VENTING_MODE:
            #if defined(__XTENSA__)
              debug("Venting");
            #endif
            FIRING_MODE = VENTING;
          break;

          case A_SETTINGS_MODE:
            #if defined(__XTENSA__)
              debug("Settings");
            #endif
            FIRING_MODE = SETTINGS;
          break;

          case A_POWER_LEVEL_1:
            #if defined(__XTENSA__)
              debug("Power Level 1");
            #endif
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_1;
          break;

          case A_POWER_LEVEL_2:
            #if defined(__XTENSA__)
              debug("Power Level 2");
            #endif
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_2;
          break;

          case A_POWER_LEVEL_3:
            #if defined(__XTENSA__)
              debug("Power Level 3");
            #endif
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_3;
          break;

          case A_POWER_LEVEL_4:
            #if defined(__XTENSA__)
              debug("Power Level 4");
            #endif
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_4;
          break;

          case A_POWER_LEVEL_5:
            #if defined(__XTENSA__)
              debug("Power Level 5");
            #endif
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_5;
          break;

          case A_ALARM_ON:
            #if defined(__XTENSA__)
              debug("Alarm On");
            #endif

            // Alarm is on.
            b_pack_alarm = true;

            bargraphFull();
            BARGRAPH_PATTERN = BG_RAMP_DOWN;

            if(b_pack_on) {
              ms_blink_leds.start(i_blink_leds);
            }
          break;

          case A_ALARM_OFF:
            #if defined(__XTENSA__)
              debug("Alarm Off");
            #endif

            // Alarm is off.
            b_pack_alarm = false;

            if(b_pack_on) {
              ms_blink_leds.stop();

              bargraphClear();
              BARGRAPH_PATTERN = BG_POWER_RAMP;

              vibrateOff(); // Stop vibration.
            }
          break;

          case A_OVERHEATING:
            #if defined(__XTENSA__)
              debug("Overheating");
            #endif

            // Pack is overheating.
            b_overheating = true;
            ms_blink_leds.start(i_blink_leds);

            bargraphFull();
            BARGRAPH_PATTERN = BG_RAMP_DOWN;
          break;

          case A_OVERHEATING_FINISHED:
            #if defined(__XTENSA__)
              debug("Vented");
            #endif

            // Venting process completed.
            b_overheating = false;
            ms_blink_leds.stop();

            i_speed_multiplier = 1; // Return to normal speed.

            bargraphClear();
            BARGRAPH_PATTERN = BG_POWER_RAMP;

            vibrateOff(); // Stop vibration.
          break;

          case A_FIRING:
            #if defined(__XTENSA__)
              debug("Firing");
            #endif

            b_firing = true; // Implies the wand is powered on.
            b_pack_on = true; // Implies the pack is powered on.
            b_wand_on = true; // Implies the wand is powered on.
            ms_blink_leds.start(i_blink_leds / i_speed_multiplier);

            bargraphClear();
            BARGRAPH_PATTERN = BG_OUTER_INNER;
          break;

          case A_FIRING_STOPPED:
            #if defined(__XTENSA__)
              debug("Idle");
            #endif

            b_firing = false;
            ms_blink_leds.stop();

            if(!b_overheating) {
              i_speed_multiplier = 1; // Return to normal speed.
            }

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
            #if defined(__XTENSA__)
              debug("Cyclotron Speed Increasing...");
            #endif

            i_speed_multiplier++;

            #if defined(__XTENSA__)
              debug(String(i_speed_multiplier));
            #endif
          break;

          case A_BARREL_EXTENDED:
            if(BARREL_STATE != BARREL_EXTENDED) {
              #if defined(__XTENSA__)
                debug("Wand Barrel Extended");
              #endif

              BARREL_STATE = BARREL_EXTENDED;
            }
          break;

          case A_BARREL_RETRACTED:
            if(BARREL_STATE != BARREL_RETRACTED) {
              #if defined(__XTENSA__)
                debug("Wand Barrel Retracted");
              #endif

              BARREL_STATE = BARREL_RETRACTED;
            }
          break;

          case A_CYCLOTRON_NORMAL_SPEED:
            #if defined(__XTENSA__)
              debug("Cyclotron Speed Reset");
            #endif

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

          default:
            // Nothing.
          break;
        }

        #if defined(__XTENSA__)
          // ESP - Alert all WebSocket clients after an API call was received.
          // Note: We only perform this action if we have data from the pack.
          // This excludes the handshake as this is received way too often.
          if(!b_wait_for_pack && comStruct.i != A_HANDSHAKE) {
            notifyWSClients(); // Send latest status to the WebSocket.
          }
        #endif
      }

      comStruct.i = 0;
      comStruct.s = 0;
    }
  }
}