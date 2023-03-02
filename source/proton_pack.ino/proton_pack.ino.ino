/********************************************************
  Haslab Proton Pack.
  January 2022.
  Michael Rajotte / gpstar
********************************************************/
/* 
 *  You need to edit wavTrigger.h and make sure you comment out the proper serial port. (Near the top of the wavTrigger.h file).
 *  We are going to use tx/rx #3 on the Mega.  __WT_USE_SERIAL3___
 */
#include <wavTrigger.h>
#include <millisDelay.h> 
#include <FastLED.h>
#include <ezButton.h>
#include <Ramp.h>

/* 
 *  SD Card sound files in order. If you have no sound, your SD card might be too slow, try another one.
 *  File naming 000_ is important as well. For music, it is 100_ and higher.
 *  Also note if you add more sounds to this list, you need to update the wavtrigger setup function to let it know the last
 *  sound effect file (very bottom of this code). The wav trigger uses this to determine how many music tracks there are if any.
 */
enum sound_fx {
  S_EMPTY, 
  S_BOOTUP,
  S_SHUTDOWN,
  S_IDLE_LOOP,
  S_IDLE_LOOP_GUN,
  S_FIRE_START,
  S_FIRE_START_SPARK,
  S_FIRE_LOOP,
  S_FIRE_LOOP_GUN,
  S_FIRE_LOOP_IMPACT,
  S_FIRING_END,
  S_FIRING_END_GUN,
  S_AFTERLIFE_BEEP_WAND,
  S_AFTERLIFE_GUN_RAMP_LOW,
  S_AFTERLIFE_GUN_RAMP_HIGH,
  S_AFTERLIFE_PACK_STARTUP,
  S_AFTERLIFE_PACK_IDLE_LOOP,
  S_WAND_SHUTDOWN,
  S_AFTERLIFE_BEEP_WAND_S1,
  S_AFTERLIFE_BEEP_WAND_S2,
  S_AFTERLIFE_BEEP_WAND_S3,
  S_AFTERLIFE_BEEP_WAND_S4,
  S_AFTERLIFE_BEEP_WAND_S5,
  S_AFTERLIFE_GUN_RAMP_1,
  S_AFTERLIFE_GUN_LOOP_1,
  S_AFTERLIFE_GUN_RAMP_2,
  S_AFTERLIFE_GUN_LOOP_2,
  S_AFTERLIFE_GUN_RAMP_DOWN_2,
  S_AFTERLIFE_GUN_RAMP_DOWN_1,
  S_IDLE_LOOP_GUN_2,
  S_IDLE_LOOP_GUN_3,
  S_IDLE_LOOP_GUN_4,
  S_IDLE_LOOP_GUN_5,
  S_IDLE_LOOP_GUN_1,
  S_PACK_BEEPING,
  S_PACK_SHUTDOWN,
  S_PACK_SHUTDOWN_AFTERLIFE,
  S_GB2_PACK_START,
  S_GB2_PACK_LOOP,
  S_GB2_PACK_OFF,
  S_CLICK,
  S_VENT,
  S_VENT_SLOW,
  S_VENT_FAST,
  S_VENT_DRY,
  S_VENT_BEEP,
  S_VENT_BEEP_3,
  S_VENT_BEEP_7,
  S_PACK_SLIME_OPEN,
  S_PACK_SLIME_CLOSE,
  S_PACK_SLIME_TANK_LOOP,
  S_SLIME_START,
  S_SLIME_LOOP,
  S_SLIME_END,
  S_STASIS_START,
  S_STASIS_LOOP,
  S_STASIS_END,
  S_MESON_START,
  S_MESON_LOOP,
  S_MESON_END,
  S_BEEP_8,
  S_VENT_SMOKE,
  S_MODE_SWITCH,
  S_BEEPS,
  S_BEEPS_ALT,
  S_SPARKS_LONG,
  S_SPARKS_LOOP,
  S_BEEPS_LOW,
  S_BEEPS_BARGRAPH,
  S_MESON_OPEN,
  S_STASIS_OPEN,
  S_FIRING_END_MID,
  S_FIRING_LOOP_GB1,
  S_CROSS_STREAMS_END,
  S_CROSS_STREAMS_START
};

/*
 * Use the following defines to change which optional NeoPixels you are using
 * in your inner cyclotron. If you are not using any, then this can be left alone.
 * Leave at least one in place, even if you are not using any.
 * 35 -> For a 35 LED NeoPixel Ring
 * 56 -> For 8 NeoPixel Jewels chained together. (7 pixels per jewel)
 */
#define CYCLOTRON_NUM_LEDS 35
//#define CYCLOTRON_NUM_LEDS 56

/*
 * You can set the default startup volume for your pack here.
 * NOTE: Make sure to set this to the same value in the Neturona Wand code.
 * If not then the startup volume will levels will not be in sync.
 * 4 = loudest
 * -70 = quietest
 */
const int STARTUP_VOLUME = 0;

/* 
 *  Default the cyclotron lights direction to clockwise. 
 *  This can be controlled by an optional switch on pin 29. 
 *  Set to false to be counter clockwise.
 */
bool b_clockwise = true;

/*
 * Set this to true if you want to know if your wand and pack are communicating.
 * This can be handy when you are first building your board and connections to test things.
 * If the wand and pack have a serial connection, you will hear a beeping sound.
 * Set to false to turn off the sound.
 */
bool b_diagnostic = false;

/*
 * -------------****** DO NOT CHANGE ANYTHING BELOW THIS LINE ******-------------
 */

 
/* 
 *  PowerCell and Cyclotron Lid LEDs.
 *  25 LEDs in the stock Haslab kit. 13 in the Powercell and 12 in the Cyclotron lid. 
 *  7 additional (32 in total) for a NeoPixel jewel that you can put into the n-filter (optional). This jewel chains off LED #4 in the lid.
 */
#define PACK_LED_PIN 53
#define PACK_NUM_LEDS 32
CRGB pack_leds[PACK_NUM_LEDS];
#define VENT_LIGHT_START 25

/*
 * Inner Cyclotron LEDs. (optional).
 * Uses pin 13.
 */
#define CYCLOTRON_LED_PIN 13
CRGB cyclotron_leds[CYCLOTRON_NUM_LEDS];

/*
 * Delay for fastled to update the addressable LEDs. 
 * We have up to 88 addressable LEDs if using NeoPixel jewels in the inner cyclotron and n-filter.
 * 0.03 ms to update 1 LED. So 3 ms should be ok. Lets bump it up to 8 just in case.
 */
const int i_fast_led_delay = 8;
millisDelay ms_fast_led;

/*
 * Powercell LEDs control.
 */
const int i_powercell_delay_1984 = 75;
const int i_powercell_delay_2021 = 40;
int i_powercell_delay = i_powercell_delay_2021;
int i_powercell_led = 0;
millisDelay ms_powercell;

/* 
 *  Cyclotron Inner Switch Panel LEDs control. (optional)
 */
const int i_cyclotron_switch_led_delay_base = 150;
int i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base;
int i_cyclotron_sw_led = 0;
int i_cyclotron_switch_led_mulitplier = 1;
millisDelay ms_cyclotron_switch_led;

/* 
 *  State of the pack.
 */
enum PACK_STATE { MODE_OFF, MODE_ON };
enum PACK_STATE PACK_STATUS;

/*
 * Pack action state.
 */
enum PACK_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE };
enum PACK_ACTION_STATE PACK_ACTION_STATUS;

/*
 * Cyclotron lid LEDs control and lid detection.
 */
const int cyclotron_led_start = 13; // First LED in the cyclotron.
int i_led_cyclotron = cyclotron_led_start; // Current cyclotron LED that we are lighting up.
const int i_1984_delay = 1050;
const int i_2021_delay = 15;
const int i_2021_ramp_delay = 300;
const int i_2021_ramp_length = 6000;
const int i_1984_ramp_length = 2000;
const int i_2021_ramp_down_length = 5000;
const int i_1984_ramp_down_length = 3000;
int i_current_ramp_speed = i_2021_delay;
int i_cyclotron_multiplier = 1;
bool b_2021_ramp_up = true;
bool b_2021_ramp_up_start = true;
bool b_2021_ramp_down_start = false;
bool b_2021_ramp_down = false;
bool b_reset_start_led = true;
rampInt r_2021_ramp;
millisDelay ms_cyclotron;
boolean b_cyclotron_lid_on = true;

/*
 * Inner cyclotron NeoPixel ring ramp control.
 * The 8 Jewel option is piggybacking off the cyclotron lid led controls above.
 * This is for the 35 LED NeoPixel ring option.
 */
millisDelay ms_cyclotron_ring;
rampInt r_inner_ramp;
const int i_inner_delay = i_2021_delay;
const int i_inner_ramp_delay = i_2021_ramp_delay;
int i_led_cyclotron_ring = 0;
bool b_inner_ramp_up = true;
bool b_inner_ramp_down = false;
const int i_2021_inner_delay = 5;
const int i_1984_inner_delay = 1;
int i_inner_current_ramp_speed = i_2021_delay - i_2021_inner_delay;
        
/* 
 *  Inner cyclotron communication
 *  This is used by the 8 NeoPixel Jewel option only.
 *  The jewel option is piggybacking it's timing off the cyclotron lid led controls.
 */
int i_cyclotron_data = 0;
int i_prev_cyclotron_data = 0;

/*
 * Cyclotron Switch Plate LEDs
 */
const int cyclotron_sw_plate_led_r1 = 4;
const int cyclotron_sw_plate_led_r2 = 5;
const int cyclotron_sw_plate_led_y1 = 6;
const int cyclotron_sw_plate_led_y2 = 7;
const int cyclotron_sw_plate_led_g1 = 8;
const int cyclotron_sw_plate_led_g2 = 9;
const int cyclotron_switch_led_green = 10; // 1984/2021 mode switch led.
const int cyclotron_switch_led_yellow = 11; // Vibration on/off switch led.
millisDelay ms_cyclotron_switch_plate_leds;
const int i_cyclotron_switch_plate_leds_delay = 1000;

/* 
 * Alarm. Used during overheating and or ribbon cable removal.
 */
const int i_alarm_delay = 500;
bool b_alarm = false;
millisDelay ms_alarm;

/* 
 *  Switches
 */
ezButton switch_cyclotron_lid(51); // Second cyclotron ground pin (brown) that we feed to the Arduino to detect if the lid is removed or not.
ezButton switch_alarm(23); // Ribbon cable removal switch
ezButton switch_mode(25); // 1984 / 2021 mode toggle switch
ezButton switch_vibration(27); // Vibration toggle switch
ezButton switch_cyclotron_direction(29); // Newly added switch for controlling the direction of the cyclotron lights. Not required. Defaults to clockwise.
ezButton switch_power(31); // Red power switch under the ion arm.
ezButton switch_smoke(37);

/* 
 *  Wav trigger
 */
wavTrigger w_trig;
int i_music_count = 0;
int i_current_music_track = 0;
const int i_music_track_start = 100; // Music tracks start on file named 100_ and higher.
boolean b_playing_music = false;
boolean b_repeat_track = false;

/* 
 *  Volume (4 = loudest, -70 = quietest)
 */
int i_volume = STARTUP_VOLUME;
int i_volume_music = STARTUP_VOLUME;

/*
 * Vibration motor settings
 */
const int vibration = 45;
int i_vibration_level = 0;
int i_vibration_level_prev = 0;
boolean b_vibration = false;

/*
 * Smoke & over heating
 */
const int smoke_pin = 39;
boolean b_smoke_enabled = false;

/*
 * Smoke for a second smoke machine or motor. I use this in the booster tube.
 */
const int smoke_booster_pin = 35;

/*
 * Fan for smoke
 */
const int fan_pin = 33;
millisDelay ms_fan_stop_timer;
const int i_fan_stop_timer = 9000;

/* 
 * Overheating and smoke timers for smoke_pin. 
 */
millisDelay ms_overheating;
const int i_overheating_delay = 4000;
boolean b_overheating = false;
const int i_smoke_timer = 30000;
millisDelay ms_smoke_timer;


/*
 * Vent light timers and delay for over heating.
 */
millisDelay ms_vent_light_on;
millisDelay ms_vent_light_off;
const int i_vent_light_delay = 50;
boolean b_vent_sounds; // A flag for playing smoke and vent sounds.

/* 
 *  Wand communication
 */

enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SETTINGS };
enum FIRING_MODES FIRING_MODE;
boolean b_wand_firing = false;
boolean b_wand_connected = false;
millisDelay ms_wand_handshake;
const int i_wand_handshake_delay = 2000;
millisDelay ms_wand_handshake_checking;
int i_wand_power_level = 1; // Power level of the wand.
int rx_byte = 0;
int prev_byte = 0;

/*
 * Firing timers
 */
millisDelay ms_firing_length_timer;
const int i_firing_timer_length = 15000; // 15 seconds. Used by ms_firing_length_timer to determine which tail_end sound effects to play.

/* 
 * Rotary encoder for volume control 
 */
#define encoder_pin_a 2
#define encoder_pin_b 3
int i_encoder_pos = 0;
int i_val_rotary;
int i_last_val_rotary;

/*
 * Misc.
 */
int i_mode_year = 2021; // 1984 or 2021
bool b_pack_on = false;
bool b_pack_shutting_down = false;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600); // Communication to the wand.

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
  TCCR5B = TCCR5B & B11111000 | B00000100;  // for PWM frequency of 122.55 Hz

  // Vibration motor
  pinMode(vibration, OUTPUT);

  // Smoke motor
  pinMode(smoke_pin, OUTPUT);

  // Second smoke motor (booster tube)
  pinMode(smoke_booster_pin, OUTPUT);

  // Fan pin for smoke
  pinMode(fan_pin, OUTPUT);
  
  // Powercell and cyclotron LEDs.
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, PACK_NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, CYCLOTRON_NUM_LEDS);
  
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

  // Start some timers
  ms_cyclotron.start(i_2021_delay);
  ms_cyclotron_ring.start(i_inner_ramp_delay);
  ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);
  ms_wand_handshake.start(1);
  ms_fast_led.start(i_fast_led_delay);

  // Tell the wand the pack is here.
  Serial2.write(0);

}

void loop() {
  w_trig.update();
  
  innerCyclotronCommunication();
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
            i_powercell_delay = i_powercell_delay_1984;
            i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base * 4;
          break;
        }

        // Vibration motor off.
        vibrationPack(0);
        i_vibration_level = 0;

        if(b_pack_shutting_down == true) {
          b_pack_shutting_down = false;
        }
      }
      
      if(b_pack_on == true) {
        // Tell the wand the pack is off, so shut down the wand as well if it is still on.
        Serial2.write(2);
      }

      b_pack_on = false;
    break;
   
    case MODE_ON:
      if(b_pack_on == false) {
        // Tell the wand the pack is on.
        Serial2.write(1);
      }
      
      b_pack_on = true;
      
      if(b_2021_ramp_down == true) {
        b_2021_ramp_down = false;
        b_2021_ramp_down_start = false;

        reset2021RampUp();
      }
        
      if(switch_alarm.getState() == LOW && b_overheating == false) {
        if(b_alarm == true) {
          // Tell the wand the pack alarm is off.
          Serial2.write(4);
          
          b_alarm = false;

          resetCyclotronLeds();
          reset2021RampUp();

          packStartup();
          
          ms_cyclotron.start(i_2021_delay);
          ms_cyclotron_ring.start(i_inner_ramp_delay);      
        }
      }   

      // Play a little bit of smoke and n-filter vent lights while firing. Just a tiny bit....
      if(b_wand_firing == true) {
        if(ms_smoke_timer.remaining() < 3000 && ms_smoke_timer.remaining() > 0) {
          // Turn on some smoke and play some vent sounds if smoke is enabled.
          if(b_smoke_enabled == true) {
            // Turn on some smoke.
            smokeControl(true);
  
            // Play some sounds with the smoke and vent lighting.
            if(b_vent_sounds == true) {
              w_trig.trackGain(S_VENT_SMOKE, -5);
              w_trig.trackPlayPoly(S_VENT_SMOKE);
              w_trig.trackGain(S_SPARKS_LOOP, -5);
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

        if(ms_smoke_timer.justFinished()) {
          ms_smoke_timer.start(i_smoke_timer / i_wand_power_level);
          b_vent_sounds = true;
        }
      }
      
      cyclotronSwitchLEDLoop();
      powercellLoop();
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
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

void packStartup() {
  PACK_STATUS = MODE_ON;
  PACK_ACTION_STATUS = ACTION_IDLE;

  if(b_alarm == true) {
    packAlarm();
  }
  else {
    w_trig.trackStop(S_PACK_BEEPING);
    w_trig.trackStop(S_PACK_SHUTDOWN_AFTERLIFE);
    
    switch(i_mode_year) {
      case 1984:
        w_trig.trackGain(S_BOOTUP, 0);
        w_trig.trackPlayPoly(S_BOOTUP, true);
         
        w_trig.trackGain(S_IDLE_LOOP, -20);
        w_trig.trackPlayPoly(S_IDLE_LOOP, true);
        w_trig.trackFade(S_IDLE_LOOP, -1, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP, 1);
      break;
  
      default:
        w_trig.trackGain(S_AFTERLIFE_PACK_STARTUP, 0);
        w_trig.trackPlayPoly(S_AFTERLIFE_PACK_STARTUP, true);
  
        w_trig.trackGain(S_AFTERLIFE_PACK_IDLE_LOOP, -20);
        w_trig.trackPlayPoly(S_AFTERLIFE_PACK_IDLE_LOOP, true);
        w_trig.trackFade(S_AFTERLIFE_PACK_IDLE_LOOP, -1, 15000, 0);
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

  w_trig.trackStop(S_BEEP_8);
  w_trig.trackStop(S_SHUTDOWN);
  w_trig.trackStop(S_PACK_SHUTDOWN);
  w_trig.trackStop(S_PACK_SHUTDOWN_AFTERLIFE);

  w_trig.trackStop(S_PACK_BEEPING);
  w_trig.trackStop(S_IDLE_LOOP);
  w_trig.trackStop(S_BOOTUP);
  w_trig.trackStop(S_AFTERLIFE_PACK_STARTUP);
  w_trig.trackStop(S_AFTERLIFE_PACK_IDLE_LOOP);

  if(b_alarm != true) {
    switch(i_mode_year) {
      case 1984:
        w_trig.trackPlayPoly(S_SHUTDOWN, true);
        w_trig.trackPlayPoly(S_PACK_SHUTDOWN, true);
      break;
  
      default:
        w_trig.trackPlayPoly(S_PACK_SHUTDOWN_AFTERLIFE, true);
      break;
    }
  }
  else {
    w_trig.trackPlayPoly(S_SHUTDOWN, true);
  }

  // Turn off the vent light if it is on.
  ventLight(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  // Turn off any smoke.
  smokeControl(false);

  // Turn off the n-filter fan.
  ms_fan_stop_timer.stop();
  fanControl(false);
  
  // Reset vent sounds flag.
  b_vent_sounds = true;
}

void checkSwitches() {
  // Cyclotron direction toggle switch.
  if(switch_cyclotron_direction.isPressed()) {
    b_clockwise = true;
  }
  else if(switch_cyclotron_direction.isReleased()) {
    b_clockwise = false;
  }
  
  // Smoke
  if(switch_smoke.getState() == HIGH) {
    if(b_smoke_enabled == true) {
      smokeControl(false);

      b_smoke_enabled = false;
    }
  }
  else {
    if(b_smoke_enabled == false) {
      b_smoke_enabled = true;
    }
  }

  // Vibration toggle switch.
  if(switch_vibration.getState() == LOW) {
    if(b_vibration == false) {
      // Tell the wand to turn vibration on.
      Serial2.write(5);
      
      w_trig.trackStop(S_BEEPS_ALT);    
      w_trig.trackGain(S_BEEPS_ALT, 0);
      w_trig.trackPlayPoly(S_BEEPS_ALT);

      b_vibration = true;
    }
  }
  else {
    if(b_vibration == true) {
      // Tell the wand to turn vibration off.
      Serial2.write(6);

      w_trig.trackStop(S_BEEPS_ALT);    
      w_trig.trackGain(S_BEEPS_ALT, 0);
      w_trig.trackPlayPoly(S_BEEPS_ALT);

      b_vibration = false;
    }
  }  

  // Play sound when the mode switch is pressed or released.
  if(switch_mode.isPressed() || switch_mode.isReleased()) {
    w_trig.trackStop(S_BEEPS_BARGRAPH);    
    w_trig.trackGain(S_BEEPS_BARGRAPH, 0);
    w_trig.trackPlayPoly(S_BEEPS_BARGRAPH);
  }
  
  
  switch(PACK_STATUS) {
    case MODE_OFF:     
      if(switch_power.isPressed() || switch_power.isReleased()) {
        // Turn the pack on.
        PACK_ACTION_STATUS = ACTION_ACTIVATE;
      }

      // Year mode. Best to adjust it only when the pack is off.
      if(r_2021_ramp.isRunning() == false && b_pack_on == false) {        
        if(switch_mode.getState() == LOW) {
          if(i_mode_year == 2021) {
            // Tell the wand to switch to 1984 mode.
            Serial2.write(7);
          }
          
          i_mode_year = 1984;
        }
        else {
          if(i_mode_year == 1984) {
            // Tell the wand to switch to 2021 mode.
            Serial2.write(8);
          }
          
          i_mode_year = 2021;
        }
      }
    break;

    case MODE_ON:
      //if(switch_power.getState() == HIGH) {
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
        if(b_2021_ramp_up == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + (i_2021_ramp_delay - r_2021_ramp.update());
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

void powercellLoop() {
  if(ms_powercell.justFinished()) {
    // Powercell
    if(i_powercell_led > PACK_NUM_LEDS - 7 - cyclotron_led_start) {
      powercellOff();
  
      i_powercell_led = 0;
    }
    
    pack_leds[i_powercell_led] = CRGB(0,0,255);

    //FastLED.show();

    i_powercell_led++;
    
    // Setup the delays again.
    int i_pc_delay = i_powercell_delay;

    switch(i_mode_year) {
      default:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + (i_2021_ramp_delay - r_2021_ramp.update());
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 5;
    }

    ms_powercell.start(i_pc_delay);
  }
}

void powercellOff() {
  for(int i = 0; i <= PACK_NUM_LEDS - 7 - cyclotron_led_start; i++) {
    pack_leds[i] = CRGB(0,0,0);
  }
  
  //FastLED.show();
  
  i_powercell_led = 0;
}

void cyclotronControl() {
  // Only reset the starting led when the pack is first started up.
  if(b_reset_start_led == true) {
    b_reset_start_led = false;
    if(b_clockwise == false && i_mode_year == 2021) {
      i_led_cyclotron = cyclotron_led_start + 2; // Start on LED #2 in anti-clockwise mode in 2021 mode.
    }
    else {
      i_led_cyclotron = cyclotron_led_start;
    }
  }
        
  if(switch_alarm.getState() == HIGH && PACK_STATUS != MODE_OFF && b_2021_ramp_down_start != true && b_overheating == false) {
    if(b_alarm == false) {
      w_trig.trackStop(S_BEEP_8);

      b_2021_ramp_up = false;
      b_inner_ramp_up = false;
      b_alarm = true;
      packAlarm();
      ms_alarm.start(i_alarm_delay);

      // Tell the wand the pack alarm is on.
      Serial2.write(3);
    }
    
    // Ribbon cable has been removed.
    cyclotronNoCable();
  }
  else if(b_overheating == true) {
    if(b_alarm == false) {
      w_trig.trackStop(S_BEEP_8);

      b_2021_ramp_up = false;
      b_inner_ramp_up = false;
      b_alarm = true;
      packAlarm();
      ms_alarm.start(i_alarm_delay);
    }
    
    cyclotronOverHeating();
  }
  else {
    if(b_2021_ramp_up_start == true) {
      b_2021_ramp_up_start = false;

      r_2021_ramp.go(0); // Reset the ramp.
      r_inner_ramp.go(0); // Inner cyclotron ramp reset.
      
      if(i_mode_year == 1984) {
        r_2021_ramp.go(i_2021_ramp_delay - i_2021_delay, i_1984_ramp_length, CIRCULAR_OUT);
        r_inner_ramp.go(i_inner_ramp_delay - (i_2021_delay - i_1984_inner_delay), i_1984_ramp_length, CIRCULAR_OUT);
      }
      else {
        r_2021_ramp.go(i_2021_ramp_delay - i_2021_delay, i_2021_ramp_length, CIRCULAR_OUT);
        r_inner_ramp.go(i_inner_ramp_delay - (i_2021_delay - i_2021_inner_delay), i_2021_ramp_length, CIRCULAR_OUT);
      }
    }
    else if(b_2021_ramp_down_start == true) {
      b_2021_ramp_down_start = false;

      r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
      r_inner_ramp.go(i_inner_current_ramp_speed); // Reset the inner cyclotron ramp.

      if(i_mode_year == 1984) {
        r_2021_ramp.go(i_2021_ramp_delay, i_1984_ramp_down_length, CIRCULAR_IN); 
        r_inner_ramp.go(i_inner_ramp_delay, i_1984_ramp_down_length, CIRCULAR_IN); 
      }
      else {
        r_2021_ramp.go(i_2021_ramp_delay, i_2021_ramp_down_length, CIRCULAR_IN);
        r_inner_ramp.go(i_inner_ramp_delay, i_2021_ramp_down_length, CIRCULAR_IN);
      }
    }
  
    if(i_mode_year == 1984) {
      cyclotron1984(i_1984_delay);
      innerCyclotronRing(i_inner_delay - i_1984_inner_delay);
    }
    else {
      cyclotron2021(i_2021_delay);
      innerCyclotronRing(i_inner_delay - i_2021_inner_delay);
    }
  }
}

void cyclotron2021(int cDelay) {
  if(ms_cyclotron.justFinished()) {
    if(i_cyclotron_multiplier > 1) {
      cDelay = cDelay / i_cyclotron_multiplier + 5;
    }
    else {
      cDelay = cDelay / i_cyclotron_multiplier;
    }

    if(b_clockwise == true) {      
      pack_leds[i_led_cyclotron] = CRGB(255,0,0);

      if(i_led_cyclotron == cyclotron_led_start) {
        pack_leds[PACK_NUM_LEDS - 7 - 1] = CRGB(0,0,0);
      }
      else {
        pack_leds[i_led_cyclotron - 1] = CRGB(0,0,0);
      }

      //FastLED.show();

      i_led_cyclotron++;
        
      if(i_led_cyclotron > PACK_NUM_LEDS - 7 - 1) {
        i_led_cyclotron = cyclotron_led_start;
      }   
    }
    else {
      pack_leds[i_led_cyclotron] = CRGB(255,0,0);
      
      if(i_led_cyclotron + 1 > PACK_NUM_LEDS - 7 - 1) {
        pack_leds[cyclotron_led_start] = CRGB(0,0,0);
      }
      else {
        pack_leds[i_led_cyclotron + 1] = CRGB(0,0,0);
      }

      //FastLED.show();

      i_led_cyclotron--;
      
      if(i_led_cyclotron < cyclotron_led_start) {
        i_led_cyclotron = PACK_NUM_LEDS - 7 - 1;
      }
    }

    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;
        ms_cyclotron.start(cDelay);
        
        i_current_ramp_speed = cDelay;
      }
      else {     
        ms_cyclotron.start(i_2021_ramp_delay - r_2021_ramp.update());
        i_current_ramp_speed = i_2021_ramp_delay - r_2021_ramp.update();

        if(i_vibration_level > r_2021_ramp.update() - 85) {
          // Do nothing.
        }
        else {
          i_vibration_level = r_2021_ramp.update() - 85;
          
          if(i_vibration_level < 0) {
            i_vibration_level = 0;
          }
        }
        
        vibrationPack(i_vibration_level);
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());

        if(r_2021_ramp.update() < 100) {
          i_vibration_level = i_vibration_level - 2;
        }
        else {
          i_vibration_level = i_vibration_level - 3;
        }
        
        if(i_vibration_level < 20) {
          i_vibration_level = 0;
        }
        
        vibrationPack(i_vibration_level);
      }
    }
    else {
      ms_cyclotron.start(cDelay);

      if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
        vibrationPack(i_vibration_level);
      }
    }
    
    // Preparing to tell the inner cyclotron which led's to turn on.
    switch(i_led_cyclotron) {
      case 14:
        i_cyclotron_data = 1;
      break;
      
      case 17:
        i_cyclotron_data = 2;
      break;
  
      case 20:
        i_cyclotron_data = 3;
      break;
  
      case 23:
        i_cyclotron_data = 4;
      break;
    }    
  }
}

void cyclotron1984(int cDelay) {
  int i_pack_vibration = 100;
  
  if(ms_cyclotron.justFinished()) {
    cDelay = cDelay / i_cyclotron_multiplier;
    
    cyclotron84LightOff(i_led_cyclotron);

    if(b_clockwise == true) {
      i_led_cyclotron = i_led_cyclotron + 3;
    }
    else {
      i_led_cyclotron = i_led_cyclotron - 3;
    }

    if(i_led_cyclotron < cyclotron_led_start) {
      i_led_cyclotron = PACK_NUM_LEDS - 7 - 3;
    }
    else if(i_led_cyclotron > PACK_NUM_LEDS - 7 - 3) {
      i_led_cyclotron = cyclotron_led_start;
    }
    
    cyclotron84LightOn(i_led_cyclotron);

    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;
        ms_cyclotron.start(cDelay);
        
        i_current_ramp_speed = cDelay;
      }
      else {        
        ms_cyclotron.start(cDelay * 1.3 - r_2021_ramp.update());
        i_current_ramp_speed = cDelay * 1.3 - r_2021_ramp.update();
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
        i_pack_vibration = 0;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());
        
        i_pack_vibration = r_2021_ramp.update() / 13;
      }
    }
    else {
      ms_cyclotron.start(cDelay);
    } 

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_pack_vibration);
    }
  }
}

void cyclotron1984Alarm(int cDelay) {
  if(ms_cyclotron.justFinished()) {
    int led1 = cyclotron_led_start + 1;
    int led2 = led1 + 3;
    int led3 = led2 + 3;
    int led4 = led3 + 3;

    pack_leds[led1] = CRGB(255,0,0);
    pack_leds[led2] = CRGB(255,0,0);
    pack_leds[led3] = CRGB(255,0,0);
    pack_leds[led4] = CRGB(255,0,0);
    
    // Prepare to tell the inner cyclotron to turn all lights on.
    innerCyclotronShowAll();
  
    ms_cyclotron.start(cDelay);
  }
}

void cyclotron84LightOn(int cLed) {
  pack_leds[cLed+1] = CRGB(255,0,0);

  //FastLED.show();

  // Preparing to tell the inner cyclotron which led's to turn on.
  switch(cLed) {
    case 13:
      i_cyclotron_data = 1;
    break;
    
    case 16:
      i_cyclotron_data = 2;
    break;

    case 19:
      i_cyclotron_data = 3;
    break;

    case 22:
      i_cyclotron_data = 4;
    break;
  }
}

void cyclotron84LightOff(int cLed) {
  pack_leds[cLed+1] = CRGB(0,0,0);
  
  // Tell the inner cyclotron to turn off the leds.
  innerCyclotronOff();
}

void cyclotronOverHeating() {  
  vibrationPack(50);
  smokeControl(true);

  if(ms_overheating.justFinished()) {
    w_trig.trackPlayPoly(S_VENT_SMOKE, true);
    smokeControl(false);
  }

  switch (i_mode_year) {
    case 2021:
      cyclotron2021(i_2021_delay * 20);
      innerCyclotronRing(i_inner_delay - i_2021_inner_delay * 20);
    break;

    case 1984:
      if(ms_alarm.justFinished()) {
        resetCyclotronLeds();
        ms_alarm.start(i_alarm_delay);
      }
      else {
        cyclotron1984Alarm(i_1984_delay);
      }
    break;
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
      
void cyclotronNoCable() {  
  vibrationPack(50);

  switch (i_mode_year) {
    case 2021:
      cyclotron2021(i_2021_delay * 10);
      innerCyclotronRing(i_inner_delay - i_2021_inner_delay * 10);
    break;

    case 1984:
      if(ms_alarm.justFinished()) {
        resetCyclotronLeds();
        ms_alarm.start(i_alarm_delay);
      }
      else {
        cyclotron1984Alarm(i_1984_delay);
      }
    break;
  }
}

void resetCyclotronLeds() {
  for(int i = cyclotron_led_start; i < PACK_NUM_LEDS; i++) {
    pack_leds[i] = CRGB(0,0,0);
  }

  //FastLED.show();

  // Only reset the start led if the pack is off or just started.
  if(b_reset_start_led == true) {
    i_led_cyclotron = cyclotron_led_start;
  }

  // Tell the inner cyclotron to turn off the leds.
  innerCyclotronOff();
  
  cyclotronSpeedRevert();
}

void innerCyclotronOff() {
  for(int i = 0; i < CYCLOTRON_NUM_LEDS; i++) {
    cyclotron_leds[i] = CRGB(0,0,0);
  }

  //FastLED.show();
}

void innerCyclotronShowAll() {
  if(b_cyclotron_lid_on != true) {
    for(int i = 0; i < CYCLOTRON_NUM_LEDS; i++) {
      cyclotron_leds[i] = CRGB(255,0,0);
    }

    //FastLED.show();
  }
}

// For NeoPixel rings, ramp up and ramp down the LEDs in the ring and set the speed. (optional)
void innerCyclotronRing(int cDelay) {
  if(ms_cyclotron_ring.justFinished()) {
    // Colour control for the inner cyclotron leds. (red,green,blue)
    int r = 0;
    int g = 0;
    int b = 0;
  
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
  
    if(i_cyclotron_multiplier > 1) {
      switch(i_cyclotron_multiplier) {
        case 5:
          cDelay = cDelay - 2.5;
        break;
        
        case 4:
          cDelay = cDelay - 2;
        break;
        
        case 3:
          cDelay = cDelay - 1.5;
        break;
        
        case 2:
          cDelay = cDelay - 1;
        break;
          
        default:
          cDelay = cDelay - 1;
        break;
      }
    }
    else {
      cDelay = cDelay / i_cyclotron_multiplier;
    }
    
    if(b_clockwise == true) {
      if(b_cyclotron_lid_on != true) {
        if(CYCLOTRON_NUM_LEDS == 35) {
          cyclotron_leds[i_led_cyclotron_ring] = CRGB(r,g,b);
  
          if(i_led_cyclotron_ring == 0) {
            cyclotron_leds[CYCLOTRON_NUM_LEDS - 1] = CRGB(0,0,0);
          }
          else {
            cyclotron_leds[i_led_cyclotron_ring - 1] = CRGB(0,0,0);
          }
          
          //FastLED.show();
        }
      }

      i_led_cyclotron_ring++;
      
      if(i_led_cyclotron_ring > CYCLOTRON_NUM_LEDS - 1) {
        i_led_cyclotron_ring = 0;
      }
    }
    else {
      if(b_cyclotron_lid_on != true) {
        if(CYCLOTRON_NUM_LEDS == 35) {
          cyclotron_leds[i_led_cyclotron_ring] = CRGB(r,g,b);
          
          if(i_led_cyclotron_ring + 1 > CYCLOTRON_NUM_LEDS - 1) {
            cyclotron_leds[0] = CRGB(0,0,0);
          }
          else {
            cyclotron_leds[i_led_cyclotron_ring + 1] = CRGB(0,0,0);
          }

          //FastLED.show();
        }
      }
      
      i_led_cyclotron_ring--;

      if(i_led_cyclotron_ring < 0) {
        i_led_cyclotron_ring = CYCLOTRON_NUM_LEDS -1;
      }
    }

    if(b_inner_ramp_up == true) {      
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_up = false;
        ms_cyclotron_ring.start(cDelay);
        
        i_inner_current_ramp_speed = cDelay;
      }
      else {     
        ms_cyclotron_ring.start(i_inner_ramp_delay - r_inner_ramp.update());
        i_inner_current_ramp_speed = i_inner_ramp_delay - r_inner_ramp.update();
      }
    }
    else if(b_inner_ramp_down == true) {
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_down = false;
      }
      else {
        ms_cyclotron_ring.start(r_inner_ramp.update());
      }
    }
    else {
      ms_cyclotron_ring.start(cDelay);
    }  
  }
}

// For 8 NeoPixel Jewel options for a inner cyclotron. (optional)
void innerCyclotronShowLED(int i_led) {  
  // Colour control for the inner cyclotron leds. (red,green,blue)
  int r = 0;
  int g = 0;
  int b = 0;

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

  // middle led for each jewel.
  // Jewel 1: 6
  // Jewel 2: 13
  // Jewel 3: 20
  // Jewel 4: 27
  // Jewel 5: 34
  // Jewel 6: 41
  // Jewel 7: 48
  // Jewel 8: 55

  // Jewel 1 & 2: 0-13
  // Jewel 3 & 4: 14-27
  // Jewel 5 & 6: 28-41
  // Jewel 7 & 8: 42-55
  
  // TODO: Add odd blue electric colours mixed in.
  
  // If using 8 NeoPixel Jewels.
  switch(i_led) {
    case 0:
      for(int i = 0; i < CYCLOTRON_NUM_LEDS; i++) {
        cyclotron_leds[i] = CRGB(0,0,0);
      }
    break;

    case 1:
      for(int i = 0; i <= 13; i++) {
        cyclotron_leds[i] = CRGB(r,g,b);
      }
    break;

    case 2:
      for(int i = 14; i <= 27; i++) {
        cyclotron_leds[i] = CRGB(r,g,b);
      }
    break;

    case 3:
      for(int i = 28; i <= 41; i++) {
        cyclotron_leds[i] = CRGB(r,g,b);
      }
    break;

    case 4:
      for(int i = 42; i <= 55; i++) {
        cyclotron_leds[i] = CRGB(r,g,b);
      }
    break;
  }

  //FastLED.show();
}

// For 8 NeoPixel Jewel options for a inner cyclotron. (optional)
void innerCyclotronCommunication() {
  if(i_cyclotron_data != i_prev_cyclotron_data && CYCLOTRON_NUM_LEDS == 56) {
    if(b_cyclotron_lid_on != true) {
     switch(i_cyclotron_data) {       
        case 1:
          // LED 1
          innerCyclotronOff();
          innerCyclotronShowLED(1);
        break;
  
        case 2:
          // LED 2
          innerCyclotronOff();
          innerCyclotronShowLED(2);
        break;
  
        case 3:
          // LED 3
          innerCyclotronOff();
          innerCyclotronShowLED(3);
        break;
  
        case 4:
          // LED 4
          innerCyclotronOff();
          innerCyclotronShowLED(4);
        break;
        
        case 6:
          innerCyclotronOff();
          innerCyclotronShowLED(1);
        break;
  
        case 7:
          innerCyclotronOff();
          innerCyclotronShowLED(2);
        break;
  
        case 8:
          innerCyclotronOff();
          innerCyclotronShowLED(3);
        break;
  
        case 9:
          innerCyclotronOff();
          innerCyclotronShowLED(4);
        break;
      }
    }
    
    i_prev_cyclotron_data = i_cyclotron_data;
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

void ventLight(boolean b_on) {
  if(b_on == true) {
    int r = random(200,255);
    int g = random(200,255);
    int b = random(200,255);

    // If doing firing smoke effects, lets change the light colours.
    if(b_wand_firing == true) {
      switch(FIRING_MODE) {
        case PROTON:
          r = 0;
          g = 55;
          b = 55;
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
    for(int i = VENT_LIGHT_START; i < PACK_NUM_LEDS; i++) {
      pack_leds[i] = CRGB(r,g,b);
    }

    //FastLED.show();
  }
  else {
    for(int i = VENT_LIGHT_START; i < PACK_NUM_LEDS; i++) {
      pack_leds[i] = CRGB(0,0,0);
    }

    //FastLED.show();
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
  ms_smoke_timer.start(i_smoke_timer / i_wand_power_level);
          
  vibrationPack(255);

  w_trig.trackGain(S_FIRE_START_SPARK, 0);
  w_trig.trackPlayPoly(S_FIRE_START_SPARK);

  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackGain(S_FIRE_START, 0);
      w_trig.trackPlayPoly(S_FIRE_START, true);
    
      w_trig.trackGain(S_FIRE_LOOP_GUN, 0);
      w_trig.trackPlayPoly(S_FIRE_LOOP_GUN, true);
      w_trig.trackFade(S_FIRE_LOOP_GUN, 0, 1000, 0);
      w_trig.trackLoop(S_FIRE_LOOP_GUN, 1);
    
      w_trig.trackGain(S_FIRE_LOOP, 0);
      w_trig.trackPlayPoly(S_FIRE_LOOP, true);
      w_trig.trackFade(S_FIRE_LOOP, 0, 1000, 0);
      w_trig.trackLoop(S_FIRE_LOOP, 1); 
    break;

    case SLIME:
      w_trig.trackGain(S_SLIME_START, 0);
      w_trig.trackPlayPoly(S_SLIME_START);
      
      w_trig.trackGain(S_SLIME_LOOP, 0);
      w_trig.trackPlayPoly(S_SLIME_LOOP);
      w_trig.trackFade(S_SLIME_LOOP, 0, 1500, 0);
      w_trig.trackLoop(S_SLIME_LOOP, 1);
    break;

    case STASIS:
      w_trig.trackGain(S_STASIS_START, 0);
      w_trig.trackPlayPoly(S_STASIS_START);
      
      w_trig.trackGain(S_STASIS_LOOP, 0);
      w_trig.trackPlayPoly(S_STASIS_LOOP);
      w_trig.trackFade(S_STASIS_LOOP, 0, 1000, 0);
      w_trig.trackLoop(S_STASIS_LOOP, 1);
    break;

    case MESON:
      w_trig.trackGain(S_MESON_START, 0);
      w_trig.trackPlayPoly(S_MESON_START);
      
      w_trig.trackGain(S_MESON_LOOP, 0);
      w_trig.trackPlayPoly(S_MESON_LOOP);
      w_trig.trackFade(S_MESON_LOOP, 0, 5500, 0);
      w_trig.trackLoop(S_MESON_LOOP, 1);
    break;

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
          // Long tail end.
          w_trig.trackPlayPoly(S_FIRING_END, true);
        }
        else if(ms_firing_length_timer.remaining() < 10000) {
          // Mid tail end.
          w_trig.trackPlayPoly(S_FIRING_END_MID, true);
        }
        else {
          // Short tail end.
          w_trig.trackPlayPoly(S_FIRING_END_GUN, true);
        }
      break;
  
      case SLIME:
        w_trig.trackGain(S_SLIME_END, 0);
        w_trig.trackPlayPoly(S_SLIME_END, true);
      break;
  
      case STASIS:
        w_trig.trackGain(S_STASIS_END, 0);
        w_trig.trackPlayPoly(S_STASIS_END, true);
        
      break;
  
      case MESON:
        w_trig.trackGain(S_MESON_END, 0);
        w_trig.trackPlayPoly(S_MESON_END, true);
      break;

      case SETTINGS:
        // Nothing
      break;
    }
  }
  
  b_wand_firing = false;

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
}

void wandStopFiringSounds() {
  // Firing sounds.
  w_trig.trackStop(S_FIRE_START);
  w_trig.trackStop(S_FIRE_START_SPARK);
  w_trig.trackStop(S_FIRE_LOOP);
  w_trig.trackStop(S_FIRE_LOOP_GUN);
  w_trig.trackStop(S_FIRE_LOOP_IMPACT);
  w_trig.trackStop(S_SLIME_START);
  w_trig.trackStop(S_SLIME_LOOP);
  w_trig.trackStop(S_SLIME_END);
  w_trig.trackStop(S_STASIS_START);
  w_trig.trackStop(S_STASIS_LOOP);
  w_trig.trackStop(S_STASIS_END);
  w_trig.trackStop(S_MESON_START);
  w_trig.trackStop(S_MESON_LOOP);
  w_trig.trackStop(S_MESON_END);
}

void packAlarm() {
  wandStopFiringSounds();

  // Pack sounds.
  w_trig.trackStop(S_AFTERLIFE_PACK_STARTUP);
  w_trig.trackStop(S_AFTERLIFE_PACK_IDLE_LOOP);
  w_trig.trackStop(S_IDLE_LOOP);
  w_trig.trackStop(S_BOOTUP);

  w_trig.trackPlayPoly(S_SHUTDOWN, true);
  w_trig.trackPlayPoly(S_PACK_SHUTDOWN, true);

  if(b_overheating != true) {
    w_trig.trackPlayPoly(S_PACK_BEEPING, true);
    w_trig.trackLoop(S_PACK_BEEPING, 1);
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

    w_trig.trackGain(S_MODE_SWITCH, 0);
    w_trig.trackPlayPoly(S_MODE_SWITCH);
    
    w_trig.trackGain(S_VENT_SMOKE, 0);
    w_trig.trackPlayPoly(S_VENT_SMOKE);

    // Play some spark sounds if the pack is running and the lid is removed.
    if(PACK_STATUS == MODE_ON) {
      w_trig.trackGain(S_SPARKS_LOOP, 0);
      w_trig.trackPlayPoly(S_SPARKS_LOOP);
    }
  }

  if(switch_cyclotron_lid.isPressed()) {
    // Play sounds when lid is mounted.
    w_trig.trackStop(S_CLICK);    
    
    w_trig.trackGain(S_CLICK, 0);
    w_trig.trackPlayPoly(S_CLICK);

    // Play some spark sounds if the pack is running and the lid is put back on                          .
    if(PACK_STATUS == MODE_ON) {
      w_trig.trackGain(S_SPARKS_LOOP, 0);
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
    if(i_mode_year == 1984) {        
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
  if(b_vibration == true) {
    if(i_level != i_vibration_level_prev) {
      i_vibration_level_prev = i_level;
      analogWrite(vibration, i_level);
    }
  }
  else {
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
      i_cyclotron_multiplier = i_cyclotron_multiplier + 4;
    break;
  }

  i_cyclotron_switch_led_mulitplier++;
}

void increaseVolume() {
  if(i_volume + 1 > 0) {
    i_volume = 0;
  }
  else {
    i_volume = i_volume + 1;
  }
  
  w_trig.masterGain(i_volume);
}

void decreaseVolume() {
  if(i_volume - 1 < -70) {
    i_volume = -70;
  }
  else {
    i_volume = i_volume - 1;
  }
  
  w_trig.masterGain(i_volume);         
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
    increaseVolume();
      
    // Tell wand to increase volume.
    Serial2.write(9);
  }
  
  if(i_val_rotary < i_last_val_rotary) {
    decreaseVolume();
      
    // Tell wand to decrease the volume.
    Serial2.write(10);
  }
  
  i_last_val_rotary = i_val_rotary;
}

void smokeControl(boolean b_smoke_on) {  
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      digitalWrite(smoke_pin, HIGH);
      smokeBooster(b_smoke_on);
    }
    else {
      digitalWrite(smoke_pin, LOW);
      smokeBooster(false);
    }

    if(b_wand_firing == true) {
      smokeBooster(b_smoke_on);
    }
  }
}

/* 
 *  Alternate 5V high pin. This one turns on while firing occasionally for smoke. I put this one in my booster tube.
 */
void smokeBooster(boolean b_smoke_on) {
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      digitalWrite(smoke_booster_pin, HIGH);
    }
    else {
      digitalWrite(smoke_booster_pin, LOW);
    }
  }
}

void fanControl(boolean b_fan_on) {
  if(b_fan_on == true) {
    digitalWrite(fan_pin, HIGH);
  }
  else {
    digitalWrite(fan_pin, LOW);
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
  else if(ms_fan_stop_timer.isRunning() && ms_fan_stop_timer.remaining() < 5000) {
    fanControl(true);
  }
}

/*
 * Check if the wand is still connected.
 */
void wandHandShake() {  
  if(b_wand_connected == true) {    
    if(ms_wand_handshake.justFinished()) {         
      if(PACK_STATUS == MODE_ON) {
        // Shut the pack down as the wand was disconnected.
        packShutdown();
      }
      
      ms_wand_handshake.start(i_wand_handshake_delay);
      
      b_wand_connected = false;
    }
    else if(ms_wand_handshake_checking.justFinished()) {  
      if(b_diagnostic == true) {
        // Play a beep sound to know if the wand is connected, while in diagnostic mode.
        w_trig.trackGain(S_VENT_BEEP, i_volume);
        w_trig.trackPlayPoly(S_VENT_BEEP);
      }
      
      ms_wand_handshake_checking.stop();
      
      // Ask the wand if it is still connected.
      Serial2.write(11);
    }
  }
  else {
    if(ms_wand_handshake.justFinished()) {        
      // Ask the wand if it connected.
      Serial2.write(11);

      ms_wand_handshake.start(i_wand_handshake_delay);
    }
  }
}

/*
 * Incoming messages from the wand.
 */
void checkWand() {
  if(Serial2.available() > 0) {
    prev_byte = rx_byte;  
    rx_byte = Serial2.read();
    
    if(b_wand_connected == true) {
      switch(rx_byte) {
        case 1:
          // The wand has been turned on.
          // Turn the pack on.
          if(PACK_STATUS != MODE_ON) {
            PACK_ACTION_STATUS = ACTION_ACTIVATE;
          }
        break;
    
        case 2:
          // The wand has been turned off.
          // Turn the pack off.
          if(PACK_STATUS != MODE_OFF) {
            PACK_ACTION_STATUS = ACTION_OFF;
          }
        break;
    
        case 3:
          // Wand is firing.
          wandFiring();
        break;
    
        case 4:
          // Wand just stopped firing.
          wandStoppedFiring();
          cyclotronSpeedRevert();
        break;
    
        case 5:
          // Proton mode
          FIRING_MODE = PROTON;
          w_trig.trackGain(S_CLICK, i_volume);
          w_trig.trackPlayPoly(S_CLICK);
          w_trig.trackGain(S_FIRE_START_SPARK, i_volume);
          w_trig.trackPlayPoly(S_FIRE_START_SPARK);
        break;
    
        case 6:
          // Slime mode
          FIRING_MODE = SLIME;
          w_trig.trackGain(S_CLICK, i_volume);
          w_trig.trackPlayPoly(S_CLICK);
          w_trig.trackGain(S_PACK_SLIME_OPEN, i_volume);
          w_trig.trackPlayPoly(S_PACK_SLIME_OPEN);
        break;
    
        case 7:
          // Stasis mode
          FIRING_MODE = STASIS;
          w_trig.trackGain(S_CLICK, i_volume);
          w_trig.trackPlayPoly(S_CLICK);
          w_trig.trackGain(S_STASIS_OPEN, i_volume);
          w_trig.trackPlayPoly(S_STASIS_OPEN);
        break;
    
        case 8:
          // Meson mode
          FIRING_MODE = MESON;
          w_trig.trackGain(S_CLICK, i_volume);
          w_trig.trackPlayPoly(S_CLICK);
          w_trig.trackGain(S_MESON_OPEN, i_volume);
          w_trig.trackPlayPoly(S_MESON_OPEN);
        break;
    
        case 9:
          // Settings mode
          FIRING_MODE = SETTINGS;
          w_trig.trackGain(S_CLICK, i_volume);
          w_trig.trackPlayPoly(S_CLICK);
        break;
    
        case 10:
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
    
          // Start the vent light.
          ventLight(true);
        break;
    
        case 11:
          // Overheating finished
          w_trig.trackGain(S_VENT_DRY, i_volume);
          b_overheating = false;
          
          // Turn off the smoke.
          smokeControl(false);
          
          b_alarm = false;
    
          resetCyclotronLeds();
          reset2021RampUp();
    
          packStartup();
    
          // Turn off the vent light
          ventLight(false);
          ms_vent_light_off.stop();
          ms_vent_light_on.stop();
          
          ms_cyclotron.start(i_2021_delay); 
        break;
    
        case 12:
          // Reset cyclotron speed.
          cyclotronSpeedRevert();
        break;
    
        case 13:
          // Speed up cyclotron.
          cyclotronSpeedIncrease();  
        break;

        case 14:
          // The wand is still here.
          ms_wand_handshake.start(i_wand_handshake_delay);
          ms_wand_handshake_checking.start(i_wand_handshake_delay / 2);
          b_wand_connected = true;
        break;
        
        case 15:
          // Play 8 overheat beeps before we overheat.
          w_trig.trackGain(S_BEEP_8, i_volume);
          w_trig.trackPlayPoly(S_BEEP_8);
        break;
    
        case 16:
          // Wand power level 1
          i_wand_power_level = 1;
        break;
    
        case 17:
          // Wand power level 2
          i_wand_power_level = 2;
        break;
    
        case 18:
          // Wand power level 3
          i_wand_power_level = 3;
        break;
    
        case 19:
          // Wand power level 4
          i_wand_power_level = 4;
        break;
    
        case 20:
          // Wand power level 5
          i_wand_power_level = 5;
        break;        
        
        case 93:
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
        
        case 94:
          // Lower overall pack volume.
          decreaseVolume();
        break;
    
        case 95:
          // Increase overall pack volume.
          increaseVolume();
        break;
          
        case 96:
          // Lower music volume.
          if(b_playing_music == true) {
            if(i_volume_music - 1 < -70) {
              i_volume_music = -70;
            }
            else {
              i_volume_music = i_volume_music - 1;
            }
    
            w_trig.trackGain(i_current_music_track, i_volume_music);
          }
        break;
    
        case 97:
          // Increase music volume.
         if(b_playing_music == true) {
            if(i_volume_music + 1 > 0) {
              i_volume_music = 0;
            }
            else {
              i_volume_music = i_volume_music + 1;
            }
    
            w_trig.trackGain(i_current_music_track, i_volume_music);
          } 
        break;
        
        case 98:
          // Stop music.
          b_playing_music = false;
          stopMusic();
        break;
    
        case 99:
          // Play music.
          b_playing_music = true;
          playMusic();
        break;
        
        default:
          // Music track number to be played.
          if(rx_byte > 99) {
            if(b_playing_music == true) {
              stopMusic();
              i_current_music_track = rx_byte;
              playMusic();
            }
            else {
              i_current_music_track = rx_byte;
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
      if(rx_byte == 14 && prev_byte == rx_byte) {        
        // Tell the wand that the pack is here.
        Serial2.write(11);

        if(i_mode_year == 1984) {
          Serial2.write(7);
        }
        else {
          Serial2.write(8);
        }

        // Stop any music.
        Serial2.write(99);
        b_playing_music = false;
        stopMusic();

        Serial2.write(i_current_music_track);

        if(b_repeat_track == true) {
          Serial2.write(12);
        }
        else {
          Serial2.write(13);
        }
        
        // Vibration on
        if(b_vibration == true) {
          Serial2.write(5);
        }
        else {
          Serial2.write(6);
        }

        // Ribbon cable alarm.
        if(b_alarm == true) {
          Serial2.write(3);
        }
        else {
          Serial2.write(4);
        }

        // Pack status
        if(PACK_STATUS != MODE_OFF) {
          Serial2.write(1);
        }
        else {
          Serial2.write(2);
        }

        // Reset volumes.
        Serial2.write(14);
        i_volume = STARTUP_VOLUME;
        i_volume_music = STARTUP_VOLUME;
        w_trig.masterGain(i_volume);
        
        b_wand_connected = true;
      }
    }
  }
}

void setupWavTrigger() {
  // If the controller is powering the WAV Trigger, we should wait for the WAV trigger to finish reset before trying to send commands.
  delay(1000);
  
  // WAV Trigger's startup at 57600
  w_trig.start();
  
  delay(10);

  w_trig.stopAllTracks();
  w_trig.samplerateOffset(0); // Reset our sample rate offset        
  w_trig.masterGain(i_volume); // Reset the master gain db. Range is -70 to +4.
  w_trig.setAmpPwr(false); // Turn off the onboard amp to draw less power if you decide to use the aux cable jack instead. If you use the output pins, you will need to turn this back on.
  
  // Enable track reporting from the WAV Trigger
  w_trig.setReporting(true);

  // Allow time for the WAV Triggers to respond with the version string and number of tracks.
  delay(350);
  
  char w_trig_version[VERSION_STRING_LEN]; // Firmware version.
  int w_num_tracks = w_trig.getNumTracks();
  w_trig.getVersion(w_trig_version, VERSION_STRING_LEN);

  // Build the music track count.
  i_music_count = w_num_tracks - S_CROSS_STREAMS_START;

  if(i_music_count > 0) {
    i_current_music_track = i_music_track_start; // Set the first track of music as file 100_
  }
  
  /*
  Serial.print(w_trig_version);
  Serial.print("\n");
  Serial.print("Number of tracks = ");
  Serial.print(w_num_tracks);
  Serial.print("\n");
  Serial.println(i_music_count);
  */
}
