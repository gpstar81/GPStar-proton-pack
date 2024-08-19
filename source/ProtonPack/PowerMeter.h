#include "Communication.h"
/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

/**
 * Power Meter (using the INA219 chip) - https://github.com/flav1972/ArduinoINA219
 * Provides support for a power-sensing chip which can detect the voltage and current
 * being provided to the Neutrona Wand. Intended for those users who want to utilize
 * a stock wand but still trigger the pack power-on and firing animations/effects.
 */
#include <INA219.h>

// Custom values for calibrating the current-sensing device.
#define SHUNT_R     0.1  // Shunt resistor in ohms (default: 0.1ohm)
#define SHUNT_MAX_V 0.2  // Max voltage across shunt (default: 0.2V)
#define BUS_MAX_V   16.0 // Sets max based on expected range (< 16V)
#define MAX_CURRENT 2.0  // Sets the expected max amperage draw (2A)

// General Variables
INA219 monitor; // Power monitor object on i2c bus using the INA219 chip.
bool b_power_meter_available = false; // Whether a power meter device exists on i2c bus, per setup() -> powerMeterInit()
bool b_pack_started_by_meter = false; // Whether the pack was started via detection of the power meter.
const uint16_t f_wand_power_up_delay = 1000; // How long to wait and ignore any wand firing events after initial power-up (ms).
const float f_wand_power_on_threshold = 0.12; // Minimum current (A) to consider as to whether a stock Neutrona Wand is powered on.
const float f_ema_alpha = 0.2; // Smoothing factor (<1) for Exponential Moving Average (EMA) [Lower Value = Smoother Averaging].

// Special Timers and Timeouts
millisDelay ms_powerup_debounce; // Timer to lock out firing when the wand powers on.

// Define an object which can store
struct PowerMeter {
  const static uint16_t StateChangeDuration = 300; // Duration (ms) for a current change to persist for action
  const static float StateChangeThreshold; // Minimum change in current (A) to consider as a potential state change
  float ShuntVoltage = 0; // V - The value used to calculate the amperage draw across the shunt resistor
  float ShuntCurrent = 0; // A - The current (amperage) reading
  float BusVoltage = 0; // V - Voltage reading from the measured device
  float BattVoltage = 0; // V - Reference voltage from device power source
  float BusPower = 0; // W - Calculation of power based on the V*A values
  float AmpHours = 0; // Ah - An estimation of power consumed over regular intervals
  float AvgCurrent = 0; // A - Smoothed running average from the ShuntCurrent value
  float LastAverage = 0; // A - Last average used when determining a state change
  unsigned int PowerReadDelay = (int) (StateChangeDuration / 4); // How often (ms) to read the volt/power levels
  unsigned long StateChanged = 0; // Time when a potential state change was detected
  unsigned long LastRead = 0; // Used to calculate Ah consumed since battery power-on
  unsigned long ReadTick = 0; // Difference of current read time - last read
  millisDelay ReadTimer; // Timer for reading latest values from power meter
};

// Set the static constant for considering a "change" based on latest current reading average.
// IOW, if we measure a difference of this much since the last average, the user initiated input.
const float PowerMeter::StateChangeThreshold = 0.08;

// Create instances of the PowerMeter object.
PowerMeter wandReading;
PowerMeter packReading;

// Forward function declarations.
void packStartup(bool firstStart);
void wandFiring();
void wandStoppedFiring();
void cyclotronSpeedRevert();

// Configure and calibrate the power meter device.
void powerMeterConfig() {
  debugln(F("Configure Power Meter"));

  // Custom configuration, defaults are RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  monitor.configure(INA219::RANGE_16V, INA219::GAIN_2_80MV, INA219::ADC_32SAMP, INA219::ADC_32SAMP, INA219::CONT_SH_BUS);

  // Calibrate with our chosen values
  monitor.calibrate(SHUNT_R, SHUNT_MAX_V, BUS_MAX_V, MAX_CURRENT);
}

// Initialize the power meter device on the i2c bus.
void powerMeterInit() {
  // Configure the PowerMeter object(s).
  packReading.PowerReadDelay = 4000;

  if (b_use_power_meter){
    uint8_t i_monitor_status = monitor.begin();

    debugln(" ");
    debug(F("Power Meter Result: "));
    debugln(i_monitor_status);

    if (i_monitor_status == 0){
      // Result of 0 indicates no problems from device detection.
      b_power_meter_available = true;
      powerMeterConfig();
      wandReading.LastRead = millis(); // For use with the Ah readings.
      wandReading.ReadTimer.start(wandReading.PowerReadDelay);
    }
    else {
      // If returning a non-zero value, device could not be reset.
      debugln(F("Unable to find power monitoring device on i2c."));
    }
  }

  // Always obtain a voltage reading directly from the pack PCB.
  packReading.ReadTimer.start(packReading.PowerReadDelay);
}

// Perform a reading of values from the power meter for the wand.
void doWandPowerReading() {
  if (b_use_power_meter && b_power_meter_available){
    // Only uncomment this debug if absolutely needed!
    //debugln(F("Reading Power Meter"));

    // Reads the latest values from the monitor.
    wandReading.ShuntVoltage = monitor.shuntVoltage();
    wandReading.ShuntCurrent = monitor.shuntCurrent();
    wandReading.BusVoltage = monitor.busVoltage();
    wandReading.BattVoltage = wandReading.BusVoltage + (wandReading.ShuntVoltage);
    wandReading.BusPower = monitor.busPower();

    // Create some new smoothed/averaged current (A) values using the latest reading.
    wandReading.AvgCurrent = f_ema_alpha * wandReading.ShuntCurrent + (1 - f_ema_alpha) * wandReading.AvgCurrent;

    // Use time and current values to calculate amp-hours consumed.
    unsigned long i_new_time = millis();
    wandReading.ReadTick = i_new_time - wandReading.LastRead;
    wandReading.AmpHours += (wandReading.ShuntCurrent * wandReading.ReadTick) / 3600000.0; // Div. by 1000 x 60 x 60
    wandReading.LastRead = i_new_time;

    // Prepare for next read -- this is security just in case the INA219 is reset by transient current.
    monitor.recalibrate();
    monitor.reconfig();
  }
}

// Sourced from https://community.particle.io/t/battery-voltage-checking/5467
// Obtains the ATMega chip's actual Vcc voltage value, using internal bandgap reference.
// This demonstrates ability to read processors Vcc voltage and the ability to maintain A/D calibration with changing Vcc.
void doPackVoltageReading() {
  // REFS1 REFS0               --> 0 1, AVcc internal ref. -Selects AVcc reference
  // MUX4 MUX3 MUX2 MUX1 MUX0  --> 11110 1.1V (VBG)        -Selects channel 30, bandgap voltage, to measure
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR)| (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  // This appears to work without the delay, but for more accurate readings it may be necessary.
  // delay(50); // Let mux settle a little to get a more stable A/D conversion.

  ADCSRA |= _BV( ADSC ); // Start a conversion.
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) ); // Wait for conversion to complete...

  // Scale the value, which returns the actual value of Vcc x 100
  const long InternalReferenceVoltage = 1115L; // Adjust this value to your boards specific internal BG voltage x1000.
  packReading.BusVoltage = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // Calculates for straight line value.
}

// Perform a reading of values from the power meter for the pack.
void doPackPowerReading() {
  // When not using a device on the i2c bus, obtain bandgap voltage from the processor.
  doPackVoltageReading();
}

// Take actions based on current power state, specifically when there is no GPStar Neutrona Wand connected.
void updateWandPowerState() {
  static uint8_t si_update; // Static var to keep up with update requests for responding to the latest readings.
  si_update = (si_update + 1) % 20; // Keep a count of updates, rolling over every 20th time.

  // Only take action to read power consumption when wand is NOT connected.
  if (!b_wand_connected){
    /**
     * Amperage Ranges
     * Note there is some slight overlap between the highest power levels at idle and the lowest firing states.
     * Because of this, we cannot simply assume a value which falls into any given range is a specific event,
     * and we must use a state-change check based on a significant AND sustained change in amperage drawn.
     *
     * Level 1 Idle: 0.13-0.15A
     * Level 2 Idle: 0.14-0.18A
     * Level 3 Idle: 0.17-0.20A
     * Level 4 Idle: 0.19-0.22A
     * Level 5 Idle: 0.21-0.25A
     *
     * Level 1 Fire: 0.23-0.27A
     * Level 2 Fire: 0.26-0.30A
     * Level 3 Fire: 0.29-0.33A
     * Level 4 Fire: 0.30-0.35A
     * Level 5 Fire: 0.34-0.45A
     */
    float f_avg_current = wandReading.AvgCurrent;
    unsigned long current_time = millis();
    bool b_state_change_lower = f_avg_current < wandReading.LastAverage - PowerMeter::StateChangeThreshold;
    bool b_state_change_higher = f_avg_current > wandReading.LastAverage + PowerMeter::StateChangeThreshold;

    // Check for a significant and sustained change in current (either higher or lower than the last state).
    if(b_state_change_lower || b_state_change_higher) {
      // Record the time when the significant change was first detected.
      if(wandReading.StateChanged == 0) {
        wandReading.StateChanged = current_time;
      }

      // Determine whether the change (+/-) took place over the expected timeframe.
      if(current_time - wandReading.StateChanged >= PowerMeter::StateChangeDuration) {
        // Update previous average current reading since we've had a sustained change in state.
        wandReading.LastAverage = f_avg_current;

        // Wand is considered "on" when above the base threshold.
        if(f_avg_current > f_wand_power_on_threshold) {
          b_wand_on = true;

          // Turn the pack on.
          if(PACK_STATE != MODE_ON) {
            packStartup(false);
            b_pack_started_by_meter = true;

            // Fake a full-power setting to the Attenuator
            serial1Send(A_POWER_LEVEL_5);

            // Tell the Attenuator the pack is powered on
            serial1Send(A_PACK_ON);

            // Just powered up, so set a delay for firing.
            ms_powerup_debounce.start(f_wand_power_up_delay);
          }
        }

        // If the wand and pack are considered "on" then determine whether firing or not.
        if(b_wand_on && PACK_STATE != MODE_OFF) {
          if(b_state_change_higher && !b_wand_firing && ms_powerup_debounce.remaining() < 1) {
            // State change was higher as means the wand is firing (via intensify only).
            i_wand_power_level = 5;
            b_firing_intensify = true;
            wandFiring();
          }

          if(b_state_change_lower && b_wand_firing) {
            // State change was lower as means the wand stopped firing.
            wandStoppedFiring();

            // Return cyclotron to normal speed.
            cyclotronSpeedRevert();
          }
        }
      }
    }
    else {
      // Reset the state change timer if the change was not significant.
      wandReading.StateChanged = 0;
    }

    // Stop firing and turn off the pack if current is below the base threshold.
    if(f_avg_current <= f_wand_power_on_threshold) {
      if(b_wand_firing) {
        // Stop firing sequence if previously firing.
        wandStoppedFiring();

        // Return cyclotron to normal speed.
        cyclotronSpeedRevert();
      }

      b_wand_on = false;

      // Turn the pack off.
      if(PACK_STATE != MODE_OFF) {
        b_pack_started_by_meter = false;
        PACK_ACTION_STATE = ACTION_OFF;
        serial1Send(A_PACK_OFF);
      }

      // Reset the state change timer and last average due to this significant event.
      wandReading.StateChanged = 0;
      wandReading.LastAverage = f_avg_current;
    }

    // Every X updates send the averaged, stable value which would determine a state change.
    // This is called whenever the power meter is available--for wand hot-swapping purposes.
    // Data is sent as integer so this is sent multiplied by 100 to get 2 decimal precision.
    if(si_update == 0) {
      serial1Send(A_WAND_POWER_AMPS, wandReading.LastAverage * 100);
    }
  }
  else {
    // Reset when not using the power meter or a GPStar wand is connected.
    wandReading.StateChanged = 0;
    wandReading.LastAverage = 0;

    // If previously started via the power meter but a GPStar wand is connected,
    // then we need to power down the pack immediately as this was unintended.
    if(b_pack_started_by_meter && PACK_STATE != MODE_OFF) {
      b_wand_on = false;
      b_pack_started_by_meter = false;
      PACK_ACTION_STATE = ACTION_OFF;
      serial1Send(A_PACK_OFF);
      serial1Send(A_POWER_LEVEL_1);
      serial1Send(A_WAND_POWER_AMPS, 0);
    }
  }
}

// Send latest voltage value to the serial1 device, if connected.
void updatePackPowerState(){
  if(b_serial1_connected) {
    // Data is sent as integer so this is already multiplied by 100 to get 2 decimal precision.
    serial1Send(A_BATTERY_VOLTAGE_PACK, packReading.BusVoltage);
  }
}

// Displays the latest gathered power meter values (for debugging only!).
// Turn on the Serial Plotter in the ArduinoIDE to view graphed results.
void wandPowerDisplay() {
  if(b_use_power_meter && b_power_meter_available && b_show_power_data) {
    // Serial.print("W.Shunt(mv):");
    // Serial.print(wandReading.ShuntVoltage);
    // Serial.print(",");

    Serial.print("W.Power(A):");
    Serial.print(wandReading.ShuntCurrent);
    Serial.print(",");

    // Serial.print("W.Bus(V)):");
    // Serial.print(wandReading.BusVoltage);
    // Serial.print(",");

    Serial.print("W.Bus(mW)):");
    Serial.print(wandReading.BusPower);
    Serial.print(",");

    // Serial.print("W.Batt(V):");
    // Serial.print(wandReading.BattVoltage);
    // Serial.print(",");

    // Serial.print("W.AmpHours:");
    // Serial.print(wandReading.AmpHours);
    // Serial.print(",");

    Serial.print("W.AvgPow(A):");
    Serial.print(wandReading.AvgCurrent);
    Serial.print(",");

    Serial.print("W.State(A):");
    Serial.println(wandReading.LastAverage);
  }
}

// Check the available timers for reading power meter data.
void checkPowerMeter(){
  if(wandReading.ReadTimer.justFinished()) {
    if(b_use_power_meter && b_power_meter_available) {
      doWandPowerReading(); // Get latest V/A readings.
      wandPowerDisplay(); // Show values on serial plotter.
      updateWandPowerState(); // Take action on V/A values.
      wandReading.ReadTimer.start(wandReading.PowerReadDelay);
    }
  }

  if(packReading.ReadTimer.justFinished()) {
      doPackPowerReading(); // Get latest voltage reading.
      updatePackPowerState(); // Take action on V/A values.
      packReading.ReadTimer.start(packReading.PowerReadDelay);
  }
}