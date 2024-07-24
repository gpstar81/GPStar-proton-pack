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
#define MAX_CURRENT 3.2  // Device maximum is stated as 3.2A

/**
 * Power Meter (using the INA219 chip)
 * https://github.com/flav1972/ArduinoINA219
 */
INA219 monitor; // Power monitor object on i2c bus using the INA219 chip.
boolean b_power_meter = false; // Whether a power meter device exists on i2c bus.
const uint8_t i_power_reading_delay = 50; // How often to read the power levels (ms).
const uint16_t i_power_display_delay = 1000; // How often to display the power levels.
millisDelay ms_power_reading; // Timer for reading latest values from power meter.
millisDelay ms_power_display; // Timer for generating output from power readings.
int16_t i_ShuntVoltageRaw = 0;
int16_t i_BusVoltageRaw = 0;
float f_ShuntVoltage = 0; // mV
float f_ShuntCurrent = 0; // A
float f_BusVoltage = 0; // V
float f_BattVoltage = 0; // V
float f_BusPower = 0; // mW
float f_AmpHours = 0; // Ah
unsigned long i_power_last_read = 0; // Used to calculate Ah est.
unsigned long i_power_read_tick; // Current read time - last read

void powerConfig(){
  debugln(F("Configure Power Meter"));

  b_power_meter = true;

  // Custom configuration, defaults are RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  monitor.configure(INA219::RANGE_16V, INA219::GAIN_2_80MV, INA219::ADC_32SAMP, INA219::ADC_32SAMP, INA219::CONT_SH_BUS);
  
  // Calibrate with our chosen values
  monitor.calibrate(SHUNT_R, SHUNT_MAX_V, BUS_MAX_V, MAX_CURRENT);
}

void powerMeterInit(){
  uint8_t i_monitor_status = monitor.begin();

  debugln(" ");
  debug(F("Power Meter Result: "));
  debugln(i_monitor_status);

  if (i_monitor_status > 0){
    // If returning a non-zero value, device could not be reset.
    debugln(F("Unable to find power monitoring device."));
  }
  else {
    powerConfig();
    i_power_last_read = millis(); // Used with 
    ms_power_reading.start(i_power_reading_delay);
    ms_power_display.start(i_power_display_delay);
  }
}

// Perform a reading of current values from the power meter.
void powerReading(){
  if (!b_power_meter) { return; }

  debugln(F("Reading Power Meter"));

  unsigned long i_new_time;

  // Reads the latest values from the monitor.  
  i_ShuntVoltageRaw = monitor.shuntVoltageRaw();
  i_BusVoltageRaw = monitor.busVoltageRaw();
  f_ShuntVoltage = monitor.shuntVoltage() * 1000;
  f_ShuntCurrent = monitor.shuntCurrent();
  f_BusVoltage = monitor.busVoltage();
  f_BattVoltage = f_BusVoltage + (f_ShuntVoltage / 1000);
  f_BusPower = monitor.busPower() * 1000;

  // Use time and values to calculate Ah estimate.
  i_new_time = millis();
  i_power_read_tick = i_new_time - i_power_last_read;
  f_AmpHours += (f_ShuntCurrent * i_power_read_tick) / 3600000.0;
  i_power_last_read = i_new_time;

  // Prepare for next read -- this is security just in case the ina219 is reset by transient current
  monitor.recalibrate();
  monitor.reconfig();
}

// Displays the latest gathered power meter values.
void powerDisplay(){
  if (!b_power_meter) { return; }

  // Serial.print("Raw Shunt Voltage: ");
  // Serial.println(i_ShuntVoltageRaw);
  
  // Serial.print("Raw Bus Voltage:   ");
  // Serial.println(i_BusVoltageRaw);
  
  // Serial.println("--");
  
  Serial.print("Shunt Voltage: ");
  Serial.print(f_ShuntVoltage, 4);
  Serial.println(" mV");
  
  Serial.print("Shunt Current:  ");
  Serial.print(f_ShuntCurrent, 4);
  Serial.println(" A");
  
  Serial.print("Bus Voltage:    ");
  Serial.print(f_BusVoltage, 4);
  Serial.println(" V");

  Serial.print("Batt Voltage:   ");
  Serial.print(f_BattVoltage, 4);
  Serial.println(" V");

  Serial.print("Bus Power:      ");
  Serial.print(f_BusPower, 2);
  Serial.println(" mW");
  
  Serial.print("Amp Hours:      ");
  Serial.print(f_AmpHours, 4);
  Serial.println(" Ah");

  Serial.println(" ");
  Serial.println(" ");
}

// Check the current timers for reading power meter data.
void checkPowerMeter(){
  if(b_power_meter){
    if(ms_power_reading.justFinished()){
      powerReading();
      ms_power_reading.start(i_power_reading_delay);
    }

    if(ms_power_display.justFinished()){
      //powerDisplay();
      ms_power_display.start(i_power_display_delay);
    }
  }
}