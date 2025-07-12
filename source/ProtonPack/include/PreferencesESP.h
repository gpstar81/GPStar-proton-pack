/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *   Adapted for ESP32 Preferences API.
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
void loadConfigEEPROM();
void loadLEDEEPROM();
void updateCRCEEPROM(uint32_t);
uint32_t getCRCEEPROM(void);
uint32_t eepromCRC(void);
void resetCyclotronLEDs();
void resetInnerCyclotronLEDs();
void resetContinuousSmoke();
void updateProtonPackLEDCounts();

// Include ESP32 Preferences library
#include <Preferences.h>

// Data structure for LED settings (stored in Preferences)
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
} gObjLEDEEPROM;

struct objConfigEEPROM {
  uint8_t stream_effects;
  uint8_t cyclotron_direction;
  uint8_t center_led_fade;
  uint8_t simulate_ring;
  uint8_t smoke_setting;
  uint8_t overheat_strobe;
  uint8_t overheat_lights_off;
  uint8_t overheat_sync_to_fan;
  uint8_t year_mode;
  uint8_t system_mode;
  uint8_t vg_powercell;
  uint8_t vg_cyclotron;
  uint8_t demo_light_mode;
  uint8_t cyclotron_three_led_toggle;
  uint8_t default_system_volume;
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
  uint8_t pack_vibration;
  uint8_t use_ribbon_cable;
} gObjConfigEEPROM;

// Save LED settings to Preferences
void saveLEDEEPROM() {
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
  uint8_t i_inner_cyclotron_led_panel = 4;
  switch(INNER_CYC_PANEL_MODE) {
    case PANEL_RGB_STATIC:
      i_inner_cyclotron_led_panel = 3;
    break;

    case PANEL_RGB_DYNAMIC:
    default:
      i_inner_cyclotron_led_panel = 4;
    break;
  }

  // Power Cell inverted toggle flag.
  uint8_t i_powercell_inverted = 1;
  if(b_powercell_invert) {
    i_powercell_inverted = 2;
  }

  gObjLEDEEPROM.powercell_count = i_powercell_leds;
  gObjLEDEEPROM.cyclotron_count = i_cyclotron_leds;
  gObjLEDEEPROM.inner_cyclotron_count = i_inner_cyclotron_cake_num_leds;
  gObjLEDEEPROM.grb_inner_cyclotron = i_grb_cyclotron_cake;
  gObjLEDEEPROM.powercell_spectral_custom = i_spectral_powercell_custom_colour;
  gObjLEDEEPROM.cyclotron_spectral_custom = i_spectral_cyclotron_custom_colour;
  gObjLEDEEPROM.cyclotron_inner_spectral_custom = i_spectral_cyclotron_inner_custom_colour;
  gObjLEDEEPROM.powercell_spectral_saturation_custom = i_spectral_powercell_custom_saturation;
  gObjLEDEEPROM.cyclotron_spectral_saturation_custom = i_spectral_cyclotron_custom_saturation;
  gObjLEDEEPROM.cyclotron_inner_spectral_saturation_custom = i_spectral_cyclotron_inner_custom_saturation;
  gObjLEDEEPROM.powercell_brightness = i_powercell_brightness;
  gObjLEDEEPROM.cyclotron_brightness = i_cyclotron_brightness;
  gObjLEDEEPROM.inner_cyclotron_brightness = i_cyclotron_inner_brightness;
  gObjLEDEEPROM.inner_panel_brightness = i_cyclotron_panel_brightness;
  gObjLEDEEPROM.cyclotron_cavity_count = i_inner_cyclotron_cavity_num_leds;
  gObjLEDEEPROM.cyclotron_cavity_type = i_inner_cyclotron_cavity_led_type;
  gObjLEDEEPROM.inner_cyclotron_led_panel = i_inner_cyclotron_led_panel;
  gObjLEDEEPROM.powercell_inverted = i_powercell_inverted;

  Preferences prefs;
  prefs.begin("led", false);
  prefs.putBytes("led", &gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
  prefs.end();

  updateCRCEEPROM(eepromCRC());
}

// Load LED settings from Preferences
void loadLEDEEPROM() {
  Preferences prefs;
  prefs.begin("led", true);
  prefs.getBytes("led", &gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
  prefs.clear();
  prefs.end();
}

// Clear LED settings in Preferences
void clearLEDEEPROM() {
  Preferences prefs;
  prefs.begin("led", false);
  prefs.clear();
  prefs.end();

  updateCRCEEPROM(eepromCRC());
}

// Save config settings to Preferences
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
  uint8_t i_overheat_sync_to_fan = 1;
  uint8_t i_year_mode_eeprom = SYSTEM_EEPROM_YEAR;
  uint8_t i_system_mode = 1; // 1 = super hero, 2 = original.

  uint8_t i_vg_powercell = 1;
  uint8_t i_vg_cyclotron = 2;
  uint8_t i_demo_light_mode = 1;
  uint8_t i_use_ribbon_cable = 1;
  uint8_t i_cyclotron_three_led_toggle = 1; // 1 = single led, 2 = three leds.
  uint8_t i_default_system_volume = 101; // <- i_eeprom_volume_master_percentage + 1
  uint8_t i_overheat_smoke_duration_level_5 = i_ms_overheating_length_5 / 1000;
  uint8_t i_overheat_smoke_duration_level_4 = i_ms_overheating_length_4 / 1000;
  uint8_t i_overheat_smoke_duration_level_3 = i_ms_overheating_length_3 / 1000;
  uint8_t i_overheat_smoke_duration_level_2 = i_ms_overheating_length_2 / 1000;
  uint8_t i_overheat_smoke_duration_level_1 = i_ms_overheating_length_1 / 1000;

  uint8_t i_smoke_continuous_level_5 = 2;
  uint8_t i_smoke_continuous_level_4 = 2;
  uint8_t i_smoke_continuous_level_3 = 2;
  uint8_t i_smoke_continuous_level_2 = 2;
  uint8_t i_smoke_continuous_level_1 = 2;

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

  if(b_overheat_sync_to_fan) {
    i_overheat_sync_to_fan = 2;
  }

  if(SYSTEM_MODE == MODE_ORIGINAL) {
    i_system_mode = 2;
  }

  if(b_powercell_colour_toggle) {
    i_vg_powercell = 2;
  }

  if(!b_cyclotron_colour_toggle) {
    i_vg_cyclotron = 1;
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

  if(!b_smoke_continuous_level_5) {
    i_smoke_continuous_level_5 = 1;
  }

  if(!b_smoke_continuous_level_4) {
    i_smoke_continuous_level_4 = 1;
  }

  if(!b_smoke_continuous_level_3) {
    i_smoke_continuous_level_3 = 1;
  }

  if(!b_smoke_continuous_level_2) {
    i_smoke_continuous_level_2 = 1;
  }

  if(!b_smoke_continuous_level_1) {
    i_smoke_continuous_level_1 = 1;
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

  gObjConfigEEPROM.stream_effects = i_proton_stream_effects;
  gObjConfigEEPROM.cyclotron_direction = i_cyclotron_direction;
  gObjConfigEEPROM.center_led_fade = i_center_led_fade;
  gObjConfigEEPROM.simulate_ring = i_simulate_ring;
  gObjConfigEEPROM.smoke_setting = i_smoke_settings;
  gObjConfigEEPROM.overheat_strobe = i_overheat_strobe;
  gObjConfigEEPROM.overheat_lights_off = i_overheat_lights_off;
  gObjConfigEEPROM.overheat_sync_to_fan = i_overheat_sync_to_fan;
  gObjConfigEEPROM.year_mode = i_year_mode_eeprom;
  gObjConfigEEPROM.system_mode = i_system_mode;
  gObjConfigEEPROM.vg_powercell = i_vg_powercell;
  gObjConfigEEPROM.vg_cyclotron = i_vg_cyclotron;
  gObjConfigEEPROM.demo_light_mode = i_demo_light_mode;
  gObjConfigEEPROM.cyclotron_three_led_toggle = i_cyclotron_three_led_toggle;
  gObjConfigEEPROM.default_system_volume = i_default_system_volume;
  gObjConfigEEPROM.overheat_smoke_duration_level_5 = i_overheat_smoke_duration_level_5;
  gObjConfigEEPROM.overheat_smoke_duration_level_4 = i_overheat_smoke_duration_level_4;
  gObjConfigEEPROM.overheat_smoke_duration_level_3 = i_overheat_smoke_duration_level_3;
  gObjConfigEEPROM.overheat_smoke_duration_level_2 = i_overheat_smoke_duration_level_2;
  gObjConfigEEPROM.overheat_smoke_duration_level_1 = i_overheat_smoke_duration_level_1;
  gObjConfigEEPROM.smoke_continuous_level_5 = i_smoke_continuous_level_5;
  gObjConfigEEPROM.smoke_continuous_level_4 = i_smoke_continuous_level_4;
  gObjConfigEEPROM.smoke_continuous_level_3 = i_smoke_continuous_level_3;
  gObjConfigEEPROM.smoke_continuous_level_2 = i_smoke_continuous_level_2;
  gObjConfigEEPROM.smoke_continuous_level_1 = i_smoke_continuous_level_1;
  gObjConfigEEPROM.pack_vibration = i_pack_vibration;
  gObjConfigEEPROM.use_ribbon_cable = i_use_ribbon_cable;

  Preferences prefs;
  prefs.begin("config", false);
  prefs.putBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
  prefs.end();

  updateCRCEEPROM(eepromCRC());
}

// Load config settings from Preferences
void loadConfigEEPROM() {
  Preferences prefs;
  prefs.begin("config", true);
  prefs.getBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
  prefs.end();
}

// Clear config settings in Preferences
void clearConfigEEPROM() {
  Preferences prefs;
  prefs.begin("config", false);
  prefs.clear();
  prefs.end();

  updateCRCEEPROM(eepromCRC());
}

// Read all user preferences from Preferences (ESP32)
void readEEPROM() {
  uint32_t storedCrc = getCRCEEPROM();
  uint32_t calcCrc = eepromCRC();
  if(storedCrc == calcCrc) {
    if(gObjLEDEEPROM.powercell_count == HASLAB_POWERCELL_LED_COUNT || gObjLEDEEPROM.powercell_count == FRUTTO_POWERCELL_LED_COUNT) {
      i_powercell_leds = gObjLEDEEPROM.powercell_count;

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

    if(gObjLEDEEPROM.cyclotron_count == HASLAB_CYCLOTRON_LED_COUNT || gObjLEDEEPROM.cyclotron_count == FRUTTO_CYCLOTRON_LED_COUNT ||
      gObjLEDEEPROM.cyclotron_count == FRUTTO_MAX_CYCLOTRON_LED_COUNT || gObjLEDEEPROM.cyclotron_count == OUTER_CYCLOTRON_LED_MAX) {
      i_cyclotron_leds = gObjLEDEEPROM.cyclotron_count;
    }
    else if(!b_power_meter_available) {
      // If no EEPROM default set and not using a stock wand, assume Frutto upgrades instead.
      i_cyclotron_leds = FRUTTO_MAX_CYCLOTRON_LED_COUNT;
    }

    if(gObjLEDEEPROM.inner_cyclotron_count == 12 || gObjLEDEEPROM.inner_cyclotron_count == 23 || gObjLEDEEPROM.inner_cyclotron_count == 24 ||
      gObjLEDEEPROM.inner_cyclotron_count == 26 || gObjLEDEEPROM.inner_cyclotron_count == 35 || gObjLEDEEPROM.inner_cyclotron_count == 36) {
      i_inner_cyclotron_cake_num_leds = gObjLEDEEPROM.inner_cyclotron_count;

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

    if(gObjLEDEEPROM.cyclotron_cavity_count > 0 && gObjLEDEEPROM.cyclotron_cavity_count < 21) {
      i_inner_cyclotron_cavity_num_leds = gObjLEDEEPROM.cyclotron_cavity_count;
    }

    if(gObjLEDEEPROM.cyclotron_cavity_type > 0 && gObjLEDEEPROM.cyclotron_cavity_type < 5) {
      if(gObjLEDEEPROM.cyclotron_cavity_type > 1) {
        // 2 = RGB, 3 = GRB, 4 = GBR.
        switch(gObjLEDEEPROM.cyclotron_cavity_type) {
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

    if(gObjLEDEEPROM.powercell_inverted > 0 && gObjLEDEEPROM.powercell_inverted < 3) {
      if(gObjLEDEEPROM.powercell_inverted > 1) {
        b_powercell_invert = true;
      }
      else {
        b_powercell_invert = false;
      }
    }

    if(gObjLEDEEPROM.inner_cyclotron_led_panel > 0 && gObjLEDEEPROM.inner_cyclotron_led_panel < 5) {
      if(gObjLEDEEPROM.inner_cyclotron_led_panel > 1) {
        // 3 = RGB Static, 4 = RGB Dynamic.
        switch(gObjLEDEEPROM.inner_cyclotron_led_panel) {
          case 3:
            INNER_CYC_PANEL_MODE = PANEL_RGB_STATIC;
          break;

          case 4:
          default:
            INNER_CYC_PANEL_MODE = PANEL_RGB_DYNAMIC;
          break;
        }
      }
    }

    if(gObjLEDEEPROM.grb_inner_cyclotron > 0 && gObjLEDEEPROM.grb_inner_cyclotron < 5) {
      if(gObjLEDEEPROM.grb_inner_cyclotron > 1) {
        CAKE_LED_TYPE = GRB_LED;
      }
      else {
        CAKE_LED_TYPE = RGB_LED;
      }
    }

    if(gObjLEDEEPROM.powercell_spectral_custom > 0 && gObjLEDEEPROM.powercell_spectral_custom != 255) {
      i_spectral_powercell_custom_colour = gObjLEDEEPROM.powercell_spectral_custom;
    }

    if(gObjLEDEEPROM.cyclotron_spectral_custom > 0 && gObjLEDEEPROM.cyclotron_spectral_custom != 255) {
      i_spectral_cyclotron_custom_colour = gObjLEDEEPROM.cyclotron_spectral_custom;
    }

    if(gObjLEDEEPROM.cyclotron_inner_spectral_custom > 0 && gObjLEDEEPROM.cyclotron_inner_spectral_custom != 255) {
      i_spectral_cyclotron_inner_custom_colour = gObjLEDEEPROM.cyclotron_inner_spectral_custom;
    }

    if(gObjLEDEEPROM.powercell_spectral_saturation_custom > 0 && gObjLEDEEPROM.powercell_spectral_saturation_custom != 255) {
      i_spectral_powercell_custom_saturation = gObjLEDEEPROM.powercell_spectral_saturation_custom;
    }

    if(gObjLEDEEPROM.cyclotron_spectral_saturation_custom > 0 && gObjLEDEEPROM.cyclotron_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_custom_saturation = gObjLEDEEPROM.cyclotron_spectral_saturation_custom;
    }

    if(gObjLEDEEPROM.cyclotron_inner_spectral_saturation_custom > 0 && gObjLEDEEPROM.cyclotron_inner_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_inner_custom_saturation = gObjLEDEEPROM.cyclotron_inner_spectral_saturation_custom;
    }

    if(gObjLEDEEPROM.powercell_brightness > 19 && gObjLEDEEPROM.powercell_brightness < 101) {
      i_powercell_brightness = gObjLEDEEPROM.powercell_brightness;
    }

    if(gObjLEDEEPROM.cyclotron_brightness > 19 && gObjLEDEEPROM.cyclotron_brightness < 101) {
      i_cyclotron_brightness = gObjLEDEEPROM.cyclotron_brightness;
    }

    if(gObjLEDEEPROM.inner_cyclotron_brightness > 19 && gObjLEDEEPROM.inner_cyclotron_brightness < 101) {
      i_cyclotron_inner_brightness = gObjLEDEEPROM.inner_cyclotron_brightness;
    }

    if(gObjLEDEEPROM.inner_panel_brightness > 19 && gObjLEDEEPROM.inner_panel_brightness < 101) {
      i_cyclotron_panel_brightness = gObjLEDEEPROM.inner_panel_brightness;
    }

    // Update the LED counts for the Proton Pack.
    resetCyclotronLEDs();
    resetInnerCyclotronLEDs();
    updateProtonPackLEDCounts();


    if(gObjConfigEEPROM.stream_effects > 0 && gObjConfigEEPROM.stream_effects < 3) {
      if(gObjConfigEEPROM.stream_effects > 1) {
        b_stream_effects = true;
      }
      else {
        b_stream_effects = false;
      }
    }

    if(gObjConfigEEPROM.cyclotron_direction > 0 && gObjConfigEEPROM.cyclotron_direction < 3) {
      if(gObjConfigEEPROM.cyclotron_direction > 1) {
        b_clockwise = true;
      }
      else {
        b_clockwise = false;
      }
    }

    if(gObjConfigEEPROM.center_led_fade > 0 && gObjConfigEEPROM.center_led_fade < 3) {
      if(gObjConfigEEPROM.center_led_fade > 1) {
        b_fade_cyclotron_led = true;
      }
      else {
        b_fade_cyclotron_led = false;
      }
    }

    if(gObjConfigEEPROM.simulate_ring > 0 && gObjConfigEEPROM.simulate_ring < 3) {
      if(gObjConfigEEPROM.simulate_ring > 1) {
        b_cyclotron_simulate_ring = true;
      }
      else {
        b_cyclotron_simulate_ring = false;
      }
    }

    if(gObjConfigEEPROM.smoke_setting > 0 && gObjConfigEEPROM.smoke_setting < 3) {
      if(gObjConfigEEPROM.smoke_setting > 1) {
        b_smoke_enabled = true;
      }
      else {
        b_smoke_enabled = false;
      }
    }

    if(gObjConfigEEPROM.overheat_strobe > 0 && gObjConfigEEPROM.overheat_strobe < 3) {
      if(gObjConfigEEPROM.overheat_strobe > 1) {
        b_overheat_strobe = true;
      }
      else {
        b_overheat_strobe = false;
      }
    }

    if(gObjConfigEEPROM.overheat_lights_off > 0 && gObjConfigEEPROM.overheat_lights_off < 3) {
      if(gObjConfigEEPROM.overheat_lights_off > 1) {
        b_overheat_lights_off = true;
      }
      else {
        b_overheat_lights_off = false;
      }
    }

    if(gObjConfigEEPROM.overheat_sync_to_fan > 0 && gObjConfigEEPROM.overheat_sync_to_fan < 3) {
      if(gObjConfigEEPROM.overheat_sync_to_fan > 1) {
        b_overheat_sync_to_fan = true;
      }
      else {
        b_overheat_sync_to_fan = false;
      }
    }

    if(gObjConfigEEPROM.year_mode > 1 && gObjConfigEEPROM.year_mode < 6) {
      // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
      switch(gObjConfigEEPROM.year_mode) {
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

    if(gObjConfigEEPROM.system_mode > 0 && gObjConfigEEPROM.system_mode < 3) {
      if(gObjConfigEEPROM.system_mode > 1) {
        SYSTEM_MODE = MODE_ORIGINAL;
      }
      else {
        SYSTEM_MODE = MODE_SUPER_HERO;
      }
    }

    if(gObjConfigEEPROM.vg_powercell > 0 && gObjConfigEEPROM.vg_powercell < 3) {
      if(gObjConfigEEPROM.vg_powercell > 1) {
        b_powercell_colour_toggle = true;
      }
      else {
        b_powercell_colour_toggle = false;
      }
    }

    if(gObjConfigEEPROM.vg_cyclotron > 0 && gObjConfigEEPROM.vg_cyclotron < 3) {
      if(gObjConfigEEPROM.vg_cyclotron > 1) {
        b_cyclotron_colour_toggle = true;
      }
      else {
        b_cyclotron_colour_toggle = false;
      }
    }

    if(gObjConfigEEPROM.demo_light_mode > 0 && gObjConfigEEPROM.demo_light_mode < 3) {
      if(gObjConfigEEPROM.demo_light_mode > 1) {
        b_demo_light_mode = true;
      }
      else {
        b_demo_light_mode = false;
      }
    }

    if(gObjConfigEEPROM.use_ribbon_cable > 0 && gObjConfigEEPROM.use_ribbon_cable < 3) {
      if(gObjConfigEEPROM.use_ribbon_cable > 1) {
        b_use_ribbon_cable = true;
      }
      else {
        b_use_ribbon_cable = false;
      }
    }

    if(gObjConfigEEPROM.cyclotron_three_led_toggle > 0 && gObjConfigEEPROM.cyclotron_three_led_toggle < 3) {
      if(gObjConfigEEPROM.cyclotron_three_led_toggle > 1) {
        b_cyclotron_single_led = false;
      }
      else {
        b_cyclotron_single_led = true;
      }
    }

    if(gObjConfigEEPROM.default_system_volume > 0 && gObjConfigEEPROM.default_system_volume < 102) {
      // EEPROM value is from 1 to 101; subtract 1 to get the correct percentage.
      i_volume_master_percentage = gObjConfigEEPROM.default_system_volume - 1;
      i_volume_master_eeprom = (MINIMUM_VOLUME + i_volume_min_adj) - ((MINIMUM_VOLUME + i_volume_min_adj) * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(gObjConfigEEPROM.overheat_smoke_duration_level_5 > 0 && gObjConfigEEPROM.overheat_smoke_duration_level_5 < 61) {
      i_ms_overheating_length_5 = gObjConfigEEPROM.overheat_smoke_duration_level_5 * 1000;
    }

    if(gObjConfigEEPROM.overheat_smoke_duration_level_4 > 0 && gObjConfigEEPROM.overheat_smoke_duration_level_4 < 61) {
      i_ms_overheating_length_4 = gObjConfigEEPROM.overheat_smoke_duration_level_4 * 1000;
    }

    if(gObjConfigEEPROM.overheat_smoke_duration_level_3 > 0 && gObjConfigEEPROM.overheat_smoke_duration_level_3 < 61) {
      i_ms_overheating_length_3 = gObjConfigEEPROM.overheat_smoke_duration_level_3 * 1000;
    }

    if(gObjConfigEEPROM.overheat_smoke_duration_level_2 > 0 && gObjConfigEEPROM.overheat_smoke_duration_level_2 < 61) {
      i_ms_overheating_length_2 = gObjConfigEEPROM.overheat_smoke_duration_level_2 * 1000;
    }

    if(gObjConfigEEPROM.overheat_smoke_duration_level_1 > 0 && gObjConfigEEPROM.overheat_smoke_duration_level_1 < 61) {
      i_ms_overheating_length_1 = gObjConfigEEPROM.overheat_smoke_duration_level_1 * 1000;
    }

    if(gObjConfigEEPROM.smoke_continuous_level_5 > 0 && gObjConfigEEPROM.smoke_continuous_level_5 < 3) {
      if(gObjConfigEEPROM.smoke_continuous_level_5 > 1) {
        b_smoke_continuous_level_5 = true;
      }
      else {
        b_smoke_continuous_level_5 = false;
      }
    }

    if(gObjConfigEEPROM.smoke_continuous_level_4 > 0 && gObjConfigEEPROM.smoke_continuous_level_4 < 3) {
      if(gObjConfigEEPROM.smoke_continuous_level_4 > 1) {
        b_smoke_continuous_level_4 = true;
      }
      else {
        b_smoke_continuous_level_4 = false;
      }
    }

    if(gObjConfigEEPROM.smoke_continuous_level_3 > 0 && gObjConfigEEPROM.smoke_continuous_level_3 < 3) {
      if(gObjConfigEEPROM.smoke_continuous_level_3 > 1) {
        b_smoke_continuous_level_3 = true;
      }
      else {
        b_smoke_continuous_level_3 = false;
      }
    }

    if(gObjConfigEEPROM.smoke_continuous_level_2 > 0 && gObjConfigEEPROM.smoke_continuous_level_2 < 3) {
      if(gObjConfigEEPROM.smoke_continuous_level_2 > 1) {
        b_smoke_continuous_level_2 = true;
      }
      else {
        b_smoke_continuous_level_2 = false;
      }
    }

    if(gObjConfigEEPROM.smoke_continuous_level_1 > 0 && gObjConfigEEPROM.smoke_continuous_level_1 < 3) {
      if(gObjConfigEEPROM.smoke_continuous_level_1 > 1) {
        b_smoke_continuous_level_1 = true;
      }
      else {
        b_smoke_continuous_level_1 = false;
      }
    }

    if(gObjConfigEEPROM.pack_vibration > 0 && gObjConfigEEPROM.pack_vibration < 6) {
      switch(gObjConfigEEPROM.pack_vibration) {
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
    // CRC mismatch; clear preferences
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);
    clearLEDEEPROM();
    clearConfigEEPROM();
  }
}

// CRC helpers for Preferences
void updateCRCEEPROM(uint32_t crc) {
  Preferences prefs;
  prefs.begin("crc", false);
  prefs.putUInt("crc", crc);
  prefs.end();
}

uint32_t getCRCEEPROM() {
  Preferences prefs;
  prefs.begin("crc", true);
  uint32_t crc = prefs.getUInt("crc");
  prefs.end();
  return crc;
}

// Calculate CRC for all stored preferences
uint32_t eepromCRC() {
  CRC32 crc;
  loadLEDEEPROM();
  loadConfigEEPROM();
  crc.update((uint8_t*)&gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
  crc.update((uint8_t*)&gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
  return crc.finalize();
}
