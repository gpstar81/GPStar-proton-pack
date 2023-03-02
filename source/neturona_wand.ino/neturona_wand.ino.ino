/********************************************************
  Hasbro Neutrona Wand.
  January 2022.
  Michael Rajotte / gpstar

  NOTE: Do not forget to unplug the TX1/RX1 cables from Serial1 while you are uploading code to your Nano.
********************************************************/
/*
 * You need to edit wavTrigger.h and make sure you comment out the proper serial port. (Near the top of the wavTrigger.h file).
 * We are going to use Pins 8 and 9 on the Nano. __WT_USE_ALTSOFTSERIAL__
 */
#include <wavTrigger.h>
/* 
 *  AltSoftSerial uses: Pin 9 = TX & Pin 8 = RX. 
 *  So Pin 9 goes to the RX of the WavTrigger and Pin 8 goes to the TX of the WavTrigger. 
 */
#include <AltSoftSerial.h>
#include <millisDelay.h> 
#include <FastLED.h>
#include <ezButton.h>

/* 
 *  SD Card sound files in order. If you have no sound, your SD card might be too slow. 
 *  Try a faster one. File naming 000_ is important as well. For music, it is 100_ and higher.
 *  Also note if you add more sounds to this list, you need to update the wavtrigger setup function to let it know the last
 *  sound effect file. The wav trigger uses this to determine how many music tracks there are if any.
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
 * You can set the default startup volume for your wand here.
 * NOTE: Make sure to set this to the same value in the Proton Pack code.
 * If not then the startup volume will levels will not be in sync.
 * 4 = loudest
 * -70 = quietest
 */
const int STARTUP_VOLUME = 0;

/*
 * Set this to true to be able to use your wand without a Proton Pack connected.
 * Otherwise the wand will wait until it is connected to a Proton Pack before it can activate.
 */
boolean b_no_pack = false;

/*
 * -------------****** DO NOT CHANGE ANYTHING BELOW THIS LINE ******-------------
 */
 
/* 
 * Wand state. 
 */
enum WAND_STATE { MODE_OFF, MODE_ON };
enum WAND_STATE WAND_STATUS;

/*
 * Various wand action states.
 */
enum WAND_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE, ACTION_FIRING, ACTION_OVERHEATING, ACTION_SETTINGS };
enum WAND_ACTION_STATE WAND_ACTION_STATUS;

/* 
 *  Barrel LEDs. There are 5 LEDs. 0 = Base, 4 = tip. These are addressable with a single pin and are RGB.
 */
#define BARREL_LED_PIN 10
#define BARREL_NUM_LEDS 5
CRGB barrel_leds[BARREL_NUM_LEDS];

/*
 * Delay for fastled to update the addressable LEDs. 
 * We have up to 5 addressable LEDs in the wand barrel.
 * 0.03 ms to update 1 LED. So 0.15 ms should be ok. Lets bump it up to 3 just in case.
 */
//const int i_fast_led_delay = 3;
//millisDelay ms_fast_led;

/* 
 *  Wav trigger
 */
wavTrigger w_trig;
uint8_t i_music_count = 0;
uint8_t i_current_music_track = 0;
const uint8_t i_music_track_start = 100; // Music tracks start on file named 100_ and higher.

/* 
 *  Volume (4 = loudest, -70 = quietest)
 */
int i_volume = STARTUP_VOLUME;
int i_volume_music = STARTUP_VOLUME;

/* 
 * Rotary encoder on the top of the wand. Changes the wand power level and controls the wand settings menu.
 * Also controls independent music volume while the pack/wand is off and if music is playing.
 */
#define r_encoderA 6
#define r_encoderB 7
static uint8_t prev_next_code = 0;
static uint16_t store=0;

/* 
 *  Vibration
 */
const uint8_t vibration = 11;
int i_vibration_level = 55;
int i_vibration_level_prev = 0;
boolean b_vibration_on = false;

/* 
 *  Various Switches on the wand.
 */
ezButton switch_wand(A0); // Contols the beeping. Top right switch on the wand.
ezButton switch_intensify(2);
ezButton switch_activate(3);
ezButton switch_vent(4); // Turns onhte vent light.
const int switch_mode = A6; // Changes firing modes or to reach the settings menu.
const int switch_barrel = A7; // Barrel extension/open switch.

/*
 * Some switch settings.
 */
const int i_switch_barrel_value = 100;
const uint8_t switch_debounce_time = 50;
const uint8_t a_switch_debounce_time = 250;
millisDelay ms_switch_mode_debounce;

/*
 * Wand lights
 */
const uint8_t led_slo_blo = 5; // There are 2 LED's attached to this pin. The slo-blo and the light on the front of the wand body. You can drive up to 2 led's from 1 pin on a arduino.
const uint8_t led_white = 12; // Blinking white light beside the vent on top of the wand.
const uint8_t led_vent = 13; // Vent light
const uint8_t led_bargraph_1 = A1;
const uint8_t led_bargraph_2 = A2;
const uint8_t led_bargraph_3 = A3;
const uint8_t led_bargraph_4 = A4;
const uint8_t led_bargraph_5 = A5;

/* 
 *  Idling timers
 */
millisDelay ms_gun_loop_1;
millisDelay ms_gun_loop_2;
millisDelay ms_white_light;
const int d_white_light_interval = 150;

/* 
 *  Overheat timers.
 */
millisDelay ms_overheat_initate;
const int i_ms_overheat_initate = 12000;
millisDelay ms_overheating;
const int i_ms_overheating = 6500;

/* 
 *  Bargraph timers
 */
millisDelay ms_bargraph;
millisDelay ms_bargraph_firing;
const int d_bargraph_ramp_interval = 120;
int i_bargraph_status = 0;

/* 
 *  A timer for controlling the wand beep. in 2021 mode.
 */
millisDelay ms_reset_sound_beep;
const int i_sound_timer = 50;

/* 
 *  Wand tip heatup timers (when changing firing modes).
 */
millisDelay ms_wand_heatup_fade;
const int i_delay_heatup = 10;
int i_heatup_counter = 0;
int i_heatdown_counter = 100;

/* 
 *  Firing timers.
 */
millisDelay ms_firing_lights;
millisDelay ms_firing_lights_end;
millisDelay ms_firing_stream_blue;
millisDelay ms_firing_stream_orange;
millisDelay ms_impact; // Mix some impact sounds while firing.
const int d_firing_lights = 20; // 20 milliseconds. Timer for adjusting the firing stream colours.
const int d_firing_stream = 100; // 100 milliseconds. Used by the firing timers to adjust stream colours.
int i_barrel_light = 0; // using this to keep track which LED in the barrel is currently lighting up.

/* 
 *  Wand power mode. Controlled by the rotary encoder on the top of the wand.
 *   1,2,3,4 = Allow's continous firing of the wand. The pack will occasionally activate it's smoke triggers while continiously firing for longer periods.
 *   At higher power settings, the pack will trigger the smoke pins earlier compared to lower power settings.
 *   5 = Highest power output. The wand/pack will overheat on this setting if you are firing for too long.
 */
const int i_power_mode_max = 5;
const int i_power_mode_min = 1;
int i_power_mode = 1;

/* 
 *  Wand / Pack communication
 */
int rx_byte = 0;

/*
 * Some pack flags which get transmitted to the wand depending on the pack status.
 */
boolean b_pack_on = false;
boolean b_pack_alarm = false;
boolean b_wait_for_pack = true;
int i_cyclotron_speed_up = 1; // For telling the pack to speed up or slow down the cyclotron lights.

/* 
 *  Wand menu & music
 */
int i_wand_menu = 5;
millisDelay ms_settings_blinking;
const int i_settings_blinking_delay = 350;
boolean b_playing_music = false;
boolean b_repeat_track = false;
millisDelay ms_check_music;
const int i_music_check_delay = 2000;
millisDelay ms_music_next_track;
const int i_music_next_track_delay = 2000;

/* 
 *  Wand firing modes
 *  Proton, Slime, Stasis, Meson, Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SETTINGS };
enum FIRING_MODES FIRING_MODE;

/*
 * Misc wand settings and flags.
 */
int year_mode = 2021;
boolean b_firing = false;
boolean b_sound_idle = false;
boolean b_beeping = false;

void setup() {
  Serial.begin(9600);

  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR2B = TCCR2B & B11111000 | B00000110; // for PWM frequency of 122.55 Hz
  
  setupWavTrigger();
  
  // Barrel LEDs
  FastLED.addLeds<NEOPIXEL, BARREL_LED_PIN>(barrel_leds, BARREL_NUM_LEDS);

  switch_wand.setDebounceTime(switch_debounce_time);
  switch_intensify.setDebounceTime(switch_debounce_time);
  switch_activate.setDebounceTime(switch_debounce_time);
  switch_vent.setDebounceTime(switch_debounce_time);

  // Rotary encoder on the top of the wand.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);
  
  pinMode(led_bargraph_1, OUTPUT);
  pinMode(led_bargraph_2, OUTPUT);
  pinMode(led_bargraph_3, OUTPUT);
  pinMode(led_bargraph_4, OUTPUT);
  pinMode(led_bargraph_5, OUTPUT);
  
  pinMode(led_slo_blo, OUTPUT);
  pinMode(led_vent, OUTPUT);
  pinMode(led_white, OUTPUT);

  pinMode(vibration, OUTPUT);
  
  // Make sure lights are off.
  wandLightsOff();
  
  // Wand status.
  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  ms_reset_sound_beep.start(i_sound_timer);

  // Setup the mode switch debounce.
  ms_switch_mode_debounce.start(1);
  i_wand_menu = 5;

  // We bootup the wand in the classic proton mode.
  FIRING_MODE = PROTON;

  // Check music timer.
  ms_check_music.start(i_music_check_delay);

  if(b_no_pack == true) {
    b_wait_for_pack = false);
    b_pack_on = true;
  }
}

void loop() { 
  if(b_wait_for_pack == true) {
    // Handshake with the pack. Telling the pack that we are here.
    Serial.write(14);

    checkPack();
    
    delay(200);
  }
  else {   
    mainLoop();
  } 
}

void mainLoop() {
  w_trig.update();
  
  checkMusic();
  checkPack();
  switchLoops();
  checkRotary();  
  checkSwitches();
  
  switch(WAND_ACTION_STATUS) {
    case ACTION_IDLE:
      // Do nothing.
    break;

    case ACTION_OFF:
      wandOff();
    break;
    
    case ACTION_FIRING:    
      if(b_pack_on == true && b_pack_alarm == false) {
        if(b_firing == false) {
          b_firing = true;
          modeFireStart();
        }
       
        if(ms_overheat_initate.justFinished() && i_power_mode == i_power_mode_max) {
          ms_overheat_initate.stop();
          modeFireStop();
          
          delay(100);
          
          WAND_ACTION_STATUS = ACTION_OVERHEATING;

          // Play overheating sounds.
          ms_overheating.start(i_ms_overheating);
          ms_settings_blinking.start(i_settings_blinking_delay);
          
          w_trig.trackGain(S_VENT_DRY, i_volume);
          w_trig.trackPlayPoly(S_VENT_DRY);

          w_trig.trackGain(S_CLICK, i_volume);
          w_trig.trackPlayPoly(S_CLICK);

          // Tell the pack we are overheating.
          Serial.write(10);
        }
        else {
          modeFiring();
          
          // Stop firing if any of the main switches are turned off or the barrel is retracted.
          if(switch_vent.getState() == HIGH || switch_wand.getState() == HIGH || analogRead(switch_barrel) > i_switch_barrel_value) {
            modeFireStop();
          }
        }
      }
      else if(b_pack_alarm == true && b_firing == true) {
        modeFireStop();
      }
    break;
    
    case ACTION_OVERHEATING:
      settingsBlinkingLights();
      
      if(ms_overheating.justFinished()) {
        ms_overheating.stop();
        ms_settings_blinking.stop();
        
        WAND_ACTION_STATUS = ACTION_IDLE;

        //w_trig.trackStop(S_VENT_BEEP);
        w_trig.trackStop(S_CLICK);
        w_trig.trackStop(S_VENT_DRY);
        
        // Tell the pack that we finished overheating.
        Serial.write(11);
      }
    break;
    
    case ACTION_ACTIVATE:
      modeActivate();
    break;

    case ACTION_SETTINGS:
      settingsBlinkingLights();
      
      switch(i_wand_menu) {
        case 5:
        // Track loop setting.
        if(switch_intensify.isPressed()) {
          if(b_repeat_track == false) {
            // Loop the track.
            b_repeat_track = true;
            w_trig.trackLoop(i_current_music_track, 1);
          }
          else {
            b_repeat_track = false;
            w_trig.trackLoop(i_current_music_track, 0);
          }
          
          // Tell pack to loop the music track.
          Serial.write(93);
        }
        break;

        // Pack / Wand overall volume.
        case 4:
          if(switch_intensify.isPressed()) {
            increaseVolume();
            
            // Tell pack to increase overall volume.
            Serial.write(95);
          }

          if(analogRead(switch_mode) > 1020 && ms_switch_mode_debounce.justFinished()) {
            decreaseVolume();
            
            // Tell pack to lower overall volume.
            Serial.write(94);
            
            ms_switch_mode_debounce.start(a_switch_debounce_time);
          }
        
        break;

        // Adjust music volume.
        case 3:
          if(b_playing_music == true) {
            if(switch_intensify.isPressed()) {
              if(i_volume_music + 1 > 0) {
                i_volume_music = 0;
              }
              else {
                i_volume_music = i_volume_music + 1;
              }

              w_trig.trackGain(i_current_music_track, i_volume_music);
              
              // Tell pack to increase music volume.
              Serial.write(97);
            }
  
            if(analogRead(switch_mode) > 1020 && ms_switch_mode_debounce.justFinished()) {
              if(i_volume_music - 1 < -70) {
                i_volume_music = -70;
              }
              else {
                i_volume_music = i_volume_music - 1;
              }

              w_trig.trackGain(i_current_music_track, i_volume_music);
              
              // Tell pack to lower music volume.
              Serial.write(96);
              
              ms_switch_mode_debounce.start(a_switch_debounce_time);
            }  
          }
        break;

        // Change music tracks.
        case 2:          
          if(switch_intensify.isPressed()) {
            if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
              if(b_playing_music == true) {               
                // Go to the first track to play it.
                stopMusic();
                i_current_music_track = i_music_track_start;
                playMusic();
              }
              else {
                i_current_music_track = i_music_track_start;
              }
            }
            else {
              // Stop the old track and play the new track if music is currently playing.
              if(b_playing_music == true) {
                stopMusic();
                i_current_music_track++;                
                playMusic();
              }
              else {
                i_current_music_track++;
              }
            }
          
            // Tell the pack which music track to change to.
            Serial.write(i_current_music_track);
          }

          if(analogRead(switch_mode) > 1020 && ms_switch_mode_debounce.justFinished()) {
            if(i_current_music_track - 1 < i_music_track_start) {
              if(b_playing_music == true) {
                // Go to the last track to play it.
                stopMusic();
                i_current_music_track = i_music_track_start + (i_music_count -1);
                playMusic();
              }
              else {
                i_current_music_track = i_music_track_start + (i_music_count -1);
              }
            }
            else {
              // Stop the old track and play the new track if music is currently playing.
              if(b_playing_music == true) {
                stopMusic();
                i_current_music_track--;
                playMusic();
              }
              else {
                i_current_music_track--;
              }
            }
          
            // Tell the pack which music track to change to.
            Serial.write(i_current_music_track);
            
            ms_switch_mode_debounce.start(a_switch_debounce_time);
          }          
        break;

        // Play music or stop music.
        case 1:          
          if(switch_intensify.isPressed()) {
            if(b_playing_music == true) {
              // Stop music
              b_playing_music = false;

              // Tell the pack to stop music.
              Serial.write(98);
              
              stopMusic();             
            }
            else {
              if(i_music_count > 0 && i_current_music_track > 99) {
                // Start music.
                b_playing_music = true;

                // Tell the pack to play music.
                Serial.write(99);
                
                playMusic();
              }
            }
          }
        break;
      }
    break;
  }
  
  switch(WAND_STATUS) {
    case MODE_OFF:

    break;

    case MODE_ON:
      if(b_vibration_on == true && WAND_ACTION_STATUS != ACTION_SETTINGS) {
        vibrationSetting();
      }
  
      // Top white light.
      if(ms_white_light.justFinished()) {
        ms_white_light.start(d_white_light_interval);
        if(digitalRead(led_white) == LOW) {
          digitalWrite(led_white, HIGH);
        }
        else {
          digitalWrite(led_white, LOW);
        }
      }

      // Ramp the bargraph up and set it to led 1 for default power level setting on a fresh start.
      if(ms_bargraph.justFinished()) {
        bargraphRampUp();
      }
      else if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        bargraphPowerCheck();
      }

      if(year_mode == 2021) {
        if(ms_gun_loop_1.justFinished()) {
          w_trig.trackGain(S_AFTERLIFE_GUN_LOOP_1, -20);
          w_trig.trackPlayPoly(S_AFTERLIFE_GUN_LOOP_1);
          w_trig.trackFade(S_AFTERLIFE_GUN_LOOP_1, -20, 200, 0);
          w_trig.trackLoop(S_AFTERLIFE_GUN_LOOP_1, 1);
          ms_gun_loop_1.stop();
        }
      }

      wandBarrelHeatUp();
  
    break;
  } 

  if(b_firing == true && WAND_ACTION_STATUS != ACTION_FIRING) {
    modeFireStop();
  }

  if(ms_firing_lights_end.justFinished()) {
    fireStreamEnd(0,0,0);
  }

  /*
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.stop();
  }
  */
}

void checkMusic() { 
  if(ms_check_music.justFinished() && ms_music_next_track.isRunning() != true) {
    ms_check_music.start(i_music_check_delay);
        
    // Loop through all the tracks if the music is not set to repeat a track.
    if(b_playing_music == true && b_repeat_track == false) {      
      if(!w_trig.isTrackPlaying(i_current_music_track)) {
        ms_check_music.stop();
                
        stopMusic();

        // Tell the pack to stop playing music.
        Serial.write(98);
            
        if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
          i_current_music_track = i_music_track_start;
        }
        else {
          i_current_music_track++;          
        }

        // Tell the pack which music track to change to.
        Serial.write(i_current_music_track);
    
        ms_music_next_track.start(i_music_next_track_delay);
      }
    }
  }

  if(ms_music_next_track.justFinished()) {
    ms_music_next_track.stop();
        
    playMusic(); 

    // Tell the pack to play music.
    Serial.write(99);
    
    ms_check_music.start(i_music_check_delay); 
  }
} 

void stopMusic() {
  w_trig.trackStop(i_current_music_track);
  w_trig.update();
}

void playMusic() {  
  w_trig.trackGain(i_current_music_track, i_volume_music);
  w_trig.trackPlayPoly(i_current_music_track, true);
  
  if(b_repeat_track == true) {
    w_trig.trackLoop(i_current_music_track, 1);
  }
  else {
    w_trig.trackLoop(i_current_music_track, 0);
  }

  w_trig.update();
}

void settingsBlinkingLights() {
  if(ms_settings_blinking.justFinished()) {
     ms_settings_blinking.start(i_settings_blinking_delay);
  }

  if(ms_settings_blinking.remaining() < i_settings_blinking_delay / 2) {
    digitalWrite(led_bargraph_1, HIGH);
    digitalWrite(led_bargraph_2, HIGH);
    digitalWrite(led_bargraph_3, HIGH);
    digitalWrite(led_bargraph_4, HIGH);

    // Indicator for looping track setting.
    if(b_repeat_track == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
      digitalWrite(led_bargraph_5, LOW);
    }
    else {
      digitalWrite(led_bargraph_5, HIGH);
    }
  }
  else {
    switch(i_wand_menu) {
      case 5:
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, LOW);
          digitalWrite(led_bargraph_5, LOW);
      break;

      case 4:
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, LOW);
          digitalWrite(led_bargraph_5, HIGH);
      break;

      case 3:
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
      break;

      case 2:
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, HIGH);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
      break;

      case 1:
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, HIGH);
          digitalWrite(led_bargraph_3, HIGH);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
      break;
    }
  }
}

// Change the WAND_STATE here based on switches changing or pressed.
void checkSwitches() {
  switch(WAND_STATUS) {
    case MODE_OFF:
     //if(switch_activate.getState() == LOW && WAND_ACTION_STATUS == ACTION_IDLE) {
     if(switch_activate.isPressed() && WAND_ACTION_STATUS == ACTION_IDLE) {
        // Turn wand and pack on.
        WAND_ACTION_STATUS = ACTION_ACTIVATE;
      }
      
      soundBeepLoopStop();
    break;

    case MODE_ON:
      if(WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        if(analogRead(switch_mode) > 1020 && ms_switch_mode_debounce.justFinished()) {
          if(i_wand_menu == 5) {
            // Cycle through the firing modes and setting menu.
            if(FIRING_MODE == PROTON) {
              FIRING_MODE = SLIME;
            }
            else if(FIRING_MODE == SLIME) {
              FIRING_MODE = STASIS;
            }
            else if(FIRING_MODE == STASIS) {
              FIRING_MODE = MESON;
            }
            else if(FIRING_MODE == MESON) {
              FIRING_MODE = SETTINGS;
            }
            else {
              FIRING_MODE = PROTON;
            }
   
            w_trig.trackGain(S_CLICK, i_volume);
            w_trig.trackPlayPoly(S_CLICK);

            switch(FIRING_MODE) {
              case SETTINGS:
                WAND_ACTION_STATUS = ACTION_SETTINGS;
                i_wand_menu = 5;
                ms_settings_blinking.start(i_settings_blinking_delay);

                // Tell the pack we are in settings mode.
                Serial.write(9);
              break;

              case MESON:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in meson mode.
                Serial.write(8);
              break;

              case STASIS:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in stasis mode.
                Serial.write(7);
              break;

              case SLIME:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in slime mode.
                Serial.write(6);
              break;

              case PROTON:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in proton mode.
                Serial.write(5);
              break;
            }
          }

          ms_switch_mode_debounce.start(a_switch_debounce_time);
        }
      }

      // Vent light and first stage of the safety system.
      if(switch_vent.getState() == LOW) {
        // Vent light and top white light on.
        digitalWrite(led_vent, LOW);

        soundIdleStart();

        if(switch_wand.getState() == LOW) {
          if(b_beeping != true) {
            // Beep loop.
            soundBeepLoop();
          }  
        }
        else {
          soundBeepLoopStop();
        }
      }
      else if(switch_vent.getState() == HIGH) {        
        // Vent light and top white light off.
        digitalWrite(led_vent, HIGH);

        soundBeepLoopStop();
        soundIdleStop();
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        if(switch_intensify.getState() == LOW && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_firing == false && b_pack_on == true && analogRead(switch_barrel) < i_switch_barrel_value) {
          WAND_ACTION_STATUS = ACTION_FIRING;
        }
      
        if(switch_intensify.getState() == HIGH && b_firing == true) {
          WAND_ACTION_STATUS = ACTION_IDLE;
        }
      
        if(switch_activate.getState() == HIGH) {
          WAND_ACTION_STATUS = ACTION_OFF;
        }
      }
      else if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
        if(switch_activate.getState() == HIGH) {
          WAND_ACTION_STATUS = ACTION_OFF;
        }
      }
    break;
  }
}

void wandOff() {
  // Tell the pack the wand is turned off.
  Serial.write(2);

  if(FIRING_MODE == SETTINGS) {
    // If the wand is shut down while we are in settings mode (can happen if the pack is manually turned off), switch the wand and pack to proton mode.
    Serial.write(5);
    FIRING_MODE = PROTON;
  }
  
  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop();
  
  vibrationOff();
  
  // Stop firing if the wand is turned off.
  if(b_firing == true) {
    modeFireStop();
  }

  w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_1);
  w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_2);
  
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_1);
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_2);
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_1);
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_2);
  w_trig.trackStop(S_BOOTUP);

  // Turn off any overheating sounds.
  //w_trig.trackStop(S_VENT_BEEP);
  w_trig.trackStop(S_CLICK);
  w_trig.trackStop(S_VENT_DRY);

  w_trig.trackStop(S_FIRE_START_SPARK);
  w_trig.trackStop(S_PACK_SLIME_OPEN);
  w_trig.trackStop(S_STASIS_START);
  w_trig.trackStop(S_MESON_START);
  
  w_trig.trackPlayPoly(S_WAND_SHUTDOWN);
  w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_DOWN_1);
  
  // Turn off some timers.
  ms_bargraph.stop();
  ms_bargraph_firing.stop();
  ms_overheat_initate.stop();
  ms_overheating.stop();
  ms_settings_blinking.stop();
    
  // Turn off remaining lights.
  wandLightsOff();
  barrelLightsOff();
}

void modeActivate() {
  // Tell the pack the wand is turned on.
  Serial.write(1);
  
  WAND_STATUS = MODE_ON;
  WAND_ACTION_STATUS = ACTION_IDLE;
  
  // Ramp up the bargraph.
  bargraphRampUp();
  
  // Turn on slo-blo light.
  analogWrite(led_slo_blo, 255);

  // Top white light.
  ms_white_light.start(d_white_light_interval);
  digitalWrite(led_white, LOW);

  switch(year_mode) {
    case 1984:
      // Nothing in 1984 mode.
    break;

    default:
      soundIdleLoop(true);

      w_trig.trackGain(S_AFTERLIFE_GUN_RAMP_1, 0);  // Preset Track to gain/volume to 0
      w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_1); // Start track
      
      ms_gun_loop_1.start(620);
    break;
  }
}

void soundIdleLoop(boolean fade) {      
  switch(i_power_mode) {
    case 1:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, -20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_1, -10, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, -10);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
     break;

     case 2:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, -20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_1, -8, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, -8);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
     break;

     case 3:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, -20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_2, -6, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, -6);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
     break;

     case 4:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, -20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_2, -4, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, -4);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
     break;

     case 5:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_5, -20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_5, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_5, -1, 2000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_5, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_5, -1);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_5, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_5, 1);
      }
     break;
    }
}

void soundIdleLoopStop() {
  w_trig.trackStop(S_IDLE_LOOP_GUN);
  w_trig.trackStop(S_IDLE_LOOP_GUN_1);
  w_trig.trackStop(S_IDLE_LOOP_GUN_2);
  w_trig.trackStop(S_IDLE_LOOP_GUN_3);
  w_trig.trackStop(S_IDLE_LOOP_GUN_4);
  w_trig.trackStop(S_IDLE_LOOP_GUN_5);
  
}

void soundIdleStart() {
  if(b_sound_idle == false) {  
    switch(year_mode) {
      case 1984:
          w_trig.trackGain(S_BOOTUP, 0);
          w_trig.trackPlayPoly(S_BOOTUP, true);

          soundIdleLoop(true);        
      break;
  
      default:
          w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_1);
          w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_1);
          w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_2);
          w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_1);
          
          w_trig.trackGain(S_AFTERLIFE_GUN_RAMP_2, 0);  // Preset Track to gain/volume to 0
          w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_2, true); // Start track

          ms_gun_loop_1.stop();
          ms_gun_loop_2.start(700);
      break;
    }
  }

  if(b_sound_idle == false) {
    b_sound_idle = true;
  }

  if(year_mode == 2021) {
    if(ms_gun_loop_2.justFinished()) {
      w_trig.trackGain(S_AFTERLIFE_GUN_LOOP_2, -15);
      w_trig.trackPlayPoly(S_AFTERLIFE_GUN_LOOP_2, true);
      w_trig.trackLoop(S_AFTERLIFE_GUN_LOOP_2, 1);
      
      ms_gun_loop_2.stop();
    }
  }
}

void soundIdleStop() {
  if(b_sound_idle == true) {
    switch(year_mode) {
      case 1984:
        w_trig.trackPlayPoly(S_WAND_SHUTDOWN, true);
      break;

      default:
        w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_DOWN_2, true);
        ms_gun_loop_1.start(1400);
        ms_gun_loop_2.stop();
      break;
    }
  }
  
  b_sound_idle = false;

  //soundBeepLoopStop();

  switch(year_mode) {
    case 1984:
      w_trig.trackStop(S_BOOTUP);
      soundIdleLoopStop();
    break;

    case 2021:
      w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_2);
      w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_2);
    break;
  }
}

void soundBeepLoopStop() {
  if(b_beeping == true) {
    b_beeping = false;
    
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S1);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S2);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S3);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S4);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S5);
    
    ms_reset_sound_beep.stop();
    ms_reset_sound_beep.start(i_sound_timer);
  }
}
void soundBeepLoop() {  
  if(ms_reset_sound_beep.justFinished()) {
    if(b_beeping == false) {
      switch(i_power_mode) {
        case 1:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S1, true);
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S1, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S1, 0);
          }
         break;
  
         case 2:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S2, true);
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S2, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S2, 0);
          }
         break;
  
         case 3:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S3, true);
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S3, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S3, 0);
          }
         break;
  
         case 4:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S4, true);
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S4, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S4, 0);
          }
         break;
  
         case 5:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S5, true);
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S5, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S5, 0);
          }
         break;
      }
      
      b_beeping = true;
      
      ms_reset_sound_beep.stop();
    }
  }
}

void modeFireStart() {
  // Stop all firing sounds first.
  w_trig.trackStop(S_FIRING_END_GUN);
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

  // Turn off any overheating sounds.
  //w_trig.trackStop(S_VENT_BEEP);
  w_trig.trackStop(S_CLICK);
  w_trig.trackStop(S_VENT_DRY);
      
  delay(50);

  // Tell the pack the wand is firing.
  Serial.write(3);

  ms_overheat_initate.stop();

  // If in high power mode on the wand, start a overheat timer.
  if(i_power_mode == i_power_mode_max) {
    ms_overheat_initate.start(i_ms_overheat_initate);
  }
  
  // Some sparks for firing start.
  w_trig.trackGain(S_FIRE_START_SPARK, 0);
  w_trig.trackPlayPoly(S_FIRE_START_SPARK);

  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackGain(S_FIRE_START, 0);
      w_trig.trackPlayPoly(S_FIRE_START);
  
      w_trig.trackGain(S_FIRE_LOOP_GUN, 0);
      w_trig.trackPlayPoly(S_FIRE_LOOP_GUN, true);
      w_trig.trackFade(S_FIRE_LOOP_GUN, 0, 1000, 0);
      w_trig.trackLoop(S_FIRE_LOOP_GUN, 1);
    break;

    case SLIME:
      w_trig.trackGain(S_SLIME_START, 0);
      w_trig.trackPlayPoly(S_SLIME_START);
      
      w_trig.trackGain(S_SLIME_LOOP, 0);
      w_trig.trackPlayPoly(S_SLIME_LOOP, true);
      w_trig.trackFade(S_SLIME_LOOP, 0, 1500, 0);
      w_trig.trackLoop(S_SLIME_LOOP, 1);
    break;

    case STASIS:
      w_trig.trackGain(S_STASIS_START, 0);
      w_trig.trackPlayPoly(S_STASIS_START);
      
      w_trig.trackGain(S_STASIS_LOOP, 0);
      w_trig.trackPlayPoly(S_STASIS_LOOP, true);
      w_trig.trackFade(S_STASIS_LOOP, 0, 1000, 0);
      w_trig.trackLoop(S_STASIS_LOOP, 1);
    break;

    case MESON:
      w_trig.trackGain(S_MESON_START, 0);
      w_trig.trackPlayPoly(S_MESON_START);
      
      w_trig.trackGain(S_MESON_LOOP, 0);
      w_trig.trackPlayPoly(S_MESON_LOOP, true);
      w_trig.trackFade(S_MESON_LOOP, 0, 5500, 0);
      w_trig.trackLoop(S_MESON_LOOP, 1);
    break;

    case SETTINGS:
      // Nothing.
    break;
  }
  
  barrelLightsOff();
  
  ms_firing_lights.start(10);
  i_barrel_light = 0;

  // Stop any bargraph ramps.
  ms_bargraph.stop();

  i_bargraph_status = 1;
  bargraphRampFiring();

  ms_impact.start(random(10,15) * 1000);
}

void modeFireStop() {
  ms_overheat_initate.stop();
  
  // Tell the pack the wand stopped firing.
  Serial.write(4);
  
  WAND_ACTION_STATUS = ACTION_IDLE;
  
  b_firing = false;

  ms_bargraph_firing.stop();
  i_bargraph_status = i_power_mode - 1;
  bargraphRampUp();
  
  ms_firing_stream_blue.stop();
  ms_firing_lights.stop();
  ms_impact.stop();

  i_barrel_light = 0;
  ms_firing_lights_end.start(10);

  // Stop all other firing sounds.
  w_trig.trackStop(S_FIRING_END_GUN);
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

  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackGain(S_FIRING_END_GUN, 40);
      w_trig.trackPlayPoly(S_FIRING_END_GUN, true); // Gun firing end sound.
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
  
  // A tiny ramp down delay, helps with the sounds.
  delay(100);
}

void modeFiring() {
  /*
   * CRGB 
   * R = green
   * G = red
   * B = blue
   * 
   * yellow = 255, 255, 0
   * mid-yellow = 150,255,0
   * orange = 40, 255, 0
   * dark orange = 20, 255, 0
   */

   // If the user changes the wand power output while firing, turn off the overheat timer.
  if(i_power_mode != i_power_mode_max && ms_overheat_initate.isRunning()) {
    ms_overheat_initate.stop();
    
    // Tell the pack to revert back to regular cyclotron speeds.
    Serial.write(12);
  }
  else if(i_power_mode == i_power_mode_max && ms_overheat_initate.remaining() == 0) {
    // If the user changes back to high power mode while firing, start up a timer.
    ms_overheat_initate.start(i_ms_overheat_initate);
  }
  
  switch(FIRING_MODE) {     
    case PROTON:
       fireStreamStart(255, 70, 0);
       fireStream(0, 0, 255);     
    break;
      
    case SLIME:
       fireStreamStart(0, 255, 45);
       fireStream(20, 200, 45);  
    break;

    case STASIS:
       fireStreamStart(0, 45, 100);
       fireStream(0, 100, 255);  
    break;

    case MESON:
       fireStreamStart(200, 200, 20);
       fireStream(190, 20, 70);
    break;

    case SETTINGS:
      // Nothing
    break;
  }

  // Bargraph loop / scroll.
  if(ms_bargraph_firing.justFinished()) {
    bargraphRampFiring();
  }

  // Mix some impact sound every 10-15 seconds while firing.
  if(ms_impact.justFinished()) {
    w_trig.trackPlayPoly(S_FIRE_LOOP_IMPACT);
    ms_impact.start(random(10,15) * 1000);
  }
}

void wandHeatUp() {
  w_trig.trackStop(S_FIRE_START_SPARK);
  w_trig.trackStop(S_PACK_SLIME_OPEN);
  w_trig.trackStop(S_STASIS_OPEN);
  w_trig.trackStop(S_MESON_OPEN);

  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackGain(S_FIRE_START_SPARK, i_volume);
      w_trig.trackPlayPoly(S_FIRE_START_SPARK);
    break;

    case SLIME:
      w_trig.trackGain(S_PACK_SLIME_OPEN, i_volume);
      w_trig.trackPlayPoly(S_PACK_SLIME_OPEN);
    break;

    case STASIS:
      w_trig.trackGain(S_STASIS_OPEN, i_volume);
      w_trig.trackPlayPoly(S_STASIS_OPEN);
    break;

    case MESON:
      w_trig.trackGain(S_MESON_OPEN, i_volume);
      w_trig.trackPlayPoly(S_MESON_OPEN);
    break;
  }

  wandBarrelPreHeatUp();
}

void wandBarrelPreHeatUp() {
  i_heatup_counter = 0;
  i_heatdown_counter = 100;
  ms_wand_heatup_fade.start(i_delay_heatup);
}

void wandBarrelHeatUp() {
  if(i_heatup_counter > 100) {
    wandBarrelHeatDown();
  }
  else if(ms_wand_heatup_fade.justFinished() && i_heatup_counter <= 100) {        
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatup_counter, i_heatup_counter, i_heatup_counter);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
  
      case SLIME:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatup_counter, 0, 0);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
  
      case STASIS:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(0, 0, i_heatup_counter);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
  
      case MESON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatup_counter, i_heatup_counter, 0);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
    } 

    i_heatup_counter++;
    ms_wand_heatup_fade.start(i_delay_heatup);
  }
}

void wandBarrelHeatDown() {
  if(ms_wand_heatup_fade.justFinished() && i_heatdown_counter > 0) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatdown_counter, i_heatdown_counter, i_heatdown_counter);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
  
      case SLIME:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatdown_counter, 0, 0);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
  
      case STASIS:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(0, 0, i_heatdown_counter);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
  
      case MESON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatdown_counter, i_heatdown_counter, 0);
        FastLED.show();
        //ms_fast_led.start(i_fast_led_delay);
      break;
    }

    i_heatdown_counter--;
    ms_wand_heatup_fade.start(i_delay_heatup);
  }
  
  if(i_heatdown_counter == 0) {
    barrelLightsOff();
  }
}

void fireStream(int r, int g, int b) {
  if(ms_firing_stream_blue.justFinished()) {
    if(i_barrel_light - 1 > -1 && i_barrel_light - 1 < BARREL_NUM_LEDS) {      
      switch(FIRING_MODE) {
        case PROTON:
          barrel_leds[i_barrel_light - 1] = CRGB(50, 255, 0);

          // Make the stream more slightly more red on higher power modes.
          switch(i_power_mode) {
            case 1:
              barrel_leds[i_barrel_light - 1] = CRGB(50, 255, 0);
            break;

            case 2:
              barrel_leds[i_barrel_light - 1] = CRGB(40, 255, 0);
            break;

            case 3:
              barrel_leds[i_barrel_light - 1] = CRGB(30, 255, 0);
            break;

            case 4:
              barrel_leds[i_barrel_light - 1] = CRGB(20, 255, 0);
            break;

            case 5:
              barrel_leds[i_barrel_light - 1] = CRGB(10, 255, 0);
            break;

            default:
              barrel_leds[i_barrel_light - 1] = CRGB(50, 255, 0);
            break;
          }
        break;

        case SLIME:
          barrel_leds[i_barrel_light - 1] = CRGB(120, 20, 45);
        break;

        case STASIS:
          barrel_leds[i_barrel_light - 1] = CRGB(15, 50, 155);
        break;

        case MESON:
          barrel_leds[i_barrel_light - 1] = CRGB(200, 200, 15);
        break;

        case SETTINGS:
          // Nothing
        break;
      }
      
      FastLED.show();
      //ms_fast_led.start(i_fast_led_delay);
    }

    if(i_barrel_light == BARREL_NUM_LEDS) {
      i_barrel_light = 0;
            
      ms_firing_stream_blue.start(d_firing_stream / 2);
    }
    else if(i_barrel_light < BARREL_NUM_LEDS) {
      barrel_leds[i_barrel_light] = CRGB(g,r,b);
      FastLED.show();
      //ms_fast_led.start(i_fast_led_delay);
            
      ms_firing_stream_blue.start(d_firing_lights);
  
      i_barrel_light++;
    }
  }
}

void barrelLightsOff() {
  ms_wand_heatup_fade.stop();
  i_heatup_counter = 0;
  i_heatdown_counter = 100;
  
  for(int i = 0; i < BARREL_NUM_LEDS; i++) {
    barrel_leds[i] = CRGB(0,0,0);
  }

  FastLED.show();
  //ms_fast_led.start(i_fast_led_delay);
}

void fireStreamStart(int r, int g, int b) {
  if(ms_firing_lights.justFinished() && i_barrel_light < BARREL_NUM_LEDS) {
    barrel_leds[i_barrel_light] = CRGB(g,r,b);
    FastLED.show();
    //ms_fast_led.start(i_fast_led_delay);
          
    ms_firing_lights.start(d_firing_lights);

    i_barrel_light++;

    if(i_barrel_light == BARREL_NUM_LEDS) {
      i_barrel_light = 0;
            
      ms_firing_lights.stop();
      ms_firing_stream_blue.start(d_firing_stream);
    }
  }
}

void fireStreamEnd(int r, int g, int b) {
  if(i_barrel_light < BARREL_NUM_LEDS) {
    barrel_leds[i_barrel_light] = CRGB(g,r,b);
    FastLED.show();
    //ms_fast_led.start(i_fast_led_delay);
          
    ms_firing_lights_end.start(d_firing_lights);

    i_barrel_light++;

    if(i_barrel_light == BARREL_NUM_LEDS) {
      i_barrel_light = 0;
      
      ms_firing_lights_end.stop();
    }
  }
}

void vibrationWand(int i_level) {
  if(b_vibration_on == true) {
    if(i_level != i_vibration_level_prev) {
      i_vibration_level_prev = i_level;
      analogWrite(vibration, i_level);
    }
  }
  else {
    analogWrite(vibration, 0);
  }
}

void bargraphRampFiring() {
  switch(i_bargraph_status) {
    case 1:
      vibrationWand(i_vibration_level + 110);
             
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, LOW);
      i_bargraph_status++;
    break;

    case 2:
      vibrationWand(i_vibration_level + 112);  
      
      digitalWrite(led_bargraph_1, HIGH);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, LOW);
      digitalWrite(led_bargraph_5, HIGH);
      i_bargraph_status++;
    break;

    case 3:
      vibrationWand(i_vibration_level + 115);
      
      digitalWrite(led_bargraph_1, HIGH);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, LOW);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
      i_bargraph_status++;
    break;

    case 4:
      vibrationWand(i_vibration_level + 112);

      digitalWrite(led_bargraph_1, HIGH);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, LOW);
      digitalWrite(led_bargraph_5, HIGH);
      i_bargraph_status++;
    break;

    case 5:
      vibrationWand(i_vibration_level + 110);
      
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, LOW);
      i_bargraph_status = 1;
    break;
  }

  // If in high power mode on the wand, change the speed of the bargraph ramp during firing based on time remaining before we overheat.
  if(i_power_mode == i_power_mode_max) {
    if(ms_overheat_initate.remaining() < i_ms_overheat_initate / 6) {
      ms_bargraph_firing.start(d_bargraph_ramp_interval / 5);
      cyclotronSpeedUp(6);
    }
    else if(ms_overheat_initate.remaining() < i_ms_overheat_initate / 5) {
      ms_bargraph_firing.start(d_bargraph_ramp_interval / 4);
      cyclotronSpeedUp(5);
    }
    else if(ms_overheat_initate.remaining() < i_ms_overheat_initate / 4) {
      ms_bargraph_firing.start(d_bargraph_ramp_interval / 3.5);
      cyclotronSpeedUp(4);    
    }
    else if(ms_overheat_initate.remaining() < i_ms_overheat_initate / 3) {
      ms_bargraph_firing.start(d_bargraph_ramp_interval / 3);
      cyclotronSpeedUp(3);
    }
    else if(ms_overheat_initate.remaining() < i_ms_overheat_initate / 2) {
      ms_bargraph_firing.start(d_bargraph_ramp_interval / 2.5);
      cyclotronSpeedUp(2);
    }
    else {
      ms_bargraph_firing.start(d_bargraph_ramp_interval / 2);
      i_cyclotron_speed_up = 1;
    }
  }
  else {  
    ms_bargraph_firing.start(d_bargraph_ramp_interval / 2);
  }
}

void cyclotronSpeedUp(int i_switch) {
  if(i_switch != i_cyclotron_speed_up) {
    if(i_switch == 4) {
      // Tell pack to start beeping before we overheat it.
      Serial.write(15);

      // Beep the wand 8 times.
      w_trig.trackGain(S_BEEP_8, i_volume);
      w_trig.trackPlayPoly(S_BEEP_8);
    }
    
    i_cyclotron_speed_up++;
    
    // Tell the pack to speed up the cyclotron.
    Serial.write(13);
  }  
}

void bargraphPowerCheck() {
  switch(i_power_mode) {
    case 1:
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
    break;

    case 2:
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
    break;

    case 3:
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, LOW);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
    break;

    case 4:
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, LOW);
      digitalWrite(led_bargraph_4, LOW);
      digitalWrite(led_bargraph_5, HIGH);
    break;

    case 5:
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, LOW);
      digitalWrite(led_bargraph_4, LOW);
      digitalWrite(led_bargraph_5, LOW);
    break;
  }
}

void bargraphRampUp() { 
  switch(i_bargraph_status) {
    case 0:
      vibrationWand(i_vibration_level + 10);
              
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
      ms_bargraph.start(d_bargraph_ramp_interval);
      i_bargraph_status++;
    break;

    case 1:
      vibrationWand(i_vibration_level + 20);
      
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
      ms_bargraph.start(d_bargraph_ramp_interval);
      i_bargraph_status++;
    break;

    case 2:
      vibrationWand(i_vibration_level + 30);
           
      digitalWrite(led_bargraph_3, LOW);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
      ms_bargraph.start(d_bargraph_ramp_interval);
      i_bargraph_status++;
    break;

    case 3:
      vibrationWand(i_vibration_level + 40);
      
      digitalWrite(led_bargraph_4, LOW);
      digitalWrite(led_bargraph_5, HIGH);
      ms_bargraph.start(d_bargraph_ramp_interval);
      i_bargraph_status++;
    break;

    case 4:
      vibrationWand(i_vibration_level + 80);
      
      digitalWrite(led_bargraph_5, LOW);

      if(i_bargraph_status + 1 == i_power_mode) {
        ms_bargraph.stop();
        i_bargraph_status = 0;
      }
      else {
        i_bargraph_status++;
        ms_bargraph.start(d_bargraph_ramp_interval);
      }
    break;
    
    case 5:
      vibrationWand(i_vibration_level + 40);
      
      digitalWrite(led_bargraph_5, HIGH);
      
      if(i_bargraph_status - 1 == i_power_mode) {
        ms_bargraph.stop();
        i_bargraph_status = 0;
      }
      else {
        i_bargraph_status++;
        ms_bargraph.start(d_bargraph_ramp_interval);
      }
    break;
    
    case 6:
      vibrationWand(i_vibration_level + 30);
          
      digitalWrite(led_bargraph_4, HIGH);
      
      if(i_bargraph_status - 3 == i_power_mode) {
        ms_bargraph.stop();
        i_bargraph_status = 0;
      }
      else {
        i_bargraph_status++;
        ms_bargraph.start(d_bargraph_ramp_interval);
      }
    break;
    
    case 7:
      vibrationWand(i_vibration_level + 20);
          
      digitalWrite(led_bargraph_3, HIGH);
      
      if(i_bargraph_status - 5 == i_power_mode) {
        ms_bargraph.stop();
        i_bargraph_status = 0;
      }
      else {
        i_bargraph_status++;
        ms_bargraph.start(d_bargraph_ramp_interval);
      }
    break;

    case 8:
      vibrationWand(i_vibration_level + 10);
          
      digitalWrite(led_bargraph_4, HIGH);
      
      if(i_bargraph_status - 7 == i_power_mode) {
        ms_bargraph.stop();
        i_bargraph_status = 0;
      }
      else {
        ms_bargraph.start(d_bargraph_ramp_interval);
        i_bargraph_status = 1;
      }
  }
}

void wandLightsOff() {
  digitalWrite(led_bargraph_1, HIGH);
  digitalWrite(led_bargraph_2, HIGH);
  digitalWrite(led_bargraph_3, HIGH);
  digitalWrite(led_bargraph_4, HIGH);
  digitalWrite(led_bargraph_5, HIGH);

  analogWrite(led_slo_blo, 0);
  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);

  i_bargraph_status = 0;
}

void vibrationOff() {
  analogWrite(vibration, 0);
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

/*
 * Top rotary dial on the wand.
 */
void checkRotary() {
  static int8_t c,val;

  if(val = readRotary()) {
    c += val;

    switch(WAND_ACTION_STATUS) {
      case ACTION_SETTINGS:
        if(WAND_STATUS == MODE_ON) {
          // Counter clockwise.
          if(prev_next_code == 0x0b) {
            if(i_wand_menu - 1 < 1) {
              i_wand_menu = 1;
            }
            else {
              i_wand_menu--;
            }
          }
  
          // Clockwise.
          if(prev_next_code == 0x07) {
            if(i_wand_menu + 1 > 5) {
              i_wand_menu = 5;
            }
            else {
              i_wand_menu++;
            }
          }
        }
      break;

      default:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_power_mode - 1 >= i_power_mode_min && WAND_STATUS == MODE_ON) {
            i_power_mode--;
            soundBeepLoopStop();
    
            switch(year_mode) {
              case 1984:
                if(switch_vent.getState() == LOW) {
                  soundIdleLoopStop();
                  soundIdleLoop(false);
                }
              break;
      
              default:
                  soundIdleLoopStop();
                  soundIdleLoop(false);
              break;
            }

            updatePackPowerLevel();
          }

          // Decrease the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
          if(WAND_STATUS == MODE_OFF && b_playing_music == true) {
            if(i_volume_music - 1 < -70) {
              i_volume_music = -70;
            }
            else {
              i_volume_music = i_volume_music - 1;
            }

            w_trig.trackGain(i_current_music_track, i_volume_music);
            
            // Tell pack to lower music volume.
            Serial.write(96);
          }

        }
        
        if(prev_next_code == 0x07) {
          if(i_power_mode + 1 <= i_power_mode_max && WAND_STATUS == MODE_ON) {
            i_power_mode++;
            soundBeepLoopStop();
    
            switch(year_mode) {
              case 1984:
                if(switch_vent.getState() == LOW) {
                  soundIdleLoopStop();
                  soundIdleLoop(false);
                }
              break;
      
              default:
                  soundIdleLoopStop();
                  soundIdleLoop(false);
              break;
            }
           
            updatePackPowerLevel();     
          }

          // Increase the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
          if(WAND_STATUS == MODE_OFF && b_playing_music == true) {
            if(i_volume_music + 1 > 0) {
              i_volume_music = 0;
            }
            else {
              i_volume_music = i_volume_music + 1;
            }

            w_trig.trackGain(i_current_music_track, i_volume_music);
            
            // Tell pack to increase music volume.
            Serial.write(97);
          }
        }
      break;
    }    
  }
}

/*
 * Tell the pack which power level the wand is at.
 */
void updatePackPowerLevel() {
  switch(i_power_mode) {
    case 5:
      // Level 5
      Serial.write(20);
    break;

    case 4:
      // Level 4
      Serial.write(19);
    break;

    case 3:
      // Level 3
      Serial.write(18);
    break;

    case 2:
      // Level 2
      Serial.write(17);
    break;

    default:
      // Level 1
      Serial.write(16);
    break;
  }
}

int8_t readRotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prev_next_code <<= 2;
  
  if(digitalRead(r_encoderB)) { 
    prev_next_code |= 0x02;
  }
  
  if(digitalRead(r_encoderA)) {
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

void vibrationSetting() {
  if(b_vibration_on == true) {
    if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING) {
      switch(i_power_mode) {
        case 1:
          vibrationWand(i_vibration_level);
        break;
    
        case 2:
          vibrationWand(i_vibration_level + 5);
        break;
    
        case 3:
          vibrationWand(i_vibration_level + 10);
        break;
    
        case 4:
          vibrationWand(i_vibration_level + 12);
        break;
    
        case 5:
          vibrationWand(i_vibration_level + 25);
        break;
      }
    }
  }
  else {
    vibrationOff();
  }
}

void switchLoops() {
  switch_wand.loop();
  switch_intensify.loop();
  switch_activate.loop();
  switch_vent.loop();
}

void wandBarrelLightsOff() {
  for(int i = 0; i < BARREL_NUM_LEDS; i++) {
    barrel_leds[i] = CRGB(0,0,0);
  }
  
  FastLED.show();
  //ms_fast_led.start(i_fast_led_delay);
}

/*
 * Pack commuication to the wand.
 */
void checkPack() {
  if(Serial.available() > 0) {
    rx_byte = Serial.read();
    
    switch(rx_byte) {      
      case 1:
        // Pack is on.
        b_pack_on = true;
      break;

      case 2:
        if(b_pack_on == true) {
          // Turn wand off.
          if(WAND_STATUS != MODE_OFF) {
            WAND_ACTION_STATUS = ACTION_OFF;
          }
        }
        
        // Pack is off.
        b_pack_on = false;
      break;

      case 3:
        // Alarm is on.
        b_pack_alarm = true;
      break;

      case 4:
        // Alarm is off.
        b_pack_alarm = false;
      break;

      case 5:
        // Vibration on.
        b_vibration_on = true;
      break;

      case 6:
        // Vibration off.
        b_vibration_on = false;
        vibrationOff();
      break;

      case 7:
        // 1984 mode.
        year_mode = 1984;
      break;

      case 8:
        // 2021 mode.
        year_mode = 2021;
      break;

      case 9:
        // Increase overall volume.
        increaseVolume();
      break;

      case 10:
        // Decrease overall volume.
        decreaseVolume();
      break;

      case 11:
        // The pack is asking us if we are still here. Respond back.
        Serial.write(14);

        b_wait_for_pack = false;
      break;

      case 12:
        // Repeat music track.
        b_repeat_track = true;
      break;

      case 13:
        // Repeat music track.
        b_repeat_track = false;
      break;

      case 14:
        // Reset volumes
        i_volume = STARTUP_VOLUME;
        i_volume_music = STARTUP_VOLUME;
        w_trig.masterGain(i_volume);
      break;
      
      case 99:
        // Stop music
        stopMusic();
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
}

void setupWavTrigger() {
    // If the controller is powering the WAV Trigger, we should wait for the WAV trigger to finish reset before trying to send commands.
  delay(1000);
  
  // WAV Trigger's startup at 57600
  w_trig.start();
  
  delay(10);

  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  w_trig.stopAllTracks();
  w_trig.samplerateOffset(0); // Reset our sample rate offset        
  w_trig.masterGain(i_volume); // Reset the master gain db. 0db is default. Range is -70 to +4. (Can go higher than 4?)
  w_trig.setAmpPwr(true); // Turn on the onboard amp. Turn it off to draw less power if you decide to use the aux cable jack instead.
  
  // Enable track reporting from the WAV Trigger
  w_trig.setReporting(true);

  // Allow time for the WAV Trigger to respond with the version string and number of tracks.
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
