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

#pragma once

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

void buzzOff() {
  if(b_buzzer_on) {
    noTone(BUZZER_PIN);
    ms_buzzer.stop();
    b_buzzer_on = false;
  }
}

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

  if(b_right_toggle_on) {
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
  }
  else {
    if(device_leds[i_device_led[1]] != CRGB::Black) {
      device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_BLACK);
    }
  }

  // Set lower LED based on the current firing mode.
  uint8_t i_scheme;
  switch(STREAM_MODE) {
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

    case HOLIDAY:
      if(b_christmas) {
        i_scheme = C_REDGREEN;
      }
      else {
        i_scheme = C_ORANGEPURPLE;
      }
    break;

    case SPECTRAL_CUSTOM:
      i_scheme = C_SPECTRAL_CUSTOM;
    break;

    case SETTINGS:
      i_scheme = C_WHITE;
    break;

    case PROTON:
    default:
      i_scheme = C_RED;
    break;
  }

  // Update the lower LED based on the scheme determined above.
  if(!b_right_toggle_on || b_blink_blank) {
    // Turn off when right toggle is off or when mid-blink.
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
    if(encoder_center.isReleased() && encoder_center.getCount() >= 1) {
      // If released and we already counted 1 press, this is a "double tap".
      CENTER_STATE = DOUBLE_PRESS;
      b_center_pressed = false;
      encoder_center.resetCount();
      ms_center_double_tap.stop();
    }
    else if(ms_center_double_tap.remaining() < 1 && encoder_center.getCount() == 1) {
      // If the double-tap counter ran out with only a single press, this was a "short" press.
      CENTER_STATE = SHORT_PRESS;
      b_center_pressed = false;
      encoder_center.resetCount();
      ms_center_double_tap.stop();
      ms_center_long_press.stop();
    }
    else if(ms_center_long_press.remaining() < 1) {
      if(b_right_toggle_center_start != b_right_toggle_on) {
        // A state change occurred for the right toggle, which we interpret as a lock-out toggle.
        b_center_lockout = !b_center_lockout;
        CENTER_STATE = NO_ACTION; // Don't count this as a long press.
        b_center_pressed = false;
        encoder_center.resetCount();
        useVibration(i_vibrate_max_time); // Give a long nudge.
        return; // We're done here as we've performed the state change.
      }
      else {
        // Consider a long-press event if the timer is run out before released.
        CENTER_STATE = LONG_PRESS;
        b_center_pressed = false;
        encoder_center.resetCount();
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
        break;

        case MENU_2:
          // A short, single press should advance to the next track.
          attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
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
        break;

        case MENU_2:
          // A double press should move back to the previous track.
          attenuatorSerialSend(A_MUSIC_PREV_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
        break;
      }
    break;

    case LONG_PRESS:
      // Toggle between the menu levels on a long press.
      // Also provides audio cues as to which menu is in use.
      switch(MENU_LEVEL) {
        case MENU_1:
          MENU_LEVEL = MENU_2; // Change menu level.
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          buzzOn(784); // Tone as note G4
        break;
        case MENU_2:
          MENU_LEVEL = MENU_1; // Change menu level.
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

/*
 * Determines if encoder was turned CW or CCW.
 */
void readEncoder() {
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
      if(b_firing && i_speed_multiplier > 2) {
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
      if(b_firing && i_speed_multiplier > 2) {
        // Tell the pack to cancel the current overheat warning.
        // Only do so after 5 turns of the dial (CCW).
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

  // Remember the last rotary value for comparison later.
  i_last_val_rotary = i_val_rotary;

  if(ms_rotary_debounce.justFinished()) {
    ms_rotary_debounce.stop();
  }
}

/*
 * Perform debounce and get current button/switch states.
 *
 * Required by the ezButton objects.
 */
void switchLoops() {
  switch_left.loop();
  switch_right.loop();
  encoder_center.loop();
}

/*
 * Monitor for interactions by user or serial comms.
 */
void mainLoop() {
  checkPack();
  switchLoops();
  checkRotaryPress();
  if(!b_center_lockout) {
    checkRotaryEncoder();
  }

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
    if(switch_left.getState() == HIGH) {
      bargraphOff(); // Clear all bargraph elements and turn off the device.
    }
  }

  /*
   * Right Toggle - Uses a pull-up resistor, so setting LOW indicates ON.
   *
   * The right toggle activates the LEDs on the device manually.
   *
   * When paired with the gpstar Proton pack controller, the LEDs
   * will change colours based on user interactions.
   *
   * Note that audio and physical feedback will also be disabled
   * when this switch is in the off position.
   */
  if(switch_right.getState() == LOW) {
    b_right_toggle_on = true;

    if(b_firing && i_speed_multiplier <= 2 && b_firing_feedback && !b_overheating && !b_pack_alarm) {
      // Give physical feedback through vibration while wand is firing, but not in an overheat/alarm state.
      useVibration(i_vibrate_min_time); // Use short bursts as this may be called multiple times in a row.
    }
    else if((b_firing && i_speed_multiplier > 2) || b_overheating || b_pack_alarm) {
      // If in pre-overheat warning, overheat, or alarm modes...

      // Sets a timer value proportional to the speed of the cyclotron.
      uint16_t i_blink_time = int(i_blink_leds / i_speed_multiplier);

      if(ms_blink_leds.justFinished()) {
        ms_blink_leds.start(i_blink_time);
      }

      if(ms_blink_leds.isRunning()) {
        if(b_firing && i_speed_multiplier >= 3 && !b_overheating) {
          // Switch to a modified bargraph pattern for the pre-overheat (venting)
          // warning while the wand is still firing.
          BARGRAPH_PATTERN = BG_INNER_PULSE;
        }

        // Adjust feedback over 1/2 of the blink time allotted.
        if(ms_blink_leds.remaining() < (i_blink_time / 2)) {
          // Denote that certain LEDs should be in the dark phase of blinking.
          b_blink_blank = true;
          vibrateOff(); // Stop vibration.
          buzzOff(); // Stop buzzer tone.
        }
        else {
          // Denote that certain LEDs should be in the lit phase of blinking.
          b_blink_blank = false;
          if(b_overheat_feedback) {
            useVibration(i_vibrate_min_time); // Provide physical feedback.
            buzzOn(523); // Tone as note C4
          }
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
    if(device_leds[i_device_led[1]] != CRGB::Black) {
      device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_BLACK);
    }
    if(device_leds[i_device_led[2]] != CRGB::Black) {
      device_leds[i_device_led[2]] = getHueAsRGB(i_device_led[2], C_BLACK);
    }
  }

  // Update LEDs using appropriate colour scheme and environment vars.
  updateLEDs();

  // Turn off buzzer if timer finished.
  if(ms_buzzer.justFinished() || ms_buzzer.remaining() < 1) {
    buzzOff();
  }

  // Turn off vibration if timer finished.
  if(ms_vibrate.justFinished() || ms_vibrate.remaining() < 1) {
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

  if(ms_packsync.justFinished()) {
    // The pack just went missing, so treat as disconnected.
    b_wait_for_pack = true;
    ms_packsync.start(i_sync_initial_delay);
  }
}