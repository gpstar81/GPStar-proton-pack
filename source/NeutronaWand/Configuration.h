/**
 *   gpstar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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
 * -------------****** CUSTOM USER CONFIGURABLE SETTINGS ******-------------
 * Change the variables below to alter the behaviour of your Neutrona wand.
 */

/*
 * You can set the default master startup volume for your wand here.
 * This gets overridden if you connect your wand to the pack.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
*/
const uint8_t STARTUP_VOLUME = 100;

/*
 * You can set the default music volume for your wand here.
 * This gets overridden if you connect your wand to the pack.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
*/
const uint8_t STARTUP_VOLUME_MUSIC = 100;

/*
 * You can set the default sound effects volume for your wand here.
 * This gets overridden if you connect your wand to the pack.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_EFFECTS = 100;

/*
 * Minimum volume that the Neutrona Wand can achieve. 
 * Values must be from 0 to -70. 0 = the loudest and -70 = the quietest.
 * Volume changes are based on percentages. 
 * If your pack is overpowering the wand at lower volumes, you can either increase the minimum value in the wand,
 * or decrease the minimum value for the pack.
*/
const int8_t MINIMUM_VOLUME = -35;

/*
 * Percentage increments of main volume change.
*/
const uint8_t VOLUME_MULTIPLIER = 2;

/*
 * Percentage increments of the music volume change..
*/
const uint8_t VOLUME_MUSIC_MULTIPLIER = 5;

/*
 * Percentage increments of the sound effects volume change.
*/
const uint8_t VOLUME_EFFECTS_MULTIPLIER = 5;

/*
 * When defined, when using the Barmeter 28 segment bargraph, it will invert the sequence.
 * Part #: BL28Z-3005SA04Y
 * Only compatible with the gpstar Neutrona Wand board, and not a Arduino Nano.
 * When using the Frutto Technologies 28 segment bargraph, this should be defined.
 
 * Defined / Enabled:
 #define GPSTAR_INVERT_BARGRAPH

 * NOT Defined / Disabled:
 //#define GPSTAR_INVERT_BARGRAPH
*/
#define GPSTAR_INVERT_BARGRAPH

/*
 * When set to true, the mode switch button to change firing modes changes to a alternate firing button.
 * Pressing this button together at the same time as the Intensify button does a cross the streams firing.
 * You can release one of the two firing buttons and the Neutrona wand will still continue to cross the streams.
 * The video game firing modes will be disabled when you enable this. 
 * This can be enabled or disabled from the Neutrona wand sub menu system.
*/
bool b_cross_the_streams = false;

/*
  * When set to true, to cross the streams you must be holding down both Intensify and the alternate firing button at the same time.
  * Releasing one or the other button will stop cross the streams but continue firing a regular proton stream.
  * This can be enabled or disabled from the Neutrona wand sub menu system.
*/
bool b_cross_the_streams_mix = false;

/*
 * When enabled, the Neutrona Wand will vibrate at all times.
 * Vibration is controlled by the vibration toggle switch in the Proton Pack and it can disable all vibration settings.
 * This can be enabled or disabled from the Neutrona wand sub menu system.
 * Default = true.
*/
bool b_vibration_on = true;

/*
 * When set to true, when b_vibration_on is set to true, the Neutrona wand will only vibrate during firing.
 * Note that vibration is controlled by the vibration toggle switch in the Proton Pack and it can disable all vibration settings.
 * This can be enabled or disabled from the Neutrona wand sub menu system.
 * Default = true.
*/
bool b_vibration_firing = true;

/*
 * Set to true to enable overheating. Overheat settings are defined below.
 * Set to false to disable overheating. This will override any overheat settings below.
 * This can be controle from the Neutrona Wand submenu system.
*/
bool b_overheat_enabled = true;

/*
 * Set to true to have your bargraph blink on/off when the Neutrona Wand and Proton Pack overheat.
 * When false, the bargraph will ramp down instead.
*/
bool b_overheat_bargraph_blink = false;

/*
 * Set to true to have your Neutrona wand boot up with errors when the top right switch (beep switch) is on while you are turning on your wand.
 * When set to false, this will be ignored.
*/
const bool b_wand_boot_errors = true;

/*
 * Which power modes do you want to be able to overheat.
 * Set to true to allow the wand and pack to overheat in that mode.
 * Set to false to disable overheating in that power mode. You will be able to continously fire instead.
 */
const bool b_overheat_mode_1 = false;
const bool b_overheat_mode_2 = false;
const bool b_overheat_mode_3 = false;
const bool b_overheat_mode_4 = false;
const bool b_overheat_mode_5 = true;

/*
 * Time in milliseconds for when overheating will initiate if enabled for that power mode.
 * Overheat only happens if enabled for that power mode (see above).
 * Example: 12000 = (12 seconds)
*/
const unsigned long int i_ms_overheat_initiate_mode_1 = 60000;
const unsigned long int i_ms_overheat_initiate_mode_2 = 30000;
const unsigned long int i_ms_overheat_initiate_mode_3 = 20000;
const unsigned long int i_ms_overheat_initiate_mode_4 = 15000;
const unsigned long int i_ms_overheat_initiate_mode_5 = 12000;

/*
 * Set this to true to be able to use your wand without a Proton Pack connected.
 * Otherwise set to false and the wand will wait until it is connected to a Proton Pack before it can activate.
*/
const bool b_no_pack = false;

/*
 * Set to false to disable the onboard amplifer on the wav trigger. 
 * Turning off the onboard amp draws less power. 
 * If using the AUX cable jack, the amp can be disabled to save power.
 * If you use the output pins directly on the wav trigger board to your speakers, you will need to enable the onboard amp.
 * NOTE: The On-board mono audio amplifier and speaker connector specifications: 2W into 4 Ohms, 1.25W into 8 Ohms
*/
const bool b_onboard_amp_enabled = true;

/*
 * Debug testing
 * Set to true to debug some switch readings.
 * Keep your wand unplugged from the pack while this is set to true.
 * It uses the USB port and tx/rx need to be free so serial information can be sent back to the Arduino IDE.
 * The wand will respond a bit slower as it is streaming serial data back. For debugging the analog switch readings only.
*/
const bool b_debug = false;

#ifdef GPSTAR_NEUTRONA_WAND_PCB
  /*
  * Set to false to ignore reading data from the EEPROM.
  */
  const bool b_eeprom = true;
#endif