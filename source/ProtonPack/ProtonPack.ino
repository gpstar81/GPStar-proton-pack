/**
 *   gpstar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gmail.com>
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

/* 
 *  You need to edit wavTrigger.h and make sure you comment out the proper serial port. (Near the top of the wavTrigger.h file).
 *  We are going to use tx/rx #3 on the Mega and on the gpstar Proton Pack micro controller board.  
 *  __WT_USE_SERIAL3___
 */
#include <wavTrigger.h>

#include <millisDelay.h> 
#include <FastLED.h>
#include <ezButton.h>
#include <Ramp.h>
#include <SerialTransfer.h>
#include "Configuration.h"
#include "MusicSounds.h"
#include "Communication.h"
#include "Header.h"

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600); // Communication to the wand.
  packComs.begin(Serial2);

  // Setup the Wav Trigger.
  setupWavTrigger();
  
  // Rotary encoder for volume control.
  pinMode(encoder_pin_a, INPUT_PULLUP);
  pinMode(encoder_pin_b, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder_pin_a), readEncoder, CHANGE);

  // Configure the various switches on the pack.
  switch_cyclotron_lid.setDebounceTime(50);
  switch_alarm.setDebounceTime(50);
  switch_mode.setDebounceTime(50);
  switch_vibration.setDebounceTime(50);
  switch_cyclotron_direction.setDebounceTime(50);
  switch_smoke.setDebounceTime(50);
  
  // Adjust the pwm frequency of the vibration motor.
  TCCR5B = (TCCR5B & B11111000) | (B00000100);  // for PWM frequency of 122.55 Hz

  // Vibration motor
  pinMode(vibration, OUTPUT);

  // Smoke motor
  pinMode(smoke_pin, OUTPUT);

  // Fan pin for the n-filter smoke.
  pinMode(fan_pin, OUTPUT);

  // Second smoke motor (booster tube)
  pinMode(smoke_booster_pin, OUTPUT);

  // A fan pin that goes off at the same time of the booster tube smoke pin.
  pinMode(fan_booster_pin, OUTPUT);
  
  // Another optional N-Filter LED.
  pinMode(i_nfilter_led_pin, OUTPUT);

  // Powercell and Cyclotron Lid.
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, PACK_NUM_LEDS);

  // Inner Cyclotron LEDs.
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, CYCLOTRON_NUM_LEDS);
  //FastLED.setMaxPowerInVoltsAndMilliamps(5,500);  // Limit draw to 500mA at 5v of power.
   
  // Cyclotron Switch Panel LEDs
  pinMode(cyclotron_sw_plate_led_r1, OUTPUT);
  pinMode(cyclotron_sw_plate_led_r2, OUTPUT);
  pinMode(cyclotron_sw_plate_led_y1, OUTPUT);
  pinMode(cyclotron_sw_plate_led_y2, OUTPUT);
  pinMode(cyclotron_sw_plate_led_g1, OUTPUT);
  pinMode(cyclotron_sw_plate_led_g2, OUTPUT);
  pinMode(cyclotron_switch_led_green, OUTPUT);
  pinMode(cyclotron_switch_led_yellow, OUTPUT);
  
  // Misc configuration before startup.
  resetCyclotronLeds();

  // Bootup the pack into Proton mode, the same as the wand.
  FIRING_MODE = PROTON;

  switch(i_mode_year) {
    case 1984:
    case 1989:
      i_current_ramp_speed = i_1984_delay * 1.3;
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;

    case 2021:          
      i_current_ramp_speed = i_2021_ramp_delay;
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;
  }

  // Start some timers
  ms_cyclotron.start(i_current_ramp_speed);
  ms_cyclotron_ring.start(i_inner_current_ramp_speed);
  ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);
  ms_wand_handshake.start(1);
  ms_fast_led.start(i_fast_led_delay);

  // Configure the vibration state.
  if(switch_vibration.getState() == LOW) {
    b_vibration_enabled = true;
  }
  else {
    b_vibration_enabled = false;
  }

  // Configure the year mode.
  if(switch_mode.getState() == LOW) {
    i_mode_year = 1984;
  }
  else {
    i_mode_year = 2021;
  }

  // Tell the wand the pack is here.
  packSerialSend(P_PACK_BOOTUP);
}

void loop() {
  w_trig.update();
  
  cyclotronSwitchPlateLEDs();

  wandHandShake();
  checkWand();
  checkFan();
  
  switch_cyclotron_lid.loop();
  switch_alarm.loop();
  switch_cyclotron_direction.loop();
  switch_mode.loop();
  switch_vibration.loop();
  switch_power.loop();
  switch_smoke.loop();

  checkSwitches();
  checkRotaryEncoder();

  switch (PACK_STATUS) {
    case MODE_OFF:
      if(b_pack_on == true) {
        b_2021_ramp_up = false;
        b_2021_ramp_up_start = false;
        b_inner_ramp_up = false;

        reset2021RampDown();

        b_pack_shutting_down = true;
      }
      
      if(b_2021_ramp_down == true && b_overheating == false && b_alarm == false) {
        cyclotronSwitchLEDLoop();        
        powercellLoop();
        cyclotronControl();
      }
      else {   
        powercellOff();     
        cyclotronSwitchLEDOff();

        // Reset the power cell timer.
        ms_powercell.stop();
        ms_powercell.start(i_powercell_delay);

        // Reset the cyclotron led switch timer.
        ms_cyclotron_switch_led.stop();
        ms_cyclotron_switch_led.start(i_cyclotron_switch_led_delay);
        
        // Need to reset the cyclotron timers.
        ms_cyclotron.start(i_2021_delay);
        ms_cyclotron_ring.start(i_inner_ramp_delay);
        
        b_overheating = false;
        b_2021_ramp_down = false;
        b_2021_ramp_down_start = false;
        b_reset_start_led = true; // reset the start led of the cyclotron.
        b_inner_ramp_down = false;
                
        resetCyclotronLeds();        
        reset2021RampUp();

        // Update Cyclotron LED timer delay and optional cyclotron led switch plate LED timers delays.
        switch(i_mode_year) {
          case 2021:
            i_powercell_delay = i_powercell_delay_2021;
            i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base;
          break;

          case 1984:
          case 1989:
            i_powercell_delay = i_powercell_delay_1984;
            i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base * 4;
          break;
        }

        // Vibration motor off.
        vibrationPack(0);
        i_vibration_level = 0;

        if(b_pack_shutting_down == true) {
          b_pack_shutting_down = false;

          clearCyclotronFades();
        }

        // Tell the wand the alarm is off.
        if(b_alarm == true) {
          b_alarm = false;
          packSerialSend(P_ALARM_OFF);
        }
      }
      
      if(b_pack_on == true) {
        // Tell the wand the pack is off, so shut down the wand as well if it is still on.
        packSerialSend(P_OFF);
      }

      b_pack_on = false;
    break;
   
    case MODE_ON:
      if(b_pack_shutting_down == true) {
        b_pack_shutting_down = false;
      }
      
      if(b_pack_on == false) {
        // Tell the wand the pack is on.
        packSerialSend(P_ON);
      }
      
      b_pack_on = true;
      
      if(b_2021_ramp_down == true) {
        b_2021_ramp_down = false;
        b_2021_ramp_down_start = false;
        b_inner_ramp_down = false;

        reset2021RampUp();
      }
        
      if(switch_alarm.getState() == LOW && b_overheating == false) {
        if(b_alarm == true) {
          if(i_mode_year == 1984 || i_mode_year == 1989) {
            // Reset the LEDs before resetting the alarm flag.
            resetCyclotronLeds();
            ms_cyclotron.start(0);
          }
          else {
            ms_cyclotron.start(i_current_ramp_speed);
          }

          ms_cyclotron_ring.start(i_inner_current_ramp_speed);  

          ventLight(false);
          
          b_alarm = false;

          reset2021RampUp();
          
          packStartup(); 
        }
      }

      // Play a little bit of smoke and n-filter vent lights while firing. Just a tiny bit....
      if(b_wand_firing == true) {
        if(ms_smoke_on.justFinished()) {
          ms_smoke_on.stop();
          ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
          b_vent_sounds = true;
        }
        
        if(ms_smoke_timer.justFinished()) {
          if(ms_smoke_on.isRunning() != true) {
            ms_smoke_on.start(i_smoke_on_time[i_wand_power_level - 1]);
          }
        }

        if(ms_smoke_on.isRunning() == true) {
          // Turn on some smoke and play some vent sounds if smoke is enabled.
          if(b_smoke_enabled == true) {
            // Turn on some smoke.
            smokeControl(true);
  
            // Play some sounds with the smoke and vent lighting.
            if(b_vent_sounds == true) {
              w_trig.trackGain(S_VENT_SMOKE, i_volume);
              w_trig.trackPlayPoly(S_VENT_SMOKE);
              w_trig.trackGain(S_SPARKS_LOOP, i_volume);
              w_trig.trackPlayPoly(S_SPARKS_LOOP);
  
              b_vent_sounds = false;
            }
          
            fanControl(true);
          }
          
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
        }
        else {
          smokeControl(false);
          ventLight(false);
          fanControl(false);
        }
      }
      
      cyclotronSwitchLEDLoop();

      if(b_overheating == true && b_overheat_lights_off == true) {
        powercellRampDown();
      }
      else {
        powercellLoop();
      }

      cyclotronControl();
    break;
   }

  switch(PACK_ACTION_STATUS) {
    case ACTION_IDLE:
      // Do nothing.
    break;

    case ACTION_OFF:
      packShutdown();
    break;

    case ACTION_ACTIVATE:
      packStartup();
    break;
  }

  // Update the LEDs
  if(ms_fast_led.justFinished()) {
    //FastLED.delay(3);
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

void packStartup() {
  PACK_STATUS = MODE_ON;
  PACK_ACTION_STATUS = ACTION_IDLE;

  if(b_alarm == true) {
    if(i_mode_year == 1984 || i_mode_year == 1989) {
      ms_cyclotron.start(0);
      ms_alarm.start(0);
    }

    packAlarm();

    // Tell the wand the pack alarm is off.
    packSerialSend(P_ALARM_ON);
  }
  else {
    // Tell the wand the pack alarm is off.
    packSerialSend(P_ALARM_OFF);

    w_trig.trackStop(S_PACK_SHUTDOWN_AFTERLIFE);
    
    switch(i_mode_year) {
      case 1984:
        w_trig.trackStop(S_PACK_RIBBON_ALARM_1);

        w_trig.trackGain(S_BOOTUP, i_volume);
        w_trig.trackPlayPoly(S_BOOTUP, true);

        w_trig.trackGain(S_IDLE_LOOP, 0);
        w_trig.trackPlayPoly(S_IDLE_LOOP, true);
        w_trig.trackFade(S_IDLE_LOOP, i_volume, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP, 1);
      break;

      case 1989:
        w_trig.trackStop(S_PACK_RIBBON_ALARM_1);

        w_trig.trackGain(S_GB2_PACK_START, i_volume);
        w_trig.trackPlayPoly(S_GB2_PACK_START, true);

        w_trig.trackGain(S_GB2_PACK_LOOP, 0);
        w_trig.trackPlayPoly(S_GB2_PACK_LOOP, true);
        w_trig.trackFade(S_GB2_PACK_LOOP, i_volume, 3000, 0);
        w_trig.trackLoop(S_GB2_PACK_LOOP, 1);
      break;

      case 2021:
        w_trig.trackStop(S_PACK_RIBBON_ALARM_1);

        w_trig.trackGain(S_AFTERLIFE_PACK_STARTUP, i_volume);
        w_trig.trackPlayPoly(S_AFTERLIFE_PACK_STARTUP, true);
  
        w_trig.trackGain(S_AFTERLIFE_PACK_IDLE_LOOP, 0);
        w_trig.trackPlayPoly(S_AFTERLIFE_PACK_IDLE_LOOP, true);
        w_trig.trackFade(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume, 18000, 0);
        w_trig.trackLoop(S_AFTERLIFE_PACK_IDLE_LOOP, 1);
      break;
    }
  }
}

void packShutdown() {
  PACK_STATUS = MODE_OFF;
  PACK_ACTION_STATUS = ACTION_IDLE;
  
  // Stop the firing if the pack is doing it.
  wandStoppedFiring();

  switch(i_mode_year) {
    case 1984:
    case 1989:
    case 2021:
      w_trig.trackStop(S_PACK_RIBBON_ALARM_1);
    break;

    // Not used.
    default:
      w_trig.trackStop(S_PACK_BEEPING);
    break;
  }

  w_trig.trackStop(S_BEEP_8);
  w_trig.trackStop(S_SHUTDOWN);

  w_trig.trackStop(S_GB2_PACK_START);
  w_trig.trackStop(S_GB2_PACK_LOOP);
  w_trig.trackStop(S_GB2_PACK_OFF);
  w_trig.trackStop(S_PACK_SHUTDOWN);
  w_trig.trackStop(S_PACK_SHUTDOWN_AFTERLIFE);
  w_trig.trackStop(S_IDLE_LOOP);
  w_trig.trackStop(S_BOOTUP);
  w_trig.trackStop(S_AFTERLIFE_PACK_STARTUP);
  w_trig.trackStop(S_AFTERLIFE_PACK_IDLE_LOOP);

  if(b_alarm != true) {
    switch(i_mode_year) {
      case 1984:      
        w_trig.trackGain(S_SHUTDOWN, i_volume);
        w_trig.trackPlayPoly(S_SHUTDOWN, true);
        w_trig.trackGain(S_PACK_SHUTDOWN, i_volume);
        w_trig.trackPlayPoly(S_PACK_SHUTDOWN, true);
      break;

      case 1989:
        w_trig.trackGain(S_SHUTDOWN, i_volume);
        w_trig.trackPlayPoly(S_SHUTDOWN, true);
        w_trig.trackGain(S_GB2_PACK_OFF, i_volume);
        w_trig.trackPlayPoly(S_GB2_PACK_OFF, true);
      break;
  
      case 2021:
        w_trig.trackGain(S_PACK_SHUTDOWN_AFTERLIFE, i_volume);
        w_trig.trackPlayPoly(S_PACK_SHUTDOWN_AFTERLIFE, true);
      break;
    }
  }
  else {
    w_trig.trackGain(S_SHUTDOWN, i_volume);
    w_trig.trackPlayPoly(S_SHUTDOWN, true);
  }

  // Turn off the vent light if it is on.
  ventLight(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  // Turn off any smoke.
  smokeControl(false);
  ms_smoke_timer.stop();
  ms_smoke_on.stop();
  
  // Turn off the n-filter fan.
  ms_fan_stop_timer.stop();
  fanControl(false);
  
  // Reset vent sounds flag.
  b_vent_sounds = true;
}

void checkSwitches() {
  // Cyclotron direction toggle switch.
  if(switch_cyclotron_direction.isPressed() || switch_cyclotron_direction.isReleased()) {
    if(b_clockwise == true) {
      b_clockwise = false;

      w_trig.trackStop(S_BEEPS_ALT);    
      w_trig.trackGain(S_BEEPS_ALT, i_volume);
      w_trig.trackPlayPoly(S_BEEPS_ALT);

      w_trig.trackStop(S_VOICE_CYCLOTRON_CLOCKWISE);
      w_trig.trackStop(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);    
      w_trig.trackGain(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE, i_volume);
      w_trig.trackPlayPoly(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE); 

      // Tell wand to play cyclotron counter clockwise voice.
      packSerialSend(P_CYCLOTRON_COUNTER_CLOCKWISE);
    }
    else {
      b_clockwise = true;

      w_trig.trackStop(S_BEEPS);
      w_trig.trackGain(S_BEEPS, i_volume);
      w_trig.trackPlayPoly(S_BEEPS);

      w_trig.trackStop(S_VOICE_CYCLOTRON_CLOCKWISE);
      w_trig.trackStop(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);    
      w_trig.trackGain(S_VOICE_CYCLOTRON_CLOCKWISE, i_volume);
      w_trig.trackPlayPoly(S_VOICE_CYCLOTRON_CLOCKWISE);
    
      // Tell wand to play cyclotron clockwise voice.
      packSerialSend(P_CYCLOTRON_CLOCKWISE);
    }
  }
  
  // Smoke
  if(switch_smoke.isPressed() || switch_smoke.isReleased()) {
    if(b_smoke_enabled == true) {
      b_smoke_enabled = false;

      w_trig.trackStop(S_VENT_DRY);
      w_trig.trackGain(S_VENT_DRY, i_volume);
      w_trig.trackPlayPoly(S_VENT_DRY);

      w_trig.trackStop(S_VOICE_SMOKE_DISABLED);
      w_trig.trackStop(S_VOICE_SMOKE_ENABLED);    
      w_trig.trackGain(S_VOICE_SMOKE_DISABLED, i_volume);
      w_trig.trackPlayPoly(S_VOICE_SMOKE_DISABLED);

      // Tell wand to play smoke disabled voice.
      packSerialSend(P_SMOKE_DISABLED);  
    }
    else {
      b_smoke_enabled = true;

      w_trig.trackStop(S_VENT_SMOKE);
      w_trig.trackGain(S_VENT_SMOKE, i_volume);
      w_trig.trackPlayPoly(S_VENT_SMOKE);

      w_trig.trackStop(S_VOICE_SMOKE_ENABLED);
      w_trig.trackStop(S_VOICE_SMOKE_DISABLED);    
      w_trig.trackGain(S_VOICE_SMOKE_ENABLED, i_volume);
      w_trig.trackPlayPoly(S_VOICE_SMOKE_ENABLED);  

      // Tell wand to play smoke enabled voice.
      packSerialSend(P_SMOKE_ENABLED);    
    }
  }

  // Vibration toggle switch.
  if(switch_vibration.isPressed() || switch_vibration.isReleased()) {
      w_trig.trackStop(S_BEEPS_ALT);    
      w_trig.trackGain(S_BEEPS_ALT, i_volume);
      w_trig.trackPlayPoly(S_BEEPS_ALT);

      if(switch_vibration.getState() == LOW) {
        if(b_vibration_enabled == false) {
          // Tell the wand to enable vibration.
          packSerialSend(P_VIBRATION_ENABLED);

          b_vibration_enabled = true;

          w_trig.trackStop(S_VOICE_VIBRATION_ENABLED);    
          w_trig.trackStop(S_VOICE_VIBRATION_DISABLED);    
          w_trig.trackGain(S_VOICE_VIBRATION_ENABLED, i_volume);
          w_trig.trackPlayPoly(S_VOICE_VIBRATION_ENABLED);          
        }
      }
      else {
        if(b_vibration_enabled == true) {
          // Tell the wand to disable vibration.
          packSerialSend(P_VIBRATION_DISABLED);

          b_vibration_enabled = false;

          w_trig.trackStop(S_VOICE_VIBRATION_DISABLED);    
          w_trig.trackStop(S_VOICE_VIBRATION_ENABLED);    
          w_trig.trackGain(S_VOICE_VIBRATION_DISABLED, i_volume);
          w_trig.trackPlayPoly(S_VOICE_VIBRATION_DISABLED);          
        }
      }
  }

  // Play sound when the year mode switch is pressed or released.
  if(switch_mode.isPressed() || switch_mode.isReleased()) {
    w_trig.trackStop(S_BEEPS_BARGRAPH);    
    w_trig.trackGain(S_BEEPS_BARGRAPH, i_volume);
    w_trig.trackPlayPoly(S_BEEPS_BARGRAPH);

    // Turn off the year mode override flag controlled by the Neutrona wand.
    b_switch_mode_override = false;
  }
  
  switch(PACK_STATUS) {
    case MODE_OFF:     
      if(switch_power.isPressed() || switch_power.isReleased()) {
        // Turn the pack on.
        PACK_ACTION_STATUS = ACTION_ACTIVATE;
      }

      // Year mode. Best to adjust it only when the pack is off.
      if(b_2021_ramp_down != true && b_pack_on == false) {
        // If switching manually by the pack toggle switch.
        if(b_switch_mode_override != true) {     
          if(switch_mode.getState() == LOW) {
            if(i_mode_year == 2021) {
              // Tell the wand to switch to 1984 mode.
              packSerialSend(P_YEAR_1984);
            }
            
            i_mode_year = 1984;
            i_mode_year_tmp = 1984;
          }
          else {
            if(i_mode_year == 1984) {
              // Tell the wand to switch to 2021 mode.
              packSerialSend(P_YEAR_AFTERLIFE);
            }

            i_mode_year = 2021;
            i_mode_year_tmp = 2021;
          }
        }
        else {
          // If the Neutrona wand sub menu setting told the Proton Pack to change years.
          switch(i_mode_year_tmp) {
            case 1984:
              if(i_mode_year == 2021) {
                // Tell the wand to switch to 1984 mode.
                packSerialSend(P_YEAR_1984);
              }

              i_mode_year = 1984;
              i_mode_year_tmp = 1984;
            break;

            case 1989:
              if(i_mode_year == 1984) {
                // Tell the wand to switch to 1989 mode.
                packSerialSend(P_YEAR_1989);
              }

              i_mode_year = 1989;
              i_mode_year_tmp = 1989;
            break;

            case 2021:
              if(i_mode_year == 1989) {
                // Tell the wand to switch to 2021 mode.
                packSerialSend(P_YEAR_AFTERLIFE);
              }

              i_mode_year = 2021;
              i_mode_year_tmp = 2021;
            break;
          }
        }

        // Reset the ramp speeds.
        switch(i_mode_year) {
          case 1984:
          case 1989:
              // Reset the ramp speeds.
              i_current_ramp_speed = i_1984_delay * 1.3;
              i_inner_current_ramp_speed = i_inner_ramp_delay;
          break;

          case 2021:
            // Reset the ramp speeds.
            i_current_ramp_speed = i_2021_ramp_delay;
            i_inner_current_ramp_speed = i_inner_ramp_delay;
          break;
        }
      }
    break;

    case MODE_ON:
      if(switch_power.isReleased() || switch_power.isPressed()) {
        // Turn the pack off.
        PACK_ACTION_STATUS = ACTION_OFF;
      }
    break;
  }
}

void cyclotronSwitchLEDOff() {
  digitalWrite(cyclotron_sw_plate_led_r1, LOW);
  digitalWrite(cyclotron_sw_plate_led_r2, LOW);
  
  digitalWrite(cyclotron_sw_plate_led_y1, LOW);
  digitalWrite(cyclotron_sw_plate_led_y2, LOW);
  
  digitalWrite(cyclotron_sw_plate_led_g1, LOW);
  digitalWrite(cyclotron_sw_plate_led_g2, LOW);
  
  i_cyclotron_sw_led = 0;
}

void cyclotronSwitchLEDLoop() {
  if(ms_cyclotron_switch_led.justFinished()) {
    if(b_cyclotron_lid_on != true) {
      if(b_alarm == true) {
        if(i_cyclotron_sw_led > 0) {
          digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
          digitalWrite(cyclotron_sw_plate_led_r2, HIGH);
          
          digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
          digitalWrite(cyclotron_sw_plate_led_y2, HIGH);
          
          digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
          digitalWrite(cyclotron_sw_plate_led_g2, HIGH);
            
          i_cyclotron_sw_led = 0;
        }
        else {
          digitalWrite(cyclotron_sw_plate_led_r1, LOW);
          digitalWrite(cyclotron_sw_plate_led_r2, LOW);
          
          digitalWrite(cyclotron_sw_plate_led_y1, LOW);
          digitalWrite(cyclotron_sw_plate_led_y2, LOW);
          
          digitalWrite(cyclotron_sw_plate_led_g1, LOW);
          digitalWrite(cyclotron_sw_plate_led_g2, LOW);
            
          i_cyclotron_sw_led++;
        }
      }
      else {
        switch(i_cyclotron_sw_led) {
          case 0:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);
      
            i_cyclotron_sw_led++;
          break;
      
          case 1:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);
      
            i_cyclotron_sw_led++;
          break;
      
          case 2:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);
      
            i_cyclotron_sw_led++;
          break;
      
          case 3:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);
      
            i_cyclotron_sw_led++;
          break;
      
          case 4:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);
      
            i_cyclotron_sw_led++;
          break;
          
          case 5:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);
      
            i_cyclotron_sw_led++;
          break;
      
          case 6:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);
            
            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);
      
            i_cyclotron_sw_led++;
          break;  
      
          case 7:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);
            
            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);
      
            i_cyclotron_sw_led = 0;
          break;
        }
      }
    }
    else {
      // No need to have the inner cyclotron switch plate LED's on when the lid is on.
     cyclotronSwitchLEDOff();
    }
    
    // Setup the delays again.
    int i_cyc_led_delay = i_cyclotron_switch_led_delay / i_cyclotron_switch_led_mulitplier;

    switch(i_mode_year) {
      case 2021:
        if(b_2021_ramp_up == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + (i_2021_ramp_delay - r_2021_ramp.update());
        }
        else if(b_2021_ramp_down == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + r_2021_ramp.update();
        }
      break;

      case 1984:
      case 1989:
        if(b_2021_ramp_up == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + (r_2021_ramp.update() - i_1984_delay);
        }
        else if(b_2021_ramp_down == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay / 6 + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_cyc_led_delay = i_cyclotron_switch_led_delay * 2;
    }

    ms_cyclotron_switch_led.start(i_cyc_led_delay);    
  }
}

void powercellRampDown() {
 if(ms_powercell.justFinished()) {
    int i_extra_delay = 0;
    
    // Powercell
    if(i_powercell_led < 0) {
      // Do Nothing.
    }
    else {     
      pack_leds[i_powercell_led] = CRGB(0,0,0);
      
      i_powercell_led--;
    }
    
    // Setup the delays again.
    int i_pc_delay = i_powercell_delay;

    switch(i_mode_year) {
      case 1984:
      case 1989:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
      break;

      case 2021:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 3;
    }

    ms_powercell.start(i_pc_delay + i_extra_delay);
  }
}

void powercellLoop() {
  if(ms_powercell.justFinished()) {
    int i_extra_delay = 0;
    
    // Powercell
    if(i_powercell_led > cyclotron_led_start - 1) {
      powercellOff();
  
      i_powercell_led = 0;
    }
    else {
      uint8_t r = 0;
      uint8_t g = 0;
      uint8_t b = 255;

      if(b_powercell_colour_toggle == true) {
        switch(FIRING_MODE) {
          case PROTON:
            r = 0;
            g = 0;
            b = 255;
          break;
      
          case SLIME:
            r = 0;
            g = 255;
            b = 0;
          break;
      
          case STASIS:
            r = 0;
            g = 0;
            b = 255;
          break;
      
          case MESON:
            r = 255;
            g = 255;
            b = 0;
          break;   
      
          default:
            r = 0;
            g = 0;
            b = 255;
          break;
        }
      }

      pack_leds[i_powercell_led] = CRGB(r,g,b);

      // Add a small delay to pause the powercell when all powercell LEDs are lit up, to match the 2021 pack.
      if(i_mode_year == 2021 && b_alarm != true && i_powercell_led == cyclotron_led_start - 1) {
        i_extra_delay = 250;
      }
      
      i_powercell_led++;
    }
    
    // Setup the delays again.
    int i_pc_delay = i_powercell_delay;

    switch(i_mode_year) {
      case 1984:
      case 1989:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
      break;

      case 2021:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 5;
    }

    // Speed up the power cell when the cyclotron speeds up before a overheat.
    unsigned int i_multiplier = 0;
    
    if(i_cyclotron_multiplier > 1) {
      switch(i_cyclotron_multiplier) {
        case 2:
          if(i_mode_year == 2021) {
            i_multiplier = 5;
          }
          else {
            i_multiplier = 10;
          }
        break;

        case 3:
          if(i_mode_year == 2021) {
            i_multiplier = 10;
          }
          else {
            i_multiplier = 20;
          }
        break;

        case 4:
          if(i_mode_year == 2021) {
            i_multiplier = 15;
          }
          else {
            i_multiplier = 30;
          }
        break;

        case 5:
          if(i_mode_year == 2021) {
            i_multiplier = 25;
          }
          else {
            i_multiplier = 40;
          }
        break;

        case 6:
          if(i_mode_year == 2021) {
            i_multiplier = 30;
          }
          else {
            i_multiplier = 50;
          }
        break;
      }
    }

    ms_powercell.start((i_pc_delay + i_extra_delay) - i_multiplier);
  }
}

void powercellOn() {
  for(int i = 0; i <= cyclotron_led_start - 1; i++) {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 255;

    if(b_powercell_colour_toggle == true) {
      switch(FIRING_MODE) {
        case PROTON:
          r = 0;
          g = 0;
          b = 255;
        break;
    
        case SLIME:
          r = 0;
          g = 255;
          b = 0;
        break;
    
        case STASIS:
          r = 0;
          g = 0;
          b = 255;
        break;
    
        case MESON:
          r = 255;
          g = 255;
          b = 0;
        break;   
    
        default:
          r = 0;
          g = 0;
          b = 255;
        break;
      }   
    }

    pack_leds[i] = CRGB(r,g,b);
  }

  i_powercell_led = cyclotron_led_start - 1;
}

void powercellOff() {
  for(int i = 0; i <= cyclotron_led_start - 1; i++) {
    pack_leds[i] = CRGB(0,0,0);
  }
   
  i_powercell_led = 0;
}

void cyclotronControl() {
  // Only reset the starting led when the pack is first started up.
  if(b_reset_start_led == true) {
    b_reset_start_led = false;
    if(b_clockwise == false) {
      if(i_mode_year == 2021) {
        i_led_cyclotron = cyclotron_led_start + 2; // Start on LED #2 in anti-clockwise mode in 2021 mode.
      }
      else {
        i_1984_counter = 1;
        i_led_cyclotron = cyclotron_led_start + i_1984_cyclotron_leds[i_1984_counter];      
      }
    }
    else {
      if(i_mode_year == 1984 || i_mode_year == 1989) {
        i_1984_counter = 3;
        i_led_cyclotron = cyclotron_led_start + i_1984_cyclotron_leds[i_1984_counter];
        
      }
      else {
        i_led_cyclotron = cyclotron_led_start;
      }
    }
  }
        
  if(switch_alarm.getState() == HIGH && PACK_STATUS != MODE_OFF && b_2021_ramp_down_start != true && b_overheating == false) {
    if(b_alarm == false) {
      w_trig.trackStop(S_BEEP_8);

      b_2021_ramp_up = false;
      b_inner_ramp_up = false;
      b_alarm = true;
      
      if(i_mode_year == 1984 || i_mode_year == 1989) {
        resetCyclotronLeds();
        ms_cyclotron.start(0);
        ms_alarm.start(0);
      }
      else {
        ms_alarm.start(i_alarm_delay);
      }

      packAlarm();

      // Tell the wand the pack alarm is on.
      packSerialSend(P_ALARM_ON);
    }
    
    // Ribbon cable has been removed.
    cyclotronNoCable();
  }
  else if(b_overheating == true) {
    if(b_alarm == false) {
      w_trig.trackStop(S_BEEP_8);

      b_2021_ramp_up = false;
      b_inner_ramp_up = false;
      
      if(i_mode_year == 1984 || i_mode_year == 1989) {
        resetCyclotronLeds();
        ms_cyclotron.start(0);
        ms_alarm.start(0);
      }
      else {
        ms_alarm.start(i_alarm_delay);
      }

      if(b_overheat_lights_off == true) {
        cyclotronLidLedsOff();
      
        if(b_cyclotron_lid_on != true) {
          innerCyclotronOff();
        }

        powercellOn();
      }

      b_alarm = true;

      packAlarm();
    }
    
    cyclotronOverHeating();
  }
  else {
    if(b_2021_ramp_up_start == true) {
      b_2021_ramp_up_start = false;

      if(i_mode_year == 1984 || i_mode_year == 1989) {
        r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
        r_2021_ramp.go(i_1984_delay, i_1984_ramp_length, CIRCULAR_OUT);

        r_inner_ramp.go(i_inner_current_ramp_speed); // Inner cyclotron ramp reset.
        r_inner_ramp.go(i_1984_inner_delay, i_1984_ramp_length, CIRCULAR_OUT);
      }
      else {
        r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
        r_2021_ramp.go(i_2021_delay, i_2021_ramp_length, CIRCULAR_OUT);
        r_inner_ramp.go(i_inner_current_ramp_speed);
        r_inner_ramp.go(i_2021_inner_delay, i_2021_ramp_length, CIRCULAR_OUT);
      }
    }
    else if(b_2021_ramp_down_start == true) {
      b_2021_ramp_down_start = false;

      r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
      r_inner_ramp.go(i_inner_current_ramp_speed); // Reset the inner cyclotron ramp.

      if(i_mode_year == 1984 || i_mode_year == 1989) {
        r_2021_ramp.go(i_1984_delay * 1.3, i_1984_ramp_down_length, CIRCULAR_IN); 

        r_inner_ramp.go(i_inner_ramp_delay, i_1984_ramp_down_length, CIRCULAR_IN); 
      }
      else {
        r_2021_ramp.go(i_2021_ramp_delay, i_2021_ramp_down_length, SINUSOIDAL_IN);
        r_inner_ramp.go(i_inner_ramp_delay, i_2021_ramp_down_length, SINUSOIDAL_IN);
      }
    }
  
    if(i_mode_year == 1984 || i_mode_year == 1989) {
      cyclotron1984(i_current_ramp_speed);
      innerCyclotronRing(i_inner_current_ramp_speed);
    }
    else {
      cyclotron2021(i_current_ramp_speed);
      innerCyclotronRing(i_inner_current_ramp_speed);
    }
  }

  cyclotronFade();
}

void cyclotronFade() {
  // Colour control for the cyclotron leds. (red,green,blue)
  int r = 0;
  int g = 0;
  int b = 0;

  switch (i_mode_year) {
    case 2021:
      for(int i = 0; i < PACK_NUM_LEDS - 7 - cyclotron_led_start; i++) {
        if(ms_cyclotron_led_fade_in[i].isRunning()) {
          i_cyclotron_led_on_status[i] = true;
          
          int i_led = ms_cyclotron_led_fade_in[i].update();
          
          if(b_cyclotron_colour_toggle == true) {
            switch(FIRING_MODE) {
              case PROTON:
                r = i_led;
                g = 0;
                b = 0;
              break;
          
              case SLIME:
                r = 0;
                g = i_led;
                b = 0;
              break;
          
              case STASIS:
                r = 0;
                g = 0;
                b = i_led;
              break;
          
              case MESON:
                r = i_led;
                g = i_led;
                b = 0;
              break;   
          
              default:
                r = i_led;
                g = 0;
                b = 0;
              break;
            }
          }
          else {
            r = i_led;
            g = 0;
            b = 0;
          }

          pack_leds[i + cyclotron_led_start] = CRGB(r,g,b);
          i_cyclotron_led_value[i] = i_led;
        }

        if(ms_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > 254 && i_cyclotron_led_on_status[i] == true) {
          if(b_cyclotron_colour_toggle == true) {
            switch(FIRING_MODE) {
              case PROTON:
                r = 255;
                g = 0;
                b = 0;
              break;
          
              case SLIME:
                r = 0;
                g = 255;
                b = 0;
              break;
          
              case STASIS:
                r = 0;
                g = 0;
                b = 255;
              break;
          
              case MESON:
                r = 255;
                g = 255;
                b = 0;
              break;   
          
              default:
                r = 255;
                g = 0;
                b = 0;
              break;
            }
          }
          else {
            r = 255;
            g = 0;
            b = 0;
          }

          pack_leds[i + cyclotron_led_start] = CRGB(r,g,b);
          i_cyclotron_led_value[i] = 255;
          i_cyclotron_led_on_status[i] = false;

          ms_cyclotron_led_fade_out[i].go(255);
          ms_cyclotron_led_fade_out[i].go(0, i_current_ramp_speed, CIRCULAR_OUT); 
        }

        if(ms_cyclotron_led_fade_out[i].isRunning() && i_cyclotron_led_on_status[i] == false) {
          int i_led = ms_cyclotron_led_fade_out[i].update();
          
          if(b_cyclotron_colour_toggle == true) {
            switch(FIRING_MODE) {
              case PROTON:
                r = i_led;
                g = 0;
                b = 0;
              break;
          
              case SLIME:
                r = 0;
                g = i_led;
                b = 0;
              break;
          
              case STASIS:
                r = 0;
                g = 0;
                b = i_led;
              break;
          
              case MESON:
                r = i_led;
                g = i_led;
                b = 0;
              break;   
          
              default:
                r = i_led;
                g = 0;
                b = 0;
              break;
            }
          }
          else {
            r = i_led;
            g = 0;
            b = 0;
          }

          pack_leds[i + cyclotron_led_start] = CRGB(r,g,b);
          i_cyclotron_led_value[i] = i_led;
        }
      
        if(ms_cyclotron_led_fade_out[i].isFinished() && i_cyclotron_led_on_status[i] == false) {
          pack_leds[i + cyclotron_led_start] = CRGB(0,0,0);
          i_cyclotron_led_value[i] = 0;
          i_cyclotron_led_on_status[i] = true;
        }
      }
    break;

    case 1984:
    case 1989:
      if(b_fade_cyclotron_led == true) {
        for(int i = 0; i < PACK_NUM_LEDS - 7 - cyclotron_led_start; i++) {
          if(ms_cyclotron_led_fade_in[i].isRunning()) {
            i_cyclotron_led_on_status[i] = true;
            int i_led = ms_cyclotron_led_fade_in[i].update();

            if(b_cyclotron_colour_toggle == true) {
              switch(FIRING_MODE) {
                case PROTON:
                  r = i_led;
                  g = 0;
                  b = 0;
                break;
            
                case SLIME:
                  r = 0;
                  g = i_led;
                  b = 0;
                break;
            
                case STASIS:
                  r = 0;
                  g = 0;
                  b = i_led;
                break;
            
                case MESON:
                  r = i_led;
                  g = i_led;
                  b = 0;
                break;   
            
                default:
                  r = i_led;
                  g = 0;
                  b = 0;
                break;
              }
            }
            else {
              r = i_led;
              g = 0;
              b = 0;
            }

            pack_leds[i + cyclotron_led_start] = CRGB(r,g,b);
            i_cyclotron_led_value[i] = i_led;
          }

          if(ms_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > 254 && i_cyclotron_led_on_status[i] == true) {
            if(b_cyclotron_colour_toggle == true) {
              switch(FIRING_MODE) {
                case PROTON:
                  r = 255;
                  g = 0;
                  b = 0;
                break;
            
                case SLIME:
                  r = 0;
                  g = 255;
                  b = 0;
                break;
            
                case STASIS:
                  r = 0;
                  g = 0;
                  b = 255;
                break;
            
                case MESON:
                  r = 255;
                  g = 255;
                  b = 0;
                break;   
            
                default:
                  r = 255;
                  g = 0;
                  b = 0;
                break;
              }
            }
            else {
              r = 255;
              g = 0;
              b = 0;
            }

            pack_leds[i + cyclotron_led_start] = CRGB(r,g,b);
            i_cyclotron_led_value[i] = 255;
          }

          if(ms_cyclotron_led_fade_out[i].isRunning()) {
            int i_led = ms_cyclotron_led_fade_out[i].update();

            if(b_cyclotron_colour_toggle == true) {
              switch(FIRING_MODE) {
                case PROTON:
                  r = i_led;
                  g = 0;
                  b = 0;
                break;
            
                case SLIME:
                  r = 0;
                  g = i_led;
                  b = 0;
                break;
            
                case STASIS:
                  r = 0;
                  g = 0;
                  b = i_led;
                break;
            
                case MESON:
                  r = i_led;
                  g = i_led;
                  b = 0;
                break;   
            
                default:
                  r = i_led;
                  g = 0;
                  b = 0;
                break;
              }
            }
            else {
              r = i_led;
              g = 0;
              b = 0;
            }

            pack_leds[i + cyclotron_led_start] = CRGB(r,g,b);
            i_cyclotron_led_value[i] = i_led;              
            i_cyclotron_led_on_status[i] = false;
          }
        
          if(ms_cyclotron_led_fade_out[i].isFinished() && i_cyclotron_led_on_status[i] == false) {
            pack_leds[i + cyclotron_led_start] = CRGB(0,0,0);
            i_cyclotron_led_value[i] = 0;
            i_cyclotron_led_on_status[i] = true;
          }
        }
      }
    break;
  }
}

void cyclotron2021(int cDelay) {
  if(ms_cyclotron.justFinished()) {
    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;
        ms_cyclotron.start(cDelay);
        
        i_current_ramp_speed = cDelay;

        i_vibration_level = i_vibration_idle_level_2021;
      }
      else {     
        ms_cyclotron.start(r_2021_ramp.update());
        
        i_current_ramp_speed = r_2021_ramp.update();

        i_vibration_level = i_vibration_level + 1;
        
        if(i_vibration_level < 30) {
          i_vibration_level = 30;
        }

        if(i_vibration_level > i_vibration_idle_level_2021) {
          i_vibration_level = i_vibration_idle_level_2021;
        }
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());

        i_current_ramp_speed = r_2021_ramp.update();
        
        if(i_current_ramp_speed > 40 && i_vibration_level > i_vibration_lowest_level + 20) {
          i_vibration_level = i_vibration_level - 1;
        }
        else if(i_current_ramp_speed > 100 && i_vibration_level > i_vibration_lowest_level) {
          i_vibration_level = i_vibration_level - 1;
        }
        
        if(i_vibration_level < i_vibration_lowest_level) {
          i_vibration_level = i_vibration_lowest_level;
        }
      }
    }
    else {
      i_current_ramp_speed = cDelay;
      
      if(i_cyclotron_multiplier > 1) {
        cDelay = cDelay / i_cyclotron_multiplier + 6;
      }

      ms_cyclotron.start(cDelay);
    }

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_vibration_level);
    }

    if(i_cyclotron_multiplier > 1) {
      cDelay = cDelay / i_cyclotron_multiplier + 6;
    }
    else {
      cDelay = cDelay / i_cyclotron_multiplier;
      
      cDelay = cDelay * 2;
    }

    if(cDelay < 1) {
      cDelay = 1;
    }
    
    if(b_clockwise == true) {
      if(i_cyclotron_led_value[i_led_cyclotron - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(255, cDelay, CIRCULAR_IN);
      }

      i_led_cyclotron++;
        
      if(i_led_cyclotron > PACK_NUM_LEDS - 7 - 1) {
        i_led_cyclotron = cyclotron_led_start;
      }   
    }
    else {
      if(i_cyclotron_led_value[i_led_cyclotron - cyclotron_led_start] == 0) {
       ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(255, cDelay, CIRCULAR_IN);
      }

      i_led_cyclotron--;
      
      if(i_led_cyclotron < cyclotron_led_start) {
        i_led_cyclotron = PACK_NUM_LEDS - 7 - 1;
      }
    }    
  }
}

void cyclotron1984(int cDelay) { 
  if(ms_cyclotron.justFinished()) {    
    cDelay = cDelay / i_cyclotron_multiplier;
    
    if(b_1984_led_start != true) {
      cyclotron84LightOff(i_led_cyclotron);
    }

    if(b_1984_led_start == true) {
      b_1984_led_start = false;
    }

    if(b_clockwise == true) {
      i_1984_counter++;
    }
    else {
      i_1984_counter--;
    }

    if(i_1984_counter > 3) {
      i_1984_counter = 0;
    }
    else if(i_1984_counter < 0) {
      i_1984_counter = 3;
    }

    i_led_cyclotron = cyclotron_led_start + i_1984_cyclotron_leds[i_1984_counter];
    
    cyclotron84LightOn(i_led_cyclotron);

    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;
        ms_cyclotron.start(cDelay);
        
        i_current_ramp_speed = cDelay;

        i_vibration_level = i_vibration_idle_level_1984;
      }
      else {       
        ms_cyclotron.start(r_2021_ramp.update());
        i_current_ramp_speed = r_2021_ramp.update();

        i_vibration_level = i_vibration_idle_level_1984;
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());
        
        i_current_ramp_speed = r_2021_ramp.update();
               
        i_vibration_level = i_vibration_level - 1;

        if(i_vibration_level < i_vibration_lowest_level) {
          i_vibration_level = i_vibration_lowest_level;
        }
      }
    }
    else {
      ms_cyclotron.start(cDelay);
    } 

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_vibration_level);
    }
  }
}

void cyclotron1984Alarm() {
  int8_t led1 = cyclotron_led_start + i_1984_cyclotron_leds[0];
  int8_t led2 = cyclotron_led_start + i_1984_cyclotron_leds[1];
  int8_t led3 = cyclotron_led_start + i_1984_cyclotron_leds[2];
  int8_t led4 = cyclotron_led_start + i_1984_cyclotron_leds[3];

  if(b_fade_cyclotron_led != true) {
    uint8_t r = 255;
    uint8_t g = 0;
    uint8_t b = 0;
    
    if(b_cyclotron_colour_toggle == true) {
      switch(FIRING_MODE) {
        case PROTON:
          r = 255;
          g = 0;
          b = 0;
        break;

        case SLIME:
          r = 0;
          g = 255;
          b = 0;
        break;

        case STASIS:
          r = 0;
          g = 0;
          b = 255;
        break;

        case MESON:
          r = 255;
          g = 255;
          b = 0;
        break;   

        default:
          r = 255;
          g = 0;
          b = 0;
        break;
      }
    }

    pack_leds[led1] = CRGB(r,g,b);
    pack_leds[led2] = CRGB(r,g,b);
    pack_leds[led3] = CRGB(r,g,b);
    pack_leds[led4] = CRGB(r,g,b);

    // Turn on all the other cyclotron LED's if required.
    if(b_cyclotron_single_led != true) {
      pack_leds[led1 + 1] = CRGB(r,g,b);

      if(led1 - 1 < cyclotron_led_start) {
        led1 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led1 = led1 - 1;
      }

      pack_leds[led1] = CRGB(r,g,b);


      pack_leds[led2 + 1] = CRGB(r,g,b);

      if(led2 - 1 < cyclotron_led_start) {
        led2 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led2 = led2 - 1;
      }

      pack_leds[led2] = CRGB(r,g,b);

      pack_leds[led3 + 1] = CRGB(r,g,b);

      if(led3 - 1 < cyclotron_led_start) {
        led3 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led3 = led3 - 1;
      }

      pack_leds[led3] = CRGB(r,g,b);
      
      pack_leds[led4 + 1] = CRGB(r,g,b);

      if(led4 - 1 < cyclotron_led_start) {
        led4 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led4 = led4 - 1;
      }

      pack_leds[led4] = CRGB(r,g,b);
    }
  }
  else {
    if(i_cyclotron_led_value[led1 - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);
    }
    
    if(i_cyclotron_led_value[led2 - cyclotron_led_start] == 0) {      
      ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led3 - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);      
    }

    if(i_cyclotron_led_value[led4 - cyclotron_led_start] == 0) {      
      ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);      
    }

    // Turn on all the other cyclotron LED's if required.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[led1 + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led1 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led1 + 1 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);
      }    

      if(led1 - 1 < cyclotron_led_start) {
        led1 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led1 = led1 - 1;
      }

      if(i_cyclotron_led_value[led1  - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led2 + 1 - cyclotron_led_start] == 0) {      
        ms_cyclotron_led_fade_in[led2 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led2 + 1 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led2 - 1 < cyclotron_led_start) {
        led2 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led2 = led2 - 1;
      }

      if(i_cyclotron_led_value[led2 - cyclotron_led_start] == 0) {      
        ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led3 + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led3 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led3 + 1 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);      
      }

      if(led3 - 1 < cyclotron_led_start) {
        led3 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led3 = led3 - 1;
      }

      if(i_cyclotron_led_value[led3 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);      
      }

      if(i_cyclotron_led_value[led4 + 1 - cyclotron_led_start] == 0) {      
        ms_cyclotron_led_fade_in[led4 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led4 + 1 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);      
      }

      if(led4 - 1 < cyclotron_led_start) {
        led4 = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        led4 = led4 - 1;
      }

      if(i_cyclotron_led_value[led4 - cyclotron_led_start] == 0) {      
        ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(255, i_1984_fade_in_delay, CIRCULAR_IN);      
      }
    }    
  }
}

void cyclotron84LightOn(int cLed) {
  if(b_fade_cyclotron_led != true) {
    uint8_t r = 255;
    uint8_t g = 0;
    uint8_t b = 0;
  
    if(b_cyclotron_colour_toggle == true) {
      switch(FIRING_MODE) {
        case PROTON:
          r = 255;
          g = 0;
          b = 0;
        break;
    
        case SLIME:
          r = 0;
          g = 255;
          b = 0;
        break;
    
        case STASIS:
          r = 0;
          g = 0;
          b = 255;
        break;
    
        case MESON:
          r = 255;
          g = 255;
          b = 0;
        break;   
    
        default:
          r = 255;
          g = 0;
          b = 0;
        break;
      }
    }

    pack_leds[cLed] = CRGB(r,g,b);

    // Turn on the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      pack_leds[cLed+1] = CRGB(r,g,b);

      if(cLed - 1 < cyclotron_led_start) {
        cLed = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        cLed = cLed - 1;
      }

      pack_leds[cLed] = CRGB(r,g,b);
    }
  }
  else {
    if(i_cyclotron_led_value[cLed - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(255, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
    }

    // Turn on the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[cLed + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[cLed + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[cLed + 1 - cyclotron_led_start].go(255, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
      }

      if(cLed - 1 < cyclotron_led_start) {
        cLed = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        cLed = cLed - 1;
      }
      
      if(i_cyclotron_led_value[cLed - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(255, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
      }
    }    
  }
}

void cyclotron84LightOff(int cLed) {
  if(b_fade_cyclotron_led != true) {
    pack_leds[cLed] = CRGB(0,0,0);

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      pack_leds[cLed + 1] = CRGB(0,0,0);
      
      if(cLed - 1 < cyclotron_led_start) {
        cLed = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        cLed = cLed - 1;
      }

      pack_leds[cLed] = CRGB(0,0,0);
    }
  }
  else {
    if(i_cyclotron_led_value[cLed - cyclotron_led_start] == 255) {
      ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(255);
      ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(0, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
    }

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[cLed + 1 - cyclotron_led_start] == 255) {
        ms_cyclotron_led_fade_out[cLed + 1 - cyclotron_led_start].go(255);
        ms_cyclotron_led_fade_out[cLed + 1 - cyclotron_led_start].go(0, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
      }

      if(cLed - 1 < cyclotron_led_start) {
        cLed = PACK_NUM_LEDS - 7 - 1;
      }
      else {
        cLed = cLed - 1;
      }

      if(i_cyclotron_led_value[cLed - cyclotron_led_start] == 255) {
        ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(255);
        ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(0, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
      }
    }        
  }
}

void cyclotronOverHeating() {  
  if(b_overheat_sync_to_fan != true) {
    smokeControl(true);
  }

  if(ms_overheating.justFinished()) {
    w_trig.trackGain(S_VENT_SMOKE, i_volume);
    w_trig.trackPlayPoly(S_VENT_SMOKE, true);

    if(b_overheat_sync_to_fan != true) {
      smokeControl(false);
    }
  }

  switch (i_mode_year) {
    case 2021:
      if(b_overheat_lights_off != true) {
        cyclotron2021(i_2021_delay * 10);
        innerCyclotronRing(i_2021_inner_delay * 14);

        vibrationPack(i_vibration_lowest_level * 2);
      }
      else if(b_overheat_lights_off == true) {
        if(i_powercell_led > 0) {
          cyclotron2021(i_2021_delay * 10);
          
          vibrationPack(i_vibration_lowest_level);
        }
        else {
          vibrationPack(0);
        }
      }
    break;

    case 1984:
    case 1989:
      if(b_overheat_lights_off != true) {
        innerCyclotronRing(i_2021_inner_delay * 14);
      }

      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);
        if(b_fade_cyclotron_led != true) {
          resetCyclotronLeds();
        }
        else {
          cyclotron84LightOff(i_1984_cyclotron_leds[0] + cyclotron_led_start);
          cyclotron84LightOff(i_1984_cyclotron_leds[1] + cyclotron_led_start);
          cyclotron84LightOff(i_1984_cyclotron_leds[2] + cyclotron_led_start);
          cyclotron84LightOff(i_1984_cyclotron_leds[3] + cyclotron_led_start);
        }
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 4) {
          if(b_overheat_lights_off != true) {
            vibrationPack(i_vibration_lowest_level);
            cyclotron1984Alarm();
          }
          else if(b_overheat_lights_off == true && i_powercell_led > 0) {
            vibrationPack(i_vibration_lowest_level);

            cyclotron1984Alarm();
          }
          else {
            vibrationPack(0);
          }
        }
      }
    break;
  }

  // Time the n-filter light to when the fan is running.
  if(ms_fan_stop_timer.isRunning() && ms_fan_stop_timer.remaining() < 3000) {
    if(b_overheat_sync_to_fan == true) {
      smokeControl(true);
    }

    // For strobing the vent light.
    if(ms_vent_light_off.justFinished()) {
      ms_vent_light_off.stop();
      ms_vent_light_on.start(i_vent_light_delay);
  
      if(b_overheat_strobe == true) {
        ventLight(true);
      }
    }
    else if(ms_vent_light_on.justFinished()) {
      ms_vent_light_on.stop();
      ms_vent_light_off.start(i_vent_light_delay);
  
      if(b_overheat_strobe == true) {
        ventLight(false);
      }
    }
  
    // For non strobing vent light option.
    if(b_overheat_strobe != true) {
      if(b_vent_light_on != true) {
        // Solid light on if strobe option turned off.
        ventLight(true);
      }
    }
  }
}
      
void cyclotronNoCable() {  
  switch (i_mode_year) {
    case 2021:
      cyclotron2021(i_2021_delay * 10);      
      innerCyclotronRing(i_2021_inner_delay * 14);

      if(ms_alarm.justFinished()) {
        ventLight(false);
        ms_alarm.start(i_1984_delay);
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 2) {
          ventLight(true);
        }
      }

      vibrationPack(i_vibration_lowest_level * 3);
    break;

    case 1984:
    case 1989:
      innerCyclotronRing(i_2021_inner_delay * 14);
      cyclotron1984(i_1984_delay * 3);
      
      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);

        // Turn off the n-filter light.
        ventLight(false);

        vibrationPack(i_vibration_lowest_level);
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 4) {   
          vibrationPack(i_vibration_idle_level_1984);

          // Turn on the n-filter light.
          ventLight(true);
        }
      }
    break;
  }
}

/*
 * Turns off the LEDs in the cyclotron lid only.
*/
void cyclotronLidLedsOff() {
  for(int i = cyclotron_led_start; i < PACK_NUM_LEDS - 7; i++) {
    pack_leds[i] = CRGB(0,0,0);
  }
}

void resetCyclotronLeds() {
  for(int i = cyclotron_led_start; i < PACK_NUM_LEDS; i++) {
    pack_leds[i] = CRGB(0,0,0);
  }

  // Turn off optional n-filter led.
  digitalWrite(i_nfilter_led_pin, LOW);

  for(int i = 0; i < PACK_NUM_LEDS - 7 - cyclotron_led_start; i++) {
      ms_cyclotron_led_fade_out[i].go(0);
      ms_cyclotron_led_fade_in[i].go(0);

      i_cyclotron_led_on_status[i] = false;
  }
  
  // Only reset the start led if the pack is off or just started.
  if(b_reset_start_led == true) {
    i_led_cyclotron = cyclotron_led_start;
  }

  // Keep the fade control fading out a light that is not on during startup.
  if(PACK_STATUS == MODE_OFF){ 
    if(b_1984_led_start != true) {
      b_1984_led_start = true;
    }
  }

  // Tell the inner cyclotron to turn off the leds.
  if(b_cyclotron_lid_on == true) {
    innerCyclotronOff();
  }
  else if(b_alarm != true || PACK_STATUS == MODE_OFF) {
    innerCyclotronOff();
  }

  cyclotronSpeedRevert();
}

void clearCyclotronFades() {
  for(int i = 0; i < PACK_NUM_LEDS - 7 - cyclotron_led_start; i++) {
    i_cyclotron_led_value[i] = 0;
  }
}

void innerCyclotronOff() {
  for(int i = 0; i < CYCLOTRON_NUM_LEDS; i++) {
    cyclotron_leds[i] = CRGB(0,0,0);
  }
}

// Unused.
/*
void innerCyclotronShowAll() {
  if(b_cyclotron_lid_on != true) {
    for(int i = 0; i < CYCLOTRON_NUM_LEDS; i++) {
      cyclotron_leds[i] = CRGB(255,0,0);
    }
  }
}
*/

// For NeoPixel rings, ramp up and ramp down the LEDs in the ring and set the speed. (optional)
void innerCyclotronRing(int cDelay) {
  if(ms_cyclotron_ring.justFinished()) {
    if(b_inner_ramp_up == true) {      
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_up = false;
        ms_cyclotron_ring.start(cDelay);
        
        i_inner_current_ramp_speed = cDelay;
      }
      else {     
        ms_cyclotron_ring.start(r_inner_ramp.update());
        i_inner_current_ramp_speed = r_inner_ramp.update();
      }
    }
    else if(b_inner_ramp_down == true) {
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_down = false;
      }
      else {
        ms_cyclotron_ring.start(r_inner_ramp.update());

        i_inner_current_ramp_speed = r_inner_ramp.update();
      }
    }
    else {
      i_inner_current_ramp_speed = cDelay;

      if(i_cyclotron_multiplier > 1) {
        if(i_cyclotron_multiplier > 4) {
          cDelay = cDelay - 4;
        }
        else {
          cDelay = cDelay - i_cyclotron_multiplier;
        }
      }

      if(cDelay < 1) {
        cDelay = 1;
      }

      ms_cyclotron_ring.start(cDelay);
    }  

    // Colour control for the inner cyclotron leds. (red,green,blue)
    uint8_t r = 255;
    uint8_t g = 0;
    uint8_t b = 0;

    if(b_cyclotron_colour_toggle == true) {
      switch(FIRING_MODE) {
        case PROTON:
          r = 255;
          g = 0;
          b = 0;
        break;
    
        case SLIME:
          r = 0;
          g = 255;
          b = 0;
        break;
    
        case STASIS:
          r = 0;
          g = 0;
          b = 255;
        break;
    
        case MESON:
          r = 255;
          g = 255;
          b = 0;
        break;   
    
        default:
          r = 255;
          g = 0;
          b = 0;
        break;
      }
    }
  
    if(i_cyclotron_multiplier > 1) {
      switch(i_cyclotron_multiplier) {
        case 6:
          cDelay = cDelay - 4;
        break;
        
        case 5:
          cDelay = cDelay - 3;
        break;
        
        case 4:
          cDelay = cDelay - 3;
        break;
        
        case 3:
          cDelay = cDelay - 2;
        break;
        
        case 2:
          cDelay = cDelay - 2;
        break;
          
        default:
          cDelay = cDelay - 1;
        break;
      }
    }
    else {
      cDelay = cDelay / i_cyclotron_multiplier;
    }

    if(cDelay < 2) {
      cDelay = 2;
    }
    
    if(b_clockwise == true) {
      if(b_cyclotron_lid_on != true) {
        if(b_grb_cyclotron == true) {
          // For GRB LEDs.
          cyclotron_leds[i_led_cyclotron_ring] = CRGB(g,r,b);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = CRGB(r,g,b);
        }

        if(i_led_cyclotron_ring == 0) {
          cyclotron_leds[CYCLOTRON_NUM_LEDS - 1] = CRGB(0,0,0);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring - 1] = CRGB(0,0,0);
        }
      }
      
      i_led_cyclotron_ring++;
      
      if(i_led_cyclotron_ring > CYCLOTRON_NUM_LEDS - 1) {
        i_led_cyclotron_ring = 0;
      }
    }
    else {
      if(b_cyclotron_lid_on != true) {
        // For GRB LEDs.
        if(b_grb_cyclotron == true) {
          cyclotron_leds[i_led_cyclotron_ring] = CRGB(g,r,b);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = CRGB(r,g,b);
        }
        
        if(i_led_cyclotron_ring + 1 > CYCLOTRON_NUM_LEDS - 1) {
          cyclotron_leds[0] = CRGB(0,0,0);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring + 1] = CRGB(0,0,0);
        }
      }
      
      i_led_cyclotron_ring--;

      if(i_led_cyclotron_ring < 0) {
        i_led_cyclotron_ring = CYCLOTRON_NUM_LEDS -1;
      }
    }
  }
}

void reset2021RampUp() {
  b_2021_ramp_up = true;
  b_2021_ramp_up_start = true;

  // Inner cyclotron ring.
  b_inner_ramp_up = true;
}

void reset2021RampDown() {
  b_2021_ramp_down = true;
  b_2021_ramp_down_start = true;

  // Inner cyclotron ring.
  b_inner_ramp_down = true;
}

void ventLight(bool b_on) {
  b_vent_light_on = b_on;

  if(b_on == true) {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    // If doing firing smoke effects, lets change the light colours.
    if(b_wand_firing == true) {
      switch(FIRING_MODE) {
        case PROTON:
          // Adjust the n-filter light colours during firing.
          switch(i_wand_power_level) {
            case 1:
              r = 200;
              g = 255;
              b = 255;
            break;

            case 2:
              r = 150;
              g = 255;
              b = 255;
            break;

            case 3:
              r = 100;
              g = 255;
              b = 230;
            break;

            case 4:
              r = 50;
              g = 255;
              b = 255;
            break;

            case 5:
              r = 0;
              g = 255;
              b = 255;
            break;

            default:
              r = 0;
              g = 255;
              b = 255;
            break;
          }
        break;
        
        case SLIME:
          r = 0;
          g = 255;
          b = 0;
        break;
        
        case STASIS:
          r = 0;
          g = 0;
          b = 255;
        break;
        
        case MESON:
          r = 255;
          g = 255;
          b = 0;
        break;   
        
        default:
          r = 50;
          g = 255;
          b = 255;
        break;
      }
    }
    else if(b_alarm == true && b_overheating != true) {
      r = 255;
      g = 0;
      b = 0;
    }
    
    for(int i = VENT_LIGHT_START; i < PACK_NUM_LEDS; i++) {
      pack_leds[i] = CRGB(r,g,b);
    }

    digitalWrite(i_nfilter_led_pin, HIGH);
  }
  else {
    for(int i = VENT_LIGHT_START; i < PACK_NUM_LEDS; i++) {
      pack_leds[i] = CRGB(0,0,0);
    }

    digitalWrite(i_nfilter_led_pin, LOW);
  }
}

void stopMusic() {
  w_trig.trackStop(i_current_music_track);

  w_trig.update();
}

void playMusic() {
  w_trig.trackGain(i_current_music_track, i_volume_music);
  w_trig.trackPlayPoly(i_current_music_track);

  if(b_repeat_track == true) {
    w_trig.trackLoop(i_current_music_track, 1);
  }
  else {
    w_trig.trackLoop(i_current_music_track, 0);
  }

  w_trig.update();
}

void wandFiring() {
  b_wand_firing = true;

  // Turn off any smoke.
  smokeControl(false);

  // Start a smoke timer to play a little bit of smoke while firing.
  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
  ms_smoke_on.stop();       
  
  vibrationPack(255);

  if(i_mode_year == 1989) {
    int8_t i_v_spark = i_volume - 10;

    if(i_v_spark < -70) {
      i_v_spark = -70;
    }

    w_trig.trackGain(S_FIRE_START_SPARK, i_v_spark);
  }
  else {
    w_trig.trackGain(S_FIRE_START_SPARK, i_volume);
  }

  w_trig.trackPlayPoly(S_FIRE_START_SPARK);

  switch(FIRING_MODE) {
    case PROTON:
      if(i_mode_year == 1989) {
        int8_t i_v_fire_start = i_volume - 10;

        if(i_v_fire_start < -70) {
          i_v_fire_start = -70;
        }

        w_trig.trackGain(S_FIRE_START, i_v_fire_start);
      }
      else {
        w_trig.trackGain(S_FIRE_START, i_volume);
      }

      w_trig.trackPlayPoly(S_FIRE_START, true);

      switch(i_wand_power_level) {
        case 1 ... 4:
          if(b_firing_intensify == true) {
            if(i_mode_year == 1989) {
              w_trig.trackGain(S_GB2_FIRE_LOOP, -70);
              w_trig.trackPlayPoly(S_GB2_FIRE_LOOP, true);
              w_trig.trackFade(S_GB2_FIRE_LOOP, i_volume, 6500, 0);
              w_trig.trackLoop(S_GB2_FIRE_LOOP, 1);

              w_trig.trackGain(S_GB2_FIRE_START, i_volume);
              w_trig.trackPlayPoly(S_GB2_FIRE_START);
            }
            else {
              w_trig.trackGain(S_GB1_FIRE_LOOP, i_volume);
              w_trig.trackPlayPoly(S_GB1_FIRE_LOOP, true);
              w_trig.trackFade(S_GB1_FIRE_LOOP, i_volume, 1000, 0);
              w_trig.trackLoop(S_GB1_FIRE_LOOP, 1);

              w_trig.trackPlayPoly(S_GB1_FIRE_START);
            }

            b_sound_firing_intensify_trigger = true;
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            w_trig.trackGain(S_FIRING_LOOP_GB1, i_volume);
            w_trig.trackPlayPoly(S_FIRING_LOOP_GB1, true);
            w_trig.trackFade(S_FIRING_LOOP_GB1, i_volume, 1000, 0);
            w_trig.trackLoop(S_FIRING_LOOP_GB1, 1);

            if(i_mode_year == 1989) {
              w_trig.trackGain(S_GB2_FIRE_START, i_volume);
              w_trig.trackPlayPoly(S_GB2_FIRE_START);
            }
            
            b_sound_firing_alt_trigger = true;
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;

        case 5:
            if(i_mode_year == 1989) {
              w_trig.trackGain(S_GB2_FIRE_START, i_volume);
              w_trig.trackPlayPoly(S_GB2_FIRE_START);
            }  
            else {
              w_trig.trackPlayPoly(S_GB1_FIRE_START_HIGH_POWER);
            }

            if(b_firing_intensify == true) {
              // Reset some sound triggers.
              b_sound_firing_intensify_trigger = true;
              w_trig.trackPlayPoly(S_GB1_FIRE_HIGH_POWER_LOOP, true);
              w_trig.trackLoop(S_GB1_FIRE_HIGH_POWER_LOOP, 1);
            }
            else {
              b_sound_firing_intensify_trigger = false;
            }

            if(b_firing_alt == true) {
              // Reset some sound triggers.
              b_sound_firing_alt_trigger = true;

              w_trig.trackGain(S_FIRING_LOOP_GB1, i_volume);
              w_trig.trackPlayPoly(S_FIRING_LOOP_GB1, true);
              w_trig.trackLoop(S_FIRING_LOOP_GB1, 1);        
            }
            else {
              b_sound_firing_alt_trigger = false;
            }
        break;
      }
    break;

    case SLIME:
      w_trig.trackGain(S_SLIME_START, i_volume);
      w_trig.trackPlayPoly(S_SLIME_START);
      
      w_trig.trackGain(S_SLIME_LOOP, i_volume);
      w_trig.trackPlayPoly(S_SLIME_LOOP);
      w_trig.trackFade(S_SLIME_LOOP, i_volume, 1500, 0);
      w_trig.trackLoop(S_SLIME_LOOP, 1);
    break;

    case STASIS:
      w_trig.trackGain(S_STASIS_START, i_volume);
      w_trig.trackPlayPoly(S_STASIS_START);
      
      w_trig.trackGain(S_STASIS_LOOP, i_volume);
      w_trig.trackPlayPoly(S_STASIS_LOOP);
      w_trig.trackFade(S_STASIS_LOOP, i_volume, 1000, 0);
      w_trig.trackLoop(S_STASIS_LOOP, 1);
    break;

    case MESON:
      w_trig.trackGain(S_MESON_START, i_volume);
      w_trig.trackPlayPoly(S_MESON_START);
      
      w_trig.trackGain(S_MESON_LOOP, i_volume);
      w_trig.trackPlayPoly(S_MESON_LOOP);
      w_trig.trackFade(S_MESON_LOOP, i_volume, 5500, 0);
      w_trig.trackLoop(S_MESON_LOOP, 1);
    break;
    
    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  // Reset some vent light timers.
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
  ms_vent_light_off.start(i_vent_light_delay);

  // Reset vent sounds flag.
  b_vent_sounds = true;

  ms_firing_length_timer.start(i_firing_timer_length);
}

void wandStoppedFiring() {
  // Stop all other firing sounds.
  wandStopFiringSounds();      
  
  if(b_wand_firing == true) {
    switch(FIRING_MODE) {
      case PROTON:
        // Play different firing end stream sound depending on how long we have been firing for.
        if(ms_firing_length_timer.remaining() < 5000) {
          // Short tail end.
          w_trig.trackGain(S_FIRING_END_GUN, i_volume);
          w_trig.trackPlayPoly(S_FIRING_END_GUN, true);
        }
        else if(ms_firing_length_timer.remaining() < 10000) {
          // Mid tail end.
          w_trig.trackGain(S_FIRING_END_MID, i_volume);
          w_trig.trackPlayPoly(S_FIRING_END_MID, true);
        }
        else {
          // Long tail end.
          w_trig.trackGain(S_FIRING_END, i_volume);
          w_trig.trackPlayPoly(S_FIRING_END, true);
        }
      break;
  
      case SLIME:
        w_trig.trackGain(S_SLIME_END, i_volume);
        w_trig.trackPlayPoly(S_SLIME_END, true);
      break;
  
      case STASIS:
        w_trig.trackGain(S_STASIS_END, i_volume);
        w_trig.trackPlayPoly(S_STASIS_END, true);
        
      break;
  
      case MESON:
        w_trig.trackGain(S_MESON_END, i_volume);
        w_trig.trackPlayPoly(S_MESON_END, true);
      break;

      case VENTING:
      case SETTINGS:
        // Nothing
      break;
    }
  }
  
  b_wand_firing = false;
  b_firing_alt = false;
  b_firing_intensify = false;

  // Reset some vent light timers.
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
  ventLight(false);

  // Reset vent sounds flag.
  b_vent_sounds = true;

  // Turn off any smoke.
  smokeControl(false);
  
  // Turn off the n-filter fan.
  fanControl(false); 

  ms_firing_length_timer.stop();
  ms_smoke_timer.stop();
  ms_smoke_on.stop();
}

void wandStopFiringSounds() {
  // Firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
      if(i_mode_year == 1989) {
        w_trig.trackStop(S_GB2_FIRE_LOOP);
        w_trig.trackStop(S_GB2_FIRE_START);
      }
      else {
        w_trig.trackStop(S_GB1_FIRE_START);
        w_trig.trackStop(S_GB1_FIRE_LOOP);
      }

      w_trig.trackStop(S_FIRING_LOOP_GB1);
      w_trig.trackStop(S_GB1_FIRE_START_HIGH_POWER);
      w_trig.trackStop(S_GB1_FIRE_HIGH_POWER_LOOP);
      w_trig.trackStop(S_FIRE_START_SPARK);
      w_trig.trackStop(S_FIRE_START);
    break;

    case SLIME:
      w_trig.trackStop(S_SLIME_START);
      w_trig.trackStop(S_SLIME_LOOP);
      w_trig.trackStop(S_SLIME_END);
    break;

    case STASIS:
      w_trig.trackStop(S_STASIS_START);
      w_trig.trackStop(S_STASIS_LOOP);
      w_trig.trackStop(S_STASIS_END);
    break;

    case MESON:
      w_trig.trackStop(S_MESON_START);
      w_trig.trackStop(S_MESON_LOOP);
      w_trig.trackStop(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing
    break;
  }

  if(b_firing_cross_streams == true) {
    switch(i_mode_year) {
      case 2021:
        w_trig.trackPlayPoly(S_AFTERLIFE_CROSS_THE_STREAMS_END, true);
      break;

      case 1984:
      case 1989:
        w_trig.trackPlayPoly(S_CROSS_STREAMS_END, true);
      break;
    }

    b_firing_cross_streams = false;
  }

  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
}

void packAlarm() {
  wandStopFiringSounds();

  // Pack sounds.
  if(i_mode_year == 1989) {
    w_trig.trackStop(S_GB2_PACK_START);
    w_trig.trackStop(S_GB2_PACK_LOOP);
  }
  else {
    w_trig.trackStop(S_AFTERLIFE_PACK_STARTUP);
    w_trig.trackStop(S_AFTERLIFE_PACK_IDLE_LOOP);
    w_trig.trackStop(S_IDLE_LOOP);
    w_trig.trackStop(S_BOOTUP);
  }

  w_trig.trackGain(S_SHUTDOWN, i_volume);
  w_trig.trackPlayPoly(S_SHUTDOWN, true);

  if(i_mode_year == 1989) {
    w_trig.trackGain(S_GB2_PACK_OFF, i_volume);
    w_trig.trackPlayPoly(S_GB2_PACK_OFF, true);
  }
  else {
    w_trig.trackGain(S_PACK_SHUTDOWN, i_volume);
    w_trig.trackPlayPoly(S_PACK_SHUTDOWN, true);
  }

  if(b_overheating != true) {
    switch(i_mode_year) {
      case 1984:
      case 1989:
      case 2021:
        w_trig.trackGain(S_PACK_RIBBON_ALARM_1, i_volume);
        w_trig.trackPlayPoly(S_PACK_RIBBON_ALARM_1, true);
        w_trig.trackLoop(S_PACK_RIBBON_ALARM_1, 1);
      break;

      // Not used.
      default:
        w_trig.trackGain(S_PACK_BEEPING, i_volume);
        w_trig.trackPlayPoly(S_PACK_BEEPING, true);
        w_trig.trackLoop(S_PACK_BEEPING, 1);
      break;
    }
  }
}

// LED's for the 1984/2021 and vibration switches.
void cyclotronSwitchPlateLEDs() {   
  if(switch_cyclotron_lid.isReleased()) {
    // Play sounds when lid is removed.
    w_trig.trackStop(S_VENT_SMOKE);
    w_trig.trackStop(S_MODE_SWITCH);
    w_trig.trackStop(S_CLICK);
    w_trig.trackStop(S_SPARKS_LOOP);
    w_trig.trackStop(S_BEEPS_BARGRAPH);

    w_trig.trackGain(S_MODE_SWITCH, i_volume);
    w_trig.trackPlayPoly(S_MODE_SWITCH);
    
    w_trig.trackGain(S_VENT_SMOKE, i_volume);
    w_trig.trackPlayPoly(S_VENT_SMOKE);

    // Play some spark sounds if the pack is running and the lid is removed.
    if(PACK_STATUS == MODE_ON) {
      w_trig.trackGain(S_SPARKS_LOOP, i_volume);
      w_trig.trackPlayPoly(S_SPARKS_LOOP);
    }
  }

  if(switch_cyclotron_lid.isPressed()) {
    // Play sounds when lid is mounted.
    w_trig.trackStop(S_CLICK);    
    w_trig.trackStop(S_VENT_DRY);

    w_trig.trackGain(S_CLICK, i_volume);
    w_trig.trackPlayPoly(S_CLICK);

    w_trig.trackGain(S_VENT_DRY, i_volume);
    w_trig.trackPlayPoly(S_VENT_DRY);

    // Play some spark sounds if the pack is running and the lid is put back on                          .
    if(PACK_STATUS == MODE_ON) {
      w_trig.trackGain(S_SPARKS_LOOP, i_volume);
      w_trig.trackPlayPoly(S_SPARKS_LOOP);
    }
  }

  if(switch_cyclotron_lid.getState() == LOW) {
    if(b_cyclotron_lid_on != true) {
      // The cyclotron lid is now on.
      b_cyclotron_lid_on = true;
      
      // Turn off inner cyclotron LEDs.
      innerCyclotronOff();
    }
  }
  else {
    if(b_cyclotron_lid_on == true) {
      // The cyclotron lid is now off.
      b_cyclotron_lid_on = false;
    }
  }

  if(b_cyclotron_lid_on != true) {
    if(i_mode_year == 1984 || i_mode_year == 1989) {        
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
        digitalWrite(cyclotron_switch_led_green, HIGH);
      }
      else {
        digitalWrite(cyclotron_switch_led_green, LOW);
      }
    }
    else {
      digitalWrite(cyclotron_switch_led_green, HIGH);
    }
    
    if(b_vibration == true) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
        digitalWrite(cyclotron_switch_led_yellow, HIGH);
      }
      else {
        digitalWrite(cyclotron_switch_led_yellow, LOW);
      }
    }
    else {
      digitalWrite(cyclotron_switch_led_yellow, HIGH);
    }  
  }
  else {
    // Keep the cyclotron switch LED's off when the lid is on.
    digitalWrite(cyclotron_switch_led_green, LOW);
    digitalWrite(cyclotron_switch_led_yellow, LOW);
  }
  
  if(ms_cyclotron_switch_plate_leds.justFinished()) {
    ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);
  }
}

void vibrationPack(int i_level) {
  if(b_vibration == true && b_vibration_enabled == true) {
    if(b_vibration_firing == true) {
      if(b_wand_firing == true) {
        if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(vibration, i_level);
        }
      }
      else {
        i_vibration_level_prev = 0;
        analogWrite(vibration, 0);
      }
    }
    else {
      if(i_level != i_vibration_level_prev) {
        i_vibration_level_prev = i_level;
        analogWrite(vibration, i_level);
      }
    }
  }
  else {
    i_vibration_level_prev = 0;
    analogWrite(vibration, 0);
  }
}

void cyclotronSpeedRevert() {
  // Stop overheat beeps.
  w_trig.trackStop(S_BEEP_8);
  
  i_cyclotron_multiplier = 1;
  i_cyclotron_switch_led_mulitplier = 1;
}

void cyclotronSpeedIncrease() {
  switch(i_mode_year) {
    case 2021:
      i_cyclotron_multiplier++;
    break;
    
    case 1984:
    case 1989:
      i_cyclotron_multiplier++;
    break;
  }

  i_cyclotron_switch_led_mulitplier++;
}

void adjustVolumeEffectsGain() {
  /*
   * If the pack is running, there will be a slight pause in the LEDs due to serial communication to the wav trigger.
   */
  for(unsigned int i=0; i <= i_last_effects_track; i++) {
    w_trig.trackGain(i, i_volume);
  }
}

void increaseVolumeEffects() {
  if(i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER > 100) {
    i_volume_percentage = 100;
  }
  else {
    i_volume_percentage = i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void decreaseVolumeEffects() {
  if(i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER < 0) {
    i_volume_percentage = 0;
  }
  else {
    i_volume_percentage = i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void increaseVolume() { 
  if(i_volume_master == -70 && MINIMUM_VOLUME > i_volume_master) {
    i_volume_master = MINIMUM_VOLUME;
  }

  if(i_volume_master_percentage + VOLUME_MULTIPLIER > 100) {
    i_volume_master_percentage = 100;
  }
  else {
    i_volume_master_percentage = i_volume_master_percentage + VOLUME_MULTIPLIER;
  }

  i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
  i_volume_revert = i_volume_master;

  w_trig.masterGain(i_volume_master);
}

void decreaseVolume() {
  if(i_volume_master == -70) {
    // Can not go any lower.
  }
  else {
    if(i_volume_master_percentage - VOLUME_MULTIPLIER < 0) {
      i_volume_master_percentage = 0;
    }
    else {
      i_volume_master_percentage = i_volume_master_percentage - VOLUME_MULTIPLIER;
    }

    i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
    i_volume_revert = i_volume_master;

    w_trig.masterGain(i_volume_master); 
  }      
}

void readEncoder() {
  if (digitalRead(encoder_pin_a) == digitalRead(encoder_pin_b)) {
    i_encoder_pos++;
  }
  else {
    i_encoder_pos--;
  }
  
  i_val_rotary = i_encoder_pos / 2.5;
}

void checkRotaryEncoder() {
  if(i_val_rotary > i_last_val_rotary) {
    if(ms_volume_check.isRunning() != true) {
      increaseVolume();
      
      // Tell wand to increase volume.
      packSerialSend(P_VOLUME_INCREASE);

      ms_volume_check.start(50);
    }
  }
  
  if(i_val_rotary < i_last_val_rotary) {
    if(ms_volume_check.isRunning() != true) {
      decreaseVolume();
        
      // Tell wand to decrease the volume.
      packSerialSend(P_VOLUME_DECREASE);

      ms_volume_check.start(50);
    }
  }
  
  i_last_val_rotary = i_val_rotary;

  if(ms_volume_check.justFinished()) {
    ms_volume_check.stop();
  }
}

/*
 * Smoke # 1. N-filter cone outlet.
 */
void smokeControl(bool b_smoke_on) {  
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_smoke_1_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_smoke_1_overheat == true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_pin, HIGH);
      }
      else {
        digitalWrite(smoke_pin, LOW);
      }
    }
    else {
      digitalWrite(smoke_pin, LOW);
    }

    smokeBooster(b_smoke_on);
  }
}

/* 
 *  Smoke # 2. I put this one in my booster tube.
 *  A scond fan pin is timed to go off at the same time as this.
 *  It is not needed for a basic smoke pump for the booster tube, but some people requested this for other purposes.
 */
void smokeBooster(bool b_smoke_on) {
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_smoke_2_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_booster_pin, HIGH);
        digitalWrite(fan_booster_pin, HIGH);
      }
      else if(b_overheating == true && b_smoke_2_overheat == true && b_wand_firing != true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_booster_pin, HIGH);
        digitalWrite(fan_booster_pin, HIGH);
      }
      else {
        digitalWrite(smoke_booster_pin, LOW);
        digitalWrite(fan_booster_pin, LOW);
      }
    }
    else {
      digitalWrite(smoke_booster_pin, LOW);
      digitalWrite(fan_booster_pin, LOW);
    }
  }
}
 
/*
 * Fan control. You can use this to switch on any device when properly hooked up with a transistor etc
 * A fan is a good idea for the n-filter for example.
 */
void fanControl(bool b_fan_on) {
  if(b_smoke_enabled == true) {
    if(b_fan_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_fan_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(fan_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_fan_overheat == true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(fan_pin, HIGH);
      }
      else {
        digitalWrite(fan_pin, LOW);
      }
    }
    else {
      digitalWrite(fan_pin, LOW);
    }
  }
}

/* 
 *  Another optional 5V pin that goes high during overheat sequences. 
 *  Perhaps this one would be good for a fan or dc motor to push some smoke into the n-filter with more force.
 */
void checkFan() {
  if(ms_fan_stop_timer.justFinished()) {
    // Turn off fan.
    fanControl(false);
    ms_fan_stop_timer.stop();
  }
  else if(ms_fan_stop_timer.isRunning() && ms_fan_stop_timer.remaining() < 3000) {
    fanControl(true);
  }
}

/*
 * Check if the wand is still connected.
 */
void wandHandShake() {  
  if(b_wand_connected == true) {    
    if(ms_wand_handshake.justFinished()) {     
          
      if(b_wand_firing == true) {
        wandStoppedFiring();
        cyclotronSpeedRevert();
      }
      
      ms_wand_handshake.start(i_wand_handshake_delay);
      
      b_wand_connected = false;

      // Where are you wand?
      packSerialSend(P_HANDSHAKE);
    }
    else if(ms_wand_handshake_checking.justFinished()) {  
      if(b_diagnostic == true) {
        // Play a beep sound to know if the wand is connected, while in diagnostic mode.
        w_trig.trackGain(S_VENT_BEEP, i_volume);
        w_trig.trackPlayPoly(S_VENT_BEEP);
      }
      
      ms_wand_handshake_checking.stop();
      
      // Ask the wand if it is still connected.
      packSerialSend(P_HANDSHAKE);
    }
  }
  else {
    if(b_wand_firing == true) {
      wandStoppedFiring();
      cyclotronSpeedRevert();
    }
    
    // Turn off overheating if the wand gets disconnected.
    if(b_overheating == true) {
      packOverheatingFinished();
    }

    if(ms_wand_handshake.justFinished()) {
      // Ask the wand if it is connected.
      packSerialSend(P_HANDSHAKE);

      ms_wand_handshake.start(i_wand_handshake_delay / 5);
    }
  }
}

void packOverheatingFinished() {
  w_trig.trackGain(S_VENT_DRY, i_volume);
  b_overheating = false;

  // Stop the fan.
  ms_fan_stop_timer.stop();

  // Turn off the n-filter fan.
  fanControl(false);

  // Turn off the smoke.
  smokeControl(false);

  // Reset the LEDs before resetting the alarm flag.
  if(i_mode_year == 1984 || i_mode_year == 1989) {
    resetCyclotronLeds();
  }

  b_alarm = false;

  if(b_overheat_lights_off == true) {
    cyclotronSpeedRevert();
    
    // Reset the ramp speeds.
    switch(i_mode_year) {
      case 1984:
      case 1989:
          // Reset the ramp speeds.
          i_current_ramp_speed = i_1984_delay * 1.3;
          i_inner_current_ramp_speed = i_inner_ramp_delay;
      break;

      case 2021:
        // Reset the ramp speeds.
        i_current_ramp_speed = i_2021_ramp_delay;
        i_inner_current_ramp_speed = i_inner_ramp_delay;
      break;
    }
  }

  reset2021RampUp();

  packStartup();

  // Turn off the vent light
  ventLight(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  ms_cyclotron.start(i_2021_delay); 
}

/*
 * Incoming messages from the wand.
 */
void checkWand() {  
  if(packComs.available()) {
    packComs.rxObj(comStruct);

    if(!packComs.currentPacketID()) {        
      if(comStruct.i > 0 && comStruct.s == W_COM_START && comStruct.e == W_COM_END) {    
        if(b_wand_connected == true) {
          switch(comStruct.i) {
            case W_ON:
              // The wand has been turned on.
              b_wand_on = true;

              // Turn the pack on.
              if(PACK_STATUS != MODE_ON) {
                PACK_ACTION_STATUS = ACTION_ACTIVATE;
              }
            break;
        
            case W_OFF:
              // The wand has been turned off.
              b_wand_on = false;

              // Turn the pack off.
              if(PACK_STATUS != MODE_OFF) {
                PACK_ACTION_STATUS = ACTION_OFF;
              }
            break;
        
            case W_FIRING:
              // Wand is firing.
              wandFiring();
            break;
        
            case W_FIRING_STOPPED:
              // Wand just stopped firing.
              wandStoppedFiring();
              cyclotronSpeedRevert();
            break;
        
            case W_PROTON_MODE:
              // Proton mode
              FIRING_MODE = PROTON;
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);
              
              if(PACK_STATUS == MODE_ON && b_wand_on == true) {
                w_trig.trackGain(S_FIRE_START_SPARK, i_volume);
                w_trig.trackPlayPoly(S_FIRE_START_SPARK);
              }
            break;
        
            case W_SLIME_MODE:
              // Slime mode
              FIRING_MODE = SLIME;
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);
              
              if(PACK_STATUS == MODE_ON && b_wand_on == true) {
                w_trig.trackGain(S_PACK_SLIME_OPEN, i_volume);
                w_trig.trackPlayPoly(S_PACK_SLIME_OPEN);
              }
            break;
        
            case W_STASIS_MODE:
              // Stasis mode
              FIRING_MODE = STASIS;
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);
              
              if(PACK_STATUS == MODE_ON && b_wand_on == true) {
                w_trig.trackGain(S_STASIS_OPEN, i_volume);
                w_trig.trackPlayPoly(S_STASIS_OPEN);
              }
            break;
        
            case W_MESON_MODE:
              // Meson mode
              FIRING_MODE = MESON;
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);
              
              if(PACK_STATUS == MODE_ON && b_wand_on == true) {
                w_trig.trackGain(S_MESON_OPEN, i_volume);
                w_trig.trackPlayPoly(S_MESON_OPEN);
              }
            break;
        
            case W_VENTING_MODE:
              // Settings mode
              FIRING_MODE = VENTING;
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);

              if(PACK_STATUS == MODE_ON && b_wand_on == true) {
                w_trig.trackGain(S_VENT_DRY, i_volume);
                w_trig.trackGain(S_MODE_SWITCH, i_volume);

                w_trig.trackPlayPoly(S_VENT_DRY);
                w_trig.trackPlayPoly(S_MODE_SWITCH);
              }
            break;

            case W_SETTINGS_MODE:
              // Settings mode
              FIRING_MODE = SETTINGS;
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);
            break;
        
            case W_OVERHEATING:
              // Overheating
              w_trig.trackStop(S_BEEP_8);
              
              w_trig.trackGain(S_VENT_SLOW, i_volume);
              w_trig.trackPlayPoly(S_VENT_SLOW);
              b_overheating = true;
        
              // Start timer for a second smoke sound.
              ms_overheating.start(i_overheating_delay);

              // Reset some vent light timers.
              ms_vent_light_off.stop();
              ms_vent_light_on.stop();
              ms_fan_stop_timer.stop();
              ms_vent_light_off.start(i_vent_light_delay);
              ms_fan_stop_timer.start(i_fan_stop_timer);
              
              // Reset the inner cyclotron speed.
              if(i_mode_year == 1984 || i_mode_year == 1989) {
                i_inner_current_ramp_speed = i_inner_ramp_delay;
              }
            break;
        
            case W_OVERHEATING_FINISHED:
              // Overheating finished
              packOverheatingFinished();
            break;
        
            case W_CYCLOTRON_NORMAL_SPEED:
              // Reset cyclotron speed.
              cyclotronSpeedRevert();
            break;
        
            case W_CYCLOTRON_INCREASE_SPEED:
              // Speed up cyclotron.
              cyclotronSpeedIncrease();  
            break;

            case W_HANDSHAKE:
              // The wand is still here.
              ms_wand_handshake.start(i_wand_handshake_delay);
              ms_wand_handshake_checking.start(i_wand_handshake_delay / 2);
              b_wand_connected = true;
            break;
            
            case W_BEEP_START:
              // Play 8 overheat beeps before we overheat.
              w_trig.trackGain(S_BEEP_8, i_volume);
              w_trig.trackPlayPoly(S_BEEP_8);
            break;
        
            case W_POWER_LEVEL_1:
              // Wand power level 1
              i_wand_power_level = 1;

              // Reset the smoke timer if the wand is firing.
              if(b_wand_firing == true) {
                if(ms_smoke_timer.isRunning() == true) {
                  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
                }
              }
            break;
        
            case W_POWER_LEVEL_2:
              // Wand power level 2
              i_wand_power_level = 2;

              // Reset the smoke timer if the wand is firing.
              if(b_wand_firing == true) {
                if(ms_smoke_timer.isRunning() == true) {
                  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
                }
              }
            break;
        
            case W_POWER_LEVEL_3:
              // Wand power level 3
              i_wand_power_level = 3;
              
              // Reset the smoke timer if the wand is firing.
              if(b_wand_firing == true) {
                if(ms_smoke_timer.isRunning() == true) {
                  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
                }
              }
            break;
        
            case W_POWER_LEVEL_4:
              // Wand power level 4
              i_wand_power_level = 4;

              // Reset the smoke timer if the wand is firing.
              if(b_wand_firing == true) {
                if(ms_smoke_timer.isRunning() == true) {
                  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
                }
              }
            break;
        
            case W_POWER_LEVEL_5:
              // Wand power level 5
              i_wand_power_level = 5;

              // Reset the smoke timer if the wand is firing.
              if(b_wand_firing == true) {
                if(ms_smoke_timer.isRunning() == true) {
                  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
                }
              }
            break;        

            case W_FIRING_INTENSIFY:
              // Wand firing in intensify mode.
              b_firing_intensify = true;

              if(b_wand_firing == true && b_sound_firing_intensify_trigger != true) {
                b_sound_firing_intensify_trigger = true;
              }
            break;

            case W_FIRING_INTENSIFY_MIX:
              // Wand firing in intensify mode.
              b_firing_intensify = true;

              if(b_wand_firing == true && b_sound_firing_intensify_trigger != true) {
                b_sound_firing_intensify_trigger = true;

                switch(i_wand_power_level) {
                  case 1 ... 4:
                    if(i_mode_year == 1989) {
                      w_trig.trackPlayPoly(S_GB2_FIRE_LOOP, true);
                      w_trig.trackLoop(S_GB2_FIRE_LOOP, 1);

                      w_trig.trackPlayPoly(S_GB2_FIRE_START);
                    }
                    else {
                      w_trig.trackPlayPoly(S_GB1_FIRE_LOOP, true);
                      w_trig.trackLoop(S_GB1_FIRE_LOOP, 1);

                      w_trig.trackPlayPoly(S_GB1_FIRE_START);
                    }
                  break;

                  case 5:
                    w_trig.trackPlayPoly(S_GB1_FIRE_HIGH_POWER_LOOP, true);
                    w_trig.trackLoop(S_GB1_FIRE_HIGH_POWER_LOOP, 1);
                  break;
                }
              }
            break;            

            case W_FIRING_INTENSIFY_STOPPED:
              // Wand no longer firing in intensify mode.
              b_firing_intensify = false;
              b_sound_firing_intensify_trigger = false;
            break;

            case W_FIRING_INTENSIFY_STOPPED_MIX:
              // Wand no longer firing in intensify mode.
              b_firing_intensify = false;
              b_sound_firing_intensify_trigger = false;

              if(b_firing_cross_streams != true) {
                switch(i_wand_power_level) {
                  case 1 ... 4:
                    if(i_mode_year == 1989) {
                      w_trig.trackStop(S_GB2_FIRE_LOOP);
                      w_trig.trackStop(S_GB2_FIRE_START);
                    }
                    else {
                      w_trig.trackStop(S_GB1_FIRE_LOOP);
                      w_trig.trackStop(S_GB1_FIRE_LOOP);
                      w_trig.trackStop(S_GB1_FIRE_START);
                    }
                  break;

                  case 5:
                    w_trig.trackStop(S_GB1_FIRE_HIGH_POWER_LOOP);
                    w_trig.trackStop(S_GB1_FIRE_HIGH_POWER_LOOP);
                  break;
                } 
              }
            break;            

            case W_FIRING_ALT:
              // Wand firing in alt mode.
              b_firing_alt = true;

              if(b_wand_firing == true && b_sound_firing_alt_trigger != true) {
                b_sound_firing_alt_trigger = true;
              }
            break;

            case W_FIRING_ALT_MIX:
              // Wand firing in alt mode.
              b_firing_alt = true;

              if(b_wand_firing == true && b_sound_firing_alt_trigger != true) {
                b_sound_firing_alt_trigger = true;
                
                w_trig.trackPlayPoly(S_FIRING_LOOP_GB1, true);
                w_trig.trackLoop(S_FIRING_LOOP_GB1, 1);
              }
            break;

            case W_FIRING_ALT_STOPPED:
              // Wand no longer firing in alt mode.
              b_firing_alt = false;
              b_sound_firing_alt_trigger = false;
            break;

            case W_FIRING_ALT_STOPPED_MIX:
              // Wand no longer firing in alt mode mix.
              b_firing_alt = false;
              b_sound_firing_alt_trigger = false;

              w_trig.trackStop(S_FIRING_LOOP_GB1);
            break;            

            case W_FIRING_CROSSING_THE_STREAMS:
              // Wand is crossing the streams.
              b_firing_cross_streams = true;

              switch(i_mode_year) {
                case 2021:
                  w_trig.trackGain(S_AFTERLIFE_CROSS_THE_STREAMS_START, i_volume + 4);
                  w_trig.trackPlayPoly(S_AFTERLIFE_CROSS_THE_STREAMS_START, true);
                break;

                case 1984:
                case 1989:
                  w_trig.trackGain(S_CROSS_STREAMS_START, i_volume + 4);
                  w_trig.trackPlayPoly(S_CROSS_STREAMS_START, true);
                break;
              }
              
              w_trig.trackGain(S_FIRE_START_SPARK, i_volume);
              w_trig.trackPlayPoly(S_FIRE_START_SPARK);
            break;

            case W_FIRING_CROSSING_THE_STREAMS_MIX:
              // Wand is crossing the streams.
              b_firing_cross_streams = true;
              
              switch(i_mode_year) {
                case 2021:
                  w_trig.trackGain(S_AFTERLIFE_CROSS_THE_STREAMS_START, i_volume + 4);
                  w_trig.trackPlayPoly(S_AFTERLIFE_CROSS_THE_STREAMS_START, true);
                break;

                case 1984:
                case 1989:
                  w_trig.trackGain(S_CROSS_STREAMS_START, i_volume + 4);
                  w_trig.trackPlayPoly(S_CROSS_STREAMS_START, true);
                break;
              }

              w_trig.trackGain(S_FIRE_START_SPARK, i_volume);
              w_trig.trackPlayPoly(S_FIRE_START_SPARK);

              w_trig.trackPlayPoly(S_FIRING_LOOP_GB1, true);
              w_trig.trackLoop(S_FIRING_LOOP_GB1, 1);

              if(i_wand_power_level != i_wand_power_level_max) {
                w_trig.trackPlayPoly(S_GB1_FIRE_HIGH_POWER_LOOP, true);
                w_trig.trackLoop(S_GB1_FIRE_HIGH_POWER_LOOP, 1);
              }

              w_trig.trackStop(S_GB2_FIRE_LOOP);
              w_trig.trackStop(S_GB1_FIRE_LOOP);
            break;

            case W_FIRING_CROSSING_THE_STREAMS_STOPPED:
              // The wand is no longer crossing the streams.
              b_firing_cross_streams = false;

              switch(i_mode_year) {
                case 2021:
                  w_trig.trackGain(S_AFTERLIFE_CROSS_THE_STREAMS_END, i_volume + 4);
                  w_trig.trackPlayPoly(S_AFTERLIFE_CROSS_THE_STREAMS_END, true);
                break;

                case 1984:
                case 1989:
                  w_trig.trackGain(S_CROSS_STREAMS_END, i_volume + 4);
                  w_trig.trackPlayPoly(S_CROSS_STREAMS_END, true);
                break;
              }

              w_trig.trackStop(S_FIRING_LOOP_GB1);
            break;

            case W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX:
              // The wand is no longer crossing the streams.
              b_firing_cross_streams = false;

              switch(i_mode_year) {
                case 2021:
                  w_trig.trackGain(S_AFTERLIFE_CROSS_THE_STREAMS_END, i_volume + 4);
                  w_trig.trackPlayPoly(S_AFTERLIFE_CROSS_THE_STREAMS_END, true);
                break;

                case 1984:
                case 1989:
                  w_trig.trackGain(S_CROSS_STREAMS_END, i_volume + 4);
                  w_trig.trackPlayPoly(S_CROSS_STREAMS_END, true);
                break;
              }
            break;

            case W_YEAR_MODES_CYCLE:
              // Toggle between the year modes.
              w_trig.trackStop(S_BEEPS_BARGRAPH);    
              w_trig.trackGain(S_BEEPS_BARGRAPH, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_BARGRAPH);

              switch(i_mode_year_tmp) {
                case 1984:
                  i_mode_year_tmp = 1989;

                  w_trig.trackStop(S_VOICE_AFTERLIFE);    
                  w_trig.trackStop(S_VOICE_1984); 
                  w_trig.trackStop(S_VOICE_1989);
                  w_trig.trackGain(S_VOICE_1989, i_volume);
                  w_trig.trackPlayPoly(S_VOICE_1989);

                  // Tell the wand to play the 1989 sound effect.
                  packSerialSend(P_MODE_1989);
                break;

                case 1989:
                  i_mode_year_tmp = 2021;

                  w_trig.trackStop(S_VOICE_AFTERLIFE);    
                  w_trig.trackStop(S_VOICE_1984);
                  w_trig.trackStop(S_VOICE_1989);
                  w_trig.trackGain(S_VOICE_AFTERLIFE, i_volume);
                  w_trig.trackPlayPoly(S_VOICE_AFTERLIFE);

                  // Tell the wand to play the 2021 sound effect.
                  packSerialSend(P_MODE_AFTERLIFE);
                break;

                case 2021:
                  i_mode_year_tmp = 1984;

                  w_trig.trackStop(S_VOICE_AFTERLIFE);    
                  w_trig.trackStop(S_VOICE_1984); 
                  w_trig.trackStop(S_VOICE_1989);
                  w_trig.trackGain(S_VOICE_1984, i_volume);
                  w_trig.trackPlayPoly(S_VOICE_1984);
                  
                  // Tell the wand to play the 1984 sound effect.
                  packSerialSend(P_MODE_1984);
                break;
              }

              // Turn on the year mode override flag. This resets when you flip the mode year toggle switch on the pack.
              b_switch_mode_override = true;
            break;

            case W_RESET_PROTON_STREAM:
              // Revert back to Proton mode. Usually because we are switching from crossing the streams to video game mode or vice versa.
              FIRING_MODE = PROTON;

              w_trig.trackStop(S_CLICK);    
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);

              w_trig.trackStop(S_VOICE_VIDEO_GAME_MODES);  
              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS_MIX);    
  
              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS);    
              w_trig.trackGain(S_VOICE_CROSS_THE_STREAMS, i_volume);
              w_trig.trackPlayPoly(S_VOICE_CROSS_THE_STREAMS);          
            break;

            case W_RESET_PROTON_STREAM_MIX:
              // Revert back to Proton mode. Usually because we are switching from crossing the streams to video game mode or vice versa.
              FIRING_MODE = PROTON;

              w_trig.trackStop(S_CLICK);    
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);

              w_trig.trackStop(S_VOICE_VIDEO_GAME_MODES);    
              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS);
              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS_MIX);

              w_trig.trackGain(S_VOICE_CROSS_THE_STREAMS_MIX, i_volume);
              w_trig.trackPlayPoly(S_VOICE_CROSS_THE_STREAMS_MIX);          
            break;

            case W_VIBRATION_DISABLED:
              // Neutrona Wand vibration disabled.
              w_trig.trackStop(S_BEEPS_ALT);    
              w_trig.trackGain(S_BEEPS_ALT, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_ALT);

              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);    
              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);    
              w_trig.trackGain(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED, i_volume);
              w_trig.trackPlayPoly(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);
            break;

            case W_VIBRATION_ENABLED:
              // Neutrona Wand Vibration enabled.
              w_trig.trackStop(S_BEEPS_ALT);    
              w_trig.trackGain(S_BEEPS_ALT, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_ALT);

              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);    
              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);    
              w_trig.trackGain(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED, i_volume);
              w_trig.trackPlayPoly(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
            break;
            
            case W_VIBRATION_FIRING_ENABLED:
              // Neutrona Wand vibration during firing only enabled.
              w_trig.trackStop(S_BEEPS_ALT);    
              w_trig.trackGain(S_BEEPS_ALT, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_ALT);

              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);    
              w_trig.trackStop(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);    
              w_trig.trackGain(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED, i_volume);
              w_trig.trackPlayPoly(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
            break;

            case W_VIBRATION_CYCLE_TOGGLE:
              w_trig.trackStop(S_BEEPS_ALT);    
              w_trig.trackGain(S_BEEPS_ALT, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_ALT);

              if(b_vibration == false) {
                b_vibration = true;
                b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

                // Proton Pack Vibration enabled.
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);    
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);    
                w_trig.trackGain(S_VOICE_PROTON_PACK_VIBRATION_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);

                packSerialSend(P_PACK_VIBRATION_ENABLED);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
              else if(b_vibration == true && b_vibration_firing != true) {
                b_vibration_firing = true;
                b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

                // Proton Pack Vibration firing enabled.
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);    
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);    
                w_trig.trackGain(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);

                packSerialSend(P_PACK_VIBRATION_FIRING_ENABLED);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
              else {
                b_vibration_firing = false;
                b_vibration = false;

                // Proton Pack Vibration disabled.
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);    
                w_trig.trackStop(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);    
                w_trig.trackGain(S_VOICE_PROTON_PACK_VIBRATION_DISABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

                packSerialSend(P_PACK_VIBRATION_DISABLED);
              }
            break;

            case W_SMOKE_TOGGLE:
              if(b_smoke_enabled == true) {
                b_smoke_enabled = false;

                w_trig.trackStop(S_VENT_DRY);
                w_trig.trackGain(S_VENT_DRY, i_volume);
                w_trig.trackPlayPoly(S_VENT_DRY);

                w_trig.trackStop(S_VOICE_SMOKE_DISABLED);
                w_trig.trackStop(S_VOICE_SMOKE_ENABLED);    
                w_trig.trackGain(S_VOICE_SMOKE_DISABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_SMOKE_DISABLED);

                // Tell the wand to play the smoke disabled voice.
                packSerialSend(P_SMOKE_DISABLED);
              }
              else {
                b_smoke_enabled = true;

                w_trig.trackStop(S_VENT_SMOKE);
                w_trig.trackGain(S_VENT_SMOKE, i_volume);
                w_trig.trackPlayPoly(S_VENT_SMOKE);

                w_trig.trackStop(S_VOICE_SMOKE_ENABLED);
                w_trig.trackStop(S_VOICE_SMOKE_DISABLED);    
                w_trig.trackGain(S_VOICE_SMOKE_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_SMOKE_ENABLED);

                // Tell the wand to play the smoke enabled voice.
                packSerialSend(P_SMOKE_ENABLED);
              }
            break;

            case W_PROTON_MODE_REVERT:
              // Revert back to Proton mode. Usually because we are switching from crossing the streams to video game mode or vice versa.
              FIRING_MODE = PROTON;

              w_trig.trackStop(S_CLICK);    
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);

              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS);   
              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS_MIX); 
              w_trig.trackStop(S_VOICE_VIDEO_GAME_MODES);    
              w_trig.trackGain(S_VOICE_VIDEO_GAME_MODES, i_volume);
              w_trig.trackPlayPoly(S_VOICE_VIDEO_GAME_MODES);          
            break;

            case W_CYCLOTRON_DIRECTION_TOGGLE:
              // Toggle the cyclotron direction.
              if(b_clockwise == true) {
                b_clockwise = false;

                w_trig.trackStop(S_BEEPS_ALT);    
                w_trig.trackGain(S_BEEPS_ALT, i_volume);
                w_trig.trackPlayPoly(S_BEEPS_ALT);

                w_trig.trackStop(S_VOICE_CYCLOTRON_CLOCKWISE);
                w_trig.trackStop(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);    
                w_trig.trackGain(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE, i_volume);
                w_trig.trackPlayPoly(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);     

                // Tell the wand to play the same sound.
                packSerialSend(P_CYCLOTRON_COUNTER_CLOCKWISE);
              }
              else {
                b_clockwise = true;

                w_trig.trackStop(S_BEEPS);
                w_trig.trackGain(S_BEEPS, i_volume);
                w_trig.trackPlayPoly(S_BEEPS);

                w_trig.trackStop(S_VOICE_CYCLOTRON_CLOCKWISE);
                w_trig.trackStop(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);    
                w_trig.trackGain(S_VOICE_CYCLOTRON_CLOCKWISE, i_volume);
                w_trig.trackPlayPoly(S_VOICE_CYCLOTRON_CLOCKWISE);

                // Tell the wand to play the same sound.
                packSerialSend(P_CYCLOTRON_CLOCKWISE);
              }
            break;

            case W_CYCLOTRON_LED_TOGGLE:
              // Toggle single LED or 3 LEDs per cyclotron lens in 1984/1989 modes.
              if(b_cyclotron_single_led == true) {
                b_cyclotron_single_led = false;

                w_trig.trackStop(S_VOICE_THREE_LED);
                w_trig.trackStop(S_VOICE_SINGLE_LED);    
                w_trig.trackGain(S_VOICE_THREE_LED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_THREE_LED);

                // Tell the wand to play the same sound.
                packSerialSend(P_CYCLOTRON_THREE_LED);
              }
              else {
                b_cyclotron_single_led = true;

                // Play Single LED voice.
                w_trig.trackStop(S_VOICE_THREE_LED);
                w_trig.trackStop(S_VOICE_SINGLE_LED);    
                w_trig.trackGain(S_VOICE_SINGLE_LED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_SINGLE_LED);

                // Tell the wand to play the same sound.
                packSerialSend(P_CYCLOTRON_SINGLE_LED);
              }
            break;

            case W_VIDEO_GAME_MODE_COLOUR_TOGGLE:
              w_trig.trackStop(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              w_trig.trackStop(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);    
              w_trig.trackStop(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              w_trig.trackStop(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              // Toggle through the various Video Game Colour Modes for the Proton Pack LEDs (if supported).
              if(b_cyclotron_colour_toggle == true && b_powercell_colour_toggle == true) {
                // Disabled, both Cyclotron and PowerCell video game colours.
                b_cyclotron_colour_toggle = false;
                b_powercell_colour_toggle = false;

                w_trig.trackGain(S_VOICE_VIDEO_GAME_COLOURS_DISABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);  
                
                // Tell the wand to play the same sound.
                packSerialSend(P_VIDEO_GAME_MODE_COLOURS_DISABLED); 
              }
              else if(b_cyclotron_colour_toggle != true && b_powercell_colour_toggle != true) {
                // PowerCell only.
                b_cyclotron_colour_toggle = false;
                b_powercell_colour_toggle = true;

                w_trig.trackGain(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);

                // Tell the wand to play the same sound.
                packSerialSend(P_VIDEO_GAME_MODE_POWER_CELL_ENABLED);                                  
              }
              else if(b_cyclotron_colour_toggle != true && b_powercell_colour_toggle == true) {
                // Cyclotron only.
                b_cyclotron_colour_toggle = true;
                b_powercell_colour_toggle = false;

                w_trig.trackGain(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

                // Tell the wand to play the same sound.
                packSerialSend(P_VIDEO_GAME_MODE_CYCLOTRON_ENABLED);
              }
              else {
                // Enabled, both Cyclotron and PowerCell video game colours.
                b_cyclotron_colour_toggle = true;
                b_powercell_colour_toggle = true;

                w_trig.trackGain(S_VOICE_VIDEO_GAME_COLOURS_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);  

                // Tell the wand to play the same sound.
                packSerialSend(P_VIDEO_GAME_MODE_COLOURS_ENABLED);                                  
              }
            break;

            case W_OVERHEATING_DISABLED:
              // Play the overheating disabled voice.
              w_trig.trackStop(S_VOICE_OVERHEAT_DISABLED);    
              w_trig.trackStop(S_VOICE_OVERHEAT_ENABLED);    
              w_trig.trackGain(S_VOICE_OVERHEAT_DISABLED, i_volume);
              w_trig.trackPlayPoly(S_VOICE_OVERHEAT_DISABLED);
            break;

            case W_OVERHEATING_ENABLED:
              // Play the overheating enabled voice.
              w_trig.trackStop(S_VOICE_OVERHEAT_DISABLED);    
              w_trig.trackStop(S_VOICE_OVERHEAT_ENABLED);    
              w_trig.trackGain(S_VOICE_OVERHEAT_ENABLED, i_volume);
              w_trig.trackPlayPoly(S_VOICE_OVERHEAT_ENABLED);
            break;
                
            case W_MENU_LEVEL_CHANGE:
              // Play a beep during a sub menu to menu level change.
              w_trig.trackStop(S_BEEPS);
              w_trig.trackGain(S_BEEPS, i_volume);
              w_trig.trackPlayPoly(S_BEEPS);
            break;
            
            case W_VOLUME_MUSIC_DECREASE:
              // Lower music volume.
              if(b_playing_music == true) {    
                if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
                  i_volume_music_percentage = 0;
                }
                else {
                  i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
                }

                i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

                w_trig.trackGain(i_current_music_track, i_volume_music);
              }
            break;
        
            case W_VOLUME_MUSIC_INCREASE:
              // Increase music volume.
            if(b_playing_music == true) {
                if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
                  i_volume_music_percentage = 100;
                }
                else {
                  i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
                }

                i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);
        
                w_trig.trackGain(i_current_music_track, i_volume_music);
              } 
            break;
            
            case W_VOLUME_SOUND_EFFECTS_DECREASE:
              // Lower the sound effects volume.
              decreaseVolumeEffects();
            break;
        
            case W_VOLUME_SOUND_EFFECTS_INCREASE:
              // Increase the sound effects volume.
              increaseVolumeEffects();
            break;
            
            case W_MUSIC_TRACK_LOOP_TOGGLE:
              // Loop the music track.
              if(b_repeat_track == false) {
                b_repeat_track = true;
                w_trig.trackLoop(i_current_music_track, 1);
              }
              else {
                b_repeat_track = false;
                w_trig.trackLoop(i_current_music_track, 0);
              }
            break;
            
            case W_SILENT_MODE:
              i_volume_revert = i_volume_master;

              // Set the master volume to silent.
              i_volume_master = -70;
              
              w_trig.masterGain(i_volume_master); // Reset the master gain.
            break;

            case W_VOLUME_REVERT:
              // Set the master volume to silent.
              i_volume_master = i_volume_revert;
              
              w_trig.masterGain(i_volume_master); // Reset the master gain.
            break;

            case W_VOLUME_DECREASE:
              // Lower overall pack volume.
              decreaseVolume();
            break;
        
            case W_VOLUME_INCREASE:
              // Increase overall pack volume.
              increaseVolume();
            break;
            
            case W_MUSIC_STOP:
              // Stop music.
              b_playing_music = false;
              stopMusic();
            break;
        
            case W_MUSIC_START:
              // Play music.
              b_playing_music = true;
              playMusic();
            break;
            
            default:
              // Music track number to be played.
              if(comStruct.i >= i_music_track_start) {
                if(b_playing_music == true) {
                  stopMusic();
                  i_current_music_track = comStruct.i;
                  playMusic();
                }
                else {
                  i_current_music_track = comStruct.i;
                }
              }
            break;
          }
        }
        else {
          /* 
          *  Check if the wand is telling us it is here after connecting it to the pack.
          *  Then Synchronise some settings between the pack and the wand.
          */
          if(comStruct.i == W_HANDSHAKE) {
            // Tell the wand that the pack is here.
            packSerialSend(P_HANDSHAKE);

            if(i_mode_year == 1984) {
              packSerialSend(P_YEAR_1984);
            }
            else if(i_mode_year == 1989) {
              packSerialSend(P_YEAR_1989);
            }
            else {
              packSerialSend(P_YEAR_AFTERLIFE);
            }

            // Stop any music.
            packSerialSend(P_MUSIC_STOP);
            b_playing_music = false;
            stopMusic();

            packSerialSend(i_current_music_track);

            if(b_repeat_track == true) {
              packSerialSend(P_MUSIC_REPEAT);
            }
            else {
              packSerialSend(P_MUSIC_NO_REPEAT);
            }
            
            // Vibration enabled or disabled from the Proton Pack toggle switch
            if(b_vibration_enabled == true) {
              packSerialSend(P_VIBRATION_ENABLED);
            }
            else {
              packSerialSend(P_VIBRATION_DISABLED);
            }

            // Ribbon cable alarm.
            if(b_alarm == true) {
              packSerialSend(P_ALARM_ON);
            }
            else {
              packSerialSend(P_ALARM_OFF);
            }

            // Pack status
            if(PACK_STATUS != MODE_OFF) {
              packSerialSend(P_ON);
            }
            else {
              packSerialSend(P_OFF);
            }

            // Put the wand into volume sync mode.
            packSerialSend(P_VOLUME_SYNC_MODE);

            // Sequence here is important. Synchronise the volume settings.
            packSerialSend(i_volume_percentage);

            packSerialSend(i_volume_master_percentage);

            packSerialSend(i_volume_music_percentage);


            if(i_volume_master == -70) {
              // Telling the wand to be silent if required.
              packSerialSend(P_MASTER_AUDIO_SILENT_MODE);
            }
            else {
              packSerialSend(P_MASTER_AUDIO_NORMAL);
            }

            b_wand_connected = true;
          }
        }
      }
    }
  }
}

void packSerialSend(int i_message) {
  sendStruct.s = P_COM_START;
  sendStruct.i = i_message;
  sendStruct.e = P_COM_END;

  packComs.sendDatum(sendStruct);
}

void setupWavTrigger() {
  // If the controller is powering the WAV Trigger, we should wait for the WAV trigger to finish reset before trying to send commands.
  delay(1000);
  
  // WAV Trigger's startup at 57600
  w_trig.start();
  
  delay(10);

  w_trig.stopAllTracks();
  w_trig.samplerateOffset(0); // Reset our sample rate offset        
  w_trig.masterGain(i_volume_master); // Reset the master gain db. Range is -70 to 0.
  w_trig.setAmpPwr(b_onboard_amp_enabled);
  
  // Enable track reporting from the WAV Trigger
  w_trig.setReporting(false);

  // Allow time for the WAV Triggers to respond with the version string and number of tracks.
  delay(350);
  
  int w_num_tracks = w_trig.getNumTracks();

  // Build the music track count.
  i_music_count = w_num_tracks - i_last_effects_track;
  
  if(i_music_count > 0) {
    i_current_music_track = i_music_track_start; // Set the first track of music as file 500_
  }
}
