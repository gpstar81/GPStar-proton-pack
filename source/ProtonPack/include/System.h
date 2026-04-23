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

#pragma once

#ifdef ESP32
  // Declare external reference to WirelessManager pointer (allocated in main.cpp after NVS init)
  extern WirelessManager* wirelessMgr;
#endif

// Forward function declarations.
void updateLEDs();
void executeCommand(uint8_t i_command, uint16_t i_value); // From Command.h

void sanitizeCyclotronMultipliers() {
  // Cyclotron multiplier: must be between 1 and 4
  if(i_cyclotron_multiplier < 1) i_cyclotron_multiplier = 1;
  if(i_cyclotron_multiplier > 4) i_cyclotron_multiplier = 4;

  // Cyclotron switch LED multiplier: must be between 1 and 4
  if(i_cyclotron_switch_led_multiplier < 1) i_cyclotron_switch_led_multiplier = 1;
  if(i_cyclotron_switch_led_multiplier > 4) i_cyclotron_switch_led_multiplier = 4;

  // Powercell multiplier: must be between 1 and 4
  if(i_powercell_multiplier < 1) i_powercell_multiplier = 1;
  if(i_powercell_multiplier > 4) i_powercell_multiplier = 4;
}

void innerCyclotronCakeOff() {
  for(uint8_t i = i_ic_cake_start; i <= i_ic_cake_end; i++) {
    cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
  }
}

void innerCyclotronCavityOff() {
  if(i_inner_cyclotron_cavity_num_leds > 0) {
    for(uint8_t i = i_ic_cavity_start; i <= i_ic_cavity_end; i++) {
      cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
    }
  }
}

void clearCyclotronFades() {
  for(uint8_t i = 0; i < OUTER_CYCLOTRON_LED_MAX; i++) {
    i_cyclotron_led_value[i] = 0;
    r_cyclotron_led_fade_out[i].go(0);
    r_cyclotron_led_fade_in[i].go(0);
    b_cyclotron_led_fading_in[i] = true;
  }
}

void resetRampSpeeds() {
  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_1984:
    case SYSTEM_1989:
      i_outer_current_ramp_speed = (uint16_t)(i_1984_delay * 1.3);
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;

    case SYSTEM_AFTERLIFE:
    default:
      i_outer_current_ramp_speed = i_2021_ramp_delay;
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;

    case SYSTEM_FROZEN_EMPIRE:
      i_outer_current_ramp_speed = (uint16_t)(i_2021_ramp_delay / 1.5);
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;
  }
}

void resetRampUp() {
  b_ramp_up = true;
  b_ramp_up_start = true;

  // Inner Cyclotron ring.
  b_inner_ramp_up = true;
}

void resetRampDown() {
  b_ramp_down = true;
  b_ramp_down_start = true;

  // Inner Cyclotron ring.
  b_inner_ramp_down = true;
}

bool isBrassPack() {
  return (gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE && !b_cyclotron_lid_on && (gpstarPack.inStreamMode(PROTON) || gpstarPack.inStreamMode(SPECTRAL_CUSTOM)));
}

void vibrationOff() {
  ms_menu_vibration.stop();
  i_vibration_level_prev = i_vibration_level_min;
  if(gpstarPack.getVibrationMode() == CYCLOTRON_MOTOR) {
    digitalWrite(VIBRATION_PIN, LOW);
  }
  else {
    analogWrite(VIBRATION_PIN, LOW);
  }
}

void ventLightLEDW(bool b_on) {
  if(b_on && ((b_wand_firing && b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) || b_overheating || b_pack_alarm)) {
    digitalWriteFast(NFILTER_LED_PIN, HIGH);
  }
  else {
    digitalWriteFast(NFILTER_LED_PIN, LOW);
  }
}

void ventLight(bool b_on) {
  uint8_t i_colour_scheme = getDeviceColour(VENT_LIGHT, gpstarPack.getStreamMode(), true);
  b_vent_light_on = b_on;

  if(b_on && b_cyclotron_lid_on) {
    // If doing firing smoke effects, let's change the light colours.
    if((b_wand_firing && b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) || b_overheating) {
      if(gpstarPack.inStreamMode(PROTON)) {
        // Override the N-Filter light colours for a proton stream.
        switch(gpstarPack.getPowerLevel()) {
          case LEVEL_1:
            i_colour_scheme = C_RED;
          break;

          case LEVEL_2:
            i_colour_scheme = C_PINK;
          break;

          case LEVEL_3:
            i_colour_scheme = C_YELLOW;
          break;

          case LEVEL_4:
            i_colour_scheme = C_LIGHT_BLUE;
          break;

          case LEVEL_5:
          default:
            i_colour_scheme = C_WHITE;
          break;
        }
      }
    }
    else if(b_wand_firing && !b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
      // If continuous fire smoke is disabled in the current power level, do not turn on the N-Filter LEDs.
      i_colour_scheme = C_BLACK;
    }
    else if(b_pack_alarm) {
      i_colour_scheme = C_RED;
    }

    for(uint8_t i = i_vent_light_start; i < i_pack_num_leds; i++) {
      pack_leds[i] = getHueAsRGB(VENT_LIGHT, i_colour_scheme); // Uses full brightness.
    }
  }
  else {
    for(uint8_t i = i_vent_light_start; i < i_pack_num_leds; i++) {
      pack_leds[i] = getHueAsRGB(VENT_LIGHT, C_BLACK);
    }
  }
}

// This function handles returning ring-simulated Cyclotron lookup table values.
uint8_t cyclotronLookupTable(uint8_t index) {
  switch(i_cyclotron_num_leds) {
    case QUAD_CYCLOTRON_LED_COUNT:
      // DIY 4 LED array.
      return PROGMEM_READU8(i_cyclotron_4led_matrix[index]);
    break;

    case HASLAB_CYCLOTRON_LED_COUNT:
      // Hasbro 12 LED array.
      return PROGMEM_READU8(i_cyclotron_12led_matrix[index]);
    break;

    case FRUTTO_CYCLOTRON_LED_COUNT:
      // Frutto 20 LED array.
      return PROGMEM_READU8(i_cyclotron_20led_matrix[index]);
    break;

    case MAX_CYCLOTRON_LED_COUNT:
    default:
      // Frutto/GPStar 36 LED array.
      return PROGMEM_READU8(i_cyclotron_36led_matrix[index]);
    break;

    case OUTER_CYCLOTRON_LED_MAX:
      // NeoPixel Ring 40 LED array.
      return PROGMEM_READU8(i_cyclotron_40led_matrix[index]);
    break;
  }
}

void wandStopFiringSounds() {
  // Stop all firing sounds.
  switch(gpstarPack.getStreamMode()) {
    case PROTON:
    default:
      switch(gpstarPack.getPowerLevel()) {
        case LEVEL_1 ... LEVEL_4:
          switch(gpstarPack.getSystemTheme()) {
            case SYSTEM_1984:
              stopEffect(S_GB1_1984_FIRE_START_SHORT);
              stopEffect(S_GB1_1984_FIRE_LOOP_PACK);
            break;
            case SYSTEM_1989:
              stopEffect(S_GB2_FIRE_START);
              stopEffect(S_GB2_FIRE_LOOP);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              stopEffect(S_GB1_FIRE_START);
              stopEffect(S_GB1_1984_FIRE_LOOP_PACK);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              stopEffect(S_FROZEN_EMPIRE_FIRE_START);
              stopEffect(S_GB1_1984_FIRE_LOOP_PACK);
            break;
          }
        break;
        case LEVEL_5:
        default:
          switch(gpstarPack.getSystemTheme()) {
            case SYSTEM_1984:
              stopEffect(S_GB1_1984_FIRE_START_HIGH_POWER);
              stopEffect(S_GB1_1984_FIRE_HIGH_POWER_LOOP);
            break;
            case SYSTEM_1989:
              stopEffect(S_GB1_FIRE_START_HIGH_POWER);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              stopEffect(S_AFTERLIFE_FIRE_START);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              stopEffect(S_FROZEN_EMPIRE_FIRE_START);
            break;
          }
        break;
      }

      if(b_stream_effects) {
        stopEffect(S_FIRE_LOOP_IMPACT);
      }

      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
    break;

    case MESON:
      // Nothing.
    break;
  }

  switch(STATUS_CTS) {
    case CTS_FIRING_1984:
      STATUS_CTS = CTS_NOT_FIRING;

      if(AUDIO_DEVICE == A_WAV_TRIGGER) {
        stopEffect(S_CROSS_STREAMS_START);
        stopEffect(S_CROSS_STREAMS_END);
      }

      playEffect(S_CROSS_STREAMS_END, false, i_volume_effects, false, 0, false);
    break;

    case CTS_FIRING_2021:
      STATUS_CTS = CTS_NOT_FIRING;

      stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
      stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

      playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects, false, 0, false);
    break;

    case CTS_NOT_FIRING:
      // Do nothing.
    break;
  }

  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;

  if(gpstarPack.inStreamMode(HOLIDAY_HALLOWEEN)) {
    stopEffect(S_HALLOWEEN_FIRING_EXTRA);
  }
  if(gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS)) {
    stopEffect(S_CHRISTMAS_FIRING_EXTRA);
  }
}

void wandExtraSoundsBeepLoopStop(bool stopNaturally) {
  if(stopNaturally) {
    // Set all beep looping to false so they stop naturally.
    stopEffectLoop(S_AFTERLIFE_BEEP_WAND_S1);
    stopEffectLoop(S_AFTERLIFE_BEEP_WAND_S2);
    stopEffectLoop(S_AFTERLIFE_BEEP_WAND_S3);
    stopEffectLoop(S_AFTERLIFE_BEEP_WAND_S4);
    stopEffectLoop(S_AFTERLIFE_BEEP_WAND_S5);
  }
  else {
    // Stop all beeps explicitly to prevent rapid switching from taking up all available channels.
    stopEffect(S_AFTERLIFE_BEEP_WAND_S1);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S2);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S3);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S4);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S5);
  }
}

void stopMashErrorSounds() {
  // Stop GB:FE button-smash sounds.
  stopEffect(S_FROZEN_EMPIRE_PACK_FREEZE_STOP);
  stopEffect(S_FROZEN_EMPIRE_BRASS_FIRE_TAIL);
  stopEffect(S_STASIS_IDLE_LOOP);
  // Stop normal button-smash sounds.
  stopEffect(S_MASH_ERROR_LOOP);
  stopEffect(S_SMASH_ERROR_RESTART);
}

void wandExtraSoundsStop() {
  stopEffect(S_AFTERLIFE_WAND_RAMP_1);
  stopEffect(S_AFTERLIFE_WAND_IDLE_1);
  stopEffect(S_AFTERLIFE_WAND_RAMP_2);
  stopEffect(S_AFTERLIFE_WAND_IDLE_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);

  stopEffect(S_WAND_BOOTUP);
  stopEffect(S_WAND_BOOTUP_SHORT);
}

void fadeoutIdleSounds() {
  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_1984:
      fadeoutEffect(S_GB1_1984_PACK_LOOP, 5000);
    break;
    case SYSTEM_1989:
      fadeoutEffect(S_GB2_PACK_LOOP, 5000);
    break;
    case SYSTEM_AFTERLIFE:
    default:
      fadeoutEffect(S_AFTERLIFE_PACK_IDLE_LOOP, 5000);
      fadeoutEffect(S_AFTERLIFE_WAND_IDLE_1, 5000);
      fadeoutEffect(S_AFTERLIFE_WAND_IDLE_2, 5000);
      fadeoutEffect(S_POWERCELL, 5000);
      wandExtraSoundsBeepLoopStop(true);
    break;
    case SYSTEM_FROZEN_EMPIRE:
      if(isBrassPack()) {
        if(b_brass_startup_loop) {
          fadeoutEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP, 5000);
        }
        else {
          fadeoutEffect(S_FROZEN_EMPIRE_BRASS_IDLE, 5000);
        }
      }

      fadeoutEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, 5000);
      fadeoutEffect(S_AFTERLIFE_WAND_IDLE_1, 5000);
      fadeoutEffect(S_AFTERLIFE_WAND_IDLE_2, 5000);
      fadeoutEffect(S_POWERCELL, 5000);
      wandExtraSoundsBeepLoopStop(true);
    break;
  }

  switch(gpstarPack.getStreamMode()) {
    case PROTON:
    case SPECTRAL:
    case SPECTRAL_CUSTOM:
    case HOLIDAY_CHRISTMAS:
    case HOLIDAY_HALLOWEEN:
    default:
      // Do nothing.
    break;
    case SLIME:
      fadeoutEffect(S_PACK_SLIME_TANK_LOOP, 5000);
    break;
    case STASIS:
      fadeoutEffect(S_STASIS_IDLE_LOOP, 5000);
    break;
    case MESON:
      fadeoutEffect(S_MESON_IDLE_LOOP, 5000);
    break;
  }
}

void packAlarm() {
  if(b_wand_firing) {
    // Preemptively stop firing sounds.
    wandStopFiringSounds();
    cyclotronSpeedRevert();
  }

  // Stop all normal pack sounds.
  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_1984:
      stopEffect(S_GB1_1984_PACK_LOOP);
      stopEffect(S_GB1_1984_BOOT_UP);
    break;
    case SYSTEM_1989:
      stopEffect(S_GB2_PACK_START);
      stopEffect(S_GB2_PACK_LOOP);
    break;
    case SYSTEM_AFTERLIFE:
    default:
      stopEffect(S_BOOTUP);
      stopEffect(S_AFTERLIFE_PACK_STARTUP);
      stopEffect(S_AFTERLIFE_PACK_IDLE_LOOP);
    break;
    case SYSTEM_FROZEN_EMPIRE:
      stopEffect(S_BOOTUP);
      stopEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP);

      if(b_brass_pack_sound_loop) {
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
        stopEffect(S_FROZEN_EMPIRE_BRASS_IDLE);
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP);
        b_brass_pack_sound_loop = false;
      }
    break;
  }

  playEffect(S_SHUTDOWN);

  if(gpstarPack.getSystemTheme() == SYSTEM_1989) {
    playEffect(S_GB2_PACK_OFF);
  }
  else {
    playEffect(S_PACK_SHUTDOWN);
  }

  switch(gpstarPack.getStreamMode()) {
    case SLIME:
      stopEffect(S_PACK_SLIME_TANK_LOOP);
    break;

    case STASIS:
      stopEffect(S_STASIS_IDLE_LOOP);
    break;

    case MESON:
      stopEffect(S_MESON_IDLE_LOOP);
    break;

    default:
      // Do nothing.
    break;
  }

  if(!b_overheating) {
    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        playEffect(S_PACK_RIBBON_ALARM_1, true);
        playEffect(S_ALARM_LOOP, true);
        playEffect(S_RIBBON_CABLE_START);
      break;
    }
  }

  // Turn off LEDs within the Cyclotron cavity if lid is not attached.
  if(!b_cyclotron_lid_on) {
    innerCyclotronCavityOff();
  }

  // Turn off the Cyclotron motor, if enabled.
  if(gpstarPack.getVibrationMode() == CYCLOTRON_MOTOR) {
    vibrationOff();
  }
}

// Returns whether we should be using the slime cyclotron effect or not.
// Pass in either the current or previous stream mode to check as needed.
bool usingSlimeCyclotron(STREAM_MODES mode) {
  if(b_cyclotron_lid_on) {
    if(mode == SLIME && b_cyclotron_colour_toggle) {
      if(i_cyclotron_num_leds == HASLAB_CYCLOTRON_LED_COUNT && !b_cyclotron_haslab_chsv_colour_change) {
        return false;
      }
      else {
        return true;
      }
    }
    else {
      return false;
    }
  }
  else {
    if(mode == SLIME) {
      return true;
    }
  }

  return false;
}

void innerCyclotronCavityUpdate(uint16_t iRampDelay) {
  // Map the value from the inner cake to the cavity lights to get current position.
  uint8_t i_midpoint = i_ic_cavity_start + (i_inner_cyclotron_cavity_num_leds / 2) - 1;
  uint8_t i_colour_scheme = C_BLACK; // Colour scheme for lighting, to be set later.
  uint8_t i_brightness = getBrightness(i_cyclotron_inner_brightness);

  // Cannot go lower than the starting point for this segment of LEDs.
  if(i_led_cyclotron_cavity < i_ic_cavity_start) {
    i_led_cyclotron_cavity = i_ic_cavity_start;
  }

  // Determine the colour for the LEDs when the cavity lights are enabled. This produces the "sparking"
  // effect as seen in GB:FE only for the Proton stream, but is also active for other select modes.
  // Currently this assumes a string of lights are wrapped around the cake from bottom to top.
  if(gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS) || isBrassPack()) {
    if(i_led_cyclotron_cavity < i_midpoint) {
      i_colour_scheme = C_YELLOW; // Always keep the lower half of LEDs yellow.
    }
    else {
      if(gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS)) {
        i_colour_scheme = C_WHITE; // Always keep the upper half of LEDs white.
      }
      else {
        // Light spiraling higher than the lower half will have variable colours.
        i_colour_scheme = getDeviceColour(CYCLOTRON_CAVITY, gpstarPack.getStreamMode(), false);
      }
    }
  }

  if(b_clockwise) {
    if(iRampDelay < 40 && !b_cyclotron_lid_on) {
      switch(CAVITY_LED_TYPE) {
        case RGB_LED:
        default:
          cyclotron_leds[i_led_cyclotron_cavity] = getHueAsRGB(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
        break;
        case GRB_LED:
          cyclotron_leds[i_led_cyclotron_cavity] = getHueAsGRB(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
        break;
        case GBR_LED:
          cyclotron_leds[i_led_cyclotron_cavity] = getHueAsGBR(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
        break;
      }

      // Set to black, which is universal for any type of LED.
      if(i_led_cyclotron_cavity == i_ic_cavity_start) {
        cyclotron_leds[i_ic_cavity_end] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
      else {
        cyclotron_leds[i_led_cyclotron_cavity - 1] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
    }

    i_led_cyclotron_cavity++;

    if(i_led_cyclotron_cavity > i_ic_cavity_end) {
      i_led_cyclotron_cavity = i_ic_cavity_start;
    }
  }
  else {
    if(iRampDelay < 40 && !b_cyclotron_lid_on) {
      switch(CAVITY_LED_TYPE) {
        case RGB_LED:
        default:
          cyclotron_leds[i_led_cyclotron_cavity] = getHueAsRGB(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
        break;
        case GRB_LED:
          cyclotron_leds[i_led_cyclotron_cavity] = getHueAsGRB(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
        break;
        case GBR_LED:
          cyclotron_leds[i_led_cyclotron_cavity] = getHueAsGBR(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
        break;
      }

      if(i_led_cyclotron_cavity + 1 > i_ic_cavity_end) {
        cyclotron_leds[i_ic_cavity_start] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
      else {
        cyclotron_leds[i_led_cyclotron_cavity + 1] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
    }

    i_led_cyclotron_cavity--;

    if(i_led_cyclotron_cavity < i_ic_cavity_start) {
      i_led_cyclotron_cavity = i_ic_cavity_end;
    }
  }
}

// For NeoPixel rings, ramp up and ramp down the LEDs in the ring and set the speed. (optional)
void innerCyclotronRingUpdate(uint16_t iRampDelay) {
  if(ms_cyclotron_ring.justFinished()) {
    if(b_inner_ramp_up) {
      if(r_inner_cyclotron_ramp.isFinished()) {
        b_inner_ramp_up = false;
        ms_cyclotron_ring.start(iRampDelay);

        i_inner_current_ramp_speed = iRampDelay;
      }
      else {
        ms_cyclotron_ring.start(r_inner_cyclotron_ramp.update());
        i_inner_current_ramp_speed = r_inner_cyclotron_ramp.update();
      }
    }
    else if(b_inner_ramp_down) {
      innerCyclotronCavityOff(); // Turn off (sparking) cavity lights.

      if(r_inner_cyclotron_ramp.isFinished()) {
        b_inner_ramp_down = false;

        if(ms_mash_lockout.isRunning() && !b_cyclotron_lid_on && !b_fading_out_frozen) {
          // If we managed to get here, we must be frozen. Set the flag.
          b_fading_out_frozen = true;
        }
      }
      else {
        ms_cyclotron_ring.start(r_inner_cyclotron_ramp.update());

        i_inner_current_ramp_speed = r_inner_cyclotron_ramp.update();
      }
    }
    else {
      i_inner_current_ramp_speed = iRampDelay;

      if(i_cyclotron_multiplier > 1) {
        if(i_cyclotron_multiplier > 4) {
          if(iRampDelay - 4 > 0) {
            iRampDelay = iRampDelay - 4;
          }
          else {
            iRampDelay = 0;
          }
        }
        else if(iRampDelay - i_cyclotron_multiplier > 0) {
          iRampDelay = iRampDelay - i_cyclotron_multiplier;
        }
        else {
          iRampDelay = 1;
        }
      }

      if(iRampDelay < 1 || iRampDelay > i_inner_current_ramp_speed) {
        iRampDelay = 1; // Ensure a sane minimum is set (must be non-zero).
      }

      ms_cyclotron_ring.start(iRampDelay);
    }

    // Colour control for the Inner Cyclotron LEDs.
    uint8_t i_brightness = getBrightness(i_cyclotron_inner_brightness);
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_INNER, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);

    if(isBrassPack() && gpstarPack.inStreamMode(PROTON)) {
      // As a "sparking" effect is predominant in GB:FE during the Proton stream,
      // the inner LED colour/brightness is altered for this mode.
      // We do not alter Spectral Custom as this is set by the user instead.
      i_brightness = getBrightness(i_cyclotron_inner_brightness / 2);
      i_colour_scheme = C_ORANGE;
    }

    if(b_clockwise) {
      if(!b_cyclotron_lid_on) {
        if(CAKE_LED_TYPE == GRB_LED) {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }

        if(i_led_cyclotron_ring == i_ic_cake_start) {
          cyclotron_leds[i_ic_cake_end] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring - 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
      }

      i_led_cyclotron_ring++;

      if(i_led_cyclotron_ring > i_ic_cake_end) {
        i_led_cyclotron_ring = i_ic_cake_start;
      }
    }
    else {
      if(!b_cyclotron_lid_on) {
        if(CAKE_LED_TYPE == GRB_LED) {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }

        if(i_led_cyclotron_ring + 1 > i_ic_cake_end) {
          cyclotron_leds[i_ic_cake_start] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring + 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
      }

      i_led_cyclotron_ring--;

      if(i_led_cyclotron_ring < i_ic_cake_start) {
        i_led_cyclotron_ring = i_ic_cake_end;
      }
    }

    // Inner Cyclotron Cavity light calculations are below.
    // Update the sparking effect only half as often as the cake is updated.
    if(i_inner_cyclotron_cavity_num_leds > 0 && (i_led_cyclotron_ring % 2) == 0 && !b_inner_ramp_down && !b_fading_out_frozen) {
      switch(i_cyclotron_multiplier) {
        case 9:
        case 8:
        case 7:
        case 6:
          // A value of 6 should be the max, but just in case this value goes higher let's catch those possible cases.
          if(iRampDelay - 4 > 2) {
            iRampDelay = iRampDelay - 4;
          }
          else {
            iRampDelay = 2;
          }
        break;

        case 5:
        case 4:
          if(iRampDelay - 3 > 2) {
            iRampDelay = iRampDelay - 3;
          }
          else {
            iRampDelay = 2;
          }
        break;

        case 3:
        case 2:
          if(iRampDelay - 2 > 2) {
            iRampDelay = iRampDelay - 2;
          }
          else {
            iRampDelay = 2;
          }
        break;

        case 1:
        default:
          // A value of 1 is considered the "normal" speed so treat it as the default.
          if(iRampDelay - 1 > 2) {
            iRampDelay = iRampDelay - 1;
          }
          else {
            iRampDelay = 2;
          }
        break;
      }

      if(iRampDelay < 2) {
        iRampDelay = 2;
      }

      // Update the inner cyclotron cavity LEDs for Frozen Empire w/ a Proton stream.
      // The delay value is just used to determine when to begin the sparking effect.
      innerCyclotronCavityUpdate(iRampDelay);
    }
  }
}

// Turns off the LEDs in the Cyclotron Lid only.
void cyclotronLidLedsOff() {
  if(!b_fade_out) {
    for(uint8_t i = 0; i < i_cyclotron_num_leds; i++) {
      pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
    }

    clearCyclotronFades();
  }
}

void resetCyclotronState() {
  // Turn off optional N-Filter LEDs.
  ventLight(false);
  ventLightLEDW(false);

  // Stop the slime Cyclotron effect timer.
  ms_cyclotron_slime_effect.stop();

  cyclotronLidLedsOff();

  // Only reset the start LED if the pack is off or just started.
  if(b_reset_start_led) {
    i_led_cyclotron = i_cyclotron_led_start;
    i_led_cyclotron_ring = i_ic_cake_start;
    i_cyclotron_fake_ring_counter = 0;
  }

  // Keep the fade control fading out a light that is not on during startup.
  if(PACK_STATE == MODE_OFF) {
    if(!b_1984_led_start) {
      b_1984_led_start = true;
    }
  }

  // Tell the Inner Cyclotron to turn off the LEDs.
  if(b_cyclotron_lid_on || !b_pack_alarm || PACK_STATE == MODE_OFF) {
    innerCyclotronCakeOff();
    innerCyclotronCavityOff();
  }

  cyclotronSpeedRevert();
}

// Stop cyclotron slime effects when exiting from Slime mode.
void stopCyclotronSlimeEffects(STREAM_MODES previousMode) {
  if(usingSlimeCyclotron(previousMode)) {
    resetCyclotronState();
    clearCyclotronFades();

    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_AFTERLIFE:
        adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
      break;
      case SYSTEM_FROZEN_EMPIRE:
        adjustGainEffect(S_FROZEN_EMPIRE_PACK_STARTUP, i_volume_effects, true, 100);
        adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
      break;
      default:
        // Do nothing otherwise.
      break;
    }
  }
}

void vibrationPack(uint8_t i_level) {
  if(gpstarPack.getVibrationMode() != VIBRATION_NEVER && gpstarPack.getVibrationMode() != CYCLOTRON_MOTOR && b_vibration_switch_on && i_level > 0) {
    if(gpstarPack.getVibrationMode() == VIBRATION_FIRING_ONLY) {
      if(b_wand_firing) {
        if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(VIBRATION_PIN, i_level);
        }
      }
      else {
        vibrationOff();
      }
    }
    else {
      if(i_level != i_vibration_level_prev) {
        i_vibration_level_prev = i_level;
        analogWrite(VIBRATION_PIN, i_level);
      }
    }
  }
  else if(gpstarPack.getVibrationMode() != CYCLOTRON_MOTOR) {
    vibrationOff();
  }
}

bool fadeOutCyclotron() {
  bool b_return = false;

  if((gpstarPack.isThemeModern()) && !usingSlimeCyclotron(gpstarPack.getStreamMode())) {
    for(uint8_t i = 0; i < OUTER_CYCLOTRON_LED_MAX; i++) {
      uint8_t i_curr_brightness = i_cyclotron_led_value[i] - 10;

      if(i_curr_brightness > i_cyclotron_led_value[i]) {
        i_curr_brightness = 0;
      }

      i_cyclotron_led_value[i] = i_curr_brightness;

      if(i_curr_brightness > 0) {

        b_return = true;

        if(cyclotronLookupTable(i) > 0) {
          pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1].maximizeBrightness(i_curr_brightness);
        }
      }
      else {
        if(cyclotronLookupTable(i) > 0) {
          pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
        }
      }
    }
  }

  return b_return;
}

bool ribbonCableAttached() {
  if(b_use_ribbon_cable) {
    if(switch_alarm.getState() == LOW) {
      // Ribbon cable is attached.
      return true;
    }
    else {
      // Ribbon cable is detached.
      return false;
    }
  }
  else {
    // If no ribbon cable switch is used, consult our runtime variable.
    if(b_manual_cable_alarm) {
      return false;
    }
    else {
      return true;
    }
  }
}

void playVentSounds() {
  playEffect(S_VENT_SMOKE);
  playEffect(S_SPARKS_LOOP);
}

void packStartup(bool fullStartup) {
  PACK_STATE = MODE_ON;
  PACK_ACTION_STATE = ACTION_IDLE;

  // Reset the vibration switch counter.
  vibrationSwitchedCount = 0;

  if(b_fadeout_idle_sounds) {
    // Start a timer that will fade out the idle SFX when complete.
    ms_delay_post.start(i_idle_fadeout_time);
  }
  else {
    // Stop the Brass Pack shutdown timer in case it's running.
    ms_delay_post.stop();
  }

  if(!fullStartup) {
    // Tell the wand the pack alarm is off.
    packSerialSend(P_ALARM_OFF, ribbonCableAttached() ? 1 : 0);

    // Tell any add-on devices that the alarm is off.
    attenuatorSerialSend(A_ALARM_OFF, ribbonCableAttached() ? 1 : 0);
  }

  // Start up the Cyclotron motor, if enabled.
  if(gpstarPack.getVibrationMode() == CYCLOTRON_MOTOR && b_vibration_switch_on) {
    digitalWrite(VIBRATION_PIN, HIGH);
  }

  stopEffect(S_PACK_RIBBON_ALARM_1);
  stopEffect(S_ALARM_LOOP);
  stopEffect(S_RIBBON_CABLE_START);
  stopEffect(S_PACK_SHUTDOWN); // This is a long track which may still be playing.
  stopEffect(S_PACK_SHUTDOWN_AFTERLIFE_ALT); // This is a long track which may still be playing.
  stopEffect(S_FROZEN_EMPIRE_BRASS_SHUTDOWN); // This is a long track which may still be playing.
  stopEffect(S_FROZEN_EMPIRE_PACK_SHUTDOWN); // This is a long track which may still be playing.

  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_1984:
      playEffect(S_GB1_1984_BOOT_UP);
      playEffect(S_GB1_1984_PACK_LOOP, true, i_volume_effects, true, 3800);
    break;

    case SYSTEM_1989:
      playEffect(S_GB2_PACK_START);
      playEffect(S_GB2_PACK_LOOP, true, i_volume_effects, true, 3800);
    break;

    case SYSTEM_AFTERLIFE:
    default:
      if(fullStartup) {
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_AFTERLIFE_PACK_STARTUP, false, i_volume_effects - i_slime_bootup_level);
          playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level, true, 18000);
        }
        else {
          playEffect(S_AFTERLIFE_PACK_STARTUP);
          playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects, true, 18000);
        }

        ms_idle_fire_fade.start(18000);
      }
      else {
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_BOOTUP, false, i_volume_effects - i_slime_bootup_level);
          playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level, true, 500);
        }
        else {
          playEffect(S_BOOTUP);
          playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects, true, 500);
        }

        ms_idle_fire_fade.start(0);
      }
    break;

    case SYSTEM_FROZEN_EMPIRE:
      if(isBrassPack()) {
        playEffect(S_BOOTUP);
        playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects, true, 500);
        if(b_brass_startup_loop) {
          playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP, true, i_volume_effects, true, 2000);
        }
        else {
          playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
          playEffect(S_FROZEN_EMPIRE_BRASS_IDLE, true, i_volume_effects, true, 2300);
        }

        b_brass_pack_sound_loop = true;
        ms_idle_fire_fade.start(0);
      }
      else {
        if(fullStartup) {
          if(gpstarPack.inStreamMode(SLIME)) {
            playEffect(S_FROZEN_EMPIRE_PACK_STARTUP, false, i_volume_effects - i_slime_bootup_level);
            playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level, true, 10000);
          }
          else {
            playEffect(S_FROZEN_EMPIRE_PACK_STARTUP);
            playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects, true, 10000);
          }

          ms_idle_fire_fade.start(10000);
        }
        else {
          if(gpstarPack.inStreamMode(SLIME)) {
            playEffect(S_BOOTUP, false, i_volume_effects - i_slime_bootup_level);
            playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level, true, 500);
          }
          else {
            playEffect(S_BOOTUP);
            playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects, true, 500);
          }

          ms_idle_fire_fade.start(0);
        }
      }
    break;
  }

  switch(gpstarPack.getStreamMode()) {
    case SLIME:
      playEffect(S_PACK_SLIME_TANK_LOOP, true, i_volume_effects, true, 900);
    break;

    case STASIS:
      playEffect(S_STASIS_IDLE_LOOP, true, i_volume_effects, true, 900);
    break;

    case MESON:
      playEffect(S_MESON_IDLE_LOOP, true, i_volume_effects, true, 900);
    break;

    default:
      // Do nothing.
    break;
  }

  ms_fadeout.stop();
  b_fade_out = false;

  // Tell the wand the pack is on.
  packSerialSend(P_ON);
  attenuatorSerialSend(A_PACK_ON);
}

// Smoke #2. Good for putting smoke in the Booster Tube.
void smokeBooster(bool b_smoke_on) {
  if(b_smoke_on) {
    if(b_smoke_enabled) {
      if(b_wand_firing && !b_overheating && b_smoke_booster_continuous_firing && b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(BOOSTER_TUBE_SMOKE_PIN, HIGH);
      }
      else if(b_overheating && b_smoke_booster_overheat && b_smoke_overheat_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(BOOSTER_TUBE_SMOKE_PIN, HIGH);
      }
      else if(b_venting && b_smoke_booster_overheat) {
        digitalWriteFast(BOOSTER_TUBE_SMOKE_PIN, HIGH);
      }
    }
    else {
      // If smoke is disabled globally, turn off.
      digitalWriteFast(BOOSTER_TUBE_SMOKE_PIN, LOW);
    }
  }
  else {
    // If we were told to turn off, turn off.
    digitalWriteFast(BOOSTER_TUBE_SMOKE_PIN, LOW);
  }
}

// Smoke #1. N-Filter cone outlet.
void smokeNFilter(bool b_smoke_on) {
  // Pass the value passed to us on to the Booster Tube smoke machine.
  smokeBooster(b_smoke_on);

  if(b_smoke_on) {
    if(b_smoke_enabled) {
      if(b_wand_firing && !b_overheating && b_smoke_nfilter_continuous_firing && b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(NFILTER_SMOKE_PIN, HIGH);
      }
      else if(b_overheating && b_smoke_nfilter_overheat && b_smoke_overheat_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(NFILTER_SMOKE_PIN, HIGH);
      }
      else if(b_venting && b_smoke_nfilter_overheat) {
        digitalWriteFast(NFILTER_SMOKE_PIN, HIGH);
      }
    }
    else {
      // If smoke is disabled globally, turn off.
      digitalWriteFast(NFILTER_SMOKE_PIN, LOW);
    }
  }
  else {
    // If we were told to turn off, turn off.
    digitalWriteFast(NFILTER_SMOKE_PIN, LOW);
  }
}

void fanBooster(bool b_fan_on) {
  if(b_fan_on) {
    if(b_smoke_enabled) {
      if(b_wand_firing && !b_overheating && b_fan_booster_continuous_firing && b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(BOOSTER_TUBE_FAN_PIN, HIGH);
      }
      else if(b_overheating && b_fan_booster_overheat && b_smoke_overheat_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(BOOSTER_TUBE_FAN_PIN, HIGH);
      }
      else if(b_venting && b_fan_booster_overheat) {
        digitalWriteFast(BOOSTER_TUBE_FAN_PIN, HIGH);
      }
    }
    else {
      // If smoke is disabled globally, turn off.
      digitalWriteFast(BOOSTER_TUBE_FAN_PIN, LOW);
    }
  }
  else {
    // If we were told to turn off, turn off.
    digitalWriteFast(BOOSTER_TUBE_FAN_PIN, LOW);
  }
}

// N-Filter Fan.
// Fan control. You can use this to switch on any device when properly hooked up with a transistor etc.
// A fan is a good idea for the N-Filter for example.
void fanNFilter(bool b_fan_on) {
  // Pass the value passed to us on to the Booster Tube fan.
  fanBooster(b_fan_on);

  if(b_fan_on) {
    if(b_smoke_enabled) {
      if(b_wand_firing && !b_overheating && b_fan_nfilter_continuous_firing && b_smoke_continuous_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(NFILTER_FAN_PIN, HIGH);
      }
      else if(b_overheating && b_fan_nfilter_overheat && b_smoke_overheat_level[(uint8_t)gpstarPack.getPowerLevel() - 1]) {
        digitalWriteFast(NFILTER_FAN_PIN, HIGH);
      }
      else if(b_venting && b_fan_nfilter_overheat) {
        digitalWriteFast(NFILTER_FAN_PIN, HIGH);
      }
    }
    else {
      // If smoke is disabled globally, turn off.
      digitalWriteFast(NFILTER_FAN_PIN, LOW);
    }
  }
  else {
    // If we were told to turn off, turn off.
    digitalWriteFast(NFILTER_FAN_PIN, LOW);
  }
}

void packShutdown() {
  PACK_STATE = MODE_OFF;
  PACK_ACTION_STATE = ACTION_IDLE;
  ms_delay_post.stop();

  if(b_wand_mash_lockout || ms_mash_lockout.isRunning()) {
    b_wand_mash_lockout = false;
    ms_mash_lockout.stop();
    if(gpstarPack.getStreamMode() != SLIME) {
      // If not in slime mode, stop the ice effect timer and melt the ice.
      innerCyclotronCakeOff();
      ms_cyclotron_slime_effect.stop();
    }
    ms_powercell.start(0);
    ms_cyclotron.start(0);
    ms_cyclotron_ring.start(0);
    stopMashErrorSounds();
    attenuatorSerialSend(A_CANCEL_LOCKOUT);
  }

  stopEffect(S_PACK_RECOVERY);
  stopEffect(S_PACK_RIBBON_ALARM_1);
  stopEffect(S_ALARM_LOOP);
  stopEffect(S_RIBBON_CABLE_START);

  if(b_wand_firing) {
    // Preemptively stop firing.
    wandStoppedFiring();
    cyclotronSpeedRevert();
  }
  else {
    // Turn off the vent lights if they were on.
    ventLight(false);
    ventLightLEDW(false);
    ms_vent_light_off.stop();
    ms_vent_light_on.stop();

    // Turn off any smoke.
    smokeNFilter(false);
    ms_smoke_timer.stop();
    ms_smoke_on.stop();

    // Turn off the fans.
    fanNFilter(false);

    // Reset vent sounds flag.
    b_vent_sounds_playing = false;
  }

  if(!b_wand_connected) {
    // If we lost connection to the wand, make sure these are stopped!
    wandExtraSoundsStop();
    wandExtraSoundsBeepLoopStop(false);
  }

  stopEffect(S_SHUTDOWN);
  stopEffect(S_STEAM_LOOP);

  if(gpstarPack.inStreamMode(SLIME)) {
    stopEffect(S_SLIME_REFILL);
    stopEffect(S_PACK_SLIME_TANK_LOOP);
  }

  if(gpstarPack.inStreamMode(STASIS)) {
    stopEffect(S_STASIS_IDLE_LOOP);
  }

  if(gpstarPack.inStreamMode(MESON)) {
    stopEffect(S_MESON_IDLE_LOOP);
  }

  if(b_powercell_sound_loop) {
    stopEffect(S_POWERCELL); // Just in case a shutdown happens and not a ramp down.
    b_powercell_sound_loop = false;
  }

  // Need to play the 'close' SFX if we already played the open one.
  if(b_overheating) {
    stopEffect(S_SLIME_EMPTY);
    stopEffect(S_VENT_OPEN);

    if(!gpstarPack.inStreamMode(SLIME)) {
      stopEffect(S_PACK_PRE_VENT);

      if(gpstarPack.isThemeModern()) {
        stopEffect(S_PACK_OVERHEAT_HOT);
      }

      playEffect(S_VENT_CLOSE);
      playEffect(S_STEAM_LOOP_FADE_OUT);
    }
  }
  else if(b_venting) {
    stopEffect(S_SLIME_EMPTY);
    stopEffect(S_QUICK_VENT_OPEN);
    playEffect(S_QUICK_VENT_CLOSE);
  }

  if(!b_pack_alarm) {
    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_1984:
        playEffect(S_PACK_SHUTDOWN);
      break;

      case SYSTEM_1989:
        playEffect(S_GB2_PACK_OFF);
      break;

      case SYSTEM_AFTERLIFE:
      default:
        playEffect(S_PACK_SHUTDOWN_AFTERLIFE_ALT);
      break;

      case SYSTEM_FROZEN_EMPIRE:
        if(b_brass_pack_sound_loop) {
          if(AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) {
            playTransitionEffect(S_FROZEN_EMPIRE_BRASS_SHUTDOWN, PROGMEM_READU16(sfx_smoke[random(5)]));
          }
          else {
            // Start a timer to play the steam effect.
            ms_delay_post.start(i_gbfe_brass_shutdown_delay);

            playEffect(S_FROZEN_EMPIRE_BRASS_SHUTDOWN);
          }
        }
        else {
          playEffect(S_FROZEN_EMPIRE_PACK_SHUTDOWN);
        }
      break;
    }
  }
  else {
    playEffect(S_SHUTDOWN);
  }

  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_1984:
      stopEffect(S_GB1_1984_BOOT_UP);
      fadeoutEffect(S_GB1_1984_PACK_LOOP);
    break;

    case SYSTEM_1989:
      stopEffect(S_GB2_PACK_START);
      fadeoutEffect(S_GB2_PACK_LOOP);
    break;

    case SYSTEM_AFTERLIFE:
    default:
      stopEffect(S_BOOTUP);
      stopEffect(S_AFTERLIFE_FIRE_END_LONG);
      stopEffect(S_AFTERLIFE_PACK_STARTUP);
      fadeoutEffect(S_AFTERLIFE_PACK_IDLE_LOOP);
    break;

    case SYSTEM_FROZEN_EMPIRE:
      stopEffect(S_BOOTUP);
      stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
      stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP);
      stopEffect(S_FROZEN_EMPIRE_PACK_STARTUP);
      fadeoutEffect(S_FROZEN_EMPIRE_BRASS_IDLE);
      fadeoutEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP);
    break;
  }

  b_ramp_up = false;
  b_ramp_up_start = false;
  b_inner_ramp_up = false;
  b_fade_out = true;

  resetRampDown();

  b_pack_shutting_down = true;
  ms_fadeout.start(0);

  // Tell the wand the pack is off, so shut down the wand if it happens to still be on.
  packSerialSend(P_OFF, b_pack_shutting_down ? 1 : 0);
  attenuatorSerialSend(A_PACK_OFF, b_pack_shutting_down ? 1 : 0);
}

void powercellOn() {
  i_powercell_led = i_powercell_num_leds - 1;

  powercellDraw();
}

void powercellOff() {
  for(uint8_t i = 0; i < i_powercell_num_leds; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, C_BLACK);
  }

  i_powercell_led = 0;
}

void innerCyclotronLEDPanelOff() {
  if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
    if(b_cyclotron_lid_on) {
      // All lights turn off while the cyclotron lid is on.
      for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end; i++) {
        cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
      }
    }
    else {
      // Otherwise the 2 switch panel lights remain on when lid is removed.
      for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
        cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
      }
    }
  }
}

void cyclotronSwitchLEDOff() {
#ifndef ESP32
  digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
  digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);

  digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
  digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);

  digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
  digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);

  if(b_cyclotron_lid_on) {
    digitalWriteFast(YEAR_TOGGLE_LED_PIN, LOW);
    digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, LOW);
  }
#endif
  i_cyclotron_sw_led = 0;

  innerCyclotronLEDPanelOff();
}

void packOffReset() {
  powercellOff();

  cyclotronSwitchLEDOff();

  ms_overheating_length.stop();
  b_overheating = false;
  b_venting = false;
  b_ramp_down = false;
  b_ramp_down_start = false;
  b_inner_ramp_down = false;
  b_reset_start_led = true; // Reset the start LED of the Cyclotron.

  resetCyclotronState();

  resetRampUp();

  // Update Power Cell LED timer delay and optional Cyclotron LED switch plate LED timer delays.
  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      i_powercell_delay = i_powercell_delay_2021;
      i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base;
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      i_powercell_delay = i_powercell_delay_1984;
      i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base * 4;
    break;
  }

  // Reset the Power Cell timer.
  ms_powercell.start(i_powercell_delay);

  // Reset the Cyclotron LED switch timer.
  ms_cyclotron_switch_led.start(i_cyclotron_switch_led_delay);

  // Need to reset the Cyclotron timers.
  ms_cyclotron.start(i_2021_delay);
  ms_cyclotron_ring.start(i_inner_ramp_delay);

  // Vibration/Cyclotron motor off.
  vibrationOff();
  i_vibration_level = i_vibration_level_min;

  if(b_pack_shutting_down) {
    b_pack_shutting_down = false;
    packSerialSend(P_OFF, b_pack_shutting_down ? 1 : 0);
    attenuatorSerialSend(A_PACK_OFF, b_pack_shutting_down ? 1 : 0);
    clearCyclotronFades();
  }

  // Tell the wand and any add-on devices that the alarm is off.
  if(b_pack_alarm) {
    b_pack_alarm = false;
    // Tell the wand that the alarm is off.
    packSerialSend(P_ALARM_OFF, ribbonCableAttached() ? 1 : 0);

    // Tell any add-on devices that the alarm is off.
    attenuatorSerialSend(A_ALARM_OFF, ribbonCableAttached() ? 1 : 0);
  }
}

void setYearModeByToggle() {
  // We have 4 year modes but only 2 toggle states, so these get grouped by their Haslab defaults.
  // Toggling the switch up/down will cycle through 1984 -> Afterlife -> 1989 -> Frozen Empire.
  if(switch_mode.getState() == LOW) {
    if(gpstarPack.isThemeModern()) {
      // When currently in Afterlife/Frozen Empire we switch to 1984 or 1989.
      if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
        gpstarPack.setSystemTheme(SYSTEM_1989);
        SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();

        // Tell the wand/attenuator to switch to 1989 mode.
        packSerialSend(P_YEAR_1989);
        attenuatorSerialSend(A_YEAR_1989);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_1989);
        }
      }
      else {
        gpstarPack.setSystemTheme(SYSTEM_1984);
        SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();

        // Tell the wand/attenuator to switch to 1984 mode.
        packSerialSend(P_YEAR_1984);
        attenuatorSerialSend(A_YEAR_1984);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_1984);
        }
      }
    }
  }
  else {
    if(gpstarPack.isTheme80s()) {
      // When currently in 1984/1989 we switch to Afterlife or Frozen Empire.
      if(gpstarPack.getSystemTheme() == SYSTEM_1984) {
        gpstarPack.setSystemTheme(SYSTEM_AFTERLIFE);
        SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();

        // Tell the wand/attenuator to switch to Afterlife mode.
        packSerialSend(P_YEAR_AFTERLIFE);
        attenuatorSerialSend(A_YEAR_AFTERLIFE);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_AFTERLIFE);
        }
      }
      else {
        gpstarPack.setSystemTheme(SYSTEM_FROZEN_EMPIRE);
        SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();

        // Tell the wand/attenuator to switch to Afterlife mode.
        packSerialSend(P_YEAR_FROZEN_EMPIRE);
        attenuatorSerialSend(A_YEAR_FROZEN_EMPIRE);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_FROZEN_EMPIRE);
        }
      }
    }
  }

  // Reset the pack variables to match the new year mode.
  resetRampSpeeds();
  packOffReset();
}

// LEDs for the 1984/2021 and vibration switches.
void cyclotronSwitchPlateLEDs() {
  bool b_brass_pack_effect_active = b_brass_pack_sound_loop || (isBrassPack() && (b_ramp_down || b_pack_alarm || b_wand_mash_lockout));

  if(!b_cyclotron_lid_on && !b_brass_pack_effect_active) {
    uint8_t i_brightness = getBrightness(i_cyclotron_panel_brightness);

    // Change colours for year theme switch indicator.
    if(gpstarPack.isTheme80s()) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
#ifndef ESP32
        digitalWriteFast(YEAR_TOGGLE_LED_PIN, HIGH);
#endif
        if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
          if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC) {
            // Static LED will always light green.
            cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
          }
          else {
            if(gpstarPack.getSystemTheme() == SYSTEM_1984) {
              // If in 1984, LED will light red.
              cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
            }
            else {
              // If in 1989, LED will light pink.
              cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_PINK, i_brightness);
            }
          }
        }
      }
      else {
#ifndef ESP32
        digitalWriteFast(YEAR_TOGGLE_LED_PIN, LOW);
#endif
        if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
          cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        }
      }
    }
    else {
#ifndef ESP32
      digitalWriteFast(YEAR_TOGGLE_LED_PIN, HIGH);
#endif
      if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
        if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC || gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
          // If using static LEDs or in Afterlife, LED will light green.
          cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
        }
        else {
          // Frozen Empire will light the LED ice blue.
          cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_LIGHT_BLUE, i_brightness);
        }
      }
    }

    // Change colours for vibration switch indicator.
    if(b_vibration_switch_on) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
#ifndef ESP32
        digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, HIGH);
#endif
        if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
          cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
        }
      }
      else {
#ifndef ESP32
        digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, LOW);
#endif
        if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
          cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        }
      }
    }
    else {
#ifndef ESP32
      digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, HIGH);
#endif
      if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
      }
    }
  }
  else {
    // Keep the Cyclotron switch LEDs off when the lid is on.
#ifndef ESP32
    digitalWriteFast(YEAR_TOGGLE_LED_PIN, LOW);
    digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, LOW);
#endif
    if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
      cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
      cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
    }
  }

  if(ms_cyclotron_switch_plate_leds.justFinished()) {
    ms_cyclotron_switch_plate_leds.repeat();
  }
}

void spectralLightsOff() {
  b_spectral_lights_on = false;

  for(uint8_t i = 0; i < i_max_pack_leds; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, C_BLACK);
  }

  for(uint8_t i = i_ic_cake_start; i <= i_ic_cake_end; i++) {
    cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
  }
}

void spectralLightsOn() {
  b_spectral_lights_on = true;

  uint8_t i_colour_scheme = getDeviceColour(POWERCELL, SPECTRAL_CUSTOM, true);
  for(uint8_t i = 0; i < i_powercell_num_leds; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, i_colour_scheme);
  }

  i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, SPECTRAL_CUSTOM, true);
  for(uint8_t i = 0; i < i_cyclotron_num_leds; i++) {
    pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme);
  }

  i_colour_scheme = getDeviceColour(CYCLOTRON_INNER, SPECTRAL_CUSTOM, true);
  for(uint8_t i = i_ic_cake_start; i <= i_ic_cake_end; i++) {
    if(CAKE_LED_TYPE == GRB_LED) {
      cyclotron_leds[i] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme);
    }
    else {
      cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme);
    }
  }

  attenuatorSerialSend(A_SPECTRAL_COLOUR_DATA);
}

void checkSwitches() {
  // Perform loop() needed by ezButton.
  switch_power.loop();
  switch_alarm.loop();
  switch_mode.loop();
  switch_vibration.loop();
  switch_cyclotron_lid.loop();
#ifndef ESP32
  switch_cyclotron_direction.loop();
  switch_smoke.loop();
#endif

  cyclotronSwitchPlateLEDs();

  if(b_disable_lid_detection) {
    // Force lid to always be "on" when this setting is enabled.
    if(!b_cyclotron_lid_on) {
      // The Cyclotron Lid is now on.
      b_cyclotron_lid_on = true;

      // Tell the connected devices.
      packSerialSend(P_CYCLOTRON_LID_ON);
      attenuatorSerialSend(A_CYCLOTRON_LID_ON);

      // Turn off Inner Cyclotron LEDs.
      innerCyclotronCakeOff();
      innerCyclotronCavityOff();
    }
  }
  else {
    // Normal lid switch handling when ignore setting is disabled.
    if(switch_cyclotron_lid.isReleased()) {
      // Play sounds when lid is removed.
      stopEffect(S_VENT_SMOKE);
      stopEffect(S_VENT_SMOKE_1);
      stopEffect(S_VENT_SMOKE_2);
      stopEffect(S_VENT_SMOKE_3);
      stopEffect(S_VENT_SMOKE_4);
      stopEffect(S_MODE_SWITCH);
      stopEffect(S_CLICK);
      stopEffect(S_SPARKS_LOOP);
      stopEffect(S_BEEPS_BARGRAPH);

      playEffect(S_MODE_SWITCH);

      // Play one of the random steam burst effects.
      playEffect(PROGMEM_READU16(sfx_smoke[random(5)]), false, i_volume_effects, true, 120);

      // Play some spark sounds if the pack is running and the lid is removed.
      if(PACK_STATE == MODE_ON) {
        playEffect(S_SPARKS_LOOP);
      }
      else {
        // Make sure we reset the cyclotron LED status if not in the EEPROM LED menu.
        if(!b_spectral_lights_on) {
          b_reset_start_led = false;
        }
      }
    }

    if(switch_cyclotron_lid.isPressed()) {
      // Play sounds when lid is mounted.
      stopEffect(S_CLICK);
      stopEffect(S_VENT_DRY);

      playEffect(S_CLICK);
      playEffect(S_VENT_DRY);

      // Play some spark sounds if the pack is running and the lid is put back on.
      if(PACK_STATE == MODE_ON) {
        playEffect(S_SPARKS_LOOP);
      }
      else {
        // Make sure we reset the cyclotron LED status if not in the EEPROM LED menu.
        if(!b_spectral_lights_on) {
          b_reset_start_led = false;
        }
      }
    }

    if(switch_cyclotron_lid.getState() == LOW) {
      if(!b_cyclotron_lid_on) {
        // The Cyclotron Lid is now on.
        b_cyclotron_lid_on = true;

        // Tell the connected devices.
        packSerialSend(P_CYCLOTRON_LID_ON);
        attenuatorSerialSend(A_CYCLOTRON_LID_ON);

        // Turn off Inner Cyclotron LEDs.
        innerCyclotronCakeOff();
        innerCyclotronCavityOff();
      }
    }
    else {
      if(b_cyclotron_lid_on) {
        // The Cyclotron Lid is now off.
        b_cyclotron_lid_on = false;

        // Make sure we clear the Outer Cyclotron LED states.
        cyclotronLidLedsOff();

        // Tell the connected devices.
        packSerialSend(P_CYCLOTRON_LID_OFF);
        attenuatorSerialSend(A_CYCLOTRON_LID_OFF);

        // Make sure the Inner Cyclotron turns on if we are in the EEPROM LED menu.
        if(b_spectral_lights_on) {
          spectralLightsOn();
        }
      }
    }
  }

#ifndef ESP32
  // Cyclotron direction toggle switch.
  if(switch_cyclotron_direction.isPressed() || switch_cyclotron_direction.isReleased()) {
    stopEffect(S_BEEPS);
    stopEffect(S_BEEPS_ALT);
    stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
    stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

    if(b_clockwise) {
      b_clockwise = false;

      playEffect(S_BEEPS_ALT);
      playEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

      // Tell wand to play Cyclotron counter clockwise voice.
      packSerialSend(P_CYCLOTRON_COUNTER_CLOCKWISE);
    }
    else {
      b_clockwise = true;

      playEffect(S_BEEPS);
      playEffect(S_VOICE_CYCLOTRON_CLOCKWISE);

      // Tell wand to play Cyclotron clockwise voice.
      packSerialSend(P_CYCLOTRON_CLOCKWISE);
    }
  }

  // Smoke
  if(switch_smoke.isPressed() || switch_smoke.isReleased()) {
    stopEffect(S_VOICE_SMOKE_DISABLED);
    stopEffect(S_VOICE_SMOKE_ENABLED);

    if(b_smoke_enabled) {
      b_smoke_enabled = false;
      smokeNFilter(false);

      stopEffect(S_VENT_DRY);

      playEffect(S_VENT_DRY);
      playEffect(S_VOICE_SMOKE_DISABLED);

      // Tell wand to play smoke disabled voice.
      packSerialSend(P_SMOKE_DISABLED);
    }
    else {
      b_smoke_enabled = true;

      stopEffect(S_VENT_SMOKE);

      playEffect(S_VENT_SMOKE);
      playEffect(S_VOICE_SMOKE_ENABLED);

      // Tell wand to play smoke enabled voice.
      packSerialSend(P_SMOKE_ENABLED);
    }
  }
#endif

  // Vibration toggle switch.
  if(switch_vibration.isPressed() || switch_vibration.isReleased()) {
    stopEffect(S_VOICE_VIBRATION_ENABLED);
    stopEffect(S_VOICE_VIBRATION_DISABLED);

    if(switch_vibration.getState() == LOW) {
      if(PACK_STATE == MODE_OFF && switch_alarm.getState() == HIGH) {
        vibrationSwitchedCount++;
      }

      if(!b_vibration_switch_on) {
        // Tell the wand to enable vibration.
        packSerialSend(P_VIBRATION_ENABLED);

        b_vibration_switch_on = true;

        playEffect(S_VOICE_VIBRATION_ENABLED);
      }
    }
    else {
      if(b_vibration_switch_on) {
        // Tell the wand to disable vibration.
        packSerialSend(P_VIBRATION_DISABLED);

        b_vibration_switch_on = false;

        playEffect(S_VOICE_VIBRATION_DISABLED);
      }
    }
  }

  if(switch_mode.isPressed() || switch_mode.isReleased()) {
    // Play a beep confirmation when the switch is flipped.
    stopEffect(S_BEEPS_BARGRAPH);
    playEffect(S_BEEPS_BARGRAPH);

    // Turn off the year mode override flag controlled by the Proton Pack.
    b_switch_mode_override = false;
  }

  if(switch_alarm.isPressed() || switch_alarm.isReleased()) {
    // Reset the vibration switch counter.
    vibrationSwitchedCount = 0;

    if(b_use_ribbon_cable) {
      // Play a sound when the ribbon cable is attached or detached.
      if(ribbonCableAttached()) {
        // Only play this sound if the pack is off to match Frozen Empire.
        if(PACK_STATE == MODE_OFF) {
          stopEffect(S_CLICK);
          playEffect(S_CLICK);
        }
      }
      else {
        stopEffect(S_RIBBON_CABLE_DETACH);
        playEffect(S_RIBBON_CABLE_DETACH);
      }
    }
  }

  if(switch_power.isPressed() || switch_power.isReleased()) {
    // Reset the vibration switch counter.
    vibrationSwitchedCount = 0;

    // When the ion arm switch is used to turn the Proton Pack on, play a extra sound effect in Afterlife or Frozen Empire.
    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        stopEffect(S_ION_ARM_SWITCH_ALT);
        playEffect(S_ION_ARM_SWITCH_ALT);
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
        if(switch_power.getState() == HIGH && PACK_STATE == MODE_ON) {
          // If shutting down from the ion arm switch in 84/89, play the extra shutdown sound.
          playEffect(S_SHUTDOWN);
        }
      break;
    }

    if(switch_power.getState() == LOW) {
      gpstarPack.setIonArmSwitch(RED_SWITCH_ON);

      // Turn the pack on if switch is moved to on position in Mode Super Hero.
      if(gpstarPack.getSystemMode() == MODE_SUPER_HERO && PACK_STATE == MODE_OFF) {
        PACK_ACTION_STATE = ACTION_ACTIVATE;
      }

      // Tell the Neutrona Wand that power to the Proton Pack is on.
      packSerialSend(P_ION_ARM_SWITCH_ON);

      // Tell the Attenuator or any other device that the power to the Proton Pack is on.
      attenuatorSerialSend(A_ION_ARM_SWITCH_ON);
    }
    else {
      gpstarPack.setIonArmSwitch(RED_SWITCH_OFF);

      if(PACK_STATE == MODE_ON) {
        PACK_ACTION_STATE = ACTION_OFF;
      }

      // Tell the Neutrona Wand that power to the Proton Pack is off.
      packSerialSend(P_ION_ARM_SWITCH_OFF);

      // Tell the Attenuator or any other device that the power to the Proton Pack is off.
      attenuatorSerialSend(A_ION_ARM_SWITCH_OFF);
    }
  }

  if(PACK_STATE == MODE_OFF) {
    // Year mode. Best to adjust it only when the pack is off.
    if(!b_pack_shutting_down && PACK_STATE != MODE_ON && !b_spectral_lights_on) {
      // If switching manually by the pack toggle switch.
      if(!b_switch_mode_override) {
        setYearModeByToggle();
      }
      else {
        // If the Neutrona Wand sub menu setting told the Proton Pack to change years.
        if(gpstarPack.getSystemTheme() != SYSTEM_THEME_TEMP) {
          switch(SYSTEM_THEME_TEMP) {
            case SYSTEM_1984:
              // Tell the wand to switch to 1984 mode.
              gpstarPack.setSystemTheme(SYSTEM_1984);

              packSerialSend(P_YEAR_1984);

              attenuatorSerialSend(A_YEAR_1984);
            break;

            case SYSTEM_1989:
              // Tell the wand to switch to 1989 mode.
              gpstarPack.setSystemTheme(SYSTEM_1989);

              packSerialSend(P_YEAR_1989);

              attenuatorSerialSend(A_YEAR_1989);
            break;

            case SYSTEM_FROZEN_EMPIRE:
              // Tell the wand to switch to Frozen Empire mode.
              gpstarPack.setSystemTheme(SYSTEM_FROZEN_EMPIRE);

              packSerialSend(P_YEAR_FROZEN_EMPIRE);

              attenuatorSerialSend(A_YEAR_FROZEN_EMPIRE);
            break;

            case SYSTEM_AFTERLIFE:
            default:
              // Tell the wand to switch to Afterlife mode.
              gpstarPack.setSystemTheme(SYSTEM_AFTERLIFE);
              SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();

              packSerialSend(P_YEAR_AFTERLIFE);

              attenuatorSerialSend(A_YEAR_AFTERLIFE);
            break;
          }

          resetRampSpeeds();
          packOffReset();
        }
      }
    }
  }
}

void cyclotronSwitchLEDUpdate() {
  // When lid is off, updates the switch panel lights using either the stock connectors for individual LEDs,
  // or via the addressable LEDs if the user has installed the custom PCB between the Pack Controller and Cake.
  if(!b_cyclotron_lid_on) {
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_PANEL, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);
    uint8_t i_brightness = getBrightness(i_cyclotron_panel_brightness);

    if(b_pack_alarm) {
      if(i_cyclotron_sw_led > 0) {
#ifndef ESP32
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
#endif
        if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
          if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC) {
            cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
            cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
            cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
            cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
            cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
            cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
          }
          else {
            // Uses all red for the alarm sequence.
            for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
              cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
            }
          }
        }
      }
      else {
#ifndef ESP32
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
#endif
        if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
          for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
            cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        }
      }
    }
    else {
      switch(i_cyclotron_sw_led) {
        case 0: // All Off
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            // All but the switch LEDs are turned off
            for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
              cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            }
          }
        break;

        case 1: // Add Green/Bottom
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC) {
              cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
              cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
            }
            else {
              cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
              cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            }
          }
        break;

        case 2: // Add Yellow/Middle
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC) {
              cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
              cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
            }
            else {
              cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
              cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            }
          }
        break;

        case 3: // Add Red/Top
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC) {
              cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
              cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
            }
            else {
              cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
              cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            }
          }
        break;

        case 4: // All Illuminated (Pause)
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            if(INNER_CYC_PANEL_MODE == PANEL_RGB_STATIC) {
              cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
              cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
              cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
              cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_brightness);
              cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
              cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_brightness);
            }
            else {
              for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
                cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
              }
            }
          }
        break;

        case 5: // Remove Green/Bottom
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        break;

        case 6: // Remove Yellow/Middle
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        break;

        case 7:// Remove Red/Top
#ifndef ESP32
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
          digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
#endif
          if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
            cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        break;
      }
    }
  }
}

void cyclotronSwitchLEDLoop() {
  if(ms_cyclotron_switch_led.justFinished()) {
    if(!b_cyclotron_lid_on) {
      // Frozen Empire brass pack sound is handled here.
      if(isBrassPack() && !b_pack_alarm && !b_overheating && !b_ramp_down && !b_wand_mash_lockout) {
        if(!b_brass_pack_sound_loop) {
          if(b_brass_startup_loop) {
            playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP, true, i_volume_effects, true, 2000);
          }
          else {
            playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
            playEffect(S_FROZEN_EMPIRE_BRASS_IDLE, true, i_volume_effects, true, 2300);
          }

          wandExtraSoundsBeepLoopStop(false);
          b_brass_pack_sound_loop = true;

          if(b_fadeout_idle_sounds) {
            ms_delay_post.start(i_idle_fadeout_time);
          }
        }
      }
      else if(b_brass_pack_sound_loop) {
        // If we were switched out of Proton or Spectral Custom, stop the brass pack sounds.
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
        stopEffect(S_FROZEN_EMPIRE_BRASS_IDLE);
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP);
        b_brass_pack_sound_loop = false;
      }

      if(b_brass_pack_sound_loop || (isBrassPack() && (b_ramp_down || b_pack_alarm || b_wand_mash_lockout))) {
        // Per user request, turn off the switch panel LEDs if brass pack is running.
        cyclotronSwitchLEDOff();
      }
      else {
        if(b_pack_alarm) {
          if(i_cyclotron_sw_led > 0) {
            i_cyclotron_sw_led = 0;
          }
          else {
            i_cyclotron_sw_led++;
          }
        }
        else {
          if(i_cyclotron_sw_led >= 7) {
            i_cyclotron_sw_led = 0;
          }
          else {
            i_cyclotron_sw_led++;
          }
        }

        // Update the LEDs.
        cyclotronSwitchLEDUpdate();
      }
    }
    else {
      // No need to have the Inner Cyclotron switch plate LEDs on when the lid is on.
      cyclotronSwitchLEDOff();

      // Stop the brass pack sound if it is playing.
      if(b_brass_pack_sound_loop) {
        if(b_fadeout_idle_sounds && !b_pack_alarm && !b_overheating && !b_ramp_down && !b_wand_mash_lockout) {
          // Restart the normal idle sounds if the lid is put back on.
          stopEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP);
          playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true);
          ms_delay_post.start(i_idle_fadeout_time);
        }

        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
        stopEffect(S_FROZEN_EMPIRE_BRASS_IDLE);
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP);
        b_brass_pack_sound_loop = false;
        updateEffectsVolume();
        packSerialSend(P_REQUEST_BEEP_SYNC);
      }
    }

    // Setup the delays again.
    uint16_t i_cyc_led_delay = i_cyclotron_switch_led_delay / i_cyclotron_switch_led_multiplier;

    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(ms_idle_fire_fade.remaining() > 0) {
          if(b_ramp_up) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + (i_2021_ramp_delay - r_outer_cyclotron_ramp.update());
          }
          else if(b_ramp_down) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + r_outer_cyclotron_ramp.update();
          }
        }
        else {
          if(b_ramp_up) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + ((i_2021_ramp_delay / 2) - r_outer_cyclotron_ramp.update());
          }
          else if(b_ramp_down) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + r_outer_cyclotron_ramp.update();
          }
        }
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_ramp_up) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + (r_outer_cyclotron_ramp.update() - i_1984_delay);
        }
        else if(b_ramp_down) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay / 6 + r_outer_cyclotron_ramp.update();
        }
      break;
    }

    if(b_pack_alarm) {
      i_cyc_led_delay = i_cyclotron_switch_led_delay * 2;
    }

    ms_cyclotron_switch_led.start(i_cyc_led_delay);
  }
}

void powercellRampDown() {
  if(ms_powercell.justFinished()) {
    if(i_powercell_led < 0) {
      // Do Nothing.
    }
    else {
      pack_leds[i_powercell_led] = getHueAsRGB(POWERCELL, C_BLACK);

      i_powercell_led--;
    }

    if(b_powercell_sound_loop) {
      stopEffectLoop(S_POWERCELL); // Turn off looping which stops the track.
      b_powercell_sound_loop = false;
    }

    // Setup the delays again.
    uint16_t i_pc_delay = i_powercell_delay;

    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_ramp_up || b_ramp_down) {
          i_pc_delay += (r_outer_cyclotron_ramp.update() - i_1984_delay);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_ramp_up || b_ramp_down) {
          i_pc_delay += r_outer_cyclotron_ramp.update();
        }
      break;
    }

    if(b_pack_alarm) {
      i_pc_delay *= 3;
    }

    ms_powercell.start(i_pc_delay);
  }
}

void powercellLoop() {
  if(ms_powercell.justFinished()) {
    uint16_t i_extra_delay = 0;

    // Power Cell
    if(i_powercell_led >= i_powercell_num_leds) {
      powercellOff();

      i_powercell_led = 0;
    }
    else {
      if(((gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE && !isBrassPack() && !b_wand_mash_lockout) || gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) && !b_ramp_up && !b_ramp_down && !b_wand_firing && !b_pack_alarm && !b_overheating) {
        if(!b_powercell_sound_loop && i_powercell_led == 0) {
          playEffect(S_POWERCELL, true, i_volume_effects - i_wand_idle_level, true, 1400);
          b_powercell_sound_loop = true;
        }
      }

      if(!b_powercell_updating) {
        powercellDraw(i_powercell_led); // Update starting at a specific LED.
        i_powercell_led++;

        // Add a small delay to pause the Power Cell when all Power Cell LEDs are lit up, to match Afterlife and Frozen Empire.
        if((gpstarPack.isThemeModern()) && !b_pack_alarm && i_powercell_led >= i_powercell_num_leds) {
          i_extra_delay = 333 - i_powercell_delay;
        }
      }
    }

    if((b_overheating || b_ramp_down || b_ramp_up || b_pack_alarm || (gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE && b_wand_mash_lockout) || isBrassPack()) && b_powercell_sound_loop) {
      stopEffectLoop(S_POWERCELL); // Turn off looping which stops the track.
      b_powercell_sound_loop = false;
    }

    // Setup the delays again.
    uint16_t i_pc_delay = i_powercell_delay;

    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_ramp_up || b_ramp_down) {
          i_pc_delay += (r_outer_cyclotron_ramp.update() - i_1984_delay);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_ramp_up || b_ramp_down) {
          i_pc_delay += r_outer_cyclotron_ramp.update();
        }
      break;
    }

    if(b_pack_alarm) {
      i_pc_delay *= 5;
    }

    // Speed up the Power Cell when the cyclotron speeds up before overheating.
    uint16_t i_multiplier = 0;

    if(i_powercell_multiplier > 1) {
      switch(i_powercell_multiplier) {
        default:
          // Do nothing.
        break;
        case 2:
          if(gpstarPack.isThemeModern()) {
            if(i_pc_delay + i_extra_delay > 5) {
              i_multiplier = 5;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 10) {
              i_multiplier = 10;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 3:
          if(gpstarPack.isThemeModern()) {
            if(i_pc_delay + i_extra_delay > 10) {
              i_multiplier = 10;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 20) {
              i_multiplier = 20;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 4:
          if(gpstarPack.isThemeModern()) {
            if(i_pc_delay + i_extra_delay > 15) {
              i_multiplier = 15;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 30) {
              i_multiplier = 30;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 5:
          if(gpstarPack.isThemeModern()) {
            if(i_pc_delay + i_extra_delay > 25) {
              i_multiplier = 25;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 40) {
              if(i_pc_delay + i_extra_delay > 40) {
                i_multiplier = 40;
              }
              else {
                i_multiplier = i_pc_delay + i_extra_delay;
              }
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 6:
          if(gpstarPack.isThemeModern()) {
            i_multiplier = 30;
          }
          else {
            if(i_pc_delay + i_extra_delay > 50) {
              i_multiplier = 50;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;
      }
    }

    ms_powercell.start((i_pc_delay + i_extra_delay) - i_multiplier);
  }
}

void powercellDraw(uint8_t i_start) {
  uint8_t i_brightness = getBrightness(i_powercell_brightness); // Calculate desired brightness.
  uint8_t i_colour_scheme = getDeviceColour(POWERCELL, gpstarPack.getStreamMode(), b_powercell_colour_toggle);

  // Sets the colour for each Power Cell LED, subject to colour toggle setting.
  for(uint8_t i = i_start; i <= i_powercell_led; i++) {
    if(i_powercell_led < i_powercell_num_leds) {
      uint8_t i_tmp_powercell_led = 0;

      if(b_powercell_invert) {
        if(i_powercell_num_leds == HASLAB_POWERCELL_LED_COUNT) {
          i_tmp_powercell_led = PROGMEM_READU8(powercell_13_invert[i]);
        }
        else {
          i_tmp_powercell_led = PROGMEM_READU8(powercell_15_invert[i]);

        }
      }
      else {
        if(i_powercell_num_leds == HASLAB_POWERCELL_LED_COUNT) {
          i_tmp_powercell_led = PROGMEM_READU8(powercell_13[i]);
        }
        else {
          i_tmp_powercell_led = PROGMEM_READU8(powercell_15[i]);
        }
      }

      // Note: Always assumed to be RGB for built-in.
      pack_leds[i_tmp_powercell_led] = getHueAsRGB(POWERCELL, i_colour_scheme, i_brightness);
    }
  }
}

// This function handles returning 1984 Cyclotron lookup table values.
uint8_t cyclotron84LookupTable(uint8_t index) {
  // First include a sanity check that will reject indexes above 3.
  if(index > 3) {
    index = 0;
  }

  if(b_clockwise) {
    switch(i_cyclotron_num_leds) {
      case HASLAB_CYCLOTRON_LED_COUNT:
        // 1984 CW 12 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_12_leds_cw[index]);
      break;

      case FRUTTO_CYCLOTRON_LED_COUNT:
        // 1984 CW 20 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_20_leds_cw[index]);
      break;

      case MAX_CYCLOTRON_LED_COUNT:
      default:
        // 1984 CW 36 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_36_leds_cw[index]);
      break;

      case OUTER_CYCLOTRON_LED_MAX:
        // 1984 CW 40 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_40_leds_cw[index]);
      break;
    }
  }
  else {
    switch(i_cyclotron_num_leds) {
      case HASLAB_CYCLOTRON_LED_COUNT:
        // 1984 CCW 12 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_12_leds_ccw[index]);
      break;

      case FRUTTO_CYCLOTRON_LED_COUNT:
        // 1984 CCW 20 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_20_leds_ccw[index]);
      break;

      case MAX_CYCLOTRON_LED_COUNT:
      default:
        // 1984 CCW 36 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_36_leds_ccw[index]);
      break;

      case OUTER_CYCLOTRON_LED_MAX:
        // 1984 CCW 40 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_40_leds_ccw[index]);
      break;
    }
  }
}

// Reset the Cyclotron LED colours.
void cyclotronColourReset() {
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs, returning full white.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_num_leds == HASLAB_CYCLOTRON_LED_COUNT && !b_cyclotron_haslab_chsv_colour_change) {
    i_colour_scheme = C_HASLAB;
  }

  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      for(uint8_t i = 0; i < OUTER_CYCLOTRON_LED_MAX; i++) {
        if(cyclotronLookupTable(i) > 0) {
          pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_cyclotron_led_value[i]);
        }
      }
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      for(uint8_t i = 0; i < i_cyclotron_num_leds; i++) {
        pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_cyclotron_led_value[i]);
      }
    break;
  }
}

// Controls the slime cyclotron fadeout effect.
void slimeCyclotronFadeout() {
  bool b_leds_fading = false;

  if(b_cyclotron_lid_on) {
    for(uint8_t i = 0; i < i_cyclotron_num_leds; i++) {
      pack_leds[i + i_cyclotron_led_start].fadeToBlackBy(1);

      if(!b_leds_fading && pack_leds[i + i_cyclotron_led_start]) {
        b_leds_fading = true;
      }
    }
  }
  else {
    for(uint8_t i = 0; i < i_inner_cyclotron_cake_num_leds; i++) {
      cyclotron_leds[i + i_ic_cake_start].fadeToBlackBy(1);

      if(!b_leds_fading && cyclotron_leds[i + i_ic_cake_start]) {
        b_leds_fading = true;
      }
    }
  }

  if(b_leds_fading) {
    // At least one LED not off yet.
    ms_cyclotron_slime_effect.start(30);
  }
  else {
    // All LEDs faded to black.
    ms_cyclotron_slime_effect.stop();
    b_ramp_down = false;
  }
}

// Controls the slime cyclotron effect.
void slimeCyclotronEffect() {
  if(ms_cyclotron_slime_effect.justFinished()) {
    if(PACK_STATE == MODE_OFF && b_ramp_down) {
      slimeCyclotronFadeout();
      return;
    }

    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);
    uint8_t i_random_lower = 50;
    uint8_t i_random_upper = 121;

    if(b_wand_firing) {
      i_random_lower = 40;

      switch(gpstarPack.getPowerLevel()) {
        case LEVEL_1:
          i_random_upper = 121;
        break;

        case LEVEL_2:
          i_random_upper = 151;
        break;

        case LEVEL_3:
          i_random_upper = 181;
        break;

        case LEVEL_4:
          i_random_upper = 201;
        break;

        case LEVEL_5:
        default:
          i_random_upper = 221;
        break;
      }
    }
    else if(b_pack_alarm || b_overheating || b_venting) {
      i_random_lower = 20;
      i_random_upper = 41;
    }

    if(b_cyclotron_lid_on) {
      for(uint8_t i = 0; i < i_cyclotron_num_leds; i++) {
        pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, random(i_random_lower, i_random_upper));
      }
    }
    else {
      for(uint8_t i = 0; i < i_inner_cyclotron_cake_num_leds; i++) {
        cyclotron_leds[i + i_ic_cake_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, random(i_random_lower, i_random_upper));
      }
    }

    if(i_random_lower == 50 && i_random_upper == 121) {
      ms_cyclotron_slime_effect.start(random(70,101));
    }
    else {
      switch(gpstarPack.getPowerLevel()) {
        case LEVEL_1:
          ms_cyclotron_slime_effect.start(100);
        break;

        case LEVEL_2:
          ms_cyclotron_slime_effect.start(90);
        break;

        case LEVEL_3:
          ms_cyclotron_slime_effect.start(80);
        break;

        case LEVEL_4:
          ms_cyclotron_slime_effect.start(70);
        break;

        case LEVEL_5:
        default:
          ms_cyclotron_slime_effect.start(60);
        break;
      }
    }
  }

  if(!b_wand_firing && !b_overheating && !b_pack_alarm) {
    vibrationPack(i_vibration_level);
  }
}

void cyclotronIceAnimation() {
  // The slime overheat animation, but blue for ice.
  if(ms_cyclotron_slime_effect.justFinished()) {
    uint8_t i_random_lower = 5;
    uint8_t i_random_upper = 21;

    if(b_cyclotron_lid_on) {
      for(int8_t i = 0; i < i_cyclotron_num_leds; i++) {
        pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_LIGHT_BLUE, random(i_random_lower, i_random_upper));
      }
    }
    else {
      for(int8_t i = 0; i < i_inner_cyclotron_cake_num_leds; i++) {
        if((i + i_ic_cake_start) != i_led_cyclotron_ring - 1 || (!b_fading_out_frozen && !b_inner_ramp_down)) {
          cyclotron_leds[i + i_ic_cake_start] = getHueAsRGB(CYCLOTRON_OUTER, C_LIGHT_BLUE, random(i_random_lower, i_random_upper));
        }
      }
    }

    switch(gpstarPack.getPowerLevel()) {
      case LEVEL_1:
        ms_cyclotron_slime_effect.start(100);
      break;

      case LEVEL_2:
        ms_cyclotron_slime_effect.start(90);
      break;

      case LEVEL_3:
        ms_cyclotron_slime_effect.start(80);
      break;

      case LEVEL_4:
        ms_cyclotron_slime_effect.start(70);
      break;

      case LEVEL_5:
      default:
        ms_cyclotron_slime_effect.start(60);
      break;
    }
  }
}

void cyclotronFade() {
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_num_leds == HASLAB_CYCLOTRON_LED_COUNT && !b_cyclotron_haslab_chsv_colour_change) {
    i_colour_scheme = C_HASLAB;
  }

  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      for(uint8_t i = 0; i < OUTER_CYCLOTRON_LED_MAX; i++) {
        if(r_cyclotron_led_fade_in[i].isRunning()) {
          b_cyclotron_led_fading_in[i] = true;

          uint8_t i_curr_brightness = r_cyclotron_led_fade_in[i].update();
          i_cyclotron_led_value[i] = i_curr_brightness;

          if(cyclotronLookupTable(i) > 0) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
        }

        uint8_t i_new_brightness = getBrightness(i_cyclotron_brightness);

        if(r_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > (i_new_brightness - 1) && b_cyclotron_led_fading_in[i]) {
          i_cyclotron_led_value[i] = i_new_brightness;
          b_cyclotron_led_fading_in[i] = false;

          r_cyclotron_led_fade_out[i].go(i_new_brightness);

          switch(i_cyclotron_num_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
            case FRUTTO_CYCLOTRON_LED_COUNT:
            case MAX_CYCLOTRON_LED_COUNT:
            default:
              r_cyclotron_led_fade_out[i].go(0, i_outer_current_ramp_speed * 3, CIRCULAR_OUT);
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
              r_cyclotron_led_fade_out[i].go(0, i_outer_current_ramp_speed * 2, CIRCULAR_OUT);
            break;
          }

          if(cyclotronLookupTable(i) > 0) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
          }
        }

        if(r_cyclotron_led_fade_out[i].isRunning()) {
          uint8_t i_curr_brightness = r_cyclotron_led_fade_out[i].update();
          i_cyclotron_led_value[i] = i_curr_brightness;

          if(cyclotronLookupTable(i) > 0) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
        }

        if(r_cyclotron_led_fade_out[i].isFinished() && !b_cyclotron_led_fading_in[i]) {
          i_cyclotron_led_value[i] = 0;
          b_cyclotron_led_fading_in[i] = true;

          if(cyclotronLookupTable(i) > 0) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          }
        }
      }
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      if(b_fade_cyclotron_led) {
        if(b_overheating && (gpstarPack.inStreamMode(HOLIDAY_HALLOWEEN) || gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS) || gpstarPack.inStreamMode(SPECTRAL))) {
          // When overheating in 84/89 and in Holiday/Spectral mode, revert to red cyclotron.
          i_colour_scheme = C_RED;
        }

        uint8_t i_new_brightness = getBrightness(i_cyclotron_brightness);

        for(uint8_t i = 0; i < i_cyclotron_num_leds; i++) {
          if(r_cyclotron_led_fade_in[i].isRunning()) {
            b_cyclotron_led_fading_in[i] = true;
            uint8_t i_curr_brightness = r_cyclotron_led_fade_in[i].update();

            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness, false, !b_overheating);
            i_cyclotron_led_value[i] = i_curr_brightness;
          }

          if(r_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] == (i_new_brightness - 1) && b_cyclotron_led_fading_in[i]) {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness, false, !b_overheating);
            i_cyclotron_led_value[i] = i_new_brightness;
          }

          if(r_cyclotron_led_fade_out[i].isRunning()) {
            uint8_t i_curr_brightness = r_cyclotron_led_fade_out[i].update();

            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness, false, !b_overheating);
            i_cyclotron_led_value[i] = i_curr_brightness;
            b_cyclotron_led_fading_in[i] = false;
          }

          if(r_cyclotron_led_fade_out[i].isFinished() && !b_cyclotron_led_fading_in[i]) {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
            i_cyclotron_led_value[i] = 0;
            b_cyclotron_led_fading_in[i] = true;
          }
        }
      }
    break;
  }
}

void cyclotron84LightOn(uint8_t cLed) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness);
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);
  uint8_t i_led_array_width = 1; // Variable to store the number of LEDs to either side of the center LED.

  /*
  if(i_cyclotron_num_leds == FRUTTO_CYCLOTRON_LED_COUNT) {
    i_led_array_width = 2;
  }
  else if(i_cyclotron_num_leds == MAX_CYCLOTRON_LED_COUNT) {
    i_led_array_width = 4;
  }
  */

  // We override the colour changes when using stock HasLab Cyclotron LEDs, returning full white.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_num_leds == HASLAB_CYCLOTRON_LED_COUNT && !b_cyclotron_haslab_chsv_colour_change) {
    i_colour_scheme = C_HASLAB;
  }

  pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
  i_cyclotron_led_value[cLed - i_cyclotron_led_start] = i_brightness;

  // Turn on the other 2 LEDs if we are allowing 3 to light up.
  if(!b_cyclotron_single_led) {
    for(uint8_t i = 1; i <= i_led_array_width; i++) {
      pack_leds[cLed + i] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      i_cyclotron_led_value[cLed + i - i_cyclotron_led_start] = i_brightness;

      uint8_t cLedTemp = cLed; // Create new temporary variable for the negative side.

      if(cLed - i < i_cyclotron_led_start) {
        cLedTemp = i_pack_num_leds - i_nfilter_jewel_leds - i;
      }
      else {
        cLedTemp = cLed - i;
      }

      pack_leds[cLedTemp] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      i_cyclotron_led_value[cLedTemp - i_cyclotron_led_start] = i_brightness;
    }
  }
}

void cyclotron84LightOff(uint8_t cLed) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness); // Calculate desired brightness.
  uint8_t i_led_array_width = 1; // Variable to store the number of LEDs to either side of the center LED.

  // Guard against divide-by-zero just in case.
  sanitizeCyclotronMultipliers();

  /*
  if(i_cyclotron_num_leds == FRUTTO_CYCLOTRON_LED_COUNT) {
    i_led_array_width = 2;
  }
  else if(i_cyclotron_num_leds == MAX_CYCLOTRON_LED_COUNT) {
    i_led_array_width = 4;
  }
  */

  if(!b_fade_cyclotron_led) {
    pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(!b_cyclotron_single_led) {
      for(uint8_t i = 1; i <= i_led_array_width; i++) {
        pack_leds[cLed + i] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

        uint8_t cLedTemp = cLed; // Create new temporary variable for the negative side.

        if(cLed - i < i_cyclotron_led_start) {
          cLedTemp = i_pack_num_leds - i_nfilter_jewel_leds - i;
        }
        else {
          cLedTemp = cLed - i;
        }

        pack_leds[cLedTemp] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      }
    }
  }
  else {
    uint8_t i_brightness_tmp = 0;

    if(i_cyclotron_led_value[cLed - i_cyclotron_led_start] == i_brightness) {
      r_cyclotron_led_fade_out[cLed - i_cyclotron_led_start].go(i_brightness);
      r_cyclotron_led_fade_out[cLed - i_cyclotron_led_start].go(i_brightness_tmp, (i_1984_delay * 1.5) / i_cyclotron_multiplier, LINEAR);
    }

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(!b_cyclotron_single_led) {
      for(uint8_t i = 1; i <= i_led_array_width; i++) {
        if(i_cyclotron_led_value[cLed + i - i_cyclotron_led_start] == i_brightness) {
          r_cyclotron_led_fade_out[cLed + i - i_cyclotron_led_start].go(i_brightness);
          r_cyclotron_led_fade_out[cLed + i - i_cyclotron_led_start].go(i_brightness_tmp, (i_1984_delay * 1.5) / i_cyclotron_multiplier, LINEAR);
        }

        uint8_t cLedTemp = cLed; // Create new temporary variable for the negative side.

        if(cLed - i < i_cyclotron_led_start) {
          cLedTemp = i_pack_num_leds - i_nfilter_jewel_leds - i;
        }
        else {
          cLedTemp = cLed - i;
        }

        if(i_cyclotron_led_value[cLedTemp - i_cyclotron_led_start] == i_brightness) {
          r_cyclotron_led_fade_out[cLedTemp - i_cyclotron_led_start].go(i_brightness);
          r_cyclotron_led_fade_out[cLedTemp - i_cyclotron_led_start].go(i_brightness_tmp, (i_1984_delay * 1.5) / i_cyclotron_multiplier, LINEAR);
        }
      }
    }
  }
}

void cyclotron1984(uint16_t iRampDelay) {
  i_fast_led_delay = FAST_LED_UPDATE_MS;

  // Guard against divide-by-zero just in case.
  sanitizeCyclotronMultipliers();

  if(ms_cyclotron.justFinished()) {
    iRampDelay = iRampDelay / i_cyclotron_multiplier;

    if(b_ramp_up) {
      if(r_outer_cyclotron_ramp.isFinished()) {
        b_ramp_up = false;

        ms_cyclotron.start(iRampDelay);
        i_outer_current_ramp_speed = iRampDelay;


        i_vibration_level = i_vibration_idle_level_1984;
      }
      else {
        ms_cyclotron.start(r_outer_cyclotron_ramp.update());
        i_outer_current_ramp_speed = r_outer_cyclotron_ramp.update();

        i_vibration_level = i_vibration_idle_level_1984;
      }
    }
    else if(b_ramp_down) {
      if(r_outer_cyclotron_ramp.isFinished()) {
        b_ramp_down = false;
      }
      else {
        ms_cyclotron.start(r_outer_cyclotron_ramp.update());
        i_outer_current_ramp_speed = r_outer_cyclotron_ramp.update();

        i_vibration_level--;

        if(i_vibration_level < i_vibration_level_min) {
          i_vibration_level = i_vibration_level_min;
        }
      }
    }
    else {
      ms_cyclotron.start(iRampDelay);
    }

    if(!b_wand_firing && !b_overheating && !b_pack_alarm) {
      vibrationPack(i_vibration_level);
    }

    if(usingSlimeCyclotron(gpstarPack.getStreamMode())) {
      return;
    }

    if(!b_1984_led_start) {
      if(b_cyclotron_lid_on) {
        cyclotron84LightOff(i_led_cyclotron);
      }
    }
    else {
      b_1984_led_start = false;
    }

    i_1984_counter++;

    if(i_1984_counter > 3) {
      i_1984_counter = 0;
    }

    i_led_cyclotron = i_cyclotron_led_start + cyclotron84LookupTable(i_1984_counter);

    if(b_cyclotron_lid_on) {
      cyclotron84LightOn(i_led_cyclotron);
    }
  }
}

void cyclotron2021(uint16_t iRampDelay) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness); // Calculate desired brightness.
  uint8_t i_curr_cyclotron_position = i_led_cyclotron - i_cyclotron_led_start; // Variable to store current cyclotron LED position.

  if(ms_cyclotron.justFinished()) {
    uint8_t i_cyclotron_matrix_led = cyclotronLookupTable(i_curr_cyclotron_position);

    if(b_ramp_up) {
      i_fast_led_delay = FAST_LED_UPDATE_MS;

      if(r_outer_cyclotron_ramp.isFinished()) {
        b_ramp_up = false;
        i_outer_current_ramp_speed = iRampDelay;

        ms_cyclotron.start(i_outer_current_ramp_speed);

        i_vibration_level = i_vibration_idle_level_2021;
      }
      else {
        i_outer_current_ramp_speed = r_outer_cyclotron_ramp.update();

        ms_cyclotron.start(i_outer_current_ramp_speed);

        i_vibration_level++;

        if(i_vibration_level < i_vibration_level_min) {
          i_vibration_level = i_vibration_level_min;
        }

        if(i_vibration_level > i_vibration_idle_level_2021) {
          i_vibration_level = i_vibration_idle_level_2021;
        }
      }
    }
    else if(b_ramp_down) {
      i_fast_led_delay = FAST_LED_UPDATE_MS;

      if(r_outer_cyclotron_ramp.isFinished()) {
        b_ramp_down = false;
      }
      else {
        i_outer_current_ramp_speed = r_outer_cyclotron_ramp.update();

        ms_cyclotron.start(i_outer_current_ramp_speed);

        if(i_outer_current_ramp_speed > 40 && i_vibration_level > i_vibration_level_min + 20) {
          i_vibration_level--;
        }
        else if(i_outer_current_ramp_speed > 100 && i_vibration_level > i_vibration_level_min) {
          i_vibration_level--;
        }

        if(i_vibration_level < i_vibration_level_min) {
          i_vibration_level = i_vibration_level_min;
        }
      }
    }
    else {
      i_outer_current_ramp_speed = iRampDelay;

      uint16_t t_iRampDelay = iRampDelay;

      switch(i_cyclotron_num_leds) {
        case OUTER_CYCLOTRON_LED_MAX:
        case MAX_CYCLOTRON_LED_COUNT:
        default:
          if(i_cyclotron_multiplier > 1) {
            if(t_iRampDelay - i_cyclotron_multiplier > 0) {
              t_iRampDelay = t_iRampDelay - i_cyclotron_multiplier;
            }
            else {
              t_iRampDelay = 0;
            }

            if(b_cyclotron_lid_on) {
              i_fast_led_delay = FAST_LED_UPDATE_MS + i_cyclotron_multiplier;
            }
            else {
              i_fast_led_delay = FAST_LED_UPDATE_MS;
            }
          }
          else {
            i_fast_led_delay = FAST_LED_UPDATE_MS;
          }

          if(i_fast_led_delay > 10) {
            i_fast_led_delay = 10;
          }
        break;

        case FRUTTO_CYCLOTRON_LED_COUNT:
        case HASLAB_CYCLOTRON_LED_COUNT:
          i_fast_led_delay = FAST_LED_UPDATE_MS;

          if(i_cyclotron_multiplier > 1) {
            if(t_iRampDelay - i_cyclotron_multiplier > 0) {
              t_iRampDelay = t_iRampDelay - i_cyclotron_multiplier;
            }
            else {
              t_iRampDelay = 0;
            }
          }
        break;
      }

      if(t_iRampDelay < 1) {
        t_iRampDelay = 1;
      }

      ms_cyclotron.start(t_iRampDelay);
    }

    if(!b_wand_firing && !b_overheating && !b_pack_alarm) {
      vibrationPack(i_vibration_level);
    }

    switch(i_cyclotron_num_leds) {
      case MAX_CYCLOTRON_LED_COUNT:
      default:
        if(i_cyclotron_multiplier > 1) {
          if(iRampDelay - i_cyclotron_multiplier > 0) {
            iRampDelay = iRampDelay - i_cyclotron_multiplier;
          }
          else {
            iRampDelay = 0;
          }
        }
        else {
          if(!(b_ramp_up || b_ramp_down)) {
            iRampDelay = iRampDelay * 3;
          }
        }
      break;

      case OUTER_CYCLOTRON_LED_MAX:
      case FRUTTO_CYCLOTRON_LED_COUNT:
        if(i_cyclotron_multiplier > 1) {
          if(iRampDelay - i_cyclotron_multiplier > 0) {
            iRampDelay = iRampDelay - i_cyclotron_multiplier;
          }
          else {
            iRampDelay = 0;
          }
        }
        else {
          iRampDelay = iRampDelay * 3;
        }
      break;

      case HASLAB_CYCLOTRON_LED_COUNT:
        if(i_cyclotron_multiplier > 1) {
          if(iRampDelay - i_cyclotron_multiplier > 0) {
            iRampDelay = iRampDelay - i_cyclotron_multiplier;
          }
          else {
            iRampDelay = 0;
          }
        }
        else {
          iRampDelay = iRampDelay * 2;
        }
      break;
    }

    if(iRampDelay < 1) {
      iRampDelay = 1;
    }

    if(usingSlimeCyclotron(gpstarPack.getStreamMode())) {
      return;
    }

    if(i_cyclotron_led_value[i_curr_cyclotron_position] == 0 && i_cyclotron_matrix_led > 0 && b_cyclotron_lid_on) {
      r_cyclotron_led_fade_in[i_curr_cyclotron_position].go(0);
      r_cyclotron_led_fade_in[i_curr_cyclotron_position].go(i_brightness, iRampDelay, CIRCULAR_IN);
    }

    uint8_t i_cyclotron_lens_gap = 0;
    if(b_cyclotron_simulate_ring) {
      switch(i_cyclotron_num_leds) {
        case OUTER_CYCLOTRON_LED_MAX:
          // Do nothing; already 0.
        break;

        case MAX_CYCLOTRON_LED_COUNT:
        default:
          if(b_ramp_down || b_ramp_up || b_pack_alarm || b_wand_mash_lockout) {
            if(i_curr_cyclotron_position == 39) {
              // Top gap between lenses is about 27 pixels wide.
              i_cyclotron_lens_gap = 27;
            }
            else if(i_curr_cyclotron_position == 19) {
              // Bottom gap between lenses is about 15 pixels wide.
              i_cyclotron_lens_gap = 15;
            }
            else {
              // Side gaps between lenses are about 21 pixels wide.
              i_cyclotron_lens_gap = 21;
            }
          }
          else {
            // When ramp to full speed is complete, set all gaps to 3 for speed.
            i_cyclotron_lens_gap = 3;
          }
        break;

        case FRUTTO_CYCLOTRON_LED_COUNT:
          if(b_ramp_down || b_ramp_up || b_pack_alarm || b_wand_mash_lockout) {
            if(i_curr_cyclotron_position > 34) {
              // Top gap between lenses is about 15 pixels wide.
              i_cyclotron_lens_gap = 15;
            }
            else if(i_curr_cyclotron_position > 14 && i_curr_cyclotron_position < 20) {
              // Bottom gap between lenses is about 9 pixels wide.
              i_cyclotron_lens_gap = 9;
            }
            else {
              // Side gaps between lenses are about 11 pixels wide.
              i_cyclotron_lens_gap = 11;
            }
          }
          else {
            // When ramp to full speed is complete, set all gaps to 3 for speed.
            i_cyclotron_lens_gap = 3;
          }
        break;

        case HASLAB_CYCLOTRON_LED_COUNT:
          if(b_ramp_down || b_ramp_up || b_pack_alarm || b_wand_mash_lockout) {
            if(i_curr_cyclotron_position > 32) {
              // Top gap between lenses is about 9 pixels wide.
              i_cyclotron_lens_gap = 9;
            }
            else if(i_curr_cyclotron_position > 12 && i_curr_cyclotron_position < 20) {
              // Bottom gap between lenses is about 5 pixels wide.
              i_cyclotron_lens_gap = 5;
            }
            else {
              // Side gaps between lenses are about 7 pixels wide.
              i_cyclotron_lens_gap = 7;
            }
          }
          else {
            // When ramp to full speed is complete, set all gaps to 3 for speed.
            i_cyclotron_lens_gap = 3;
          }
        break;
      }
    }

    if(b_clockwise) {
      if(i_cyclotron_matrix_led == 0 && i_cyclotron_fake_ring_counter < i_cyclotron_lens_gap) {
        i_cyclotron_fake_ring_counter++;
      }
      else {
        i_cyclotron_fake_ring_counter = 0;

        if(i_cyclotron_matrix_led == 0) {
          // Skip to the next valid LED value in the array.
          for(uint8_t i = i_led_cyclotron; i < OUTER_CYCLOTRON_LED_MAX + i_cyclotron_led_start; i++) {
            if(cyclotronLookupTable(i - i_cyclotron_led_start) > 0) {
              i_led_cyclotron = i;
              break;
            }
            else if(i == i_powercell_num_leds + OUTER_CYCLOTRON_LED_MAX - 1) {
              // Reset back to the start of the loop.
              i_led_cyclotron = i_cyclotron_led_start;
            }
          }
        }
        else {
          i_led_cyclotron++;
        }
      }

      if(i_led_cyclotron > i_powercell_num_leds + OUTER_CYCLOTRON_LED_MAX - 1) {
        i_led_cyclotron = i_cyclotron_led_start;
      }
    }
    else {
      if(i_cyclotron_matrix_led == 0 && i_cyclotron_fake_ring_counter < i_cyclotron_lens_gap) {
        i_cyclotron_fake_ring_counter++;
      }
      else {
        i_cyclotron_fake_ring_counter = 0;

        if(i_cyclotron_matrix_led == 0) {
          // Skip to the next valid LED value in the array.
          for(uint8_t i = i_led_cyclotron; i > i_cyclotron_led_start; i--) {
            if(cyclotronLookupTable(i - i_cyclotron_led_start) > 0) {
              i_led_cyclotron = i;
              break;
            }
          }
        }
        else {
          i_led_cyclotron--;
        }
      }

      if(i_led_cyclotron < i_cyclotron_led_start) {
        i_led_cyclotron = i_powercell_num_leds + OUTER_CYCLOTRON_LED_MAX - 1;
      }
    }
  }
}

void cyclotronNoCable() {
  if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
    innerCyclotronRingUpdate(i_2021_inner_delay * 16);
  }

  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      cyclotron2021(i_2021_delay * 10);

      if(ms_alarm.justFinished()) {
        ventLight(false);
        ventLightLEDW(false);
        ms_alarm.start(i_1984_delay);
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 2) {
          ventLight(true);
          ventLightLEDW(true);
        }
      }

      vibrationPack(i_vibration_level_min * 3);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      cyclotron1984(i_1984_delay * 3);

      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);

        // Turn off the N-Filter lights.
        ventLight(false);
        ventLightLEDW(false);

        vibrationPack(i_vibration_level_min);
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 4) {
          vibrationPack(i_vibration_idle_level_1984);

          // Turn on the N-Filter lights.
          ventLight(true);
          ventLightLEDW(true);
        }
      }
    break;
  }
}

void cyclotron1984Alarm() {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness);
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, gpstarPack.getStreamMode(), b_cyclotron_colour_toggle);
  uint8_t led1 = i_cyclotron_led_start + cyclotron84LookupTable(0);
  uint8_t led2 = i_cyclotron_led_start + cyclotron84LookupTable(1);
  uint8_t led3 = i_cyclotron_led_start + cyclotron84LookupTable(2);
  uint8_t led4 = i_cyclotron_led_start + cyclotron84LookupTable(3);
  uint8_t i_led_array_width = 1; // Variable to store the number of LEDs to either side of the center LED.

  if(gpstarPack.inStreamMode(HOLIDAY_HALLOWEEN) || gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS) || gpstarPack.inStreamMode(SPECTRAL)) {
    // When in an alarm state in 84/89 and in Holiday/Spectral mode, revert to red cyclotron.
    i_colour_scheme = C_RED;
  }

  /*
  if(i_cyclotron_num_leds == FRUTTO_CYCLOTRON_LED_COUNT) {
    i_led_array_width = 2;
  }
  else if(i_cyclotron_num_leds == MAX_CYCLOTRON_LED_COUNT) {
    i_led_array_width = 4;
  }
  */

  if(!b_fade_cyclotron_led) {
    pack_leds[led1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led2] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led3] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led4] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

    // Turn on all the other cyclotron LEDs if required.
    if(!b_cyclotron_single_led) {
      for(uint8_t i = 1; i <= i_led_array_width; i++) {
        pack_leds[led1 + i] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

        if(led1 - i < i_cyclotron_led_start) {
          led1 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led1 = led1 - i;
        }

        pack_leds[led1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
        pack_leds[led2 + i] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

        if(led2 - i < i_cyclotron_led_start) {
          led2 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led2 = led2 - i;
        }

        pack_leds[led2] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
        pack_leds[led3 + i] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

        if(led3 - i < i_cyclotron_led_start) {
          led3 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led3 = led3 - i;
        }

        pack_leds[led3] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
        pack_leds[led4 + i] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

        if(led4 - i < i_cyclotron_led_start) {
          led4 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led4 = led4 - i;
        }

        pack_leds[led4] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      }
    }
  }
  else {
    uint8_t i_brightness_tmp = 0;
    bool b_fade_in_now = i_cyclotron_led_value[led1 - i_cyclotron_led_start] == i_brightness_tmp ||
                         i_cyclotron_led_value[led2 - i_cyclotron_led_start] == i_brightness_tmp ||
                         i_cyclotron_led_value[led3 - i_cyclotron_led_start] == i_brightness_tmp ||
                         i_cyclotron_led_value[led4 - i_cyclotron_led_start] == i_brightness_tmp;

    if(b_fade_in_now) {
      clearCyclotronFades();
      r_cyclotron_led_fade_in[led1 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
      r_cyclotron_led_fade_in[led2 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
      r_cyclotron_led_fade_in[led3 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
      r_cyclotron_led_fade_in[led4 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
    }

    // Turn on all the other cyclotron LEDs if required.
    if(!b_cyclotron_single_led) {
      for(uint8_t i = 1; i <= i_led_array_width; i++) {
        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led1 + i - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(led1 - i < i_cyclotron_led_start) {
          led1 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led1 = led1 - i;
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led1 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led2 + i - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(led2 - i < i_cyclotron_led_start) {
          led2 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led2 = led2 - i;
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led2 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led3 + i - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(led3 - i < i_cyclotron_led_start) {
          led3 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led3 = led3 - i;
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led3 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led4 + i - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }

        if(led4 - i < i_cyclotron_led_start) {
          led4 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
        else {
          led4 = led4 - i;
        }

        if(b_fade_in_now) {
          r_cyclotron_led_fade_in[led4 - i_cyclotron_led_start].go(i_brightness, (i_1984_delay * 1.5), LINEAR);
        }
      }
    }
  }
}

void packOverheatingFinished() {
  if(!b_wand_syncing) {
    packSerialSend(P_OVERHEATING_FINISHED);
  }

  ms_overheating_length.stop();

  stopEffect(S_STEAM_LOOP);
  stopEffect(S_SLIME_REFILL);

  if(!gpstarPack.inStreamMode(SLIME)) {
    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        playEffect(S_VENT_CLOSE_ALT);
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
        playEffect(S_VENT_CLOSE);
      break;
    }

    if(gpstarPack.isThemeModern()) {
      stopEffect(S_PACK_OVERHEAT_HOT);
    }

    playEffect(S_STEAM_LOOP_FADE_OUT);
  }

  b_overheating = false;

  // Turn off the smoke.
  smokeNFilter(false);

  // Stop the fans.
  fanNFilter(false);

  // Reset the LEDs before resetting the alarm flag.
  if((gpstarPack.isTheme80s()) && !usingSlimeCyclotron(gpstarPack.getStreamMode())) {
    resetCyclotronState();
  }

  b_pack_alarm = false;

  attenuatorSerialSend(A_OVERHEATING_FINISHED);

  if(b_overheat_lights_off) {
    cyclotronSpeedRevert();

    // Reset the cyclotron ramp speeds.
    resetRampSpeeds();
  }

  resetRampUp();

  packStartup(false); // Start the pack using an abbreviated startup sequence.

  // Turn off the vent lights
  ventLight(false);
  ventLightLEDW(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  ms_cyclotron.start(i_2021_delay);
}

void packOverheatingStart() {
  if(gpstarPack.inStreamMode(SLIME)) {
    playEffect(S_SLIME_EMPTY);
  }
  else {
    playEffect(S_PACK_PRE_VENT);

    if(gpstarPack.isThemeModern()) {
      playEffect(S_PACK_OVERHEAT_HOT, true);
    }

    playEffect(S_VENT_OPEN);
  }

  b_overheating = true;

  // Start timer for a second smoke sound.
  if(gpstarPack.inStreamMode(SLIME)) {
    ms_overheating.start(i_overheating_delay - 1000);
  }
  else {
    ms_overheating.start(i_overheating_delay);

    // Reset some vent light timers.
    ms_vent_light_off.stop();
    ms_vent_light_on.stop();
    ms_vent_light_off.start(i_vent_light_delay);
  }

  // Reset the Inner Cyclotron speed.
  if(gpstarPack.isTheme80s()) {
    i_inner_current_ramp_speed = i_inner_ramp_delay;
    clearCyclotronFades();
  }

  attenuatorSerialSend(A_OVERHEATING);
}

void cyclotronOverheating() {
  if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
    smokeNFilter(true);
  }

  if(ms_overheating.justFinished()) {
    if(gpstarPack.inStreamMode(SLIME)) {
      // Play the sound of slime refilling the tank.
      playEffect(S_SLIME_REFILL, true);
    }
    else {
      playEffect(S_AIR_RELEASE);

      // Play one of the random steam burst effects.
      playEffect(PROGMEM_READU16(sfx_smoke[random(5)]), false, i_volume_effects, true, 120);

      // Fade in the steam release loop.
      playEffect(S_STEAM_LOOP, true, i_volume_effects, true, 1000);
    }

    switch(gpstarPack.getPowerLevel()) {
      case LEVEL_1:
        ms_overheating_length.start(i_ms_overheating_length_1);
      break;

      case LEVEL_2:
        ms_overheating_length.start(i_ms_overheating_length_2);
      break;

      case LEVEL_3:
        ms_overheating_length.start(i_ms_overheating_length_3);
      break;

      case LEVEL_4:
        ms_overheating_length.start(i_ms_overheating_length_4);
      break;

      case LEVEL_5:
      default:
        ms_overheating_length.start(i_ms_overheating_length_5);
      break;
    }

    if(!b_overheat_sync_to_fan) {
      smokeNFilter(false);
    }
  }

  if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
    if(!b_overheat_lights_off || (b_overheat_lights_off && i_powercell_led > 0)) {
      innerCyclotronRingUpdate(i_2021_inner_delay * 16);
    }
  }

  // The cyclotron lights during the entire overheating sequence
  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      if(!b_overheat_lights_off) {
        cyclotron2021(i_2021_delay * 10);

        vibrationPack(i_vibration_level_min * 2);
      }
      else {
        if(i_powercell_led > 0) {
          cyclotron2021(i_2021_delay * 10);

          vibrationPack(i_vibration_level_min);
        }
        else {
          vibrationPack(0);
          innerCyclotronCakeOff();
        }
      }
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);

        if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
          if(!b_fade_cyclotron_led) {
            resetCyclotronState();
          }
          else {
            for(uint8_t i = 0; i < 4; i++) {
              cyclotron84LightOff(cyclotron84LookupTable(i) + i_cyclotron_led_start);
            }
          }
        }
      }
      else if(ms_alarm.remaining() < i_1984_delay / 4) {
        if(!b_overheat_lights_off) {
          vibrationPack(i_vibration_level_min);

          if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
            cyclotron1984Alarm();
          }
        }
        else if(i_powercell_led > 0) {
          vibrationPack(i_vibration_level_min);

          if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
            cyclotron1984Alarm();
          }
        }
        else {
          vibrationPack(0);
          innerCyclotronCakeOff();
        }
      }
    break;
  }

  if(ms_overheating_length.isRunning() && !gpstarPack.inStreamMode(SLIME)) {
    if(b_overheat_sync_to_fan) {
      smokeNFilter(true);
    }

    // Turn the fans on.
    fanNFilter(true);

    // For strobing the vent light.
    if(ms_vent_light_off.justFinished()) {
      ms_vent_light_off.stop();
      ms_vent_light_on.start(i_vent_light_delay);

      if(b_overheat_strobe) {
        ventLight(true);
      }
    }
    else if(ms_vent_light_on.justFinished()) {
      ms_vent_light_on.stop();
      ms_vent_light_off.start(i_vent_light_delay);

      if(b_overheat_strobe) {
        ventLight(false);
      }
    }

    // For non-strobing vent light option.
    if(!b_overheat_strobe) {
      if(!b_vent_light_on) {
        // Solid light on if strobe option turned off.
        ventLight(true);
      }
    }

    ventLightLEDW(true);
  }

  if(ms_overheating_length.justFinished()) {
    // Tell the Neutrona Wand the overheating is finished.
    packOverheatingFinished();
  }
}

void cyclotronControl() {
  // Only reset the starting LED when the pack is first started up.
  if(b_reset_start_led) {
    b_reset_start_led = false;
    i_cyclotron_fake_ring_counter = 0;
    i_led_cyclotron_ring = i_ic_cake_start;

    switch(gpstarPack.getSystemTheme()) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_clockwise) {
          i_led_cyclotron = i_cyclotron_led_start;
        }
        else {
          i_led_cyclotron = i_cyclotron_led_start + 2; // Start on LED #2 in counter-clockwise mode in 2021 mode.
        }
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
        i_1984_counter = 3;
      break;
    }
  }

  if(!ribbonCableAttached() && PACK_STATE != MODE_OFF && !b_ramp_down_start && !b_overheating) {
    if(!b_pack_alarm) {
      b_ramp_up = false;
      b_inner_ramp_up = false;
      b_pack_alarm = true;

      if(gpstarPack.isTheme80s()) {
        if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
          resetCyclotronState();
        }

        ms_cyclotron.start(0);
        ms_alarm.start(0);
      }
      else {
        ms_alarm.start(i_alarm_delay);
      }

      packAlarm();

      // Tell the wand the pack alarm is on.
      packSerialSend(P_ALARM_ON, ribbonCableAttached() ? 1 : 0);

      // Tell any add-on devices that the alarm is on.
      attenuatorSerialSend(A_ALARM_ON, ribbonCableAttached() ? 1 : 0);
    }

    // Ribbon cable has been removed.
    cyclotronNoCable();
  }
  else if(b_overheating) {
    if(!b_pack_alarm) {
      b_ramp_up = false;
      b_inner_ramp_up = false;

      if(gpstarPack.isTheme80s()) {
        if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
          resetCyclotronState();
        }
        else {
          clearCyclotronFades();
        }

        ms_cyclotron.start(0);
        ms_alarm.start(0);
      }
      else {
        ms_alarm.start(i_alarm_delay);
      }

      if(b_overheat_lights_off) {
        powercellOn();
      }

      b_pack_alarm = true;

      packAlarm();
    }

    cyclotronOverheating();
  }
  else {
    if(b_ramp_up_start) {
      b_ramp_up_start = false;

      r_outer_cyclotron_ramp.go(i_outer_current_ramp_speed); // Reset the ramp.
      r_inner_cyclotron_ramp.go(i_inner_current_ramp_speed); // Reset the Inner Cyclotron ramp.

      switch(gpstarPack.getSystemTheme()) {
        case SYSTEM_1984:
        case SYSTEM_1989:
          r_outer_cyclotron_ramp.go(i_1984_delay, i_1984_ramp_length, CIRCULAR_OUT);
          r_inner_cyclotron_ramp.go(i_1984_inner_delay, i_1984_ramp_length, CIRCULAR_OUT);
        break;

        case SYSTEM_AFTERLIFE:
        case SYSTEM_FROZEN_EMPIRE:
        default:
          if(ms_idle_fire_fade.remaining() > 0) {
            // Full Afterlife startup sequence ramps.
            r_outer_cyclotron_ramp.go(i_2021_delay, i_2021_ramp_length, QUARTIC_OUT);
            r_inner_cyclotron_ramp.go(i_2021_inner_delay, i_2021_ramp_length, QUARTIC_OUT);
          }
          else {
            if(b_brass_pack_sound_loop) {
              // Faster startup for brass pack.
              r_outer_cyclotron_ramp.go(i_2021_delay, (uint16_t)(i_2021_ramp_length / 4), QUADRATIC_OUT);
              r_inner_cyclotron_ramp.go(i_2021_inner_delay, (uint16_t)(i_2021_ramp_length / 4), QUADRATIC_OUT);
            }
            else {
              // Abbreviated Afterlife/Frozen Empire startup.
              i_outer_current_ramp_speed = i_2021_delay;
              i_inner_current_ramp_speed = i_2021_inner_delay;
            }
          }
        break;
      }
    }
    else if(b_ramp_down_start) {
      b_ramp_down_start = false;

      r_outer_cyclotron_ramp.go(i_outer_current_ramp_speed); // Reset the ramp.
      r_inner_cyclotron_ramp.go(i_inner_current_ramp_speed); // Reset the Inner Cyclotron ramp.

      if(gpstarPack.getSystemTheme() == SYSTEM_1984) {
        r_outer_cyclotron_ramp.go((uint16_t)(i_1984_delay * 1.3), i_1984_ramp_down_length, CIRCULAR_IN);
        r_inner_cyclotron_ramp.go(i_inner_ramp_delay, i_1984_ramp_down_length, CIRCULAR_IN);
      }
      else if(gpstarPack.getSystemTheme() == SYSTEM_1989) {
        r_outer_cyclotron_ramp.go((uint16_t)(i_1984_delay * 1.3), i_1989_ramp_down_length, CIRCULAR_IN);
        r_inner_cyclotron_ramp.go(i_inner_ramp_delay, i_1989_ramp_down_length, CIRCULAR_IN);
      }
      else {
        if(ms_mash_lockout.isRunning()) {
          r_outer_cyclotron_ramp.go(i_2021_ramp_delay, ms_mash_lockout.delay() / 3, QUARTIC_IN);
          r_inner_cyclotron_ramp.go(i_inner_ramp_delay, ms_mash_lockout.delay() / 3, QUARTIC_IN);
        }
        else if(gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE && !b_brass_pack_sound_loop) {
          r_outer_cyclotron_ramp.go(i_2021_ramp_delay, i_2021_ramp_down_length / 4, QUARTIC_IN);
          r_inner_cyclotron_ramp.go(i_inner_ramp_delay, i_2021_ramp_down_length / 4, QUARTIC_IN);
        }
        else if(b_brass_pack_sound_loop) {
          r_outer_cyclotron_ramp.go(i_2021_ramp_delay, i_brass_ramp_down_length, QUARTIC_IN);
          r_inner_cyclotron_ramp.go(i_inner_ramp_delay, i_brass_ramp_down_length, QUARTIC_IN);
        }
        else {
          r_outer_cyclotron_ramp.go(i_2021_ramp_delay, i_2021_ramp_down_length, QUARTIC_IN);
          r_inner_cyclotron_ramp.go(i_inner_ramp_delay, i_2021_ramp_down_length, QUARTIC_IN);
        }
      }
    }

    if(gpstarPack.isTheme80s()) {
      cyclotron1984(i_outer_current_ramp_speed);
    }
    else {
      cyclotron2021(i_outer_current_ramp_speed);
    }

    if(ms_mash_lockout.isRunning() && gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE) {
      // If we aren't frozen over but the mash timer is running, add some ice!
      ms_cyclotron_slime_effect.start(0);
      cyclotronIceAnimation();
    }

    if(!b_cyclotron_lid_on) {
      if(!usingSlimeCyclotron(gpstarPack.getStreamMode())) {
        innerCyclotronRingUpdate(i_inner_current_ramp_speed);
      }

      if(b_fading_out_frozen) {
        // Control the inner cyclotron LED fadeout here.
        uint16_t i_brightness_percent = ms_mash_lockout.delay() - (ms_mash_lockout.delay() - ms_mash_lockout.remaining());
        i_brightness_percent = (100 * i_brightness_percent) / ms_mash_lockout.delay();
        if(i_brightness_percent < 15) {
          b_fading_out_frozen = false;
          i_brightness_percent = 0;
        }

        uint8_t i_brightness = getBrightness(i_brightness_percent);
        if(b_clockwise) {
          if(i_led_cyclotron_ring == i_ic_cake_start) {
            if(CAKE_LED_TYPE == GRB_LED) {
              cyclotron_leds[i_ic_cake_end] = getHueAsGRB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
            else {
              cyclotron_leds[i_ic_cake_end] = getHueAsRGB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
          }
          else {
            if(CAKE_LED_TYPE == GRB_LED) {
              cyclotron_leds[i_led_cyclotron_ring - 1] = getHueAsGRB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
            else {
              cyclotron_leds[i_led_cyclotron_ring - 1] = getHueAsRGB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
          }
        }
        else {
          if(i_led_cyclotron_ring + 1 > i_ic_cake_end) {
            if(CAKE_LED_TYPE == GRB_LED) {
              cyclotron_leds[i_ic_cake_start] = getHueAsGRB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
            else {
              cyclotron_leds[i_ic_cake_start] = getHueAsRGB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
          }
          else {
            if(CAKE_LED_TYPE == GRB_LED) {
              cyclotron_leds[i_led_cyclotron_ring + 1] = getHueAsGRB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
            else {
              cyclotron_leds[i_led_cyclotron_ring + 1] = getHueAsRGB(CYCLOTRON_INNER, C_ORANGE, i_brightness);
            }
          }
        }
      }
    }
  }

  // If we are in slime mode, call the slime effect functions instead.
  if(usingSlimeCyclotron(gpstarPack.getStreamMode())) {
    if(PACK_STATE == MODE_ON && !ms_cyclotron_slime_effect.isRunning()) {
      // Make sure we've started the slime effect timer if it hasn't been started already.
      ms_cyclotron_slime_effect.start(0);
    }

    slimeCyclotronEffect();
    return;
  }

  if(b_cyclotron_lid_on) {
    cyclotronFade();
  }
}

void stopOverheatBeepWarnings() {
  switch(gpstarPack.getSystemTheme()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      stopEffect(S_PACK_BEEPS_OVERHEAT);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      stopEffect(S_BEEP_8);
    break;
  }
}

void packVentingFinished() {
  packSerialSend(P_VENTING_FINISHED);

  ms_overheating_length.stop();
  ms_smoke_on.stop();

  stopEffect(S_STEAM_LOOP);
  stopEffect(S_SLIME_REFILL);
  stopEffect(S_QUICK_VENT_OPEN);
  playEffect(S_QUICK_VENT_CLOSE);

  if(gpstarPack.inStreamMode(SLIME)) {
    playEffect(S_PACK_SLIME_TANK_LOOP, true, i_volume_effects, true, 1500);
  }
  else {
    playEffect(S_STEAM_LOOP_FADE_OUT);
  }

  b_venting = false;

  attenuatorSerialSend(A_VENTING_FINISHED);

  // Turn off the smoke.
  smokeNFilter(false);

  // Stop the fans.
  fanNFilter(false);

  // Turn off the vent lights
  ventLight(false);
  ventLightLEDW(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
}

void packVenting() {
  if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
    smokeNFilter(true);
  }

  if(ms_overheating.justFinished()) {
    if(gpstarPack.inStreamMode(SLIME)) {
      // Play the sound of slime refilling the tank.
      playEffect(S_SLIME_REFILL, true);
    }
    else {
      // Play one of the random steam burst effects.
      playEffect(PROGMEM_READU16(sfx_smoke[random(5)]), false, i_volume_effects, true, 120);

      // Fade in the steam release loop.
      playEffect(S_STEAM_LOOP, true, i_volume_effects, true, 1000);
    }

    switch(gpstarPack.getPowerLevel()) {
      case LEVEL_1:
        ms_overheating_length.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);

        if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);
        }
      break;

      case LEVEL_2:
        ms_overheating_length.start(i_ms_overheating_length_2 >= 4000 ? i_ms_overheating_length_2 / 2 : 2000);

        if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_2 >= 4000 ? i_ms_overheating_length_2 / 2 : 2000);
        }
      break;

      case LEVEL_3:
        ms_overheating_length.start(i_ms_overheating_length_3 >= 4000 ? i_ms_overheating_length_3 / 2 : 2000);

        if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_3 >= 4000 ? i_ms_overheating_length_3 / 2 : 2000);
        }
      break;

      case LEVEL_4:
        ms_overheating_length.start(i_ms_overheating_length_4 >= 4000 ? i_ms_overheating_length_4 / 2 : 2000);

        if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_4 >= 4000 ? i_ms_overheating_length_4 / 2 : 2000);
        }
      break;

      case LEVEL_5:
      default:
        ms_overheating_length.start(i_ms_overheating_length_5 >= 4000 ? i_ms_overheating_length_5 / 2 : 2000);

        if(!b_overheat_sync_to_fan && !gpstarPack.inStreamMode(SLIME)) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_5 >= 4000 ? i_ms_overheating_length_5 / 2 : 2000);
        }
      break;
    }
  }

  if(ms_smoke_on.remaining() < 1 && !b_overheat_sync_to_fan) {
    smokeNFilter(false);
  }

  if(ms_overheating_length.isRunning() && !gpstarPack.inStreamMode(SLIME)) {
    if(b_overheat_sync_to_fan) {
      smokeNFilter(true);
    }

    // Turn the fans on.
    fanNFilter(true);

    // For strobing the vent light.
    if(ms_vent_light_off.justFinished()) {
      ms_vent_light_off.stop();
      ms_vent_light_on.start(i_vent_light_delay);

      if(b_overheat_strobe) {
        ventLight(true);
      }
    }
    else if(ms_vent_light_on.justFinished()) {
      ms_vent_light_on.stop();
      ms_vent_light_off.start(i_vent_light_delay);

      if(b_overheat_strobe) {
        ventLight(false);
      }
    }

    // For non-strobing vent light option.
    if(!b_overheat_strobe) {
      if(!b_vent_light_on) {
        // Solid light on if strobe option turned off.
        ventLight(true);
      }
    }

    ventLightLEDW(true);
  }

  if(ms_overheating_length.justFinished()) {
    // Tell the Neutrona Wand the venting is finished.
    packVentingFinished();
  }
}

void packVentingStart() {
  stopEffect(S_SLIME_EMPTY);
  stopEffect(S_PACK_SLIME_TANK_LOOP);
  stopEffect(S_QUICK_VENT_CLOSE);

  playEffect(S_QUICK_VENT_OPEN);

  if(gpstarPack.inStreamMode(SLIME)) {
    playEffect(S_SLIME_EMPTY);
  }
  else {
    // Reset some vent light timers.
    ms_vent_light_on.stop();
    ms_vent_light_off.start(i_vent_light_delay);
  }

  b_venting = true;

  // Start venting timer.
  ms_overheating.start(0);

  // Reset Cyclotron speed.
  cyclotronSpeedRevert();

  attenuatorSerialSend(A_VENTING);
}

void modeFireStartSounds() {
  // If idle fadeout is enabled and counting down, start the idle fadeout early.
  if(b_fadeout_idle_sounds && ms_delay_post.isRunning()) {
    ms_delay_post.stop();
    fadeoutIdleSounds();
  }

  switch(gpstarPack.getStreamMode()) {
    case PROTON:
    default:
      switch(gpstarPack.getPowerLevel()) {
        case LEVEL_1 ... LEVEL_4:
          if(b_firing_intensify) {
            switch(gpstarPack.getSystemTheme()) {
              case SYSTEM_1984:
                playEffect(S_GB1_1984_FIRE_START_SHORT, false, i_volume_effects, false, 0, false);
                playEffect(S_GB1_1984_FIRE_LOOP_PACK, true, i_volume_effects, true, 250);
              break;

              case SYSTEM_1989:
                playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
                playEffect(S_GB2_FIRE_LOOP, true, i_volume_effects, true, 6500);
              break;

              case SYSTEM_AFTERLIFE:
              default:
                playEffect(S_AFTERLIFE_FIRE_START, false, i_volume_effects, false, 0, false);
                playEffect(S_GB1_1984_FIRE_LOOP_PACK, true, i_volume_effects, true, 275);
              break;

              case SYSTEM_FROZEN_EMPIRE:
                playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
                playEffect(S_GB1_1984_FIRE_LOOP_PACK, true, i_volume_effects, true, 200);
              break;
            }

            b_sound_firing_intensify_trigger = true;
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt) {
            if(gpstarPack.getSystemTheme() == SYSTEM_1989) {
              playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 6500);
            }
            else if(gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 300);
            }
            else {
              playEffect(S_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 300);
            }

            b_sound_firing_alt_trigger = true;
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;

        case LEVEL_5:
        default:
          switch(gpstarPack.getSystemTheme()) {
            case SYSTEM_1984:
              playEffect(S_GB1_1984_FIRE_START_HIGH_POWER, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_1989:
              playEffect(S_GB1_FIRE_START_HIGH_POWER, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_AFTERLIFE:
            default:
              playEffect(S_AFTERLIFE_FIRE_START, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_FROZEN_EMPIRE:
              playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
            break;
          }

          if(b_firing_intensify) {
            // Reset some sound triggers.
            b_sound_firing_intensify_trigger = true;
            if(gpstarPack.getSystemTheme() == SYSTEM_1984) {
              playEffect(S_GB1_1984_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 1700);
            }
            else if(gpstarPack.getSystemTheme() == SYSTEM_1989) {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 700);
            }
            else {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 300);
            }
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt) {
            // Reset some sound triggers.
            b_sound_firing_alt_trigger = true;
            if(gpstarPack.getSystemTheme() == SYSTEM_1989) {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 700);
            }
            else if(gpstarPack.getSystemTheme() == SYSTEM_1984) {
              playEffect(S_GB1_1984_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 1700);
            }
            else {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 300);
            }
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;
      }
    break;

    case SLIME:
      stopEffect(S_SLIME_END);
      playEffect(S_SLIME_START, false, i_volume_effects, false, 0, false);
      playEffect(S_SLIME_LOOP, true, i_volume_effects, true, 850);
    break;

    case STASIS:
      stopEffect(S_STASIS_END);
      playEffect(S_STASIS_START, false, i_volume_effects, false, 0, false);
      playEffect(S_STASIS_LOOP, true, i_volume_effects, true, 1000);
    break;

    case MESON:
      if(i_audio_version < 109) {
        // Only play a single pulse; the wand will signal us to play more.
        playEffect(S_MESON_FIRE_PULSE, false, i_volume_effects, false, 0, false);
      }
      else {
        // Make sure we stop any currently-playing pulse sound first!
        stopEffect(S_MESON_FIRE_PULSE);

        // Use the new rapid-fire feature.
        switch(gpstarPack.getPowerLevel()) {
          case LEVEL_1:
            playRapidEffect(S_MESON_FIRE_PULSE, i_meson_blast_delay_level_1);
          break;
          case LEVEL_2:
            playRapidEffect(S_MESON_FIRE_PULSE, i_meson_blast_delay_level_2);
          break;
          case LEVEL_3:
            playRapidEffect(S_MESON_FIRE_PULSE, i_meson_blast_delay_level_3);
          break;
          case LEVEL_4:
            playRapidEffect(S_MESON_FIRE_PULSE, i_meson_blast_delay_level_4);
          break;
          case LEVEL_5:
            playRapidEffect(S_MESON_FIRE_PULSE, i_meson_blast_delay_level_5);
          default:
          break;
        }
      }
    break;
  }

  // Adjust the gain with the Afterlife idling sound effect while firing.
  if((gpstarPack.isThemeModern()) && gpstarPack.getPowerLevel() < MAX_POWER_LEVEL) {
    if(ms_idle_fire_fade.remaining() < 3000) {
      if(gpstarPack.inStreamMode(SLIME)) {
        if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, 100);
        }
        else {
          adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, 100);
        }
      }
      else {
        if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - i_firing_idle_level, true, 100);
        }
        else {
          adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects - i_firing_idle_level, true, 100);
        }
      }
    }
    else {
      if(gpstarPack.inStreamMode(SLIME)) {
        if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, ms_idle_fire_fade.remaining());
        }
        else {
          adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, ms_idle_fire_fade.remaining());
        }
      }
      else {
        if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - i_firing_idle_level, true, ms_idle_fire_fade.remaining());
        }
        else {
          adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects - i_firing_idle_level, true, ms_idle_fire_fade.remaining());
        }
      }
    }
  }

  if(gpstarPack.inStreamMode(HOLIDAY_HALLOWEEN)) {
    playEffect(S_HALLOWEEN_FIRING_EXTRA, false, i_volume_effects, true, 100, false);
  }
  if(gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS)) {
    playEffect(S_CHRISTMAS_FIRING_EXTRA, false, i_volume_effects, true, 100, false);
  }
}

void mixExtraFiringEffects() {
// Disabled on ESP32 since the GPStar Wand II controls impact sounds with the IMU via serial.
#ifndef ESP32
  // Mix in additional effects for the proton stream to enhance the experience.
  if(ms_firing_sound_mix.justFinished() && gpstarPack.inStreamMode(PROTON) && STATUS_CTS == CTS_NOT_FIRING && b_stream_effects) {
    uint8_t i_random = 0;

    switch(i_last_firing_effect_mix) {
      case S_FIRE_SPARKS:
        i_random = random(0,2);
      break;

      case S_FIRE_SPARKS_3:
      case S_FIRE_SPARKS_4:
        i_random = 3;
      break;

      case S_FIRE_SPARKS_5:
        i_random = 2;
      break;

      case S_FIRE_SPARKS_2:
        i_random = 1;
      break;

      default:
        // If no firing effect has played yet.
        i_random = 3;
      break;
    }

    uint16_t i_s_random = random(2,4) * 1000; // 2 or 3 seconds

    switch(i_random) {
      case 3:
        playEffect(S_FIRE_SPARKS, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS;

        ms_firing_sound_mix.start(i_s_random * 5);
      break;

      case 2:
        playEffect(S_FIRE_SPARKS_4, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_4;

        ms_firing_sound_mix.start(i_s_random);
      break;

      case 1:
        playEffect(S_FIRE_SPARKS_3, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_3;

        ms_firing_sound_mix.start(i_s_random);
      break;

      case 0:
        playEffect(S_FIRE_SPARKS_2, false, i_volume_effects, false, 0, false);
        playEffect(S_FIRE_SPARKS_5, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_5;

        ms_firing_sound_mix.start(1800);
      break;

      default:
        // This will never trigger because i_random will only ever be 0~3.
        playEffect(S_FIRE_SPARKS_2, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_2;

        ms_firing_sound_mix.start(500);
      break;
    }
  }
#endif
}

void wandFiring() {
  if(b_powercell_sound_loop) {
    // Turn off the powercell sound effect.
    stopEffect(S_POWERCELL);
    b_powercell_sound_loop = false;
  }

  if(b_brass_pack_sound_loop) {
    // Mute the brass pack loop while firing.
    stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
    stopEffect(S_FROZEN_EMPIRE_BRASS_IDLE);
    stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP);
  }

  modeFireStartSounds();

  b_wand_firing = true;
  attenuatorSerialSend(A_FIRING);

  if(b_stream_effects && STATUS_CTS == CTS_NOT_FIRING) {
    uint16_t i_s_random = random(7,15) * 1000;
    ms_firing_sound_mix.start(i_s_random);
  }

  // Turn off any smoke.
  smokeNFilter(false);

  // Start a smoke timer to play a little bit of smoke while firing.
  if(!gpstarPack.inStreamMode(SLIME)) {
    ms_smoke_timer.start(PROGMEM_READU16(i_smoke_timer[(uint8_t)gpstarPack.getPowerLevel() - 1]));
    ms_smoke_on.stop();
  }

  // Just in case a semi-auto was fired before we started firing a stream, stop its vibration timer.
  ms_menu_vibration.stop();

  vibrationPack(255);

  // Reset some vent light timers.
  ms_vent_light_on.stop();
  ms_vent_light_off.start(i_vent_light_delay);

  // Reset vent sounds flag.
  b_vent_sounds_playing = false;

  ms_firing_length_timer.start(i_firing_timer_length);
}

void modeFireStopSounds() {
  if(b_wand_firing) {
    switch(gpstarPack.getStreamMode()) {
      case PROTON:
      default:
        switch(gpstarPack.getSystemTheme()) {
          case SYSTEM_1984:
            if(gpstarPack.getPowerLevel() != MAX_POWER_LEVEL) {
              // Play different firing end stream sound depending on how long we have been firing for.
              if(ms_firing_length_timer.remaining() < 5000) {
                // Long firing tail end.
                playEffect(S_FIRING_END_MID, false, i_volume_effects, false, 0, false);
              }
              else if(ms_firing_length_timer.remaining() < 10000) {
                // Mid firing tail end.
                playEffect(S_FIRING_END, false, i_volume_effects, false, 0, false);
              }
              else {
                // Short firing tail end.
                playEffect(S_GB1_1984_FIRE_END_SHORT, false, i_volume_effects, false, 0, false);
              }
            }
            else {
              // Play different firing end stream sound depending on how long we have been firing for.
              if(ms_firing_length_timer.remaining() < 5000) {
                // Long tail end.
                playEffect(S_GB1_1984_FIRE_END_HIGH_POWER, false, i_volume_effects, false, 0, false);
              }
              else if(ms_firing_length_timer.remaining() < 10000) {
                // Mid tail end.
                playEffect(S_GB1_1984_FIRE_END_MID_HIGH_POWER, false, i_volume_effects, false, 0, false);
              }
              else {
                // Short tail end.
                playEffect(S_GB1_1984_FIRE_END_SHORT_HIGH_POWER, false, i_volume_effects, false, 0, false);
              }
            }
          break;

          case SYSTEM_1989:
            // Play different firing end stream sound depending on how long we have been firing for.
            if(ms_firing_length_timer.remaining() < 5000) {
              // Long tail end.
              playEffect(S_FIRING_END_GUN, false, i_volume_effects, false, 0, false);
            }
            else if(ms_firing_length_timer.remaining() < 10000) {
              // Mid tail end.
              playEffect(S_FIRING_END_MID, false, i_volume_effects, false, 0, false);
            }
            else {
              // Short tail end.
              playEffect(S_FIRING_END, false, i_volume_effects, false, 0, false);
            }
          break;

          case SYSTEM_AFTERLIFE:
          default:
            // Play different firing end stream sound depending on how long we have been firing for.
            if(ms_firing_length_timer.remaining() < 5000) {
              // Long firing tail end.
              playEffect(S_AFTERLIFE_FIRE_END_LONG, false, i_volume_effects, false, 0, false);
            }
            else if(ms_firing_length_timer.remaining() < 10000) {
              // Mid firing tail end.
              playEffect(S_AFTERLIFE_FIRE_END_MID, false, i_volume_effects, false, 0, false);
            }
            else {
              // Short firing tail end.
              playEffect(S_AFTERLIFE_FIRE_END_SHORT, false, i_volume_effects, false, 0, false);
            }
          break;

          case SYSTEM_FROZEN_EMPIRE:
            // Frozen Empire replaces all firing tail sounds with just a "thump".
            playEffect(S_AFTERLIFE_FIRE_END_MID, false, i_volume_effects, false, 0, false);
          break;
        }
      break;

      case SLIME:
        playEffect(S_SLIME_END, false, i_volume_effects, false, 0, false);
      break;

      case STASIS:
        playEffect(S_STASIS_END, false, i_volume_effects, false, 0, false);
      break;

      case MESON:
        // Set the pulse to stop looping.
        stopEffectLoop(S_MESON_FIRE_PULSE);
      break;
    }

    // Adjust the gain with the Afterlife idling track.
    if((gpstarPack.isThemeModern()) && gpstarPack.getPowerLevel() < MAX_POWER_LEVEL) {
      if(ms_idle_fire_fade.remaining() < 1000) {
        if(gpstarPack.inStreamMode(SLIME)) {
          if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
            adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, 30);
          }
          else {
            adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, 30);
          }
        }
        else {
          if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
            adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 30);
          }
          else {
            adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects, true, 30);
          }
        }
      }
      else {
        if(gpstarPack.inStreamMode(SLIME)) {
          if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
            adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, ms_idle_fire_fade.remaining());
          }
          else {
            adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects - i_slime_idle_level, true, ms_idle_fire_fade.remaining());
          }
        }
        else {
          if(gpstarPack.getSystemTheme() == SYSTEM_AFTERLIFE) {
            adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, ms_idle_fire_fade.remaining());
          }
          else {
            adjustGainEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, i_volume_effects, true, ms_idle_fire_fade.remaining());
          }
        }
      }
    }
  }

  wandStopFiringSounds();
}

void wandStoppedFiring() {
  if(b_brass_pack_sound_loop) {
    // Restore the volume for the brass pack loop.
    if(b_brass_startup_loop) {
      playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP, true, i_volume_effects, true, 2000);
    }
    else {
      playEffect(S_FROZEN_EMPIRE_BRASS_IDLE, true, i_volume_effects, true, 1000);
    }
  }

  modeFireStopSounds();

  ms_firing_sound_mix.stop();

  b_wand_firing = false;
  b_firing_alt = false;
  b_firing_intensify = false;

  attenuatorSerialSend(A_FIRING_STOPPED);

  // Reset some vent light timers.
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
  ventLight(false);
  ventLightLEDW(false);

  // Reset vent sounds flag.
  b_vent_sounds_playing = false;

  // Turn off any smoke.
  smokeNFilter(false);

  // Turn off the fans.
  fanNFilter(false);

  // Stop overheating-related timers.
  ms_firing_length_timer.stop();
  ms_smoke_timer.stop();
  ms_smoke_on.stop();
  ms_delay_post_2.stop();

  // Stop overheat beeps.
  stopOverheatBeepWarnings();
}

void checkMenuVibration() {
  if(gpstarPack.getVibrationMode() != CYCLOTRON_MOTOR) {
    if(ms_menu_vibration.justFinished()) {
      vibrationOff();
    }
    else if(ms_menu_vibration.isRunning()) {
      if(PACK_STATE == MODE_OFF) {
        // If we're off we must be in the EEPROM Config Menu; vibrate at 59%.
        analogWrite(VIBRATION_PIN, 150);
      }
      else {
        // If we're on we must be firing a semi-auto blast; vibrate at 71%.
        analogWrite(VIBRATION_PIN, 180);
      }
    }
  }
}

void cyclotronSpeedRevert() {
  i_cyclotron_multiplier = 1;
  i_cyclotron_switch_led_multiplier = 1;
  i_powercell_multiplier = 1;
}

void cyclotronSpeedIncrease() {
  i_cyclotron_multiplier++;
  i_cyclotron_switch_led_multiplier++;
  i_powercell_multiplier++;

  sanitizeCyclotronMultipliers();
}

int8_t readRotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prev_next_code <<= 2;

  if(digitalReadFast(ROTARY_ENCODER_B)) {
    prev_next_code |= 0x02;
  }

  if(digitalReadFast(ROTARY_ENCODER_A)) {
    prev_next_code |= 0x01;
  }

  prev_next_code &= 0x0f;

  // If valid then store as 16 bit data.
  if(rot_enc_table[prev_next_code]) {
    store <<= 4;
    store |= prev_next_code;

    if((store&0xff) == 0x2b) {
      return -1;
    }

    if((store&0xff) == 0x17) {
      return 1;
    }
  }

  return 0;
}

void checkRotaryEncoder() {
  if(readRotary() != 0) {
    // Only continue if the limiter has expired.
    if(ms_rotary_encoder.remaining() > 0) {
      return;
    }
    else {
      ms_rotary_encoder.start(i_rotary_encoder_delay);
    }

    // Clockwise
    if(prev_next_code == 0x0b) {
      increaseVolume();
    }

    // Counter Clockwise
    if(prev_next_code == 0x07) {
      decreaseVolume();
    }
  }
}

void restartFromWandMash() {
  if(b_wand_mash_lockout) {
    b_wand_mash_lockout = false;
    b_fading_out_frozen = false;
    stopMashErrorSounds();

    if(PACK_STATE == MODE_ON) {
      switch(gpstarPack.getSystemTheme()) {
        case SYSTEM_FROZEN_EMPIRE:
          // Play pack restart sound depending on lid on/off.
          playEffect(S_PACK_RECOVERY);
          if(gpstarPack.inStreamMode(SLIME)) {
            playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level, true, 500);
          }
          else {
            playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects, true, 2000);
          }
          if(b_brass_pack_sound_loop) {
            if(b_brass_startup_loop) {
              playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP, true, i_volume_effects, true, 2000);
            }
            else {
              playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
              playEffect(S_FROZEN_EMPIRE_BRASS_IDLE, true, i_volume_effects, true, 2300);
            }
          }

          // Reset the lighting timers.
          b_ramp_down = false;
          b_inner_ramp_down = false;
          resetRampUp();
          ms_mash_lockout.stop();
          if(gpstarPack.getStreamMode() != SLIME) {
            // If not in slime mode, stop the ice effect timer and melt the ice.
            cyclotronLidLedsOff();
            innerCyclotronCakeOff();
            ms_cyclotron_slime_effect.stop();
          }
          ms_powercell.start(0);
          ms_cyclotron.start(0);
          ms_cyclotron_ring.start(0);

          // Restart the Cyclotron motor, if enabled.
          if(gpstarPack.getVibrationMode() == CYCLOTRON_MOTOR && b_vibration_switch_on) {
            digitalWrite(VIBRATION_PIN, HIGH);
          }
        break;
        default:
          // Play pack restart sound.
          playEffect(S_SMASH_ERROR_RESTART);
        break;
      }
    }

    // Notify the Attenuator we are no longer locked out.
    attenuatorSerialSend(A_CANCEL_LOCKOUT);
  }
}

void wandExtraSoundsBeepLoop() {
  if(!b_overheating) {
    // Explicitly stop beeps first just in case it tries to double up.
    wandExtraSoundsBeepLoopStop(false);

    switch(gpstarPack.getPowerLevel()) {
      case LEVEL_1:
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_AFTERLIFE_BEEP_WAND_S1, true, i_volume_effects - i_slime_idle_level);
        }
        else {
          playEffect(S_AFTERLIFE_BEEP_WAND_S1, true, i_volume_effects - i_wand_idle_level);
        }
      break;

      case LEVEL_2:
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_AFTERLIFE_BEEP_WAND_S2, true, i_volume_effects - i_slime_idle_level);
        }
        else {
          playEffect(S_AFTERLIFE_BEEP_WAND_S2, true, i_volume_effects - i_wand_idle_level);
        }
      break;

      case LEVEL_3:
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_AFTERLIFE_BEEP_WAND_S3, true, i_volume_effects - i_slime_idle_level);
        }
        else {
          playEffect(S_AFTERLIFE_BEEP_WAND_S3, true, i_volume_effects - i_wand_idle_level);
        }
      break;

      case LEVEL_4:
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_AFTERLIFE_BEEP_WAND_S4, true, i_volume_effects - i_slime_idle_level);
        }
        else {
          playEffect(S_AFTERLIFE_BEEP_WAND_S4, true, i_volume_effects - i_wand_idle_level);
        }
      break;

      case LEVEL_5:
      default:
        if(gpstarPack.inStreamMode(SLIME)) {
          playEffect(S_AFTERLIFE_BEEP_WAND_S5, true, i_volume_effects - i_slime_idle_level);
        }
        else {
          playEffect(S_AFTERLIFE_BEEP_WAND_S5, true, i_volume_effects - i_wand_idle_level);
        }
      break;
    }
  }
}

// It is very important that S_1 up to S_60 follow each other in order on the Micro SD Card and sound effects enum.
void overheatVoiceIndicator(uint16_t i_tmp_length) {
  i_tmp_length = i_tmp_length / i_overheat_delay_increment;

  uint16_t i_tmp_sound = (S_1 - 1) + i_tmp_length;

  stopEffect(i_tmp_sound - 1);
  stopEffect(i_tmp_sound);
  stopEffect(i_tmp_sound + 1);
  playEffect(i_tmp_sound);
}

void overheatIncrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheating_length_5 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_5 = i_ms_overheating_length_5 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_5);
      }
    break;

    case 4:
      if(i_ms_overheating_length_4 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_4 = i_ms_overheating_length_4 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_4);
      }
    break;

    case 3:
      if(i_ms_overheating_length_3 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_3 = i_ms_overheating_length_3 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_3);
      }
    break;

    case 2:
      if(i_ms_overheating_length_2 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_2 = i_ms_overheating_length_2 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheating_length_1 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_1 = i_ms_overheating_length_1 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_1);
      }
    break;
  }
}

void overheatDecrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheating_length_5 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_5 = i_ms_overheating_length_5 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_5);
      }
    break;

    case 4:
      if(i_ms_overheating_length_4 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_4 = i_ms_overheating_length_4 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_4);
      }
    break;

    case 3:
      if(i_ms_overheating_length_3 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_3 = i_ms_overheating_length_3 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_3);
      }
    break;

    case 2:
      if(i_ms_overheating_length_2 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_2 = i_ms_overheating_length_2 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheating_length_1 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_1 = i_ms_overheating_length_1 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_1);
      }
    break;
  }
}

// Update the LED counts for the Proton Pack.
void updateProtonPackLEDCounts() {
  // Calculate the "pack" which is the standard Powercell plus Cyclotron Lid,
  // and optionally an N-filter LED jewel array at the end of that chain
  i_pack_num_leds = i_powercell_num_leds + i_cyclotron_num_leds + i_nfilter_jewel_leds;
  i_cyclotron_led_start = i_powercell_num_leds;
  i_vent_light_start = i_powercell_num_leds + i_cyclotron_num_leds;

  // Calculate the inner cyclotron which may consist of the optional components:
  // [in order...] Switch Panel + Cake Lights + Cavity Lights
  if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
    i_ic_panel_end = i_inner_cyclotron_panel_num_leds - 1;
    i_ic_cake_start = i_ic_panel_end + 1;
    i_ic_cake_end = i_ic_cake_start + i_inner_cyclotron_cake_num_leds - 1;
    i_ic_cavity_start = i_ic_cake_end + 1;
    i_ic_cavity_end = i_ic_cavity_start + i_inner_cyclotron_cavity_num_leds - 1;
  }
  else {
    i_ic_panel_end = 0;
    i_ic_cake_start = 0;
    i_ic_cake_end = i_ic_cake_start + i_inner_cyclotron_cake_num_leds - 1;
    i_ic_cavity_start = i_ic_cake_end + 1;
    i_ic_cavity_end = i_ic_cavity_start + i_inner_cyclotron_cavity_num_leds - 1;
  }
}

// Update the LED counts for the inner cyclotron, if we are using the addon LED panel or not.
void resetInnerCyclotronLEDs() {
  if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
    // For clarity, these are added in the order by which the devices would be connected in the chain.
    i_inner_cyclotron_panel_num_leds = INNER_CYCLOTRON_LED_PANEL_MAX; // Maximum is 8 (2 above switches, 6 on the side)
  }
  else {
    // Without the inner panel we just use the dedicated LED ports on the controller for single-colour LEDs.
    // The inner chain just reduces count to the inner cake plus extra cavity lights for the "sparking" FX.
    i_inner_cyclotron_panel_num_leds = 0; // Set to 0 when this device is not enabled for use.
  }
}

void resetCyclotronLEDs() {
  switch(i_cyclotron_num_leds) {
    // For a 40 LED Neopixel ring.
    case OUTER_CYCLOTRON_LED_MAX:
      i_2021_delay = CYCLOTRON_DELAY_2021_40_LED;
    break;

    // For Frutto Technology Max Cyclotron (36) LEDs.
    case MAX_CYCLOTRON_LED_COUNT:
    default:
      i_2021_delay = CYCLOTRON_DELAY_2021_36_LED;
    break;

    // For Frutto Technology Cyclotron (20) LEDs.
    case FRUTTO_CYCLOTRON_LED_COUNT:
      i_2021_delay = CYCLOTRON_DELAY_2021_20_LED;
    break;

    // Default HasLab (12) LEDs.
    case HASLAB_CYCLOTRON_LED_COUNT:
      i_2021_delay = CYCLOTRON_DELAY_2021_12_LED;
    break;
  }
}

void updateContinuousSmoke() {
  b_smoke_continuous_level[0] = b_smoke_continuous_level_1;
  b_smoke_continuous_level[1] = b_smoke_continuous_level_2;
  b_smoke_continuous_level[2] = b_smoke_continuous_level_3;
  b_smoke_continuous_level[3] = b_smoke_continuous_level_4;
  b_smoke_continuous_level[4] = b_smoke_continuous_level_5;
}

void startWandMashLockout(uint16_t i_timeout) {
  if(!b_wand_mash_lockout) {
    if(PACK_STATE == MODE_ON) {
      if(b_wand_firing) {
        wandStoppedFiring();
      }

      switch(gpstarPack.getStreamMode()) {
        case PROTON:
        default:
          switch(gpstarPack.getSystemTheme()) {
            case SYSTEM_1984:
              if(gpstarPack.getPowerLevel() != MAX_POWER_LEVEL) {
                stopEffect(S_FIRING_END);
                stopEffect(S_FIRING_END_MID);
                stopEffect(S_GB1_1984_FIRE_END_SHORT);
              }
              else {
                stopEffect(S_GB1_1984_FIRE_END_HIGH_POWER);
                stopEffect(S_GB1_1984_FIRE_END_MID_HIGH_POWER);
                stopEffect(S_GB1_1984_FIRE_END_SHORT_HIGH_POWER);
              }
            break;
            case SYSTEM_1989:
              stopEffect(S_FIRING_END_GUN);
              stopEffect(S_FIRING_END_MID);
              stopEffect(S_FIRING_END);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              stopEffect(S_AFTERLIFE_FIRE_END_SHORT);
              stopEffect(S_AFTERLIFE_FIRE_END_MID);
              stopEffect(S_AFTERLIFE_FIRE_END_LONG);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              stopEffect(S_AFTERLIFE_FIRE_END_MID);
            break;
          }

          stopEffect(S_CROSS_STREAMS_END);
          stopEffect(S_CROSS_STREAMS_START);
          stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
          stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
        break;
        case SLIME:
          stopEffect(S_SLIME_END);
        break;
        case STASIS:
          stopEffect(S_STASIS_END);
        break;
      }

      // Flag that the button mash error sequence is in effect.
      b_wand_mash_lockout = true;
      stopMashErrorSounds();

      // Notify the Attenuator we are locked out.
      attenuatorSerialSend(A_SYSTEM_LOCKOUT, i_timeout);

      // Play special sounds for the Frozen Empire theme and begin a freeze-up effect.
      if(gpstarPack.getSystemTheme() == SYSTEM_FROZEN_EMPIRE) {
        playEffect(S_FROZEN_EMPIRE_PACK_FREEZE_STOP);
        playEffect(S_STASIS_IDLE_LOOP, true, i_volume_effects, true, 2500);

        if(b_brass_pack_sound_loop) {
          playEffect(S_FROZEN_EMPIRE_BRASS_FIRE_TAIL);
          stopEffect(S_FROZEN_EMPIRE_BRASS_IDLE);
          stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
          stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT_LOOP);
        }

        stopEffect(S_BOOTUP);
        stopEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP);

        // Stop the cyclotron motor, if enabled.
        if(gpstarPack.getVibrationMode() == CYCLOTRON_MOTOR) {
          vibrationOff();
        }

        // Stop all light functions by use of adjusting the timers.
        ms_mash_lockout.start(i_timeout);
        resetRampDown();
      }
    }
  }
}

uint8_t getRampPercentage(ramp &input) {
  // Gets the whole percentage completion of a given ramp.
  return (100 * input.getPosition()) / input.getDuration();
}

void systemPOST() {
  uint8_t i_tmp_led1 = i_cyclotron_led_start + cyclotron84LookupTable(0);
  uint8_t i_tmp_led2 = i_cyclotron_led_start + cyclotron84LookupTable(1);
  uint8_t i_tmp_led3 = i_cyclotron_led_start + cyclotron84LookupTable(2);
  uint8_t i_tmp_led4 = i_cyclotron_led_start + cyclotron84LookupTable(3);
  uint8_t i_tmp_led5 = i_pack_num_leds - 1;

  uint8_t i_tmp_powercell_led = i_post_powercell_up;

  colours c_outer_cyclotron_colour = ((i_cyclotron_num_leds == HASLAB_CYCLOTRON_LED_COUNT && !b_cyclotron_haslab_chsv_colour_change) ? C_WHITE : C_RED);

  if(i_post_powercell_up < i_powercell_num_leds && ms_delay_post.justFinished()) {
    if(b_powercell_invert) {
      if(i_powercell_num_leds == HASLAB_POWERCELL_LED_COUNT) {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_13_invert[i_post_powercell_up]);
      }
      else {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_15_invert[i_post_powercell_up]);

      }
    }
    else {
      if(i_powercell_num_leds == HASLAB_POWERCELL_LED_COUNT) {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_13[i_post_powercell_up]);
      }
      else {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_15[i_post_powercell_up]);
      }
    }

    pack_leds[i_tmp_powercell_led] = getHueAsRGB(POWERCELL, C_MID_BLUE);

    if((i_post_powercell_up % 5) == 0) {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_WHITE);

      if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
        cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_RED);
        cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED);
        cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE);
        cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE);
        cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN);
        cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN);
        cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN);
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE);
      }
#ifndef ESP32
      else {
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
        digitalWriteFast(YEAR_TOGGLE_LED_PIN, HIGH);
        digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, HIGH);
      }
#endif
    }
    else {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

      if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
        cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
      }
#ifndef ESP32
      else {
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
        digitalWriteFast(YEAR_TOGGLE_LED_PIN, LOW);
        digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, LOW);
      }
#endif
    }

    i_post_powercell_up++;

    if(i_post_powercell_up == i_powercell_num_leds) {
      ms_delay_post.stop();
      ms_delay_post_2.start(30);
    }
    else {
      ms_delay_post.start(30);
    }
  }

  if(i_post_powercell_down < i_powercell_num_leds && ms_delay_post_2.justFinished()) {
    if(b_powercell_invert) {
      if(i_powercell_num_leds == HASLAB_POWERCELL_LED_COUNT) {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_13_invert[i_post_powercell_down]);
      }
      else {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_15_invert[i_post_powercell_down]);

      }
    }
    else {
      if(i_powercell_num_leds == HASLAB_POWERCELL_LED_COUNT) {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_13[i_post_powercell_down]);
      }
      else {
        i_tmp_powercell_led = PROGMEM_READU8(powercell_15[i_post_powercell_down]);
      }
    }

    pack_leds[(i_powercell_num_leds - 1) - i_tmp_powercell_led] = getHueAsRGB(POWERCELL, C_BLACK); // Ramp up and ramp down.
    //pack_leds[i_post_powercell_down] = getHueAsRGB(POWERCELL, C_BLACK); // Ramp up and ramp away.

    if((i_post_powercell_down % 5) == 0) {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_WHITE);

      if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
        cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_RED);
        cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED);
        cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE);
        cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE);
        cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN);
        cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN);
        cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN);
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE);
      }
#ifndef ESP32
      else {
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, HIGH);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, HIGH);
        digitalWriteFast(YEAR_TOGGLE_LED_PIN, HIGH);
        digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, HIGH);
      }
#endif
    }
    else {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

      if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
        cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
      }
#ifndef ESP32
      else {
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_R2_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_Y2_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G1_PIN, LOW);
        digitalWriteFast(CYCLOTRON_SWITCH_LED_G2_PIN, LOW);
        digitalWriteFast(YEAR_TOGGLE_LED_PIN, LOW);
        digitalWriteFast(VIBRATION_TOGGLE_LED_PIN, LOW);
      }
#endif
    }

    i_post_powercell_down++;

    if(i_post_powercell_down == i_powercell_num_leds) {
      ms_delay_post_2.stop();
      ms_delay_post_3.start(30);
    }
    else {
      ms_delay_post_2.start(30);
    }
  }

  if(i_post_fade > 0 && ms_delay_post_3.justFinished()) {
    pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour, i_post_fade);
    pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour, i_post_fade);
    pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour, i_post_fade);
    pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, c_outer_cyclotron_colour, i_post_fade);
    pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_WHITE, i_post_fade);

    if(INNER_CYC_PANEL_MODE != PANEL_DISABLED) {
      cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_post_fade);
      cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_post_fade);
      cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_post_fade);
      cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_post_fade);
      cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_post_fade);
      cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_post_fade);
      cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_GREEN, i_post_fade);
      cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_ORANGE, i_post_fade);
    }

    uint8_t i_inner_cake_divisor = 7;
    uint8_t i_inner_cake_counter = 0;
    switch(i_inner_cyclotron_cake_num_leds) {
      case 36:
      case 35:
      default:
        // Do nothing; 7 is already the correct divisor.
      break;
      case 26:
      case 24:
      case 23:
        i_inner_cake_divisor = 9;
      break;
      case 12:
        i_inner_cake_divisor = 21;
      break;
    }

    if(b_clockwise) {
      i_inner_cake_counter = ((255 - i_post_fade) / i_inner_cake_divisor) + i_ic_cake_start;
    }
    else {
      i_inner_cake_counter = (i_post_fade / i_inner_cake_divisor) + i_ic_cake_start;
    }

    if(i_inner_cake_counter <= i_ic_cake_end) {
      if(CAKE_LED_TYPE == GRB_LED) {
        cyclotron_leds[i_inner_cake_counter] = getHueAsGRB(CYCLOTRON_INNER, C_RED);
      }
      else {
        cyclotron_leds[i_inner_cake_counter] = getHueAsRGB(CYCLOTRON_INNER, C_RED);
      }
    }

    if(b_clockwise) {
      if(i_inner_cake_counter - 1 >= i_ic_cake_start && i_inner_cake_counter - 1 <= i_ic_cake_end) {
        cyclotron_leds[i_inner_cake_counter - 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
      }
    }
    else {
      if(i_inner_cake_counter + 1 <= i_ic_cake_end) {
        cyclotron_leds[i_inner_cake_counter + 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
      }
    }

    i_post_fade--;

    if(i_post_fade == 0) {
      ms_delay_post_3.stop();

      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

      cyclotronSwitchLEDOff();
      innerCyclotronCakeOff();

      packSerialSend(P_POST_FINISH);

      b_pack_post_finish = true;
      updateLEDs();
      delay(FAST_LED_UPDATE_MS); // Delay to give the LEDs a chance to finish updating.
    }
    else {
      ms_delay_post_3.start(5);
    }
  }

  // Push the update to the LEDs.
  updateLEDs();
}

void resetWifiCommand() {
  bool b_reset_success = false;
  #ifdef ESP32
    // If GPStar Pack II, reset our own Wifi password.
    wirelessMgr->resetWifiPassword();
    b_reset_success = true;
  #else
    // If not ESP32, send a message to the wireless module to have it reset its password.
    if(b_attenuator_connected) {
      attenuatorSerialSend(A_RESET_WIFI_PASSWORD);
      b_reset_success = true;
    }
  #endif

  // Play voice confirmation if successful.
  if(b_reset_success) {
    playEffect(S_VOICE_PACK_WIFI_RESET);
  }
}

/*
 * Prevent stream mode change if wand is firing, in an error state, or VG modes are disabled.
 * Note this is only used for the web UI, not Wand/Attenuator switching.
 */
bool canChangeStreamMode() {
  if(PACK_STATE != MODE_ON) {
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

  if(gpstarPack.getSystemMode() == MODE_ORIGINAL) {
    // Original mode does not support VG stream modes.
    sendDebug(F("canChangeStreamMode() -> BLOCKED: System is in original mode"));
    return false;
  }

  if(!gpstarPack.supportsAnyAlternateStreams()) {
    // At a minimum one or more VG modes must be enabled to allow changes beyond PROTON.
    sendDebug(F("canChangeStreamMode() -> BLOCKED: no VG or Spectral modes are enabled"));
    return false;
  }

  return true;
}

/*
 * Change the current stream mode to a new mode, if allowed.
 * Note this is only used for the web UI, not Wand/Attenuator switching.
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

  // Continue to change the stream mode.
  executeCommand(A_SET_STREAM_MODE, (uint8_t)new_mode);
  ms_streamchange.start(i_stream_change_delay); // Restart debounce timer.
  return true;
}

#ifdef ESP32
void readTemperature() {
  // Read the HDC1080 and store the current temperature readings in C and F.
  if(b_temp_sensor_detected) {
    if(!ms_temp_read.isRunning()) {
      tempSensor.startAcquisition(GuL::HDC1080::Channel::TEMPERATURE);
      ms_temp_read.start(i_temp_read_delay); // Read every N seconds.
    }
    else if(ms_temp_read.justFinished()) {
      f_temperature_c = tempSensor.getTemperature(); // Read value in Celsius
      f_temperature_f = (f_temperature_c * 1.8) + 32; // Convert Celsius to Fahrenheit
      debugf("\t\tTemp: %.1f C (%.1f F)\n", f_temperature_c, f_temperature_f);

      // Send value to the Attenuator, multiplied by 100 to avoid float issues.
      attenuatorSerialSend(A_TEMPERATURE_PACK, f_temperature_c * 100);
    }
  }
}
#endif