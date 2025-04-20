/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
bool b_pack_started_by_meter = false; // Whether the pack was started via detection through the power meter.
bool b_wand_just_started = false; // Whether the wand was just started via the power meter, used to debounce the startup process.
bool b_wand_overheated = false; // Whether the wand overheated, as if it did we should ignore power off events.
const uint16_t i_wand_overheat_delay = 14600; // How many milliseconds of continuous firing before we lock into overheating mode.
const uint16_t i_wand_overheat_duration = 2500; // How long to play the alarm for before going into the full overheat sequence on the pack.
const uint16_t i_wand_startup_delay = 2750; // How many milliseconds after wand startup before we allow detecting firing events.
const float f_ema_alpha = 0.2; // Smoothing factor (<1) for Exponential Moving Average (EMA) [Lower Value = Smoother Averaging].
float f_sliding_window[20] = {0.0}; // Sliding window for detecting state changes, initialized to 0.
float f_accumulator = 0.0; // Accumulator used for sliding window averaging operations.
float f_diff_average = 0.0; // Stores the result of the sliding window average operation.

// Define an object which can store
struct PowerMeter {
  float ShuntVoltage = 0; // mV - Millivolts read to calculate the amperage draw across the shunt resistor
  float ShuntCurrent = 0; // A - The current (amperage) reading via the shunt resistor
  float BusVoltage = 0;   // mV - Voltage reading from the measured device
  float BattVoltage = 0;  // V - Reference voltage from device power source
  float BusPower = 0;     // W - Calculation of power based on the bus mV*A values
  float AmpHours = 0;     // Ah - An estimation of power consumed over regular intervals
  float RawPower = 0;     // W - Calculation of power based on raw V*A values (non-smoothed)
  float AvgPower = 0;     // A - Running average from the RawPower value (smoothed)
  uint16_t PowerReadDelay = 20; // How often (ms) to read levels for changes
  unsigned long LastRead = 0;     // Used to calculate Ah consumed since battery power-on
  unsigned long ReadTick = 0;     // Difference of current read time - last read
  millisDelay ReadTimer;          // Timer for reading latest values from power meter
};

// Create instances of the PowerMeter object.
PowerMeter wandReading;
PowerMeter packReading;

// Forward function declarations.
void packStartup(bool firstStart);
void wandFiring();
void wandStoppedFiring();
void cyclotronSpeedRevert();
void packOverheatingStart();

// Configure and calibrate the power meter device.
void powerMeterConfig() {
  debugln(F("Configure Power Meter"));

  // Custom configuration, defaults are RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  monitor.configure(INA219::RANGE_16V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);

  // Calibrate with our chosen values
  monitor.calibrate(SHUNT_R, SHUNT_MAX_V, BUS_MAX_V, MAX_CURRENT);
}

// Initialize the power meter device on the i2c bus.
void powerMeterInit() {
  // Configure the PowerMeter object(s).
  packReading.PowerReadDelay = 4000; // Read from the pack every 4s instead of every 20ms.

  uint8_t i_monitor_status = monitor.begin();

  debugln(F(" "));
  debug(F("Power Meter Result: "));
  debugln(i_monitor_status);

  if(i_monitor_status == 0) {
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

  // Always obtain a voltage reading directly from the pack PCB.
  packReading.ReadTimer.start(packReading.PowerReadDelay);
}

// Perform a reading of values from the power meter for the wand.
void doWandPowerReading() {
  // Slide the 20-parameter-wide window.
  for(uint8_t i = 0; i < 19; i++) {
    f_sliding_window[i] = f_sliding_window[i+1];
  }

  // Reset the accumulator.
  f_accumulator = 0.0;

  // Only uncomment this debug if absolutely needed!
  //debugln(F("Reading Power Meter"));

  // Reads the latest values from the monitor.
  wandReading.ShuntVoltage = monitor.shuntVoltage();
  wandReading.ShuntCurrent = monitor.shuntCurrent();
  wandReading.BusVoltage = monitor.busVoltage();
  wandReading.BusPower = monitor.busPower();

  // Update the smoothed current (A) values using the latest reading using an exponential moving average.
  wandReading.BattVoltage = wandReading.BusVoltage + wandReading.ShuntVoltage; // Total Volts
  wandReading.RawPower = wandReading.BattVoltage * wandReading.ShuntCurrent; // P(W) = V*A
  wandReading.AvgPower = (f_ema_alpha * wandReading.RawPower) + ((1 - f_ema_alpha) * wandReading.AvgPower);

  // Add the latest EMA'd reading to the end of the window.
  f_sliding_window[19] = wandReading.AvgPower;

  // Use time and current (A) values to calculate amp-hours consumed.
  unsigned long i_new_time = millis();
  wandReading.ReadTick = i_new_time - wandReading.LastRead;
  wandReading.AmpHours += (wandReading.ShuntCurrent * wandReading.ReadTick) / 3600000.0; // Div. by 1000 x 60 x 60
  wandReading.LastRead = i_new_time;

  // Prepare for next read -- this is security just in case the INA219 is reset by transient current.
  monitor.recalibrate();
  monitor.reconfig();
}

// Sourced from https://community.particle.io/t/battery-voltage-checking/5467
// Obtains the ATMega chip's actual Vcc voltage value, using internal bandgap reference.
// This demonstrates ability to read MCU's Vcc voltage and the ability to maintain A/D calibration with changing Vcc.
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
  // Obtain bandgap voltage from the microcontroller.
  doPackVoltageReading();
}

// Take actions based on current power state, specifically when there is no GPStar Neutrona Wand connected.
void updateWandPowerState() {
  static uint8_t si_update; // Static var to keep up with update requests for responding to the latest readings.
  si_update = (si_update + 1) % 20; // Keep a count of updates, rolling over every 20th time.

  // Every X updates send the averaged, stable value which would determine a state change.
  // This is called whenever the power meter is available--for wand hot-swapping purposes.
  // Data is sent as integer so this is sent multiplied by 100 to get 2 decimal precision.
  if(si_update == 0) {
    serial1Send(A_WAND_POWER_AMPS, f_sliding_window[19] * 100);
  }

  // Handle packside overheat sequence.
  if(b_wand_overheated) {
    if(ms_delay_post_2.justFinished()) {
      if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
        // Stop alarm sound.
        stopEffect(S_PACK_BEEPS_OVERHEAT);
      }

      // Enter overheat sequence.
      b_wand_overheated = false;
      wandStoppedFiring();
      cyclotronSpeedRevert();
      packOverheatingStart();
      ms_delay_post_2.stop();
    }
  }

  // Handle wand overheating sequence. Hasbro wand locks into overheating at 15 seconds of continuous fire.
  if(ms_delay_post_2.justFinished() && !b_wand_overheated) {
    // We're locked into the overheating sequence. Start playing the overheat sound.
    switch(SYSTEM_YEAR) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
        playEffect(S_PACK_BEEPS_OVERHEAT, true);
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
      default:
        playEffect(S_BEEP_8);
      break;
    }

    b_wand_overheated = true;
    ms_delay_post_2.start(i_wand_overheat_duration);
  }

  // First we check for wand on/off using a 3-parameter-wide window.
  for(uint8_t i = 17; i < 19; i++) {
    f_accumulator += (f_sliding_window[i+1] - f_sliding_window[i]);
  }

  // Average the change between both pairs of values.
  f_diff_average = f_accumulator / 2.0;
  f_accumulator = 0.0; // Reset the accumulator.

  if(!b_wand_on) {
    // Also we'll do a 20-parameter average just in case we missed the initial spike.
    for(uint8_t i = 0; i < 20; i++) {
      f_accumulator += f_sliding_window[i];
    }

    float f_on_average = f_accumulator / 20.0;
    f_accumulator = 0.0; // Reset the accumulator.

    if(f_diff_average > 0.09 || (f_diff_average > 0.002 && f_on_average > 0.8)) {
      // We need to poison the window after detecting a startup to prevent false firing triggers.
      f_diff_average = 0.0;
      for (uint8_t i = 0; i < 20; i++) {
        f_sliding_window[i] = 0.0;
      }

      // Wand must have been fully activated, so set variables accordingly.
      b_wand_on = true;
      b_wand_just_started = true;

      // The Hasbro wand cannot fire for 2.75 seconds after activation, so add a null period.
      ms_delay_post_3.start(i_wand_startup_delay);

      // Turn the pack on.
      if(PACK_STATE != MODE_ON) {
        packStartup(false);
        b_pack_started_by_meter = true;
        b_wand_overheated = false;

        // Fake a full-power proton stream setting to the Attenuator
        serial1Send(A_POWER_LEVEL_5);
        serial1Send(A_PROTON_MODE);

        // Tell the Attenuator the pack is powered on
        serial1Send(A_PACK_ON);
      }
    }
    else if(b_pack_started_by_meter) {
      // If we did not turn on, we can't have been started by the meter.
      b_pack_started_by_meter = false;
    }
  }
  else {
    if(ms_delay_post_3.justFinished()) {
      // Startup delay timer expired, so reset the "just started" flag.
      b_wand_just_started = false;
    }

    if((f_diff_average < -0.02 && f_sliding_window[19] < 0.55) || f_sliding_window[19] < 0.1) {
      if(!b_wand_overheated && !b_overheating) {
        // Wand must have been shut off.
        if(b_wand_firing) {
          // Stop firing sequence if previously firing.
          wandStoppedFiring();

          // Return cyclotron to normal speed.
          cyclotronSpeedRevert();
        }

        // Turn the pack off.
        if(PACK_STATE != MODE_OFF) {
          PACK_ACTION_STATE = ACTION_OFF;
          serial1Send(A_PACK_OFF);
        }
      }

      b_wand_on = false;
      b_pack_started_by_meter = false;
    }
    else if(PACK_STATE == MODE_OFF) {
      b_pack_started_by_meter = false; // Make sure this is kept as false since the pack was manually shut down.
    }
    else if(!b_wand_just_started) {
      if(!b_wand_firing && !b_wand_overheated && !b_overheating) {
        // Start firing checks use an 11-parameter-wide window.
        for (uint8_t i = 9; i < 19; i++) {
          if (((f_sliding_window[i + 1] - f_sliding_window[i]) <= 0.002) || (f_sliding_window[i + 1] - f_sliding_window[i]) > 0.07) {
            // If we went negative or jumped too quickly, reset the accumulator and exit.
            f_accumulator = 0.0;
            break;
          }

          f_accumulator += (f_sliding_window[i + 1] - f_sliding_window[i]);
        }

        f_diff_average = (f_accumulator / 4.0) * 1000.0;
        f_accumulator = 0.0; // Reset the accumulator.
        float f_range = f_sliding_window[19] - f_sliding_window[9]; // Store the range of the window.
        bool b_positive_rate = false; // Temp flag to determine if all detected diffs are positive.

        for(uint8_t i = 9; i < 19; i++) {
          if(f_sliding_window[i + 1] - f_sliding_window[i] < 0.0) {
            // If any diff in the window is negative, stop checking.
            b_positive_rate = false;
            break;
          }
          else if(i == 18) {
            // If we got here, we must be entirely positive.
            b_positive_rate = true;
          }
        }

        if (f_diff_average > 28.5 && f_diff_average < 45.0 || (f_range > 0.26f && b_positive_rate)) {
          // With this big a jump, we must have started firing.
          ms_delay_post_2.start(i_wand_overheat_delay);
          i_wand_power_level = 5;
          b_firing_intensify = true;
          wandFiring();
        }
      }
      else if(!b_wand_overheated && !b_overheating) {
        // Stop firing checks use a 20-parameter-wide window.
        for (uint8_t i = 0; i < 19; i++) {
          f_accumulator += (f_sliding_window[i + 1] - f_sliding_window[i]);
        }

        f_diff_average = (f_accumulator / 19.0) * 1000.0;
        f_accumulator = 0.0; // Reset the accumulator.
        bool b_negative_rate = false; // Temp flag to determine if all detected diffs are negative.

        for(uint8_t i = 0; i < 19; i++) {
          if(f_sliding_window[i + 1] - f_sliding_window[i] > 0.0) {
            // If any diff in the window is positive, stop checking.
            b_negative_rate = false;
            break;
          }
          else if(i == 18) {
            // If we got here, we must be entirely negative.
            b_negative_rate = true;
          }
        }

        if (f_diff_average <= -7.5f && b_negative_rate) {
          // We must have stopped firing.
          wandStoppedFiring();

          // Return cyclotron to normal speed.
          cyclotronSpeedRevert();
        }
      }
    }
  }
}

// Send latest voltage value to the serial1 device, if connected.
void updatePackPowerState() {
  if(b_serial1_connected) {
    // Data is sent as uint16_t so this is already multiplied by 100 to get 2 decimal precision.
    serial1Send(A_BATTERY_VOLTAGE_PACK, packReading.BusVoltage);
  }
}

// Displays the latest gathered power meter values (for debugging only!).
// Turn on the Serial Plotter in the ArduinoIDE to view graphed results.
void wandPowerDisplay() {
  if(b_show_power_data) {
    // Serial.print(F("W.Shunt(mV):"));
    // Serial.print(wandReading.ShuntVoltage, 4);
    // Serial.print(F(","));

    // Serial.print(F("W.Shunt(A):"));
    // Serial.print(wandReading.ShuntCurrent, 4);
    // Serial.print(F(","));

    Serial.print(F("W.Raw(W):"));
    Serial.print(wandReading.RawPower, 4);
    Serial.print(F(","));

    // Serial.print(F("W.Bus(V)):"));
    // Serial.print(wandReading.BusVoltage, 4);
    // Serial.print(F(","));

    // Serial.print(F("W.Bus(W)):"));
    // Serial.print(wandReading.BusPower, 4);
    // Serial.print(F(","));

    // Serial.print(F("W.Batt(V):"));
    // Serial.print(wandReading.BattVoltage, 4);
    // Serial.print(F(","));

    // Serial.print(F("W.AmpHours:"));
    // Serial.print(wandReading.AmpHours, 4);
    // Serial.print(F(","));

    Serial.print(F("W.AvgPow(W):"));
    Serial.print(wandReading.AvgPower, 4);
    Serial.print(F(","));
  }
}

// Check the available timers for reading power meter data.
void checkPowerMeter() {
  if(wandReading.ReadTimer.justFinished()) {
    // Only perform GPStar Lite functions if a GPStar Neutrona Wand is not connected.
    if(!b_wand_connected && !b_wand_syncing) {
      doWandPowerReading(); // Get latest V/A readings.
      wandPowerDisplay(); // Show values on serial plotter.
      updateWandPowerState(); // Take action on V/A values.
    }
    else {
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

    wandReading.ReadTimer.start(wandReading.PowerReadDelay);
    //wandReading.ReadTimer.repeat();
  }

  if(packReading.ReadTimer.justFinished()) {
    doPackPowerReading(); // Get latest voltage reading.
    updatePackPowerState(); // Take action on V/A values.
    packReading.ReadTimer.repeat();
  }
}