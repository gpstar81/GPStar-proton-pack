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
#define MAX_CURRENT 2.0  // Sets the expected max amperage draw

/*
 * Power Meter (using the INA219 chip)
 * https://github.com/flav1972/ArduinoINA219
 */
INA219 monitor; // Power monitor object on i2c bus using the INA219 chip.
boolean b_power_meter_avail = false; // Whether a power meter device exists on i2c bus.
const uint8_t i_power_reading_delay = 100; // How often to read the power levels (ms).
const uint8_t i_power_action_delay = 200; // How often to take action on levels (ms).
millisDelay ms_power_reading; // Timer for reading latest values from power meter.
millisDelay ms_power_actions; // Timer for reading latest values from power meter.
millisDelay ms_pack_power; // Timer for reading latest values from power meter.
struct PowerMeter {
  float f_ShuntVoltage = 0; // mV
  float f_ShuntCurrent = 0; // A
  float f_BusVoltage = 0; // V
  float f_BattVoltage = 0; // V
  float f_PackVoltage = 0; // V
  float f_BusPower = 0; // mW
  float f_AmpHours = 0; // Ah
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
      powerConfig();
      meterReading.i_last_read = millis(); // For use with the Ah readings.
      ms_power_reading.start(i_power_reading_delay);
      ms_power_actions.start(i_power_action_delay);
    }
    else {
      // If returning a non-zero value, device could not be reset.
      debugln(F("Unable to find power monitoring device on i2c."));
    }
  }

  // Obtain a voltage reading directly from the pack PCB.
  ms_pack_power.start(i_power_reading_delay * 50);
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
  if (b_power_meter_avail){
    // Only uncomment this debug if absolutely needed!
    //debugln(F("Reading Power Meter"));

    // Reads the latest values from the monitor.  
    meterReading.f_ShuntVoltage = monitor.shuntVoltage() * 1000;
    meterReading.f_ShuntCurrent = monitor.shuntCurrent();
    meterReading.f_BusVoltage = monitor.busVoltage();
    meterReading.f_BattVoltage = meterReading.f_BusVoltage + (meterReading.f_ShuntVoltage / 1000);
    meterReading.f_BusPower = monitor.busPower() * 1000;

    // Use time and values to calculate Ah estimate.
    unsigned long i_new_time = millis();
    meterReading.i_read_tick = i_new_time - meterReading.i_last_read;
    meterReading.f_AmpHours += (meterReading.f_ShuntCurrent * meterReading.i_read_tick) / 3600000.0;
    meterReading.i_last_read = i_new_time;

    if (b_show_power_data){
      // Prints values for use with the Serial Plotter to graph the data.
      Serial.print("Volts:");
      Serial.print(meterReading.f_BusVoltage);
      Serial.print(",Amps:");
      Serial.println(meterReading.f_ShuntCurrent);
    }

    // Prepare for next read -- this is security just in case the ina219 is reset by transient current
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
     * - Lower Toggle: Spike to 0.08-0.11A
     * - Upper Toggle: Spike to 0.09-0.12A
     * - Activate: Range 0.14-0.20A
     * - Firing: Range 0.25-0.40A
     */
    if(meterReading.f_ShuntCurrent > 0.12) {
      b_wand_on = true;

      // Turn the pack on.
      if(PACK_STATE != MODE_ON) {
        packStartup(false);
        serial1Send(A_PACK_ON);
      }

      if(meterReading.f_ShuntCurrent > 0.16) {
        // Wand is firing.
        wandFiring();
      }
      else {
        // Wand just stopped firing.
        if(b_wand_firing == true) {
          wandStoppedFiring();

          // Return cyclotron to normal speed.
          cyclotronSpeedRevert();
        }
      }
    }
    else {
      b_wand_on = false;

      // Turn the pack off.
      if(PACK_STATE != MODE_OFF) {
        PACK_ACTION_STATE = ACTION_OFF;
        serial1Send(A_PACK_OFF);
      }
    }
  }
}

// Check the current timers for reading power meter data.
void checkPowerMeter(){
  if(ms_power_reading.justFinished()) {
    if(b_power_meter_avail) {
      powerReading();
      ms_power_reading.start(i_power_reading_delay);
    }
  }

  if(ms_power_actions.justFinished()) {
      updatePowerState();
      ms_power_actions.start(i_power_action_delay);
  }

  if(ms_pack_power.justFinished()) {
      doVoltageCheck();
      ms_pack_power.start(i_power_reading_delay * 50);
  }
}

// Displays the latest gathered power meter values.
void powerDisplay() {
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