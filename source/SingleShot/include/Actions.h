/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#pragma once

// Arms/Disarms the power-on reminder (if enabled).
void setPowerOnReminder(bool enable) {
  if(enable && b_power_on_indicator) {
    // Arm the power indicator timer.
    ms_power_indicator.start(i_ms_power_indicator);
  }
  else {
    // Disarm the power indicator timer.
    ms_power_indicator.stop();
  }
}

// Function to handle blinking for the power-on reminder (if enabled).
void checkPowerOnReminder() {
  if(DEVICE_ACTION_STATUS == ACTION_IDLE) {
    if(ms_power_indicator.justFinished()) {
      // Blink the Clippard LED.
      if(led_Clippard.getState() == led_Clippard.Off) {
        led_Clippard.turnOn();
      }
      else {
        led_Clippard.turnOff();
      }

      // Restart the blink timer.
      ms_power_indicator.start(i_ms_power_indicator_blink);
    }
  }
}

// Check the state of the grip button to determine whether we have entered the settings menu.
void gripButtonCheck() {
  // Proceed if device is in an idle state or in either the settings or EEPROM menus.
  if(DEVICE_ACTION_STATUS == ACTION_IDLE || DEVICE_ACTION_STATUS == ACTION_SETTINGS || DEVICE_ACTION_STATUS == ACTION_CONFIG_EEPROM_MENU) {
    if(switch_grip.pushed() && !(switch_device.on() && switch_vent.on())) {
      // Switch between firing mode and settings mode, but only when right toggles are both off.
      if(DEVICE_ACTION_STATUS != ACTION_SETTINGS && DEVICE_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && !switch_vent.on() && !switch_device.on()) {
        // Not currently in the settings menu so set that as the current action.
        DEVICE_ACTION_STATUS = ACTION_SETTINGS;
        ms_settings_blink.start(i_settings_blink_delay);
        deviceEnterMenu();
        return;
      }
      else if(DEVICE_ACTION_STATUS == ACTION_SETTINGS && DEVICE_MENU_LEVEL == MENU_LEVEL_1 && MENU_OPTION_LEVEL == OPTION_5) {
        // Only exit the settings menu when at option #5 on menu level 1.
        DEVICE_ACTION_STATUS = ACTION_IDLE;
        ms_settings_blink.stop();
        deviceExitMenu();
        return;
      }
      else if(DEVICE_ACTION_STATUS == ACTION_CONFIG_EEPROM_MENU && DEVICE_MENU_LEVEL == MENU_LEVEL_1 && MENU_OPTION_LEVEL == OPTION_5) {
        // Only exit the settings menu when at option #5 on menu level 1.
        DEVICE_ACTION_STATUS = ACTION_IDLE;
        ms_settings_blink.stop();
        bargraph.clear();
        deviceExitEEPROMMenu();
        return;
      }
    }
  }
}

// Perform actions based on button press while in the settings menu.
void settingsMenuCheck() {
  if(DEVICE_ACTION_STATUS != ACTION_SETTINGS) {
    return; // Leave if not in the settings menu.
  }

  switch(DEVICE_MENU_LEVEL) {
    case MENU_LEVEL_1:
      switch(MENU_OPTION_LEVEL) {
        case OPTION_5:
          // Intensify: Enable/Disable Music Track Looping
          if(b_playing_music && switch_intensify.pushed()) {
            toggleMusicLoop();
            debugln(F("Toggle Music Loop"));
          }

          // Grip: Exits the menu system
          // Allow the method gripButtonCheck() handle this on the next loop
        break;

        case OPTION_4:
          // Grip + Dial = System Volume
          if(switch_grip.on()) {
            if(encoder.STATE == ENCODER_CW) {
              // Increase the master system volume.
              increaseVolume();
              debug(F("Menu, System Vol+ "));
              debugln(i_volume_master);
            }
            else if(encoder.STATE == ENCODER_CCW) {
              // Decrease the master system volume.
              decreaseVolume();
              debug(F("Menu, System Vol- "));
              debugln(i_volume_master);
            }
          }
        break;

        case OPTION_3:
          // Grip + Dial = Effects Volume
          if(switch_grip.on()) {
            if(encoder.STATE == ENCODER_CW) {
              // Increase the effects volume.
              increaseVolumeEffects();
              debug(F("Menu, Effects Vol+ "));
              debugln(i_volume_effects);
            }
            else if(encoder.STATE == ENCODER_CCW) {
              // Decrease the effects volume.
              decreaseVolumeEffects();
              debug(F("Menu, Effects Vol- "));
              debugln(i_volume_effects);
            }
          }
        break;

        case OPTION_2:
          // Intensify: Previous Track
          if(b_playing_music && switch_intensify.pushed()) {
            musicPrevTrack();
            debugln(F("Prev Track"));
          }

          // Grip: Next Track
          if(b_playing_music && switch_grip.pushed()) {
            musicNextTrack();
            debugln(F("Next Track"));
          }
        break;

        case OPTION_1:
          // Intensify: Start/Stop Music
          if(switch_intensify.pushed()) {
            if(!b_playing_music) {
              playMusic();
              debugln(F("Play Music"));
            }
            else {
              stopMusic();
              debugln(F("Stop Music"));
            }
          }

          // Grip + Dial = Music Volume
          if(switch_grip.on()) {
            if(encoder.STATE == ENCODER_CW) {
              // Increase the music volume.
              increaseVolumeMusic();
              debug(F("Menu, Music Vol+ "));
              debugln(i_volume_music);
            }
            else if(encoder.STATE == ENCODER_CCW) {
              // Decrease the music volume.
              decreaseVolumeMusic();
              debug(F("Menu, Music Vol- "));
              debugln(i_volume_music);
            }
          }
        break;
      }
    break;

    case MENU_LEVEL_2:
      switch(MENU_OPTION_LEVEL) {
        case OPTION_5:
          // Intensify: Enable/Disable auto vent light intensity.
          if(switch_intensify.pushed()) {
            if(b_vent_light_control) {
              // Disable the auto vent light intensity feature.
              b_vent_light_control = false;

              stopEffect(S_VOICE_VENT_AUTO_INTENSITY_ENABLED);
              stopEffect(S_VOICE_VENT_AUTO_INTENSITY_DISABLED);

              playEffect(S_VOICE_VENT_AUTO_INTENSITY_DISABLED);
            }
            else {
              // Enable the auto vent light intensity feature.
              b_vent_light_control = true;

              stopEffect(S_VOICE_VENT_AUTO_INTENSITY_ENABLED);
              stopEffect(S_VOICE_VENT_AUTO_INTENSITY_DISABLED);

              playEffect(S_VOICE_VENT_AUTO_INTENSITY_ENABLED);
            }
          }

          // Grip: Enable/Disable RGB vent light support.
          if(switch_grip.pushed()) {
            if(b_rgb_vent_light) {
              // Disable the RGB vent light functionality.
              b_rgb_vent_light = false;

              stopEffect(S_VOICE_RGB_VENT_LIGHTS_ENABLED);
              stopEffect(S_VOICE_RGB_VENT_LIGHTS_DISABLED);

              playEffect(S_VOICE_RGB_VENT_LIGHTS_DISABLED);
            }
            else {
              // Enable the RGB vent light functionality.
              b_rgb_vent_light = true;

              stopEffect(S_VOICE_RGB_VENT_LIGHTS_ENABLED);
              stopEffect(S_VOICE_RGB_VENT_LIGHTS_DISABLED);

              playEffect(S_VOICE_RGB_VENT_LIGHTS_ENABLED);
            }
          }
        break;
        default:
          // Currently no-op for all other options.
        break;
      }
    break;

    case MENU_LEVEL_3:
      // No-Op. Not currently used.
    break;

    case MENU_LEVEL_4:
      // No-Op. Not currently used.
    break;

    case MENU_LEVEL_5:
      // No-Op. Not currently used.
    break;
  }
}

void checkDeviceAction() {
  switch(DEVICE_STATUS) {
    case MODE_OFF:
      // Reset the count of the device switch
      if(!switch_intensify.on()) {
        deviceSwitchedCount = 0;
        ventSwitchedCount = 0;
      }

      if(switch_intensify.on() && ventSwitchedCount >= 5 && DEVICE_ACTION_STATUS != ACTION_SETTINGS && DEVICE_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
        // Enter the Config EEPROM menu if holding the Intensify button while toggling the vent switch (lower right) a minimum of 5 times, while not in a menu already.
        stopEffect(S_BEEPS);
        playEffect(S_BEEPS);

        stopEffect(S_VOICE_EEPROM_CONFIG_MENU);
        playEffect(S_VOICE_EEPROM_CONFIG_MENU);

        DEVICE_ACTION_STATUS = ACTION_CONFIG_EEPROM_MENU;
        ventSwitchedCount = 0; // We did it, now clear the count.
        ms_settings_blink.start(i_settings_blink_delay);
        deviceEnterMenu();
        return;
      }

      // Determine if the special grip button has been pressed (eg. firing, menu operation);
      gripButtonCheck();

      // When device is currently off but gets activated while idle, this is when we fully activate the device.
      if(switch_activate.on() && DEVICE_ACTION_STATUS == ACTION_IDLE) {
        // Activate the device if previously idle.
        DEVICE_ACTION_STATUS = ACTION_ACTIVATE;
      }

      checkPowerOnReminder();
    break;

    case MODE_ERROR:
      if(ms_error_blink.remaining() < i_error_blink_delay / 2) {
        led_Clippard.turnOff();
        led_SloBlo.turnOff();
        ventTopLightControl(false);
        led_Hat2.turnOff();
      }
      else {
        led_Clippard.turnOn();
        led_SloBlo.turnOn();
        ventTopLightControl(true);
        led_Hat2.turnOn();
      }

      if(ms_error_blink.justFinished()) {
        ms_error_blink.repeat();

        playEffect(S_BEEPS_LOW);
        playEffect(S_BEEPS);
      }

      if(ms_warning_blink.justFinished()) {
        playEffect(S_BEEPS);

        ms_warning_blink.repeat();
      }

      // If the activate is switched off during error mode, reset the device.
      if(!switch_activate.on()) {
        b_device_boot_error_on = false;
        deviceOff();
      }
    break;

    case MODE_ON:
      if(!ms_warning_blink.isRunning() && !ms_error_blink.isRunning()) {
        // Hat 2 stays solid while the Single-Shot Blaster is on.
        led_Hat2.turnOn();
      }

      // Top white light.
      if(ms_white_light.justFinished()) {
        ms_white_light.repeat();
        if(vent_leds[1]) {
          ventTopLightControl(false);
        }
        else {
          ventTopLightControl(true);
        }
      }

      // Update vibration level based on power level.
      vibrationSetting();

      // Determine if the special grip button has been pressed (eg. firing, menu operation);
      gripButtonCheck();

      // Determine the light status on the device and any beeps.
      deviceLightControlCheck();

      // Check if we should fire, or if the device was turned off.
      fireControlCheck();
    break;
  }

  // Use the current action status to determine next steps.
  switch(DEVICE_ACTION_STATUS) {
    case ACTION_IDLE:
    default:
      // No-op, add actions here as needed.
    break;

    case ACTION_OFF:
      deviceOff();
      bargraph.off();
    break;

    case ACTION_ACTIVATE:
      modeActivate();
    break;

    case ACTION_FIRING:
      if(ms_single_blast.justFinished()) {
        // Reset the barrel before starting a new pulse.
        barrelLightsOff();

        switch(POWER_LEVEL) {
          case LEVEL_1:
          default:
            ms_single_blast.start(i_single_blast_delay_level_1);
          break;
          case LEVEL_2:
            ms_single_blast.start(i_single_blast_delay_level_2);
          break;
          case LEVEL_3:
            ms_single_blast.start(i_single_blast_delay_level_3);
          break;
          case LEVEL_4:
            ms_single_blast.start(i_single_blast_delay_level_4);
          break;
          case LEVEL_5:
            ms_single_blast.start(i_single_blast_delay_level_5);
          break;
        }
      }

      if(!b_firing) {
        b_firing = true;
        modeFireStart();
      }

      modeFiring();

      // Stop firing if any of the main switches are turned off.
      if(!switch_vent.on() || !switch_device.on()) {
        modeFireStop();
      }
    break;

    case ACTION_ERROR:
      // No-op, add actions here as needed.
    break;

    case ACTION_SETTINGS:
      // Respond to button actions based on menu level/option.
      settingsMenuCheck();
    break;

    case ACTION_CONFIG_EEPROM_MENU:
      // TODO: Re-introduce Config EEPROM menu options for this device
    break;
  }

  if(b_firing && DEVICE_ACTION_STATUS != ACTION_FIRING) {
    // User is firing but we've switched into an action that is not firing.
    modeFireStop();
  }
}

void encoderChangedMenuOption() {
  if(encoder.STATE == ENCODER_IDLE) {
    return; // Leave if no change has occurred.
  }

  if(switch_intensify.on() || switch_grip.on()) {
    // If either of these buttons is pressed while turning the rotary dial,
    // then we assume the user is not actually intending to change menu level.
    return;
  }

  // Handle menu navigation based on rotation of the encoder
  if(encoder.STATE == ENCODER_CW) {
    if(decreaseOptionLevel()) {
      bargraph.showBars(MENU_OPTION_LEVEL); // Update change to menu.
    }
  }
  else if(encoder.STATE == ENCODER_CCW) {
    if(increaseOptionLevel()) {
      bargraph.showBars(MENU_OPTION_LEVEL); // Update change to menu.
    }
  }
}

// Performs an action directly related to actions via the encoder.
void checkEncoderAction() {
  if(encoder.STATE == ENCODER_IDLE) {
    return; // Leave if no change has occurred.
  }

  switch(DEVICE_STATUS) {
    case MODE_OFF:
      if(b_playing_music && DEVICE_ACTION_STATUS != ACTION_SETTINGS) {
        // If playing music while off, and NOT in the settings menu, change the music volume only.
        if(encoder.STATE == ENCODER_CW) {
            // Increase the music volume.
            increaseVolumeMusic();
            debug(F("Device Off, Music Vol+ "));
            debugln(i_volume_music);
        }
        else if(encoder.STATE == ENCODER_CCW) {
            // Decrease the music volume.
            decreaseVolumeMusic();
            debug(F("Device Off, Music Vol- "));
            debugln(i_volume_music);
        }
      }

      switch(DEVICE_ACTION_STATUS) {
        case ACTION_IDLE:
        case ACTION_OFF:
        case ACTION_ACTIVATE:
        case ACTION_FIRING:
        case ACTION_ERROR:
          // No-Op.
        break;

        case ACTION_SETTINGS:
          // Perform menu and option navigation.
          encoderChangedMenuOption();

          // Respond to button actions based on menu level/option.
          settingsMenuCheck();
        break;

        case ACTION_CONFIG_EEPROM_MENU:
          // Perform menu and option navigation.
          encoderChangedMenuOption();
        break;
      }
    break; // MODE_OFF

    case MODE_ERROR:
      // Allow fast change of system volume when in error mode.
      if(encoder.STATE == ENCODER_CW) {
        // Increase the overall system volume.
        increaseVolume();
        debug(F("Error, System Vol+ "));
        debugln(i_volume_master);
      }
      else if(encoder.STATE == ENCODER_CCW) {
        // Decrease the overall system volume.
        decreaseVolume();
        debug(F("Error, System Vol- "));
        debugln(i_volume_master);
      }
    break; // MODE_ERROR

    case MODE_ON:
      if(DEVICE_ACTION_STATUS == ACTION_SETTINGS && !switch_intensify.on() && !switch_vent.on() && !switch_device.on()) {
        // Perform menu and option navigation while device is activated but in settings mode.
        encoderChangedMenuOption();
      }

      // Cater to actions while the device is on and idle
      if(DEVICE_ACTION_STATUS == ACTION_IDLE) {
        // Toggle switches are all on, which means power level can be changed.
        if(switch_activate.on() && switch_device.on() && switch_vent.on()) {
          if(encoder.STATE == ENCODER_CW) {
            if(increasePowerLevel()) {
              soundIdleLoopStop();
              soundIdleLoop(false);
            }
          }
          else if(encoder.STATE == ENCODER_CCW) {
            if(decreasePowerLevel()) {
              soundIdleLoopStop();
              soundIdleLoop(false);
            }
          }
        }

        // Intensify button is pressed while activated, but the device/vent toggles are both off.
        if(switch_intensify.on() && switch_activate.on() && !switch_vent.on() && !switch_device.on()) {
          if(encoder.STATE == ENCODER_CW) {
            // Increase the master system volume.
            increaseVolume();
            debug(F("Device On, System Vol+ "));
            debugln(i_volume_master);
          }
          else if(encoder.STATE == ENCODER_CCW) {
            // Decrease the master system volume.
            decreaseVolume();
            debug(F("Device On, System Vol- "));
            debugln(i_volume_master);
          }
        }
      }
    break; // MODE_ON
  }
}