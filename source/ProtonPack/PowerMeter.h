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
/*
 * Power Meter (using the INA219 chip)
 * https://github.com/flav1972/ArduinoINA219
 */
#include <INA219.h>

// Custom values for calibrating the current-sensing device.
#define SHUNT_R     0.1  // Shunt resistor in ohms (default: 0.1)
#define SHUNT_MAX_V 0.2  // Max voltage across shunt (default: 0.2)
#define BUS_MAX_V   16.0 // Sets max for a 12V battery (5V nominal)
#define MAX_CURRENT 1.0  // Sets the expected max amperage (A) draw

// General Variables
INA219 monitor; // Power monitor object on i2c bus using the INA219 chip.
bool b_power_meter_avail = false; // Whether a power meter device exists on i2c bus.
const uint16_t i_pack_reading_delay = 5000; // Multiplier for pack voltage readings.
const float f_power_on_threshold = 0.13; // Minimum current (A) to consider as to whether a stock Neutrona Wand is powered on.
const float f_ema_alpha = 0.2; // Smoothing factor (<1) for Exponential Moving Average (EMA) [Lower Value = Smoother Averaging].

// Timers
millisDelay ms_power_reading; // Timer for reading latest values from power meter.
millisDelay ms_pack_power; // Timer for reading latest voltage from the pack itself.
millisDelay ms_powerup_debounce; // Timer to lock out firing when the wand powers on.

// Define an object which can store
struct PowerMeter {
  const static uint8_t PowerReadDelay = 50; // How often to read the power levels (ms).
  const static uint16_t PowerupDelay = 1000; // How long to ignore firing after power-up (ms).
  const static uint16_t StateChangeDuration = 100; // Duration (ms) for a current change to persist for action.
  const static float StateChangeThreshold; // Minimum change in current (A) to consider as a potential state change.
  float ShuntVoltage = 0; // mV
  float ShuntCurrent = 0; // A
  float BusVoltage = 0; // V
  float BattVoltage = 0; // V
  float PackVoltage = 0; // V
  float BusPower = 0; // mW
  float AmpHours = 0; // Ah
  float LastAverage = 0; // A
  float AvgCurrent = 0; // A
  unsigned long StateChanged = 0; // Time when a potential state change was detected
  unsigned long LastRead = 0; // Used to calculate Ah used
  unsigned long ReadTick = 0; // Current read time - last read
};

// Set the static constant for considering changes to wand readings.
const float PowerMeter::StateChangeThreshold = 0.06;

// Create instance of the PowerMeter object.
PowerMeter wandReading;

// Forward function declarations.
void packStartup(bool firstStart);
void wandFiring();
void wandStoppedFiring();
void cyclotronSpeedRevert();

// Configure and calibrate the power meter device.
void powerMeterConfig() {
  debugln(F("Configure Power Meter"));

  b_power_meter_avail = true;

  // Custom configuration, defaults are RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  monitor.configure(INA219::RANGE_16V, INA219::GAIN_2_80MV, INA219::ADC_32SAMP, INA219::ADC_32SAMP, INA219::CONT_SH_BUS);

  // Calibrate with our chosen values
  monitor.calibrate(SHUNT_R, SHUNT_MAX_V, BUS_MAX_V, MAX_CURRENT);
}

// Initialize the power meter device on the i2c bus.
void powerMeterInit() {
  if (b_use_power_meter){
    uint8_t i_monitor_status = monitor.begin();

    debugln(" ");
    debug(F("Power Meter Result: "));
    debugln(i_monitor_status);

    if (i_monitor_status == 0){
      powerMeterConfig();
      wandReading.LastRead = millis(); // For use with the Ah readings.
      ms_power_reading.start(PowerMeter::PowerReadDelay);
    }
    else {
      // If returning a non-zero value, device could not be reset.
      debugln(F("Unable to find power monitoring device on i2c."));
    }
  }

  // Obtain a voltage reading directly from the pack PCB.
  ms_pack_power.start(i_pack_reading_delay);
}

// Sourced from https://community.particle.io/t/battery-voltage-checking/5467
// Obtains the ATMega chip's actual Vcc voltage value, using internal bandgap reference.
// This demonstrates ability to read processors Vcc voltage and the ability to maintain A/D calibration with changing Vcc.
void doPackVoltageCheck() {
  // REFS1 REFS0               --> 0 1, AVcc internal ref. -Selects AVcc reference
  // MUX4 MUX3 MUX2 MUX1 MUX0  --> 11110 1.1V (VBG)        -Selects channel 30, bandgap voltage, to measure
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR)| (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  // This appears to work without the delay, but for more accurate readings it may be necessary.
  // delay(50); // Let mux settle a little to get a more stable A/D conversion.

  ADCSRA |= _BV( ADSC ); // Start a conversion.
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) ); // Wait for conversion to complete...

  // Scale the value, which returns the actual value of Vcc x 100
  const long InternalReferenceVoltage = 1115L; // Adjust this value to your boards specific internal BG voltage x1000.
  wandReading.PackVoltage = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // Calculates for straight line value.

  // Send current voltage value to the serial1 device, if connected.
  if(b_serial1_connected) {
    serial1Send(A_BATTERY_VOLTAGE_PACK, wandReading.PackVoltage);
  }
}

// Perform a reading of current values from the power meter.
void doWandPowerReading() {
  if (b_use_power_meter && b_power_meter_avail){
    // Only uncomment this debug if absolutely needed!
    //debugln(F("Reading Power Meter"));

    // Reads the latest values from the monitor.  
    wandReading.ShuntVoltage = monitor.shuntVoltage() * 1000;
    wandReading.ShuntCurrent = monitor.shuntCurrent();
    wandReading.BusVoltage = monitor.busVoltage();
    wandReading.BattVoltage = wandReading.BusVoltage + (wandReading.ShuntVoltage / 1000);
    wandReading.BusPower = monitor.busPower() * 1000;

    // Create some new smoothed/averaged current (A) values using the latest reading.
    wandReading.AvgCurrent = f_ema_alpha * wandReading.ShuntCurrent + (1 - f_ema_alpha) * wandReading.AvgCurrent;

    // Use time and current values to calculate amp-hours consumed.
    unsigned long i_new_time = millis();
    wandReading.ReadTick = i_new_time - wandReading.LastRead;
    wandReading.AmpHours += (wandReading.ShuntCurrent * wandReading.ReadTick) / 3600000.0;
    wandReading.LastRead = i_new_time;

    if (b_show_power_data){
      // Prints values for use with the Serial Plotter to graph the data.
      // Serial.print("Volts:");
      // Serial.print(wandReading.BusVoltage);
      // Serial.print(",");
      Serial.print("AvgAmps:");
      Serial.print(wandReading.AvgCurrent);
      Serial.print(",");
      Serial.print("LastAvg:");
      Serial.println(wandReading.LastAverage);
    }

    // Prepare for next read -- this is security just in case the INA219 is reset by transient current.
    monitor.recalibrate();
    monitor.reconfig();
  }
  else {
    // Perform a check using the bandgap voltage.
    doPackVoltageCheck();
  }
}

// Take actions based on current power state, specifically if there is no GPStar Neutrona Wand connected.
void updateWandPowerState() {
  // Only take action when wand is NOT connected.
  if (b_use_power_meter && b_power_meter_avail && !b_wand_connected){
    /**
     * Amperage Ranges
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
    boolean b_state_change_lower = f_avg_current < wandReading.LastAverage - PowerMeter::StateChangeThreshold;
    boolean b_state_change_higher = f_avg_current > wandReading.LastAverage + PowerMeter::StateChangeThreshold;

    // Check for a significant and sustained change in current.
    if(b_state_change_lower || b_state_change_higher) {
      // Record the time when the significant change was first detected.
      if(wandReading.StateChanged == 0) {
        wandReading.StateChanged = current_time;
      }

      // Determine whether the change (+/-) took place over the expected timeframe.
      if(current_time - wandReading.StateChanged >= PowerMeter::StateChangeDuration) {
        // Update previous average current reading since we've had a sustained change in state.
        wandReading.LastAverage = f_avg_current;

        // Wand is considered "on" when above the base change.
        if(f_avg_current > f_power_on_threshold) {
          b_wand_on = true;

          // Turn the pack on.
          if(PACK_STATE != MODE_ON) {
            packStartup(false);

            // Fake a full-power setting to the Attenuator
            serial1Send(A_POWER_LEVEL_5);

            // Tell the Attenuator the pack is powered on
            serial1Send(A_PACK_ON);

            // Just powered up, so set a delay for firing.
            ms_powerup_debounce.start(PowerMeter::PowerupDelay);
          }
        }

        // If the wand and pack are considered "on" then determine whether firing or not.
        if(b_wand_on && PACK_STATE != MODE_OFF) {
          if(b_state_change_higher && !b_wand_firing && ms_powerup_debounce.remaining() < 1) {
            // State change was higher as means the wand is firing.
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
    if(f_avg_current <= f_power_on_threshold) {
      if(b_wand_firing) {
        // Stop firing sequence if previously firing.
        wandStoppedFiring();

        // Return cyclotron to normal speed.
        cyclotronSpeedRevert();
      }

      b_wand_on = false;

      // Turn the pack off.
      if(PACK_STATE != MODE_OFF) {
        PACK_ACTION_STATE = ACTION_OFF;
        serial1Send(A_PACK_OFF);
      }

      // Reset the state change timer and last average due to this significant event.
      wandReading.StateChanged = 0;
      wandReading.LastAverage = f_avg_current;
    }
  }
  else {
    // Reset when not using the power meter or a GPStar wand is connected.
    wandReading.StateChanged = 0;
    wandReading.LastAverage = 0;
  }
}

// Check the current timers for reading power meter data.
void checkPowerMeter(){
  if(ms_power_reading.justFinished()) {
    if(b_use_power_meter && b_power_meter_avail) {
      doWandPowerReading(); // Get latest V/A readings.
      updateWandPowerState(); // Take action on values.
      ms_power_reading.start(PowerMeter::PowerReadDelay);
    }
  }

  if(ms_pack_power.justFinished()) {
      doPackVoltageCheck(); // Get latest reading.
      ms_pack_power.start(i_pack_reading_delay);
  }
}

// Displays the latest gathered power meter values.
void wandPowerDisplay() {
  if(b_use_power_meter && b_power_meter_avail) {
    Serial.print("Shunt Voltage: ");
    Serial.print(wandReading.ShuntVoltage, 4);
    Serial.println(" mV");

    Serial.print("Shunt Current:  ");
    Serial.print(wandReading.ShuntCurrent, 4);
    Serial.println(" A");

    Serial.print("Bus Voltage:    ");
    Serial.print(wandReading.BusVoltage, 4);
    Serial.println(" V");

    Serial.print("Batt Voltage:   ");
    Serial.print(wandReading.BattVoltage, 4);
    Serial.println(" V");

    Serial.print("Bus Power:      ");
    Serial.print(wandReading.BusPower, 2);
    Serial.println(" mW");

    Serial.print("Amp Hours:      ");
    Serial.print(wandReading.AmpHours, 4);
    Serial.println(" Ah");

    Serial.println(" ");
  }
}