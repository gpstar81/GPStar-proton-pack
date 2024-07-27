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

// Custom values for calibrating the power meter
#define SHUNT_R     0.1  // Shunt resistor in ohms (default: 0.1)
#define SHUNT_MAX_V 0.2  // Max voltage across shunt (default: 0.2)
#define BUS_MAX_V   16.0 // Sets max for a 12V battery (5V nominal)
#define MAX_CURRENT 1.0  // Sets the expected max amperage draw

/*
 * Power Meter (using the INA219 chip)
 * https://github.com/flav1972/ArduinoINA219
 */
// Variables
INA219 monitor; // Power monitor object on i2c bus using the INA219 chip.
boolean b_power_meter_avail = false; // Whether a power meter device exists on i2c bus.
const uint8_t i_power_reading_delay = 50; // How often to read the power levels (ms).
const uint8_t i_pack_reading_factor = 100; // Multiplier for pack voltage readings.
const uint16_t i_powerup_delay = 1000; // How long to ignore firing after power-up (ms).
const float f_ema_alpha = 0.1; // Smoothing factor for Exponential Moving Average (EMA) [Lower = Smoother].
const float f_power_on_threshold = 0.13; // Minimum current (A) to consider whether the wand is powered on.
const float f_current_change_threshold = 0.10; // Minimum change in current (A) to consider a state change.
const uint16_t i_state_change_duration = 200; // Duration (ms) for the current change to persist for action.
// Timers
millisDelay ms_power_reading; // Timer for reading latest values from power meter.
millisDelay ms_pack_power; // Timer for reading latest values from power meter.
millisDelay ms_powerup_debounce; // Timer for locking out firing when the wand powers on.
// Objects
struct PowerMeter {
  float f_ShuntVoltage = 0; // mV
  float f_ShuntCurrent = 0; // A
  float f_BusVoltage = 0; // V
  float f_BattVoltage = 0; // V
  float f_PackVoltage = 0; // V
  float f_BusPower = 0; // mW
  float f_AmpHours = 0; // Ah
  float f_LastAverage = 0; // A
  float f_AvgCurrent = 0; // A
  unsigned long i_state_change_start_time = 0; // Time when a potential state change was detected
  unsigned long i_last_read = 0; // Used to calculate Ah used
  unsigned long i_read_tick; // Current read time - last read
} meterReading;

// Forward function declarations.
void packStartup(bool firstStart);
void wandFiring();
void wandStoppedFiring();
void cyclotronSpeedRevert();

// Configure and calibrate the power meter device.
void powerConfig() {
  debugln(F("Configure Power Meter"));

  b_power_meter_avail = true;

  // Custom configuration, defaults are RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  monitor.configure(INA219::RANGE_16V, INA219::GAIN_4_160MV, INA219::ADC_128SAMP, INA219::ADC_128SAMP, INA219::CONT_SH_BUS);

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
      powerConfig();
      meterReading.i_last_read = millis(); // For use with the Ah readings.
      ms_power_reading.start(i_power_reading_delay);
    }
    else {
      // If returning a non-zero value, device could not be reset.
      debugln(F("Unable to find power monitoring device on i2c."));
    }
  }

  // Obtain a voltage reading directly from the pack PCB.
  ms_pack_power.start(i_power_reading_delay * i_pack_reading_factor);
}

// Sourced from https://community.particle.io/t/battery-voltage-checking/5467
// Obtains the ATMega chip's actual Vcc voltage value, using internal bandgap reference.
// This demonstrates ability to read processors Vcc voltage and the ability to maintain A/D calibration with changing Vcc.
void doVoltageCheck() {
  // REFS1 REFS0               --> 0 1, AVcc internal ref. -Selects AVcc reference
  // MUX4 MUX3 MUX2 MUX1 MUX0  --> 11110 1.1V (VBG)        -Selects channel 30, bandgap voltage, to measure
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR)| (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  // This appears to work without the delay, but for more accurate readings it may be necessary.
  // delay(50); // Let mux settle a little to get a more stable A/D conversion.

  ADCSRA |= _BV( ADSC ); // Start a conversion.
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) ); // Wait for conversion to complete...

  // Scale the value, which returns the actual value of Vcc x 100
  const long InternalReferenceVoltage = 1115L; // Adjust this value to your boards specific internal BG voltage x1000.
  meterReading.f_PackVoltage = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // Calculates for straight line value.

  // Send current voltage value to the serial1 device, if connected.
  if(b_serial1_connected) {
    serial1Send(A_BATTERY_VOLTAGE_PACK, meterReading.f_PackVoltage);
  }
}

// Perform a reading of current values from the power meter.
void powerReading() {
  if (b_use_power_meter && b_power_meter_avail){
    // Only uncomment this debug if absolutely needed!
    //debugln(F("Reading Power Meter"));

    // Reads the latest values from the monitor.  
    meterReading.f_ShuntVoltage = monitor.shuntVoltage() * 1000;
    meterReading.f_ShuntCurrent = monitor.shuntCurrent();
    meterReading.f_BusVoltage = monitor.busVoltage();
    meterReading.f_BattVoltage = meterReading.f_BusVoltage + (meterReading.f_ShuntVoltage / 1000);
    meterReading.f_BusPower = monitor.busPower() * 1000;

    // Create some new smoothed/averaged curent values using the latest reading.
    meterReading.f_AvgCurrent = f_ema_alpha * meterReading.f_ShuntCurrent + (1 - f_ema_alpha) * meterReading.f_AvgCurrent;

    // Use time and values to calculate Ah estimate.
    unsigned long i_new_time = millis();
    meterReading.i_read_tick = i_new_time - meterReading.i_last_read;
    meterReading.f_AmpHours += (meterReading.f_ShuntCurrent * meterReading.i_read_tick) / 3600000.0;
    meterReading.i_last_read = i_new_time;

    if (b_show_power_data){
      // Prints values for use with the Serial Plotter to graph the data.
      // Serial.print("Volts:");
      // Serial.print(meterReading.f_BusVoltage);
      // Serial.print(",");
      Serial.print("AvgAmps:");
      Serial.print(meterReading.f_AvgCurrent);
      Serial.print(",");
      Serial.print("LastAvg:");
      Serial.println(meterReading.f_LastAverage);
    }

    // Prepare for next read -- this is security just in case the INA219 is reset by transient current.
    monitor.recalibrate();
    monitor.reconfig();
  }
  else {
    // Perform a check using the bandgap voltage.
    doVoltageCheck();
  }
}

// Take actions based on current power state, specifically if there is no GPStar Neutrona Wand connected.
void updatePowerState() {
  // Only take action when wand is NOT connected.
  if (b_use_power_meter && b_power_meter_avail && !b_wand_connected){
    /**
     * Current Readings
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
    float f_avg_current = meterReading.f_AvgCurrent;
    unsigned long current_time = millis();
    boolean b_state_change_lower = f_avg_current < meterReading.f_LastAverage - f_current_change_threshold;
    boolean b_state_change_higher = f_avg_current > meterReading.f_LastAverage + f_current_change_threshold;

    // Check for a significant and sustained change in current.
    if(b_state_change_lower || b_state_change_higher) {
      // Record the time when the significant change was first detected.
      if(meterReading.i_state_change_start_time == 0) {
        meterReading.i_state_change_start_time = current_time;
      }

      // Determine whether the change (+/-) took place over the expected timeframe.
      if(current_time - meterReading.i_state_change_start_time >= i_state_change_duration) {
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
            ms_powerup_debounce.start(i_powerup_delay);
          }
        }

        // If the pack is considered "on" then determine whether firing or not.
        if(PACK_STATE != MODE_OFF) {
          if(b_state_change_higher && ms_powerup_debounce.remaining() < 1) {
            // State change was sustained higher as means the wand is firing.
            if(!b_wand_firing) {
              wandFiring();
            }
          }
          if(b_state_change_lower && b_wand_firing) {
            // State change was sustained higher as means the wand stopped firing.
            wandStoppedFiring();

            // Return cyclotron to normal speed.
            cyclotronSpeedRevert();
          }
        }
      }
    }
    else {
      // Reset the state change timer if the change was not significant.
      meterReading.i_state_change_start_time = 0;
    }

    // Update previous average current reading.
    meterReading.f_LastAverage = f_avg_current;

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

      // Reset the state change timer due to this significant event.
      meterReading.i_state_change_start_time = 0;
    }
  }
}

// Check the current timers for reading power meter data.
void checkPowerMeter(){
  if(ms_power_reading.justFinished()) {
    if(b_use_power_meter && b_power_meter_avail) {
      powerReading();
      updatePowerState();
      ms_power_reading.start(i_power_reading_delay);
    }
  }

  if(ms_pack_power.justFinished()) {
      doVoltageCheck();
      ms_pack_power.start(i_power_reading_delay * i_pack_reading_factor);
  }
}

// Displays the latest gathered power meter values.
void powerDisplay() {
  if(b_use_power_meter && b_power_meter_avail) {
    Serial.print("Shunt Voltage: ");
    Serial.print(meterReading.f_ShuntVoltage, 4);
    Serial.println(" mV");

    Serial.print("Shunt Current:  ");
    Serial.print(meterReading.f_ShuntCurrent, 4);
    Serial.println(" A");

    Serial.print("Bus Voltage:    ");
    Serial.print(meterReading.f_BusVoltage, 4);
    Serial.println(" V");

    Serial.print("Batt Voltage:   ");
    Serial.print(meterReading.f_BattVoltage, 4);
    Serial.println(" V");

    Serial.print("Bus Power:      ");
    Serial.print(meterReading.f_BusPower, 2);
    Serial.println(" mW");

    Serial.print("Amp Hours:      ");
    Serial.print(meterReading.f_AmpHours, 4);
    Serial.println(" Ah");

    Serial.println(" ");
  }
}