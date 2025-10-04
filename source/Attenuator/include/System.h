/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#pragma once

// Writes a debug message to the serial console or sends to the WebSocket.
void sendDebug(const String message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET)
    ws.textAll(message); // Send a copy to the WebSocket.
  #endif
}

// Obtain a list of partitions for this device.
void printPartitions() {
  const esp_partition_t *partition;
  esp_partition_iterator_t iterator = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if(iterator == nullptr) {
    debugln(F("No partitions found."));
    return;
  }

  debugln(F("Partitions:"));
  while(iterator != nullptr) {
    partition = esp_partition_get(iterator);
    debugf("Label: %s, Size: %lu bytes, Address: 0x%08lx\n",
           partition->label,
           partition->size,
           partition->address);
    iterator = esp_partition_next(iterator);
  }

  esp_partition_iterator_release(iterator);  // Release the iterator once done
}

/*
 * Prevent stream mode change if wand is firing or in an error state.
 */
bool canChangeStreamMode() {
  if(b_wand_firing || b_overheating || b_pack_alarm || b_pack_shutting_down) {
    return false;
  }
  return true;
}

/*
 * Change the current stream mode to a new mode, if allowed.
 */
void changeStreamMode(STREAM_MODES new_mode) {
  if(!canChangeStreamMode()) {
    debugln("Stream mode change not allowed while pack is firing or in error state.");
    return;
  }

  // Continue to change the stream mode.
  // @TODO: Add checks for the spectral modes being available.
  switch(new_mode) {
    case PROTON:
      attenuatorSerialSend(A_PROTON_MODE);
    break;
    case STASIS:
      attenuatorSerialSend(A_STASIS_MODE);
    break;
    case SLIME:
      attenuatorSerialSend(A_SLIME_MODE);
    break;
    case MESON:
      attenuatorSerialSend(A_MESON_MODE);
    break;
    case SPECTRAL:
      attenuatorSerialSend(A_SPECTRAL_MODE);
    break;
    case HOLIDAY_HALLOWEEN:
      attenuatorSerialSend(A_HALLOWEEN_MODE);
    break;
    case HOLIDAY_CHRISTMAS:
      attenuatorSerialSend(A_CHRISTMAS_MODE);
    break;
    case SPECTRAL_CUSTOM:
      attenuatorSerialSend(A_SPECTRAL_CUSTOM_MODE);
    break;
    default:
      debugln("Invalid Stream Mode");
    break;
  }
}

/*
 * Turns off the front-facing device LEDs (except top LED).
 */
void deviceLightsOff() {
  // Turn off the LEDs by setting to black.
  if(device_leds[i_device_led[1]] != CRGB::Black) {
    device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_BLACK);
  }
  if(device_leds[i_device_led[2]] != CRGB::Black) {
    device_leds[i_device_led[2]] = getHueAsRGB(i_device_led[2], C_BLACK);
  }
}

/*
 * Runs the buzzer for a set period of time using a provided frequency.
 */
void buzzOn(uint16_t i_freq) {
  if(b_enable_buzzer) {
    if(!b_buzzer_on) {
      // Ensures only a single tone is emitted per call to this method.
      tone(BUZZER_PIN, i_freq);
      ms_buzzer.start(i_buzzer_max_time);
      b_buzzer_on = true;
    }
  }
}

/*
 * Stops the buzzer if it is currently active.
 */
void buzzOff() {
  if(b_buzzer_on) {
    noTone(BUZZER_PIN);
    ms_buzzer.stop();
    b_buzzer_on = false;
  }
}

/*
 * Runs the vibration motor for a set period of time (ms).
 */
void useVibration(uint16_t i_duration) {
  if(b_enable_vibration) {
    if(!b_vibrate_on) {
      // Ensures only vibration is started once per call to this method.
      analogWrite(VIBRATION_PIN, i_max_power);

      // Set timer for shorter of given duration or max runtime.
      ms_vibrate.start(min(i_duration, i_vibrate_max_time));
    }
    b_vibrate_on = true;
  }
}

/*
 * Stops the vibration motor if it is currently active.
 */
void vibrateOff() {
  if(b_vibrate_on) {
    analogWrite(VIBRATION_PIN, i_min_power);
    ms_vibrate.stop();
    b_vibrate_on = false;
  }
}

/*
 * Determine the current state of any LEDs before next FastLED refresh.
 */
void updateLEDs() {
  // ESP - Change top LED colour based on wireless connections.
  if(i_ap_client_count > 0 || i_ws_client_count > 0) {
    // Change to green when clients are connected remotely.
    i_top_led_colour = C_GREEN;
  }
  else {
    // Return to red if no wireless clients are connected.
    i_top_led_colour = C_RED;
  }

  if(b_wait_for_pack) {
    // Keep LED as purple while still awaiting pack synchronization.
    i_top_led_colour = C_PURPLE;
  }

  // Update the top LED based on certain system statuses.
  switch(MENU_LEVEL) {
    case MENU_1:
      // Keep indicator solid.
      ms_top_blink.stop(); // Stop the blink timer which won't be used at this menu level.
      b_top_led_off = false; // Denotes LED is not in an off (blinking) state, but solid.
      device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], i_top_led_colour, i_top_led_brightness);
    break;

    case MENU_2:
      // Blink the LED when in this menu level.
      if(ms_top_blink.remaining() < 1) {
        ms_top_blink.start(i_top_blink_delay); // Restart the timer to change state.
        b_top_led_off = !b_top_led_off; // Whatever the last value, just flip it.
      }

      if(b_top_led_off) {
        // Not completely dark but very dim (1/10th of the normal brightness).
        device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], i_top_led_colour, int(i_top_led_brightness / 10));
      }
      else {
        // Return to normal brightness for the current top LED colour.
        device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], i_top_led_colour, i_top_led_brightness);
      }
    break;
  }

  // Set upper LED based on alarm or overheating state, when connected.
  // Otherwise, use the standard pattern/colour for illumination.
  if(b_pack_alarm || b_overheating) {
    device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_RED_FADE);
  }
  else {
    switch(RAD_LENS_IDLE) {
      case ORANGE_FADE:
        device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_ORANGE_FADE);
      break;
      case AMBER_PULSE:
      default:
        device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_AMBER_PULSE);
      break;
    }
  }

  // Set lower LED based on the current firing mode.
  uint8_t i_scheme;
  switch(STREAM_MODE) {
    case PROTON:
    default:
      i_scheme = C_RED;
    break;

    case SLIME:
      if(SYSTEM_YEAR == SYSTEM_1989) {
        i_scheme = C_PINK;
      }
      else {
        i_scheme = C_GREEN;
      }
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

    case HOLIDAY_HALLOWEEN:
      i_scheme = C_ORANGEPURPLE;
    break;

    case HOLIDAY_CHRISTMAS:
      i_scheme = C_REDGREEN;
    break;

    case SPECTRAL_CUSTOM:
      i_scheme = C_SPECTRAL_CUSTOM;
    break;

    case SETTINGS:
      i_scheme = C_WHITE;
    break;
  }

  // Update the lower LED based on the scheme determined above.
  if(b_blink_blank) {
    // Turn off when in mid-blink state.
    if(device_leds[i_device_led[2]] != CRGB::Black) {
      device_leds[i_device_led[2]] = getHueAsRGB(i_device_led[2], C_BLACK);
    }
  }
  else {
    device_leds[i_device_led[2]] = getHueAsRGB(i_device_led[2], i_scheme);
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
    b_center_pressed = true; // Denote the dial center button was pressed.

    // Track what state the right toggle was in at the start of the center press.
    b_right_toggle_center_start = b_right_toggle_on;
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
      if(b_right_toggle_center_start != b_right_toggle_on) {
        // A state change occurred for the right toggle, which we interpret as a lock-out toggle.
        b_center_lockout = !b_center_lockout;
        CENTER_STATE = NO_ACTION; // Don't count this as a long press.
        b_center_pressed = false;
        i_press_count = 0;
        useVibration(i_vibrate_max_time); // Give a long nudge.
        return; // We're done here as we've performed the state change.
      }
      else {
        // Consider a long-press event if the timer is run out before released.
        CENTER_STATE = LONG_PRESS;
        b_center_pressed = false;
        i_press_count = 0;
      }
    }
  }

  switch(CENTER_STATE) {
    case SHORT_PRESS:
      // Perform action for short press based on current menu level.
      switch(MENU_LEVEL) {
        case MENU_1:
          // A short, single press should start or stop the music.
          attenuatorSerialSend(A_MUSIC_START_STOP);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          debug("Rotary: Music Start/Stop");
        break;

        case MENU_2:
          // A short, single press should advance to the next track.
          attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          debug("Rotary: Next Track");
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
          debug("Rotary: Toggle Mute");
        break;

        case MENU_2:
          // A double press should move back to the previous track.
          attenuatorSerialSend(A_MUSIC_PREV_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          debug("Rotary: Previous Track");
        break;
      }
    break;

    case LONG_PRESS:
      // Toggle between the menu levels on a long press.
      // Also provides audio cues as to which menu is in use.
      switch(MENU_LEVEL) {
        case MENU_1:
          MENU_LEVEL = MENU_2; // Change menu level.
          debug("Rotary: Menu 2");
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          buzzOn(784); // Tone as note G4
        break;
        case MENU_2:
          MENU_LEVEL = MENU_1; // Change menu level.
          debug("Rotary: Menu 1");
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          buzzOn(440); // Tone as note A4
        break;
      }
    break;

    default:
      // eg. NO_ACTION - No-op
    break;
  }
}

// Add these variables (would go in your main file or header)
// volatile uint8_t encoder_state = 0;
// volatile int16_t encoder_count = 0;
// const int8_t encoder_table[] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};

/*
 * Improved encoder reading with state validation
 * Call this from interrupt or main loop
 */
void readEncoderImproved() {
  // Read current pin states
  uint8_t pinA = digitalRead(r_encoderA);
  uint8_t pinB = digitalRead(r_encoderB);
  
  // Update state machine
  encoder_state = ((encoder_state << 2) | (pinA << 1) | pinB) & 0x0F;
  
  // Use lookup table for direction (eliminates floating point math)
  int8_t direction = encoder_table[encoder_state];
  if (direction != 0) {
    encoder_count += direction;
  }
}

/*
 * Determines if encoder was turned CW or CCW with improved accuracy.
 */
void checkRotaryEncoderImproved() {
  static int16_t last_encoder_count = 0;
  static uint32_t last_action_time = 0;
  const uint16_t min_action_interval = 50; // Minimum ms between actions
  
  // Calculate steps moved (typically 4 counts per detent)
  int16_t steps = (encoder_count - last_encoder_count) / 4;
  
  if (steps != 0 && (millis() - last_action_time) > min_action_interval) {
    // Determine direction
    if (steps > 0) {
      DIAL_ROTATION = CLOCKWISE;
    } else {
      DIAL_ROTATION = COUNTERCLOCKWISE;
    }
    
    // Perform actions (existing switch statement code)
    switch(DIAL_ROTATION) {
      case CLOCKWISE:
        if(!ms_rotary_debounce.isRunning()) {
          if(b_wand_firing && i_cyclotron_multiplier > 2) {
            // Do the actual attenuation for the Proton Pack!
            // Cancels an overheat warning when firing and cyclotron state is higher than 2.
            // Only do so after 5 turns of the dial (CW).
            i_rotary_count++;
            if(i_rotary_count % 5 == 0) {
              attenuatorSerialSend(A_WARNING_CANCELLED);
              debug("Rotary: Overheat Cancelled");
              i_rotary_count = 0;
            }
          }
          else if(!b_wand_firing) {
            // Perform action based on the current menu level.
            switch(MENU_LEVEL) {
              case MENU_1:
                // Tell pack to increase overall volume.
                attenuatorSerialSend(A_VOLUME_INCREASE);
                debug("Rotary: Master Volume+");
              break;

              case MENU_2:
                // Tell pack to increase effects volume.
                attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
                debug("Rotary: Effects Volume+");
              break;

              case MENU_STREAM:
                // Change to the previous stream mode.
                if(canChangeStreamMode()) {
                  switch(STREAM_MODE) {
                    case PROTON:
                      changeStreamMode(SPECTRAL_CUSTOM);
                    break;
                    case STASIS:
                      changeStreamMode(PROTON);
                    break;
                    case SLIME:
                      changeStreamMode(STASIS);
                    break;
                    case MESON:
                      changeStreamMode(SLIME);
                    break;
                    case SPECTRAL:
                      changeStreamMode(MESON);
                    break;
                    case HOLIDAY_HALLOWEEN:
                      changeStreamMode(SPECTRAL);
                    break;
                    case HOLIDAY_CHRISTMAS:
                      changeStreamMode(HOLIDAY_HALLOWEEN);
                    break;
                    case SPECTRAL_CUSTOM:
                      changeStreamMode(HOLIDAY_CHRISTMAS);
                    break;
                    default:
                      debugln("Invalid Stream Mode");
                    break;
                  }
                }
                debug("Rotary: Previous Stream Mode");
              break;
            }
          }

          ms_rotary_debounce.start(rotary_debounce_time);
        }
      break;

      case COUNTERCLOCKWISE:
        if(!ms_rotary_debounce.isRunning()) {
          if(b_wand_firing && i_cyclotron_multiplier > 2) {
            // Do the actual attenuation for the Proton Pack!
            // Cancels an overheat warning when firing and cyclotron state is higher than 2.
            // Only do so after 5 turns of the dial (CCW).
            i_rotary_count++;
            if(i_rotary_count % 5 == 0) {
              attenuatorSerialSend(A_WARNING_CANCELLED);
              debug("Rotary: Overheat Cancelled");
              i_rotary_count = 0;
            }
          }
          else if(!b_wand_firing) {
            // Perform action based on the current menu level.
            switch(MENU_LEVEL) {
              case MENU_1:
                // Tell pack to decrease overall volume.
                attenuatorSerialSend(A_VOLUME_DECREASE);
                debug("Rotary: Master Volume-");
              break;

              case MENU_2:
                // Tell pack to decrease effects volume.
                attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
                debug("Rotary: Effects Volume-");
              break;

              case MENU_STREAM:
                // Change to the next stream mode.
                if(canChangeStreamMode()) {
                  switch(STREAM_MODE) {
                    case PROTON:
                      changeStreamMode(STASIS);
                    break;
                    case STASIS:
                      changeStreamMode(SLIME);
                    break;
                    case SLIME:
                      changeStreamMode(MESON);
                    break;
                    case MESON:
                      changeStreamMode(SPECTRAL);
                    break;
                    case SPECTRAL:
                      changeStreamMode(HOLIDAY_HALLOWEEN);
                    break;
                    case HOLIDAY_HALLOWEEN:
                      changeStreamMode(HOLIDAY_CHRISTMAS);
                    break;
                    case HOLIDAY_CHRISTMAS:
                      changeStreamMode(SPECTRAL_CUSTOM);
                    break;
                    case SPECTRAL_CUSTOM:
                      changeStreamMode(PROTON);
                    break;
                    default:
                      debugln("Invalid Stream Mode");
                    break;
                  }
                }
                debug("Rotary: Next Stream Mode");
              break;
            }
          }

          ms_rotary_debounce.start(rotary_debounce_time);
        }
      break;

      NO_ACTION:
      default:
        // No action taken.
      break;
    }

    // Remember the last rotary value for comparison later.
    i_last_val_rotary = i_val_rotary;

    if(ms_rotary_debounce.justFinished()) {
      ms_rotary_debounce.stop();
    }
  }
}

/*
 * Enhanced encoder checking with direction confirmation
 */
void checkRotaryEncoderWithConfirmation() {
  static int16_t last_stable_pos = 0;
  static int16_t pending_pos = 0;
  static uint32_t last_change_time = 0;
  static uint8_t consecutive_readings = 0;
  const uint8_t required_readings = 3; // Require 3 consistent readings
  const uint16_t stability_time = 20; // ms
  
  // Get current position
  int16_t current_pos = i_encoder_pos / 2.5;
  
  if (current_pos != pending_pos) {
    // Position changed, reset confirmation counter
    pending_pos = current_pos;
    consecutive_readings = 1;
    last_change_time = millis();
  } else if (current_pos != last_stable_pos) {
    // Same as pending, increment confirmation
    if ((millis() - last_change_time) < stability_time) {
      consecutive_readings++;
    } else {
      // Too much time passed, reset
      consecutive_readings = 1;
      last_change_time = millis();
    }
  }
  
  // Only act if we have enough consecutive readings
  if (consecutive_readings >= required_readings && current_pos != last_stable_pos) {
    // Determine direction
    if (current_pos > last_stable_pos) {
      DIAL_ROTATION = CLOCKWISE;
    } else {
      DIAL_ROTATION = COUNTERCLOCKWISE;
    }
    
    // Perform actions only if not in debounce period
    if (!ms_rotary_debounce.isRunning()) {
      switch(DIAL_ROTATION) {
        case CLOCKWISE:
          if(b_wand_firing && i_cyclotron_multiplier > 2) {
            // Do the actual attenuation for the Proton Pack!
            // Cancels an overheat warning when firing and cyclotron state is higher than 2.
            // Only do so after 5 turns of the dial (CW).
            i_rotary_count++;
            if(i_rotary_count % 5 == 0) {
              attenuatorSerialSend(A_WARNING_CANCELLED);
              debug("Rotary: Overheat Cancelled");
              i_rotary_count = 0;
            }
          }
          else if(!b_wand_firing) {
            // Perform action based on the current menu level.
            switch(MENU_LEVEL) {
              case MENU_1:
                // Tell pack to increase overall volume.
                attenuatorSerialSend(A_VOLUME_INCREASE);
                debug("Rotary: Master Volume+");
              break;

              case MENU_2:
                // Tell pack to increase effects volume.
                attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
                debug("Rotary: Effects Volume+");
              break;

              case MENU_STREAM:
                // Change to the previous stream mode.
                if(canChangeStreamMode()) {
                  switch(STREAM_MODE) {
                    case PROTON:
                      changeStreamMode(SPECTRAL_CUSTOM);
                    break;
                    case STASIS:
                      changeStreamMode(PROTON);
                    break;
                    case SLIME:
                      changeStreamMode(STASIS);
                    break;
                    case MESON:
                      changeStreamMode(SLIME);
                    break;
                    case SPECTRAL:
                      changeStreamMode(MESON);
                    break;
                    case HOLIDAY_HALLOWEEN:
                      changeStreamMode(SPECTRAL);
                    break;
                    case HOLIDAY_CHRISTMAS:
                      changeStreamMode(HOLIDAY_HALLOWEEN);
                    break;
                    case SPECTRAL_CUSTOM:
                      changeStreamMode(HOLIDAY_CHRISTMAS);
                    break;
                    default:
                      debugln("Invalid Stream Mode");
                    break;
                  }
                }
                debug("Rotary: Previous Stream Mode");
              break;
            }
          }

          ms_rotary_debounce.start(rotary_debounce_time);
        }
      break;

      case COUNTERCLOCKWISE:
        if(!ms_rotary_debounce.isRunning()) {
          if(b_wand_firing && i_cyclotron_multiplier > 2) {
            // Do the actual attenuation for the Proton Pack!
            // Cancels an overheat warning when firing and cyclotron state is higher than 2.
            // Only do so after 5 turns of the dial (CCW).
            i_rotary_count++;
            if(i_rotary_count % 5 == 0) {
              attenuatorSerialSend(A_WARNING_CANCELLED);
              debug("Rotary: Overheat Cancelled");
              i_rotary_count = 0;
            }
          }
          else if(!b_wand_firing) {
            // Perform action based on the current menu level.
            switch(MENU_LEVEL) {
              case MENU_1:
                // Tell pack to decrease overall volume.
                attenuatorSerialSend(A_VOLUME_DECREASE);
                debug("Rotary: Master Volume-");
              break;

              case MENU_2:
                // Tell pack to decrease effects volume.
                attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
                debug("Rotary: Effects Volume-");
              break;

              case MENU_STREAM:
                // Change to the next stream mode.
                if(canChangeStreamMode()) {
                  switch(STREAM_MODE) {
                    case PROTON:
                      changeStreamMode(STASIS);
                    break;
                    case STASIS:
                      changeStreamMode(SLIME);
                    break;EVEL == MENU_STREAM) {
                    case SLIME:   MENU_LEVEL = MENU_1;
                      changeStreamMode(MESON);
                    break;changed menu state.
                    case MESON:
                      changeStreamMode(SPECTRAL);; // Tone as note G4
                    break; }
                    case SPECTRAL: }




































































































































































































}  }    vibrateOff();  if(ms_vibrate.justFinished() || ms_vibrate.remaining() < 1) {  // Turn off vibration if timer finished.  }    buzzOff();    // Return to dial menu level 1 if previously in stream select.    b_right_toggle_on = false;  else {  }    }      b_blink_blank = false;    else {    }      }        }          }            buzzOn(523); // Tone as note C4            useVibration(i_vibrate_min_time); // Provide physical feedback.          if(b_overheat_feedback) {          b_blink_blank = false;          // Denote that certain LEDs should be in the lit phase of blinking.        else {        }          buzzOff(); // Stop buzzer tone.          vibrateOff(); // Stop vibration.          b_blink_blank = true;          // Denote that certain LEDs should be in the dark phase of blinking.        if(ms_blink_leds.remaining() < (i_blink_time / 2)) {        // Adjust feedback over 1/2 of the blink time allotted.        }          BARGRAPH_PATTERN = BG_INNER_PULSE;          // warning while the wand is still firing.          // Switch to a modified bargraph pattern for the pre-overheat (venting)        if(b_wand_firing && i_cyclotron_multiplier >= 3 && !b_overheating) {      if(ms_blink_leds.isRunning()) {      }        ms_blink_leds.start(i_blink_time);      if(ms_blink_leds.justFinished()) {      uint16_t i_blink_time = int(i_blink_leds / i_cyclotron_multiplier);      // Sets a timer value proportional to the speed of the cyclotron.      // If in pre-overheat warning, overheat, or alarm modes...    else if((b_wand_firing && i_cyclotron_multiplier > 2) || b_overheating || b_pack_alarm) {    }      useVibration(i_vibrate_min_time); // Use short bursts as this may be called multiple times in a row.      // Give physical feedback through vibration while wand is firing, but not in an overheat/alarm state.    if(b_wand_firing && i_cyclotron_multiplier <= 2 && b_firing_feedback && !b_overheating && !b_pack_alarm) {    MENU_LEVEL = MENU_STREAM; // Set encoder menu to stream mode selection.    b_right_toggle_on = true;  if(switch_right.getState() == LOW) {   */   * When the switch is off the encoder will return to volume/track control.   *   * of the bargraph will change pattern.   * via vibration and buzzer will be provided as needed, and animation   * change colours based on user interactions. Additionally, feedback   * When paired with the gpstar Proton pack controller, the LEDs will   *   * The right toggle activates the stream-mode selection via the encoder.   *   * Right Toggle - Uses a pull-up resistor, so setting LOW indicates ON.  /*  }    }      }        bargraphOff(); // Clear all bargraph elements and turn off the device.      if(BARGRAPH_STATE != BG_OFF) {    if(switch_left.getState() == HIGH) {  else {  }    }      BARGRAPH_PATTERN = BG_POWER_RAMP; // Bargraph idling loop.      bargraphReset(); // Enable bargraph for use (resets variables and turns it on).    if(BARGRAPH_STATE == BG_OFF && !(b_overheating || b_pack_alarm)) {  if(b_pack_on) {  // This supports pack connection or standalone operation.  // Turn on the bargraph when certain conditions are met.  }    }      }        }          b_pack_on = false;          // Only force the pack bool to false if in standalone mode.        if(!b_comms_open && !b_wait_for_pack && !ms_packsync.isRunning()) {        attenuatorSerialSend(A_TURN_PACK_OFF);      if(b_pack_on) {      b_left_toggle_on = false;    else {    }      }        }          b_pack_on = true;          // Only force the pack bool to true if in standalone mode.        if(!b_comms_open && !b_wait_for_pack && !ms_packsync.isRunning()) {        attenuatorSerialSend(A_TURN_PACK_ON);      if(!b_pack_on) {      b_left_toggle_on = true;    if(switch_left.getState() == LOW) {  if(switch_left.isPressed() || switch_left.isReleased()) {  // Turns the pack on or off (when paired) via left toggle.   */   * on the bargraph which will display a static, pre-set pattern.   * When not paired with the GPStar Proton Pack controller, will turn   * Standalone:   *   * bargraph (whether stock 5-LED version or 28-segment by Frutto).   * enable and display an animation which matches the Neutrona Wand   * pack on or off. When the pack is on the bargraph will automatically   * When paired with the gpstar Proton Pack controller, will turn the   * Paired:   *   * Left Toggle - Uses a pull-up resistor, so setting LOW indicates ON.  /*  }    checkRotaryEncoder();    // Check for rotation only when center lockout is NOT active, or if in stream mode.  if(!b_center_lockout || MENU_LEVEL == MENU_STREAM) {  }    checkRotaryPress();    // Check for a press event (long/short) only if not in stream select mode.  if (MENU_LEVEL != MENU_STREAM) {  switchLoops();void checkUserInputs() { */ * Monitor for interactions by user input./*}  encoder_center.loop();  switch_right.loop();  switch_left.loop();void switchLoops() { */ * Required by the ezButton objects. * * Perform debounce and get current button/switch states./*}  }    }      ms_rotary_debounce.stop();    if(ms_rotary_debounce.justFinished()) {    i_last_val_rotary = i_val_rotary;    // Remember the last rotary value for comparison later.    }      break;        // No action taken.      default:      NO_ACTION:      break;        }          ms_rotary_debounce.start(rotary_debounce_time);          }            }              break;                debug("Rotary: Next Stream Mode");                }                  }                    break;                      debugln("Invalid Stream Mode");                    default:                    break;                      changeStreamMode(PROTON);                    case SPECTRAL_CUSTOM:                    break;                      changeStreamMode(SPECTRAL_CUSTOM);                    case HOLIDAY_CHRISTMAS:                    break;                      changeStreamMode(HOLIDAY_CHRISTMAS);                    case HOLIDAY_HALLOWEEN:                    break;                      changeStreamMode(HOLIDAY_HALLOWEEN);
  // Turn off buzzer if timer finished.
  if(ms_buzzer.justFinished() || ms_buzzer.remaining() < 1) {
    buzzOff();
  }

  // Turn off vibration if timer finished.
  if(ms_vibrate.justFinished() || ms_vibrate.remaining() < 1) {
    vibrateOff();
  }
}