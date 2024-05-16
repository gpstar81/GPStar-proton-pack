/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Wand state.
 */
enum WAND_STATE { MODE_OFF, MODE_ON, MODE_ERROR };
enum WAND_STATE WAND_STATUS;

/*
 * Various wand action states.
 */
enum WAND_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE, ACTION_FIRING, ACTION_OVERHEATING, ACTION_VENTING, ACTION_SETTINGS, ACTION_ERROR, ACTION_LED_EEPROM_MENU, ACTION_CONFIG_EEPROM_MENU };
enum WAND_ACTION_STATE WAND_ACTION_STATUS;

/*
 * System modes.
 * Super Hero: A idealised system based on the close up of the Super Hero Proton Pack and Neutrona Wand in the 1984 Rooftop closeup scene and what is shown in Afterlife. (Different toggle switch sequences for turning on the pack and wand)
 * Original: Based on the original operational manual during production of GB1. (Wand toggle switches must be on before the cyclotron can turn on from the Wand only.)
 * Super Hero will be the default system mode.
 */
enum SYSTEM_MODES { MODE_SUPER_HERO, MODE_ORIGINAL };
enum SYSTEM_MODES SYSTEM_MODE;

/*
 * Which year mode the Proton Pack is set into which may not be the same the user prefers for the wand.
 * Though this can/will be used if YEAR_DEFAULT is specified by the user as the WAND_YEAR_MODE.
 */
enum SYSTEM_YEARS { SYSTEM_1984, SYSTEM_1989, SYSTEM_AFTERLIFE, SYSTEM_FROZEN_EMPIRE };
enum SYSTEM_YEARS SYSTEM_YEAR;

/*
 * Which year mode the Neutrona Wand is set into, regardless of which year the Proton Pack is in.
 * This affects just various wand beep sound effects, loops and lighting and bargraph.
 * However the bargraph animations can be overridden with BARGRAPH_MODE and BARGRAPH_FIRING_ANIMATION
 * YEAR_DEFAULT lets the system choose based on the Proton Pack year settings.
 */
enum WAND_YEAR_MODES { YEAR_DEFAULT, YEAR_1984, YEAR_1989, YEAR_AFTERLIFE, YEAR_FROZEN_EMPIRE };
enum WAND_YEAR_MODES WAND_YEAR_MODE;

/*
 * Bargraph modes.
 * Super Hero: Mimics the super hero bargraph animations from the Neutrona Wand closeup in the 1984 rooftop. This is the default for 1984/1989 and Super Hero Mode.
 * Original: Mimics the original diagrams and instructions based on production notes and in Afterlife. This is the default for Afterlife and Mode Original.
 */
enum BARGRAPH_MODES { BARGRAPH_SUPER_HERO, BARGRAPH_ORIGINAL };
enum BARGRAPH_MODES BARGRAPH_MODE;

/*
 * Used for manually toggling between the different bragraph modes and saving to the EEPROM memory.
 * This is the setting you want to change as the system uses this to configure the BARGRAPH_MODE.
 */
enum BARGRAPH_MODES_EEPROM { BARGRAPH_EEPROM_DEFAULT, BARGRAPH_EEPROM_SUPER_HERO, BARGRAPH_EEPROM_ORIGINAL };
enum BARGRAPH_MODES_EEPROM BARGRAPH_MODE_EEPROM;

/*
 * Bargraph Firing Animations.
 * Animation Super Hero: Mimics the fandom animations of the bargraph scrolling up and down with 2 lines with it merging in the middle. This is the default for 1984/1989 and Super Hero Mode.
 * Animation Original: Mimics the original diagrams and instructions based on production notes. This is the default for Afterlife and Mode Original.
 */
enum BARGRAPH_FIRING_ANIMATIONS { BARGRAPH_ANIMATION_SUPER_HERO, BARGRAPH_ANIMATION_ORIGINAL };
enum BARGRAPH_FIRING_ANIMATIONS BARGRAPH_FIRING_ANIMATION;

/*
 * Used for manually toggling between the different bragraph firing animation modes and saving to the EEPROM memory.
 * This is the setting you want to change as the system uses this to configure the BARGRAPH_FIRING_ANIMATION.
 */
enum BARGRAPH_EEPROM_FIRING_ANIMATIONS { BARGRAPH_EEPROM_ANIMATION_DEFAULT, BARGRAPH_EEPROM_ANIMATION_SUPER_HERO, BARGRAPH_EEPROM_ANIMATION_ORIGINAL };
enum BARGRAPH_EEPROM_FIRING_ANIMATIONS BARGRAPH_EEPROM_FIRING_ANIMATION;

/*
 * Which CTS "Cross The Streams" year mode the Neutrona Wand is set to. The Proton Pack will match this when set.
 * This affects which CTS "Cross The Streams" sounds are used as the sound effects are different depending on the year.
 * CTS_DEFAULT lets the system choose based on the year setting of the Proton Pack.
 */
enum WAND_YEAR_CTS_SETTING { CTS_DEFAULT, CTS_1984, CTS_1989, CTS_AFTERLIFE, CTS_FROZEN_EMPIRE };
enum WAND_YEAR_CTS_SETTING WAND_YEAR_CTS;

/*
 * For MODE_ORIGINAL. For blinking the slo-blo light when the cyclotron is not on.
 */
millisDelay ms_slo_blo_blink;
const uint16_t i_slo_blo_blink_delay = 500;

/*
 * Control for the Meson Shock Blast sound effects.
*/
millisDelay ms_meson_blast;
const uint16_t i_meson_blast_delay_level_5 = 140;
const uint16_t i_meson_blast_delay_level_4 = 160;
const uint16_t i_meson_blast_delay_level_3 = 180;
const uint16_t i_meson_blast_delay_level_2 = 200;
const uint16_t i_meson_blast_delay_level_1 = 220;

/*
 * Barrel LEDs
 * The Hasbro Neutrona Wand has 5 LEDs. 0 = Base, 4 = tip. These are addressable with a single pin and are RGB.
 * Support for up to 49 LEDs from Frutto Technology (body of 48 with a "strobe tip" which is also RGB).
 * When using the 48 LED option the standard white LED will be swapped for the 49th LED of the Frutto option.
 */
#define BARREL_LEDS_MAX 49 // The maximum number of barrel LEDs supported (Frutto = 48 + Strobe Tip).
#define BARREL_LED_PIN 10
CRGB barrel_leds[BARREL_LEDS_MAX];
// This is the array of LEDs in the order by which they should be illuminated for effects. LED number 12 is the very tip which will be white (by default).
const uint8_t frutto_barrel[48] PROGMEM = {0, 25, 24, 48, 1, 26, 23, 47, 2, 27, 22, 46, 3, 28, 21, 45, 4, 29, 20, 44, 5, 30, 19, 43, 6, 31, 18, 42, 7, 32, 17, 41, 8, 33, 16, 40, 9, 34, 15, 39, 10, 35, 14, 38, 11, 36, 13, 37};

/*
 * How many LEDs are in your Neutrona Wand Barrel.
 * Default setting is 5: for the Hasbro Neturona Wand.
 * Supported options: Stock (5) and Frutto Technology (48 + Strobe Tip)
 */
uint8_t i_num_barrel_leds = 5;
enum WAND_BARREL_LED_COUNTS { LEDS_5, LEDS_48 };
enum WAND_BARREL_LED_COUNTS WAND_BARREL_LED_COUNT;

/*
 * Delay for fastled to update the addressable LEDs.
 * We have up to 5 addressable LEDs in the wand barrel.
 * The Frutto barrel has up to 49 addressable LEDs.
 * 0.03 ms to update 1 LED. So 1.47 ms should be okay? Let's bump it up to 3 just in case.
 */
#define FAST_LED_UPDATE_MS 3
uint8_t i_fast_led_delay = FAST_LED_UPDATE_MS;
millisDelay ms_fast_led;

/*
 * Non-addressable LEDs
 */
const uint8_t led_slo_blo = 8; // SLO-BLO LED. (Red LED)
const uint8_t led_front_left = 9; // LED underneath the Clippard valve. (Orange or White LED)
const uint8_t led_white = 12; // Blinking white light beside the vent on top of the wand.
const uint8_t led_vent = 13; // Vent light
const uint8_t led_hat_1 = 22; // Hat light at front of the wand near the barrel tip. (Orange LED)
const uint8_t led_hat_2 = 23; // Hat light at top of the wand body near vent. (Orange or White LED)
const uint8_t led_barrel_tip = 24; // White LED at tip of the wand barrel. (White LED)

/*
 * Time in milliseconds for blinking the top white LED while the wand is on.
 * By default this is set to the blink cycle used on the Afterlife props.
 * On first system start a random value will be selected for GB1/GB2 mode.
 * Common values are as follows:
 * GB1 Spengler, GB1 Venkman (Sedgewick): 666
 * GB2 Spengler: 500
 * GB1/GB2 Stantz, GB2 Venkman (Courtroom): 333
 * GB1 Venkman (Rooftop): 417
 * GB2 Venkman (Vigo), GB2 Zeddemore: 375
 * Afterlife (all props): 146
 */
const uint16_t i_afterlife_blink_interval = 146;
const uint16_t i_classic_blink_intervals[5] = {333, 375, 417, 500, 666};
uint8_t i_classic_blink_index = 0;
uint16_t d_white_light_interval = i_afterlife_blink_interval;

/*
 * Rotary encoder on the top of the wand. Changes the wand power level and controls the wand settings menu.
 * Also controls independent music volume while the pack/wand is off and if music is playing.
 */
#define r_encoderA 6
#define r_encoderB 7
millisDelay ms_firing_mode_switch; // Timer for rotary firing mode select speed limit.
const uint8_t i_firing_mode_switch_delay = 50; // Time to delay switching firing modes.
static uint8_t prev_next_code = 0;
static uint16_t store = 0;

/*
 * Vibration
 *
 * Vibration default is based on the toggle switch position from the Proton Pack. These are references for the EEPROM menu. Empty is a zero value, not used in the EEPROM.
 */
enum VIBRATION_MODES_EEPROM { VIBRATION_EMPTY, VIBRATION_ALWAYS, VIBRATION_FIRING_ONLY, VIBRATION_NONE, VIBRATION_DEFAULT };
enum VIBRATION_MODES_EEPROM VIBRATION_MODE_EEPROM;
const uint8_t vibration = 11;
const uint8_t i_vibration_level_min = 65;
uint8_t i_vibration_level = i_vibration_level_min;
uint8_t i_vibration_level_prev = 0;
bool b_menu_vibration_active = false; // Used to make sure a vibration menu call only occurs once per activation.
millisDelay ms_menu_vibration; // Timer to do non-blocking confirmation buzzing in the vibration menu.

/*
 * Enable or disable vibration control for the Neutrona Wand.
 * When set to false, there will be no vibration enabled for the Neutrona Wand.
 * This is toggled by the Proton Pack vibration toggle switch.
 */
bool b_vibration_enabled = true;

/*
 * Various Switches on the wand.
 */
Switch switch_intensify(2);
Switch switch_activate(3);
Switch switch_vent(4); // Turns on the vent light. Bottom right switch on the wand.
Switch switch_wand(A0); // Controls the beeping. Top right switch on the wand.
Switch switch_mode(A6); // Changes firing modes, crosses streams, or used in settings menus.
Switch switch_barrel(A7); // Checks whether barrel is retracted or not.
bool b_switch_barrel_extended = true; // Set to true for bootup to prevent sound from playing erroneously. The Neutrona Wand will adjust as necessary.
bool b_all_switch_activation = false; // Used to check if Activate was flipped to on while the vent switch was already in the on position for sound purposes.
uint8_t ventSwitchedCount = 0;
uint8_t wandSwitchedCount = 0;

/*
 * Hasbro bargraph LEDs.
 */
const uint8_t led_bargraph_1 = A1;
const uint8_t led_bargraph_2 = A2;
const uint8_t led_bargraph_3 = A3;
const uint8_t led_bargraph_4 = A4;
const uint8_t led_bargraph_5 = A5;

const uint8_t i_bargraph_segments_5_led = 5;
const uint8_t i_bargraph_5_led_invert[i_bargraph_segments_5_led] PROGMEM = {led_bargraph_5, led_bargraph_4, led_bargraph_3, led_bargraph_2, led_bargraph_1};
const uint8_t i_bargraph_5_led_normal[i_bargraph_segments_5_led] PROGMEM = {led_bargraph_1, led_bargraph_2, led_bargraph_3, led_bargraph_4, led_bargraph_5};
bool b_bargraph_status_5[i_bargraph_segments_5_led] = {};

/*
 * Idling timers
 */
millisDelay ms_gun_loop_1;
millisDelay ms_gun_loop_2;
millisDelay ms_white_light;
uint16_t i_gun_loop_1 = 1768; // 1660
uint16_t i_gun_loop_2 = 1653; // 1500

/*
 * Overheat timers
 */
millisDelay ms_overheat_initiate;
millisDelay ms_overheating; // This timer is only used when using the Neutrona Wand without a Proton Pack.
const uint16_t i_ms_overheating = 3000; // Overheating for 3 seconds. This is only used when using the Neutrona Wand without a Proton Pack.
bool b_overheat_level[5] = { b_overheat_level_1, b_overheat_level_2, b_overheat_level_3, b_overheat_level_4, b_overheat_level_5 };
uint32_t i_ms_overheat_initiate[5] = { i_ms_overheat_initiate_level_1, i_ms_overheat_initiate_level_2, i_ms_overheat_initiate_level_3, i_ms_overheat_initiate_level_4, i_ms_overheat_initiate_level_5 };
const uint16_t i_overheat_delay_increment = 1000; // Used to increment the overheat delays by 1000 milliseconds.
const uint16_t i_overheat_delay_max = 60000; // The maximum amount of time before an overheat sequence starts while firing. 60 seconds because of uint16_t and voice limitations.

/*
 * Stock Hasbro Bargraph timers
 */
millisDelay ms_bargraph;
millisDelay ms_bargraph_firing;
const uint8_t d_bargraph_ramp_interval = 120;
uint8_t i_bargraph_status = 0;

/*
 * (Optional) Barmeter 28-segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
 */
HT16K33 ht_bargraph;

// Used to scan the i2c bus and to locate the 28-segment bargraph.
#define WIRE Wire

/*
 * Used to change to 28-segment bargraph features.
 * The Frutto 28-segment bargraph is automatically detected on boot and sets this to true.
 * Part #: BL28Z-3005SA04Y
 */
bool b_28segment_bargraph = false;

const uint8_t i_bargraph_interval = 4;
const uint8_t i_bargraph_wait = 180;
bool b_bargraph_up = false;
millisDelay ms_bargraph_alt;
uint8_t i_bargraph_status_alt = 0;
const uint8_t d_bargraph_ramp_interval_alt = 40;
const uint8_t i_bargraph_multiplier_ramp_1984 = 3;
const uint8_t i_bargraph_multiplier_ramp_2021 = 16;
uint16_t i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;

/*
 * (Optional) Barmeter 28-segment bargraph mapping.
 * Part #: BL28Z-3005SA04Y

 * Segment Layout:
 * 5: full: 23 - 27  (5 segments)
 * 4: 3/4: 17 - 22	 (6 segments)
 * 3: 1/2: 12 - 16	 (5 segments)
 * 2: 1/4: 5 - 11	   (7 segments)
 * 1: none: 0 - 4	   (5 segments)
 */
const uint8_t i_bargraph_segments = 28;
const uint8_t i_bargraph_invert[i_bargraph_segments] PROGMEM = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
const uint8_t i_bargraph_normal[i_bargraph_segments] PROGMEM = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
bool b_bargraph_status[i_bargraph_segments] = {};

/*
 * (Optional) Support for Video Game Accessories (coming soon)
 */
// bool b_overheat_indicators[13] = {false, false, false, false, false, false, false, false, false, false, false, false, false};

/*
 * Timers for the optional hat lights.
 * Also used for vent lights during error modes.
 */
millisDelay ms_hat_1;
millisDelay ms_hat_2;
const uint8_t i_hat_1_delay = 100;
const uint16_t i_hat_2_delay = 400;

/*
 * A timer for controlling the wand beep in Afterlife & Frozen Empire mode.
 */
millisDelay ms_reset_sound_beep;
const uint8_t i_sound_timer = 150;

/*
 * Wand tip heatup timers (when changing firing modes).
 */
millisDelay ms_wand_heatup_fade;
const uint8_t i_delay_heatup = 5;
uint8_t i_heatup_counter = 0;
uint8_t i_heatdown_counter = 100;

/*
 * Wand Firing Modes + Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM };
enum FIRING_MODES FIRING_MODE;

/*
 * Firing timers.
 */
millisDelay ms_firing_lights;
millisDelay ms_firing_lights_end;
millisDelay ms_firing_effect_end;
millisDelay ms_firing_stream_effects;
millisDelay ms_firing_pulse;
millisDelay ms_impact; // Mix some impact sounds while firing.
millisDelay ms_firing_sound_mix; // Mix additional impact sounds for standalone Neutrona Wand.
millisDelay ms_semi_automatic_check; // Timer used for the semi-automatic firing modes.
const uint16_t i_boson_dart_rate = 2000; // Boson Dart firing rate.
const uint16_t i_shock_blast_rate = 600; // Shock Blast firing rate.
const uint16_t i_slime_tether_rate = 750; // Slime Tether firing rate.
const uint16_t i_meson_collider_rate = 250; // Meson Collider firing rate.
const uint8_t d_firing_pulse = 18; // Used to drive semi-automatic firing stream effect timers. Default: 18ms.
const uint8_t d_firing_stream = 100; // Used to drive all stream effects timers. Default: 100ms.
uint8_t i_barrel_light = 0; // Used to keep track which LED in the barrel is currently lighting up.
uint8_t i_pulse_step = 0; // Used to keep track of which pulse animation step we are on.
uint8_t i_slime_tether_count = 0; // Used to keep track of how many slime tethers have been fired.
uint16_t i_last_firing_effect_mix = 0; // Used by standalone Neutrona Wand.

/*
 * Wand power level. Controlled by the rotary encoder on the top of the wand.
 * You can enable or disable overheating for each power level individually in the user adjustable values at the top of this file.
 */
const uint8_t i_power_level_max = 5;
const uint8_t i_power_level_min = 1;
uint8_t i_power_level = 1;
uint8_t i_power_level_prev = 1;

/*
 * Wand / Pack communication
 */
SerialTransfer wandComs;

/*
 * Wand Connection State
 * Used to identify the state of the wand as it connects to a Proton Pack.
 * These should be mutually exclusive and non-overlapping states for the wand communications.
 */
enum WAND_CONN_STATES { NC_BENCHTEST, PACK_DISCONNECTED, SYNCHRONIZING, PACK_CONNECTED };
enum WAND_CONN_STATES WAND_CONN_STATE;

/*
 * Some pack flags which get transmitted to the wand depending on the pack status.
 */
bool b_pack_on = false; // Denotes the pack has been powered on.
bool b_pack_alarm = false; // Denotes the pack alarm is sounding (ribbon cable disconnected).
bool b_pack_ion_arm_switch_on = false; // For MODE_ORIGINAL. Lets us know if the Proton Pack Ion Arm switch is on to give power to the pack & wand.
bool b_pack_cyclotron_lid_on = false; // For SYSTEM_FROZEN_EMPIRE. Lets us know if the pack's cyclotron lid is on or not. Default to false to favor FE effects unless told otherwise.
bool b_sync_light = false; // Toggle for the state of the white LED beside the vent light which gets blinked as a sync operation is attempted.
uint8_t i_cyclotron_speed_up = 1; // For telling the pack to speed up or slow down the Cyclotron lights.
millisDelay ms_packsync; // Timer for attempting synchronization with a connected pack.
millisDelay ms_handshake; // Timer for attempting a keepalive handshake with a connected pack.
const uint16_t i_sync_initial_delay = 750; // Delay to re-try the initial handshake with a proton pack.
const uint16_t i_heartbeat_delay = 3250; // Delay to send a heartbeat (handshake) to a connected proton pack.

/*
 * Wand Menu
 */
enum WAND_MENU_LEVELS { MENU_LEVEL_1, MENU_LEVEL_2, MENU_LEVEL_3, MENU_LEVEL_4, MENU_LEVEL_5 };
enum WAND_MENU_LEVELS WAND_MENU_LEVEL;
uint8_t i_wand_menu = 5;
const uint16_t i_settings_blinking_delay = 350;
millisDelay ms_settings_blinking;

/*
 * Misc wand settings and flags.
 */
bool b_firing = false; // Check for general firing state.
bool b_firing_intensify = false; // Check for Intensify button activity.
bool b_firing_alt = false; // Check for Barrel Wing Button firing activity for CTS.
bool b_firing_cross_streams = false; // Check for CTS firing activity.
bool b_firing_semi_automatic = false; // Check for semi-automatic firing modes.
bool b_sound_firing_intensify_trigger = false;
bool b_sound_firing_alt_trigger = false;
bool b_sound_firing_cross_the_streams = false;
bool b_sound_firing_cross_the_streams_mix = false;
bool b_sound_idle = false;
bool b_beeping = false;
bool b_sound_afterlife_idle_2_fade = true;
bool b_wand_boot_error_on = false;

/*
 * Button Mashing Lock-out - Prevents excessive user input via the primary/secondary firing buttons.
 * This ensures that the user is not exceeding what would be considered "normal" for firing of the wand,
 * otherwise an error mode will be engaged to provide a cool-down period. This does not apply to any
 * prolonged firing which would trigger the overheat or venting sequences; only rapid firing bursts.
 */
millisDelay ms_bmash;                  // Timer for the button mash lock-out period.
uint16_t i_bmash_delay = 2000;     // Time period in which we consider rapid firing.
uint16_t i_bmash_cool_down = 3000; // Time period for the lock-out of user input.
uint8_t i_bmash_count = 0;             // Current count for rapid firing bursts.
uint8_t i_bmash_max = 7;               // Burst count we consider before the lock-out.
uint8_t i_bmash_spark_index = 0;       // Current spark number for the spark effect (0~2).
bool b_wand_mash_error = false;        // Indicates if wand is in a lock-out phase.

/*
 * Used during the overheating sequences.
*/
millisDelay ms_blink_sound_timer_1;
millisDelay ms_blink_sound_timer_2;
const uint16_t i_blink_sound_timer = 400;

/*
 * A timer to turn on some Neutrona Wand lights when the system is shut down after some inactivity, as a reminder you left your power on to the system.
*/
millisDelay ms_power_indicator;
millisDelay ms_power_indicator_blink;
const uint32_t i_ms_power_indicator = 60000; // 1 Minute -> 60000
const uint16_t i_ms_power_indicator_blink = 1000;

/*
 * Function prototypes.
 */
void wandSerialSend(uint8_t i_command, uint16_t i_value);
void wandSerialSend(uint8_t i_command);
void wandSerialSendData(uint8_t i_message);
void checkPack();
void checkWandAction();
void ventSwitched(void* n = nullptr);
void wandSwitched(void* n = nullptr);
