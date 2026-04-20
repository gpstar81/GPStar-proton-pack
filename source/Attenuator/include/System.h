/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Prevent stream mode change if wand is firing, in an error state, or VG modes are disabled.
 */
bool canChangeStreamMode() {
  if(!b_pack_on) {
    sendDebug(F("canChangeStreamMode() -> BLOCKED: Pack is not on"));
    return false;
  }

  if(b_wand_firing) {
    sendDebug(F("canChangeStreamMode() -> BLOCKED: Wand is firing"));
    return false;
  }

  if(b_overheating) {
    sendDebug(F("canChangeStreamMode() -> BLOCKED: Pack is overheating"));
    return false;
  }

  if(b_pack_alarm) {
    sendDebug(F("canChangeStreamMode() -> BLOCKED: Pack alarm is active"));
    return false;
  }

  if(b_pack_shutting_down) {
    // Cannot change mode while pack is actively shutting down.
    sendDebug(F("canChangeStreamMode() -> BLOCKED: Pack is shutting down"));
    return false;
  }

  if(gpstarSystem.getSystemMode() == MODE_ORIGINAL) {
    // Original mode does not support VG stream modes.
    sendDebug(F("canChangeStreamMode() -> BLOCKED: System is in original mode"));
    return false;
  }

  if(!gpstarSystem.supportsAnyAlternateStreams()) {
    // At a minimum one or more VG modes must be enabled to allow changes beyond PROTON.
    sendDebug(F("canChangeStreamMode() -> BLOCKED: no VG or spectral modes are enabled"));
    return false;
  }

  return true;
}

/*
 * Helper function to send the appropriate serial command for a stream mode.
 * Offers a final check to ensure only a supported mode will be sent.
 */
void sendStreamModeCommand(STREAM_MODES new_mode) {
  debugf("sendStreamModeCommand() called with mode: %d\n", new_mode);

  if(gpstarSystem.supportsStreamMode(new_mode)) {
    attenuatorSerialSend(A_SET_STREAM_MODE, (uint8_t)new_mode);
  }
}

/*
 * Change the current stream mode to a specific mode, if allowed.
 */
bool changeStreamMode(STREAM_MODES new_mode) {
  if(!canChangeStreamMode()) {
    sendDebug(F("Stream mode change not allowed while pack is firing or in error state."));
    return false;
  }

  // Debounce rapid calls to avoid flooding the serial interface.
  if(ms_streamchange.remaining() > 0) {
    sendDebug(F("Stream mode change suppressed due to debounce timer."));
    return false;
  }

  debugf("changeStreamMode(STREAM_MODES) called with mode: %d\n", new_mode);
  debugf("Current stream mode: %d\n", gpstarSystem.getStreamMode());

  // Send the command for the requested mode.
  sendStreamModeCommand(new_mode);
  ms_streamchange.start(i_stream_change_delay); // Restart debounce timer.
  return true;
}

/*
 * Change the current stream mode by encoder direction (next/previous), if allowed.
 */
bool changeStreamMode(ENCODER_STATES direction) {
  if(!canChangeStreamMode()) {
    sendDebug(F("Stream mode change not allowed while pack is firing or in error state."));
    return false;
  }

  // Debounce rapid calls to avoid flooding the serial interface.
  if(ms_streamchange.remaining() > 0) {
    sendDebug(F("Stream mode change suppressed due to debounce timer."));
    return false;
  }

  debugf("changeStreamMode(ENCODER_STATES) called with direction: %d\n", direction);
  debugf("Current stream mode: %d\n", gpstarSystem.getStreamMode());

  STREAM_MODES new_mode = gpstarSystem.getStreamMode();
  switch(direction){
    case ENCODER_CCW:
      // Counter-clockwise for next mode, like the dial on the wand.
      new_mode = gpstarSystem.nextStreamMode();
      debugf("ENCODER_CCW: calculated next mode: %d\n", new_mode);
    break;
    case ENCODER_CW:
      // Clockwise rotation for previous mode, like the dial on the wand.
      new_mode = gpstarSystem.previousStreamMode();
      debugf("ENCODER_CW: calculated previous mode: %d\n", new_mode);
    break;
    default:
      sendDebug(F("Invalid encoder direction"));
      return false;
    break;
  }

  // Send the command for the calculated mode.
  sendStreamModeCommand(new_mode);
  ms_streamchange.start(i_stream_change_delay); // Restart debounce timer.
  return true;
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
      // Keep indicator solid when in this menu level (indicates default/stable).
      ms_top_blink.stop(); // Stop the blink timer which won't be used at this menu level.
      b_top_led_alt = false; // Denotes LED is not in an off (blinking) state, but solid.
      device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], i_top_led_colour, i_top_led_brightness, b_grb_leds);
    break;

    case MENU_2:
      // Blink the LED when in this menu level.
      if(ms_top_blink.remaining() < 1) {
        ms_top_blink.start(i_top_blink_delay); // Restart the timer to change state.
        b_top_led_alt = !b_top_led_alt; // Whatever the last value, just flip it.
      }

      if(b_top_led_alt) {
        // For an alternate state use yellow to indicate this menu mode.
        device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], C_YELLOW, i_top_led_brightness, b_grb_leds);
      }
      else {
        // Return to normal brightness for the current top LED colour.
        device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], i_top_led_colour, i_top_led_brightness, b_grb_leds);
      }
    break;

    case MENU_STREAM:
      // Blink the LED when in this menu level, but at 2x the rate of the MENU_2 option.
      if(ms_top_blink.remaining() < 1) {
        ms_top_blink.start(int(i_top_blink_delay / 2)); // Restart the timer to change state.
        b_top_led_alt = !b_top_led_alt; // Whatever the last value, just flip it.
      }

      if(b_top_led_alt) {
        // For an alternate state use white to indicate this menu mode.
        device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], C_WHITE, i_top_led_brightness, b_grb_leds);
      }
      else {
        // Return to the normal, current top LED colour.
        device_leds[i_device_led[0]] = getHueAsRGB(i_device_led[0], i_top_led_colour, i_top_led_brightness, b_grb_leds);
      }
    break;
  }

  // Set upper LED based on alarm or overheating state, when connected.
  // Otherwise, use the standard pattern/colour for illumination.
  if(b_pack_alarm || b_overheating) {
    device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_RED_FADE, 255, b_grb_leds);
  }
  else {
    switch(RAD_LENS_IDLE) {
      case ORANGE_FADE:
        device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_ORANGE_FADE, 255, b_grb_leds);
      break;
      case AMBER_PULSE:
      default:
        device_leds[i_device_led[1]] = getHueAsRGB(i_device_led[1], C_AMBER_PULSE, 255, b_grb_leds);
      break;
    }
  }

  // Set lower LED based on the current firing mode.
  uint8_t i_scheme;
  switch(gpstarSystem.getStreamMode()) {
    case PROTON:
      i_scheme = C_RED;
    break;

    case SLIME:
      if(gpstarSystem.getSystemTheme() == SYSTEM_1989) {
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

    default:
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
    device_leds[i_device_led[2]] = getHueAsRGB(i_device_led[2], i_scheme, 255, b_grb_leds);
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
          sendDebug(F("Rotary: Music Start/Stop"));
        break;

        case MENU_2:
          // A short, single press should advance to the next track.
          attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          sendDebug(F("Rotary: Next Track"));
        break;

        case MENU_STREAM:
          attenuatorSerialSend(A_MANUAL_QUICK_VENT);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          sendDebug(F("Rotary: Quick Vent"));
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
          sendDebug(F("Rotary: Toggle Mute"));
        break;

        case MENU_2:
          // A double press should move back to the previous track.
          attenuatorSerialSend(A_MUSIC_PREV_TRACK);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          sendDebug(F("Rotary: Previous Track"));
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
          sendDebug(F("Rotary: Menu 2"));
        break;

        case MENU_2:
          MENU_LEVEL = MENU_1; // Change menu level.
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          buzzOn(440); // Tone as note A4
          sendDebug(F("Rotary: Menu 1"));
        break;

        case MENU_STREAM:
          attenuatorSerialSend(A_MANUAL_OVERHEAT);
          useVibration(i_vibrate_min_time); // Give a quick nudge.
          sendDebug(F("Rotary: Forced Overheat"));
        break;
      }
    break;

    default:
      // eg. NO_ACTION - No-op
    break;
  }
}

/*
 * Rotary Dial Rotation
 *
 * Performs action based turning the dial.
 */
void checkRotaryEncoder() {
  encoder.check();

  // Consume the last event and handle as necessary. This uses a special method to read and
  // immediately clear the last encoder STATE value so it won't be lost by scheduling/order.
  switch(encoder.consumeState()) {
    case ENCODER_CW:
      if(b_wand_firing && b_wand_connected && i_cyclotron_multiplier > 2) {
        // Do the actual attenuation for the Proton Pack!
        // Cancels an overheat warning when firing and cyclotron state is higher than 2.
        // Only do so after 5 turns of the dial (CW).
        i_rotary_count++;
        if(i_rotary_count % 5 == 0) {
          attenuatorSerialSend(A_WARNING_CANCELLED);
          sendDebug(F("Rotary: Overheat Cancelled"));
          i_rotary_count = 0;
        }
      }
      else if(!b_wand_firing) {
        // Perform action based on the current menu level.
        switch(MENU_LEVEL) {
          case MENU_1:
            // Tell pack to increase overall volume.
            attenuatorSerialSend(A_VOLUME_INCREASE);
            sendDebug(F("Rotary: Master Volume+"));
          break;

          case MENU_2:
            // Tell pack to increase effects volume.
            attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
            sendDebug(F("Rotary: Effects Volume+"));
          break;

          case MENU_STREAM:
            // Change to the previous stream mode.
            if(canChangeStreamMode()) {
              changeStreamMode(ENCODER_CW);
              sendDebug(F("Rotary: Previous Stream Mode"));
            }
          break;
        }
      }
    break;

    case ENCODER_CCW:
      if(b_wand_firing && b_wand_connected && i_cyclotron_multiplier > 2) {
        // Do the actual attenuation for the Proton Pack!
        // Cancels an overheat warning when firing and cyclotron state is higher than 2.
        // Only do so after 5 turns of the dial (CCW).
        i_rotary_count++;
        if(i_rotary_count % 5 == 0) {
          attenuatorSerialSend(A_WARNING_CANCELLED);
          sendDebug(F("Rotary: Overheat Cancelled"));
          i_rotary_count = 0;
        }
      }
      else if(!b_wand_firing) {
        // Perform action based on the current menu level.
        switch(MENU_LEVEL) {
          case MENU_1:
            // Tell pack to decrease overall volume.
            attenuatorSerialSend(A_VOLUME_DECREASE);
            sendDebug(F("Rotary: Master Volume-"));
          break;

          case MENU_2:
            // Tell pack to decrease effects volume.
            attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
            sendDebug(F("Rotary: Effects Volume-"));
          break;

          case MENU_STREAM:
            // Change to the next stream mode.
            if(canChangeStreamMode()) {
              changeStreamMode(ENCODER_CCW);
              sendDebug(F("Rotary: Next Stream Mode"));
            }
          break;
        }
      }
    break;

    default:
      // No action taken.
    break;
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
 * Monitor for interactions by user input.
 */
void checkUserInputs() {
  switchLoops();
  checkRotaryPress();

  if(!b_center_lockout || MENU_LEVEL == MENU_STREAM) {
    // Check for rotation only when center lockout is NOT active, or if in stream mode.
    checkRotaryEncoder();
  }

  /*
   * Left (Outer) Toggle - Uses a pull-up resistor, so setting LOW indicates ON.
   *
   * Paired:
   * When paired with the gpstar Proton Pack controller, will turn the
   * pack on or off. When the pack is on the bargraph will automatically
   * enable and display an animation which matches the Neutrona Wand
   * bargraph (whether stock 5-LED version or GPStar bargraph).
   *
   * Standalone: No effect/function.
   */

  // Turns the pack on or off (when paired) via left toggle.
  if(switch_left.isPressed() || switch_left.isReleased()) {
    b_left_toggle_on = (b_left_toggle_inverted && switch_left.getState() == HIGH) || (!b_left_toggle_inverted && switch_left.getState() == LOW);

    if(b_left_toggle_on) {
      if(!b_pack_on) {
        attenuatorSerialSend(A_TURN_PACK_ON);

        if(!b_comms_open && !b_wait_for_pack && !ms_packsync.isRunning()) {
          // Only force the pack bool to true if in standalone mode.
          b_pack_on = true;
        }
      }
    }
    else {
      if(b_pack_on) {
        attenuatorSerialSend(A_TURN_PACK_OFF);

        if(!b_comms_open && !b_wait_for_pack && !ms_packsync.isRunning()) {
          // Only force the pack bool to false if in standalone mode.
          b_pack_on = false;
        }
      }
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
    if(!b_left_toggle_on) {
      if(BARGRAPH_STATE != BG_OFF) {
        bargraphOff(); // Clear all bargraph elements and turn off the device.
      }
    }
  }

  /*
   * Right (Inner) Toggle - Uses a pull-up resistor, so setting LOW indicates ON.
   *
   * The right toggle activates the stream-mode selection via the encoder.
   *
   * When paired with the gpstar Proton pack controller, the LEDs will
   * change colours based on user interactions. Additionally, feedback
   * via vibration and buzzer will be provided as needed, and animation
   * of the bargraph will change pattern.
   *
   * When the switch is off the encoder will return to volume/track control.
   */
  b_right_toggle_on = (b_right_toggle_inverted && switch_right.getState() == HIGH) || (!b_right_toggle_inverted && switch_right.getState() == LOW);

  if(b_right_toggle_on) {
    // Only enter stream select mode if wand is not firing or in an error state.
    if(canChangeStreamMode()) {
      // Enter the stream select mode for the top dial.
      if(MENU_LEVEL != MENU_STREAM) {
        MENU_LEVEL = MENU_STREAM;

        // Provide feedback that the system changed menu state.
        useVibration(i_vibrate_min_time); // Give a quick nudge.
        buzzOn(784); // Tone as note G4
      }
    }

    if(b_wand_firing && i_cyclotron_multiplier < 3 && b_firing_feedback && !(b_overheating || b_pack_alarm)) {
      // Give physical feedback through vibration while wand is firing, but not in an overheat/alarm state.
      useVibration(i_vibrate_min_time); // Use short bursts as this may be called multiple times in a row.
    }
    else if((b_wand_firing && i_cyclotron_multiplier > 2) || b_overheating || b_pack_alarm) {
      // If in pre-overheat warning, overheat, or alarm modes...

      // Sets a timer value proportional to the speed of the cyclotron.
      uint16_t i_blink_time = int(i_blink_leds / i_cyclotron_multiplier);

      if(ms_blink_leds.justFinished()) {
        ms_blink_leds.start(i_blink_time);
      }

      if(ms_blink_leds.isRunning()) {
        if(b_wand_firing && i_cyclotron_multiplier > 2 && !b_overheating) {
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
    // Return to dial menu level 1 if previously in stream select.
    if(MENU_LEVEL == MENU_STREAM) {
      MENU_LEVEL = MENU_1;

      // Provide feedback that the system changed menu state.
      useVibration(i_vibrate_min_time); // Give a quick nudge.
      buzzOn(440); // Tone as note A4
    }
  }

  // Turn off buzzer if timer finished.
  if(ms_buzzer.justFinished() || ms_buzzer.remaining() < 1) {
    buzzOff();
  }

  // Turn off vibration if timer finished.
  if(ms_vibrate.justFinished() || ms_vibrate.remaining() < 1) {
    vibrateOff();
  }
}
