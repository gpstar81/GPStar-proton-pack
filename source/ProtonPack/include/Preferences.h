/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

/*
 * User Preference Storage/Retrieval via EEPROM
 *
 * This library of functions controls the storing, clearing, reading, and management
 * of user preferences by way of the EEPROM storage area of the ATMega2560 chipset.
 * Values are stored a simple integer types, though they are mapped to boolean and
 * other datatypes as necessary during the reading/storing actions. Additionally,
 * a user may not even be using the EEPROM storage area and thus defaults may be
 * set when values are not present.
 */

/*
 * Function prototypes.
 */
void readEEPROM();
void clearConfigEEPROM();
void clearLEDEEPROM();
void saveConfigEEPROM();
void saveLEDEEPROM();
void updateCRCEEPROM();
uint32_t eepromCRC(void);
void resetCyclotronLEDs();
void resetInnerCyclotronLEDs();
void resetContinuousSmoke();
void updateProtonPackLEDCounts();

/*
 * General EEPROM Variables
 */
uint16_t i_eepromAddress = 0; // The address in the EEPROM to start reading from.

/*
 * Data structure object for LED settings which are saved into the EEPROM memory.
 */
struct objLEDEEPROM {
  uint8_t powercell_count;
  uint8_t cyclotron_count;
  uint8_t inner_cyclotron_count;
  uint8_t grb_inner_cyclotron;
  uint8_t powercell_spectral_custom;
  uint8_t cyclotron_spectral_custom;
  uint8_t cyclotron_inner_spectral_custom;
  uint8_t powercell_spectral_saturation_custom;
  uint8_t cyclotron_spectral_saturation_custom;
  uint8_t cyclotron_inner_spectral_saturation_custom;
  uint8_t powercell_brightness;
  uint8_t cyclotron_brightness;
  uint8_t inner_cyclotron_brightness;
  uint8_t inner_panel_brightness;
  uint8_t cyclotron_cavity_count;
  uint8_t cyclotron_cavity_type;
  uint8_t inner_cyclotron_led_panel;
  uint8_t powercell_inverted;
};

/*
 * Data structure object for customizations which are saved into the EEPROM memory.
 */
struct objConfigEEPROM {
  uint8_t stream_effects;
  uint8_t cyclotron_direction;
  uint8_t center_led_fade; // Used for the 1984/1989 themes.
  uint8_t simulate_ring; // Used for the Afterlife/Frozen Empire themes.
  uint8_t smoke_setting;
  uint8_t overheat_strobe;
  uint8_t overheat_lights_off;
  uint8_t overheat_sync_to_fan;
  uint8_t year_mode; // 1984, 1989, Afterlife, Frozen Empire or the Proton Pack toggle switch default.
  uint8_t system_mode; // Super Hero or Mode Original.
  uint8_t vg_powercell; // For disabling or enabling video game colours for the Power Cell.
  uint8_t vg_cyclotron; // For disabling or enabling video game colours for the Cyclotron.
  uint8_t demo_light_mode; // Enables pack startup automatically at bootup (battery power-on).
  uint8_t cyclotron_three_led_toggle; // Toggles between the 1-LED or 3-LED for 84/89 modes.
  uint8_t default_system_volume; // Default master volume at bootup (battery power-on)
  uint8_t overheat_smoke_duration_level_5;
  uint8_t overheat_smoke_duration_level_4;
  uint8_t overheat_smoke_duration_level_3;
  uint8_t overheat_smoke_duration_level_2;
  uint8_t overheat_smoke_duration_level_1;
  uint8_t smoke_continuous_level_5;
  uint8_t smoke_continuous_level_4;
  uint8_t smoke_continuous_level_3;
  uint8_t smoke_continuous_level_2;
  uint8_t smoke_continuous_level_1;
  uint8_t pack_vibration; // Sets the vibration mode for the pack (multi-option).
  uint8_t use_ribbon_cable; // Enable/disable the ribbon cable alarm (useful for DIY packs).
};

/*
 * Read all user preferences from Proton Pack controller EEPROM.
 */
void readEEPROM() {
  // Get the stored CRC from the EEPROM.
  uint32_t l_crc_check;
  EEPROM.get(EEPROM.length() - sizeof(eepromCRC()), l_crc_check);

  // Check if the calculated CRC matches the stored CRC value in the EEPROM.
  if(eepromCRC() == l_crc_check) {
    // Read our LED object from the EEPROM.
    objLEDEEPROM obj_led_eeprom;
    EEPROM.get(i_eepromAddress, obj_led_eeprom);

    if(obj_led_eeprom.powercell_count > 0 && obj_led_eeprom.powercell_count != 255) {
      i_powercell_leds = obj_led_eeprom.powercell_count;

      switch(i_powercell_leds) {
        case FRUTTO_POWERCELL_LED_COUNT:
          // 15 Power Cell LEDs.
          i_powercell_delay_1984 = POWERCELL_DELAY_1984_15_LED;
          i_powercell_delay_2021 = POWERCELL_DELAY_2021_15_LED;
        break;

        case HASLAB_POWERCELL_LED_COUNT:
        default:
          // 13 Power Cell LEDs.
          i_powercell_delay_1984 = POWERCELL_DELAY_1984_13_LED;
          i_powercell_delay_2021 = POWERCELL_DELAY_2021_13_LED;
        break;
      }
    }
    else if(!b_power_meter_available) {
      // If no EEPROM default set and not using a stock wand, assume Frutto upgrades instead.
      i_powercell_leds = FRUTTO_POWERCELL_LED_COUNT;
      i_powercell_delay_1984 = POWERCELL_DELAY_1984_15_LED;
      i_powercell_delay_2021 = POWERCELL_DELAY_2021_15_LED;
    }

    if(obj_led_eeprom.cyclotron_count > 0 && obj_led_eeprom.cyclotron_count != 255) {
      i_cyclotron_leds = obj_led_eeprom.cyclotron_count;
    }
    else if(!b_power_meter_available) {
      // If no EEPROM default set and not using a stock wand, assume Frutto upgrades instead.
      i_cyclotron_leds = FRUTTO_MAX_CYCLOTRON_LED_COUNT;
    }

    if(obj_led_eeprom.inner_cyclotron_count > 0 && obj_led_eeprom.inner_cyclotron_count != 255) {
      i_inner_cyclotron_cake_num_leds = obj_led_eeprom.inner_cyclotron_count;

      switch(i_inner_cyclotron_cake_num_leds) {
        case 12:
          i_1984_inner_delay = INNER_CYCLOTRON_DELAY_1984_12_LED;
          i_2021_inner_delay = INNER_CYCLOTRON_DELAY_2021_12_LED;
        break;

        case 23:
          i_1984_inner_delay = INNER_CYCLOTRON_DELAY_1984_23_LED;
          i_2021_inner_delay = INNER_CYCLOTRON_DELAY_2021_23_LED;
        break;

        case 24:
          i_1984_inner_delay = INNER_CYCLOTRON_DELAY_1984_24_LED;
          i_2021_inner_delay = INNER_CYCLOTRON_DELAY_2021_24_LED;
        break;

        case 26:
          i_1984_inner_delay = INNER_CYCLOTRON_DELAY_1984_26_LED;
          i_2021_inner_delay = INNER_CYCLOTRON_DELAY_2021_26_LED;
        break;

        case 35:
          i_1984_inner_delay = INNER_CYCLOTRON_DELAY_1984_35_LED;
          i_2021_inner_delay = INNER_CYCLOTRON_DELAY_2021_35_LED;
        break;

        case 36:
        default:
          i_1984_inner_delay = INNER_CYCLOTRON_DELAY_1984_36_LED;
          i_2021_inner_delay = INNER_CYCLOTRON_DELAY_2021_36_LED;
        break;
      }
    }

    if(obj_led_eeprom.cyclotron_cavity_count > 0 && obj_led_eeprom.cyclotron_cavity_count != 255) {
      if(obj_led_eeprom.cyclotron_cavity_count > 20) {
        i_inner_cyclotron_cavity_num_leds = 20;
      }
      else {
        i_inner_cyclotron_cavity_num_leds = obj_led_eeprom.cyclotron_cavity_count;
      }
    }

    if(obj_led_eeprom.cyclotron_cavity_type > 0 && obj_led_eeprom.cyclotron_cavity_type != 255) {
      if(obj_led_eeprom.cyclotron_cavity_type > 1) {
        // 2 = RGB, 3 = GRB, 4 = GBR.
        switch(obj_led_eeprom.cyclotron_cavity_type) {
          case 2:
          default:
            CAVITY_LED_TYPE = RGB_LED;
          break;

          case 3:
            CAVITY_LED_TYPE = GRB_LED;
          break;

          case 4:
            CAVITY_LED_TYPE = GBR_LED;
          break;
        }
      }
    }

    if(obj_led_eeprom.powercell_inverted > 0 && obj_led_eeprom.powercell_inverted != 255) {
      if(obj_led_eeprom.powercell_inverted > 1) {
        b_powercell_invert = true;
      }
      else {
        b_powercell_invert = false;
      }
    }

    if(obj_led_eeprom.inner_cyclotron_led_panel > 0 && obj_led_eeprom.inner_cyclotron_led_panel != 255) {
      if(obj_led_eeprom.inner_cyclotron_led_panel > 1) {
        // 2 = Individual, 3 = RGB Static, 4 = RGB Dynamic.
        switch(obj_led_eeprom.inner_cyclotron_led_panel) {
          case 2:
          default:
            INNER_CYC_PANEL_MODE = PANEL_INDIVIDUAL;
          break;

          case 3:
            INNER_CYC_PANEL_MODE = PANEL_RGB_STATIC;
          break;

          case 4:
            INNER_CYC_PANEL_MODE = PANEL_RGB_DYNAMIC;
          break;
        }
      }
    }

    if(obj_led_eeprom.grb_inner_cyclotron > 0 && obj_led_eeprom.grb_inner_cyclotron != 255) {
      if(obj_led_eeprom.grb_inner_cyclotron > 1) {
        CAKE_LED_TYPE = GRB_LED;
      }
      else {
        CAKE_LED_TYPE = RGB_LED;
      }
    }

    if(obj_led_eeprom.powercell_spectral_custom > 0 && obj_led_eeprom.powercell_spectral_custom != 255) {
      i_spectral_powercell_custom_colour = obj_led_eeprom.powercell_spectral_custom;
    }

    if(obj_led_eeprom.cyclotron_spectral_custom > 0 && obj_led_eeprom.cyclotron_spectral_custom != 255) {
      i_spectral_cyclotron_custom_colour = obj_led_eeprom.cyclotron_spectral_custom;
    }

    if(obj_led_eeprom.cyclotron_inner_spectral_custom > 0 && obj_led_eeprom.cyclotron_inner_spectral_custom != 255) {
      i_spectral_cyclotron_inner_custom_colour = obj_led_eeprom.cyclotron_inner_spectral_custom;
    }

    if(obj_led_eeprom.powercell_spectral_saturation_custom > 0 && obj_led_eeprom.powercell_spectral_saturation_custom != 255) {
      i_spectral_powercell_custom_saturation = obj_led_eeprom.powercell_spectral_saturation_custom;
    }

    if(obj_led_eeprom.cyclotron_spectral_saturation_custom > 0 && obj_led_eeprom.cyclotron_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_custom_saturation = obj_led_eeprom.cyclotron_spectral_saturation_custom;
    }

    if(obj_led_eeprom.cyclotron_inner_spectral_saturation_custom > 0 && obj_led_eeprom.cyclotron_inner_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_inner_custom_saturation = obj_led_eeprom.cyclotron_inner_spectral_saturation_custom;
    }

    if(obj_led_eeprom.powercell_brightness > 19 && obj_led_eeprom.powercell_brightness != 255) {
      i_powercell_brightness = obj_led_eeprom.powercell_brightness;
    }

    if(obj_led_eeprom.cyclotron_brightness > 19 && obj_led_eeprom.cyclotron_brightness != 255) {
      i_cyclotron_brightness = obj_led_eeprom.cyclotron_brightness;
    }

    if(obj_led_eeprom.inner_cyclotron_brightness > 19 && obj_led_eeprom.inner_cyclotron_brightness != 255) {
      i_cyclotron_inner_brightness = obj_led_eeprom.inner_cyclotron_brightness;
    }

    if(obj_led_eeprom.inner_panel_brightness > 19 && obj_led_eeprom.inner_panel_brightness != 255) {
      i_cyclotron_panel_brightness = obj_led_eeprom.inner_panel_brightness;
    }

    // Update the LED counts for the Proton Pack.
    resetCyclotronLEDs();
    resetInnerCyclotronLEDs();
    updateProtonPackLEDCounts();

    // Read our configuration object from the EEPROM.
    objConfigEEPROM obj_config_eeprom;
    uint16_t i_eepromConfigAddress = i_eepromAddress + sizeof(objLEDEEPROM);

    EEPROM.get(i_eepromConfigAddress, obj_config_eeprom);

    if(obj_config_eeprom.stream_effects > 0 && obj_config_eeprom.stream_effects != 255) {
      if(obj_config_eeprom.stream_effects > 1) {
        b_stream_effects = true;
      }
      else {
        b_stream_effects = false;
      }
    }

    if(obj_config_eeprom.cyclotron_direction > 0 && obj_config_eeprom.cyclotron_direction != 255) {
      if(obj_config_eeprom.cyclotron_direction > 1) {
        b_clockwise = true;
      }
      else {
        b_clockwise = false;
      }
    }

    if(obj_config_eeprom.center_led_fade > 0 && obj_config_eeprom.center_led_fade != 255) {
      if(obj_config_eeprom.center_led_fade > 1) {
        b_fade_cyclotron_led = true;
      }
      else {
        b_fade_cyclotron_led = false;
      }
    }

    if(obj_config_eeprom.simulate_ring > 0 && obj_config_eeprom.simulate_ring != 255) {
      if(obj_config_eeprom.simulate_ring > 1) {
        b_cyclotron_simulate_ring = true;
      }
      else {
        b_cyclotron_simulate_ring = false;
      }
    }

    if(obj_config_eeprom.smoke_setting > 0 && obj_config_eeprom.smoke_setting != 255) {
      if(obj_config_eeprom.smoke_setting > 1) {
        b_smoke_enabled = true;
      }
      else {
        b_smoke_enabled = false;
      }
    }

    if(obj_config_eeprom.overheat_strobe > 0 && obj_config_eeprom.overheat_strobe != 255) {
      if(obj_config_eeprom.overheat_strobe > 1) {
        b_overheat_strobe = true;
      }
      else {
        b_overheat_strobe = false;
      }
    }

    if(obj_config_eeprom.overheat_lights_off > 0 && obj_config_eeprom.overheat_lights_off != 255) {
      if(obj_config_eeprom.overheat_lights_off > 1) {
        b_overheat_lights_off = true;
      }
      else {
        b_overheat_lights_off = false;
      }
    }

    if(obj_config_eeprom.overheat_sync_to_fan > 0 && obj_config_eeprom.overheat_sync_to_fan != 255) {
      if(obj_config_eeprom.overheat_sync_to_fan > 1) {
        b_overheat_sync_to_fan = true;
      }
      else {
        b_overheat_sync_to_fan = false;
      }
    }

    if(obj_config_eeprom.year_mode > 1 && obj_config_eeprom.year_mode != 255) {
      // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
      switch(obj_config_eeprom.year_mode) {
        case 2:
          SYSTEM_YEAR = SYSTEM_1984;
        break;

        case 3:
          SYSTEM_YEAR = SYSTEM_1989;
        break;

        case 4:
        default:
          SYSTEM_YEAR = SYSTEM_AFTERLIFE;
        break;

        case 5:
          SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
        break;
      }

      // Update additional variables once the system year is set from the stored EEPROM preferences.
      SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
      SYSTEM_EEPROM_YEAR = SYSTEM_YEAR;

      // Set the switch override to true, so the toggle switch in the Proton Pack does not override the year settings during the bootup process.
      b_switch_mode_override = true;
    }

    if(obj_config_eeprom.system_mode > 0 && obj_config_eeprom.system_mode != 255) {
      if(obj_config_eeprom.system_mode > 1) {
        SYSTEM_MODE = MODE_ORIGINAL;
      }
      else {
        SYSTEM_MODE = MODE_SUPER_HERO;
      }
    }

    if(obj_config_eeprom.vg_powercell > 0 && obj_config_eeprom.vg_powercell != 255) {
      if(obj_config_eeprom.vg_powercell > 1) {
        b_powercell_colour_toggle = true;
      }
      else {
        b_powercell_colour_toggle = false;
      }
    }

    if(obj_config_eeprom.vg_cyclotron > 0 && obj_config_eeprom.vg_cyclotron != 255) {
      if(obj_config_eeprom.vg_cyclotron > 1) {
        b_cyclotron_colour_toggle = true;
      }
      else {
        b_cyclotron_colour_toggle = false;
      }
    }

    if(obj_config_eeprom.demo_light_mode > 0 && obj_config_eeprom.demo_light_mode != 255) {
      if(obj_config_eeprom.demo_light_mode > 1) {
        b_demo_light_mode = true;
      }
      else {
        b_demo_light_mode = false;
      }
    }

    if(obj_config_eeprom.use_ribbon_cable > 0 && obj_config_eeprom.use_ribbon_cable != 255) {
      if(obj_config_eeprom.use_ribbon_cable > 1) {
        b_use_ribbon_cable = true;
      }
      else {
        b_use_ribbon_cable = false;
      }
    }

    if(obj_config_eeprom.cyclotron_three_led_toggle > 0 && obj_config_eeprom.cyclotron_three_led_toggle != 255) {
      if(obj_config_eeprom.cyclotron_three_led_toggle > 1) {
        b_cyclotron_single_led = false;
      }
      else {
        b_cyclotron_single_led = true;
      }
    }

    if(obj_config_eeprom.default_system_volume > 0 && obj_config_eeprom.default_system_volume <= 101) {
      // EEPROM value is from 1 to 101; subtract 1 to get the correct percentage.
      i_volume_master_percentage = obj_config_eeprom.default_system_volume - 1;
      i_volume_master_eeprom = (MINIMUM_VOLUME + i_volume_min_adj) - ((MINIMUM_VOLUME + i_volume_min_adj) * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_5 > 0 && obj_config_eeprom.overheat_smoke_duration_level_5 != 255) {
      i_ms_overheating_length_5 = obj_config_eeprom.overheat_smoke_duration_level_5 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_4 > 0 && obj_config_eeprom.overheat_smoke_duration_level_4 != 255) {
      i_ms_overheating_length_4 = obj_config_eeprom.overheat_smoke_duration_level_4 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_3 > 0 && obj_config_eeprom.overheat_smoke_duration_level_3 != 255) {
      i_ms_overheating_length_3 = obj_config_eeprom.overheat_smoke_duration_level_3 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_2 > 0 && obj_config_eeprom.overheat_smoke_duration_level_2 != 255) {
      i_ms_overheating_length_2 = obj_config_eeprom.overheat_smoke_duration_level_2 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_1 > 0 && obj_config_eeprom.overheat_smoke_duration_level_1 != 255) {
      i_ms_overheating_length_1 = obj_config_eeprom.overheat_smoke_duration_level_1 * 1000;
    }

    if(obj_config_eeprom.smoke_continuous_level_5 > 0 && obj_config_eeprom.smoke_continuous_level_5 != 255) {
      if(obj_config_eeprom.smoke_continuous_level_5 > 1) {
        b_smoke_continuous_level_5 = true;
      }
      else {
        b_smoke_continuous_level_5 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_level_4 > 0 && obj_config_eeprom.smoke_continuous_level_4 != 255) {
      if(obj_config_eeprom.smoke_continuous_level_4 > 1) {
        b_smoke_continuous_level_4 = true;
      }
      else {
        b_smoke_continuous_level_4 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_level_3 > 0 && obj_config_eeprom.smoke_continuous_level_3 != 255) {
      if(obj_config_eeprom.smoke_continuous_level_3 > 1) {
        b_smoke_continuous_level_3 = true;
      }
      else {
        b_smoke_continuous_level_3 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_level_2 > 0 && obj_config_eeprom.smoke_continuous_level_2 != 255) {
      if(obj_config_eeprom.smoke_continuous_level_2 > 1) {
        b_smoke_continuous_level_2 = true;
      }
      else {
        b_smoke_continuous_level_2 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_level_1 > 0 && obj_config_eeprom.smoke_continuous_level_1 != 255) {
      if(obj_config_eeprom.smoke_continuous_level_1 > 1) {
        b_smoke_continuous_level_1 = true;
      }
      else {
        b_smoke_continuous_level_1 = false;
      }
    }

    if(obj_config_eeprom.pack_vibration > 0 && obj_config_eeprom.pack_vibration != 255) {
      switch(obj_config_eeprom.pack_vibration) {
        case 5:
          VIBRATION_MODE_EEPROM = CYCLOTRON_MOTOR;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 4:
        default:
          // Vibrate while firing only, on/off determined by switch.
          VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
        break;

        case 3:
          VIBRATION_MODE_EEPROM = VIBRATION_NONE;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 2:
          b_vibration_switch_on = true; // Override the vibration toggle switch.
          VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 1:
          b_vibration_switch_on = true; // Override the vibration toggle switch.
          VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;
      }
    }
  }
  else {
    // CRC doesn't match; let's clear the EEPROMs to be safe.
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);

    clearConfigEEPROM();
    clearLEDEEPROM();
  }

  resetContinuousSmoke();
}

void clearLEDEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(uint16_t i = 0; i < sizeof(objLEDEEPROM); i++) {
    EEPROM.update(i, 0);
  }

  updateCRCEEPROM();
}

void saveLEDEEPROM() {
  // Power Cell LEDs
  // Cyclotron LEDs
  // Inner Cyclotron LEDs
  // Inner Cyclotron LED Panel toggle flag

  // GRB / RGB Inner Cyclotron toggle flag
  uint8_t i_grb_cyclotron_cake = 1;
  if(CAKE_LED_TYPE == GRB_LED) {
    i_grb_cyclotron_cake = 2;
  }

  // 2 = RGB, 3 = GRB, 4 = GBR.
  uint8_t i_inner_cyclotron_cavity_led_type = 2;
  switch(CAVITY_LED_TYPE) {
    case RGB_LED:
    default:
      i_inner_cyclotron_cavity_led_type = 2;
    break;

    case GRB_LED:
      i_inner_cyclotron_cavity_led_type = 3;
    break;

    case GBR_LED:
      i_inner_cyclotron_cavity_led_type = 4;
    break;
  }

  // 2 = Individual, 3 = RGB Static, 4 = RGB Dynamic.
  uint8_t i_inner_cyclotron_led_panel = 2;
  switch(INNER_CYC_PANEL_MODE) {
    case PANEL_INDIVIDUAL:
    default:
      i_inner_cyclotron_led_panel = 2;
    break;

    case PANEL_RGB_STATIC:
      i_inner_cyclotron_led_panel = 3;
    break;

    case PANEL_RGB_DYNAMIC:
      i_inner_cyclotron_led_panel = 4;
    break;
  }

  // Power Cell inverted toggle flag.
  uint8_t i_powercell_inverted = 1;
  if(b_powercell_invert) {
    i_powercell_inverted = 2;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objLEDEEPROM obj_led_eeprom = {
    i_powercell_leds,
    i_cyclotron_leds,
    i_inner_cyclotron_cake_num_leds,
    i_grb_cyclotron_cake,
    i_spectral_powercell_custom_colour,
    i_spectral_cyclotron_custom_colour,
    i_spectral_cyclotron_inner_custom_colour,
    i_spectral_powercell_custom_saturation,
    i_spectral_cyclotron_custom_saturation,
    i_spectral_cyclotron_inner_custom_saturation,
    i_powercell_brightness,
    i_cyclotron_brightness,
    i_cyclotron_inner_brightness,
    i_cyclotron_panel_brightness,
    i_inner_cyclotron_cavity_num_leds,
    i_inner_cyclotron_cavity_led_type,
    i_inner_cyclotron_led_panel,
    i_powercell_inverted
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromAddress, obj_led_eeprom);

  updateCRCEEPROM();
}

void clearConfigEEPROM() {
  // Clear out the EEPROM data for the configuration settings only.
  uint16_t i_eepromConfigAddress = i_eepromAddress + sizeof(objLEDEEPROM);

  for(uint16_t i = 0; i < sizeof(objConfigEEPROM); i++) {
    EEPROM.update(i_eepromConfigAddress, 0);

    i_eepromConfigAddress++;
  }

  updateCRCEEPROM();
}

void saveConfigEEPROM() {
  // Convert the current EEPROM volume value into a percentage.
  uint8_t i_eeprom_volume_master_percentage = 100 * ((MINIMUM_VOLUME + i_volume_min_adj) - i_volume_master_eeprom) / (MINIMUM_VOLUME + i_volume_min_adj);

  // 1 = false, 2 = true.
  uint8_t i_proton_stream_effects = 2;
  uint8_t i_cyclotron_direction = 2; // 1 = counter-clockwise, 2 = clockwise.
  uint8_t i_center_led_fade = 2;
  uint8_t i_simulate_ring = 2;
  uint8_t i_smoke_settings = 2;

  uint8_t i_overheat_strobe = 2;
  uint8_t i_overheat_lights_off = 2;
  uint8_t i_overheat_sync_to_fan = 2;
  uint8_t i_year_mode_eeprom = SYSTEM_EEPROM_YEAR;
  uint8_t i_system_mode = 1; // 1 = super hero, 2 = original.

  uint8_t i_vg_powercell = 1;
  uint8_t i_vg_cyclotron = 1;
  uint8_t i_demo_light_mode = 1;
  uint8_t i_use_ribbon_cable = 1;
  uint8_t i_cyclotron_three_led_toggle = 1; // 1 = single led, 2 = three leds.
  uint8_t i_default_system_volume = 101; // <- i_eeprom_volume_master_percentage + 1
  uint8_t i_overheat_smoke_duration_level_5 = i_ms_overheating_length_5 / 1000;
  uint8_t i_overheat_smoke_duration_level_4 = i_ms_overheating_length_4 / 1000;
  uint8_t i_overheat_smoke_duration_level_3 = i_ms_overheating_length_3 / 1000;
  uint8_t i_overheat_smoke_duration_level_2 = i_ms_overheating_length_2 / 1000;
  uint8_t i_overheat_smoke_duration_level_1 = i_ms_overheating_length_1 / 1000;

  uint8_t i_smoke_continuous_level_5 = 1;
  uint8_t i_smoke_continuous_level_4 = 1;
  uint8_t i_smoke_continuous_level_3 = 1;
  uint8_t i_smoke_continuous_level_2 = 1;
  uint8_t i_smoke_continuous_level_1 = 1;

  uint8_t i_pack_vibration = 4; // 1 = always, 2 = when firing, 3 = off, 4 = default.

  if(!b_stream_effects) {
    i_proton_stream_effects = 1;
  }

  if(!b_clockwise) {
    i_cyclotron_direction = 1;
  }

  if(!b_fade_cyclotron_led) {
    i_center_led_fade = 1;
  }

  if(!b_cyclotron_simulate_ring) {
    i_simulate_ring = 1;
  }

  if(!b_smoke_enabled) {
    i_smoke_settings = 1;
  }

  if(!b_overheat_strobe) {
    i_overheat_strobe = 1;
  }

  if(!b_overheat_lights_off) {
    i_overheat_lights_off = 1;
  }

  if(!b_overheat_sync_to_fan) {
    i_overheat_sync_to_fan = 1;
  }

  if(SYSTEM_MODE == MODE_ORIGINAL) {
    i_system_mode = 2;
  }

  if(b_powercell_colour_toggle) {
    i_vg_powercell = 2;
  }

  if(b_cyclotron_colour_toggle) {
    i_vg_cyclotron = 2;
  }

  if(b_demo_light_mode) {
    i_demo_light_mode = 2;
  }

  if(b_use_ribbon_cable) {
    i_use_ribbon_cable = 2;
  }

  if(!b_cyclotron_single_led) {
    i_cyclotron_three_led_toggle = 2;
  }

  if(i_eeprom_volume_master_percentage <= 100) {
    // Need to add 1 to this because the EEPROM cannot contain a 0 value.
    i_default_system_volume = i_eeprom_volume_master_percentage + 1;
  }

  if(b_smoke_continuous_level_5) {
    i_smoke_continuous_level_5 = 2;
  }

  if(b_smoke_continuous_level_4) {
    i_smoke_continuous_level_4 = 2;
  }

  if(b_smoke_continuous_level_3) {
    i_smoke_continuous_level_3 = 2;
  }

  if(b_smoke_continuous_level_2) {
    i_smoke_continuous_level_2 = 2;
  }

  if(b_smoke_continuous_level_1) {
    i_smoke_continuous_level_1 = 2;
  }

  switch(VIBRATION_MODE_EEPROM) {
    case VIBRATION_ALWAYS:
      i_pack_vibration = 1;
    break;

    case VIBRATION_FIRING_ONLY:
      i_pack_vibration = 2;
    break;

    case VIBRATION_NONE:
      i_pack_vibration = 3;
    break;

    case VIBRATION_DEFAULT:
    default:
      i_pack_vibration = 4;
    break;

    case CYCLOTRON_MOTOR:
      i_pack_vibration = 5;
    break;
  }

  uint16_t i_eepromConfigAddress = i_eepromAddress + sizeof(objLEDEEPROM);

  objConfigEEPROM obj_led_eeprom = {
    i_proton_stream_effects,
    i_cyclotron_direction,
    i_center_led_fade,
    i_simulate_ring,
    i_smoke_settings,
    i_overheat_strobe,
    i_overheat_lights_off,
    i_overheat_sync_to_fan,
    i_year_mode_eeprom,
    i_system_mode,
    i_vg_powercell,
    i_vg_cyclotron,
    i_demo_light_mode,
    i_cyclotron_three_led_toggle,
    i_default_system_volume,
    i_overheat_smoke_duration_level_5,
    i_overheat_smoke_duration_level_4,
    i_overheat_smoke_duration_level_3,
    i_overheat_smoke_duration_level_2,
    i_overheat_smoke_duration_level_1,
    i_smoke_continuous_level_5,
    i_smoke_continuous_level_4,
    i_smoke_continuous_level_3,
    i_smoke_continuous_level_2,
    i_smoke_continuous_level_1,
    i_pack_vibration,
    i_use_ribbon_cable
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromConfigAddress, obj_led_eeprom);

  updateCRCEEPROM();
}

// Update the CRC in the EEPROM.
void updateCRCEEPROM() {
  EEPROM.put(EEPROM.length() - sizeof(eepromCRC()), eepromCRC());
}

uint32_t eepromCRC(void) {
  CRC32 crc;

  for(uint16_t index = 0; index < (i_eepromAddress + sizeof(objConfigEEPROM) + sizeof(objLEDEEPROM)); index++) {
    crc.update(EEPROM[index]);
  }

  crc.update(sizeof(objConfigEEPROM));
  crc.update(sizeof(objLEDEEPROM));

  return (uint32_t)crc.finalize();
}
