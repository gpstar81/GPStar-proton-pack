/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS3TRC.h>

/*
 * Magnetometer and IMU
 * Defines all device objects and variables.
 */
Adafruit_LIS3MDL myMAG;
Adafruit_LSM6DS3TRC myIMU;
bool b_mag_found = false;
bool b_imu_found = false;
millisDelay ms_sensor_delay;
const uint16_t i_sensor_delay = 200; // Delay between sensor reads in milliseconds.

// Constant: FILTER_ALPHA
// Purpose: Controls the smoothing factor for exponential moving average filtering (0 < FILTER_ALPHA <= 1).
const float FILTER_ALPHA = 0.2f;

// Struct: MotionData
// Purpose: Holds all motion sensor readings for magnetometer, accelerometer, gyroscope, and calculated heading.
// Attributes:
//   - magX, magY, magZ: Magnetometer readings (uTesla)
//   - accelX, accelY, accelZ: Accelerometer readings (m/s^2)
//   - gyroX, gyroY, gyroZ: Gyroscope readings (rads/s)
//   - heading: Compass heading in degrees (0-360°), derived from magX and magY
struct MotionData {
  float magX;
  float magY;
  float magZ;
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;
  float heading;
};

// Global object to hold the latest raw sensor readings.
MotionData motionData;

// Global object to hold the latest filtered sensor readings.
MotionData filteredMotionData;

// Offset to correct magnetometer orientation on the PCB.
const float HEADING_OFFSET_DEG = 270.0f;

// Indicate if the device is mounted upside down (inverted); set true for installation orientation.
bool isDeviceInstalled = false; // true: Components DOWN (Installed), false: Components UP (Deveolopment)

// Function: calculateHeading
// Purpose: Computes the compass heading (in degrees) from magnetometer X and Y values, applying a device-specific offset and optional inversion for mounting.
// Inputs:
//   - float magX: Magnetometer X-axis reading
//   - float magY: Magnetometer Y-axis reading
// Outputs:
//   - float: Compass heading in degrees (0-360°)
float calculateHeading(float magX, float magY) {
  float headingRad = atan2(magY, magX);
  float headingDeg = headingRad * (180.0f / PI);

  // Apply offset for physical chip orientation
  headingDeg -= HEADING_OFFSET_DEG;

  // Normalize to 0-360°
  while (headingDeg < 0.0f) {
    headingDeg += 360.0f;
  }
  while (headingDeg >= 360.0f) {
    headingDeg -= 360.0f;
  }

  // Invert heading if device is mounted upside down
  if (isDeviceInstalled) {
    headingDeg = 360.0f - headingDeg;
    // Normalize again to 0-360°
    while (headingDeg < 0.0f) {
      headingDeg += 360.0f;
    }
    while (headingDeg >= 360.0f) {
      headingDeg -= 360.0f;
    }
  }

  return headingDeg;
}

// Function: updateFilteredMotionData
// Purpose: Applies exponential moving average filtering to raw motionData and updates filteredMotionData.
// Inputs: None (uses global motionData and filteredMotionData)
// Outputs: None (updates filteredMotionData)
void updateFilteredMotionData() {
  filteredMotionData.magX    = FILTER_ALPHA * motionData.magX    + (1.0f - FILTER_ALPHA) * filteredMotionData.magX;
  filteredMotionData.magY    = FILTER_ALPHA * motionData.magY    + (1.0f - FILTER_ALPHA) * filteredMotionData.magY;
  filteredMotionData.magZ    = FILTER_ALPHA * motionData.magZ    + (1.0f - FILTER_ALPHA) * filteredMotionData.magZ;
  filteredMotionData.accelX  = FILTER_ALPHA * motionData.accelX  + (1.0f - FILTER_ALPHA) * filteredMotionData.accelX;
  filteredMotionData.accelY  = FILTER_ALPHA * motionData.accelY  + (1.0f - FILTER_ALPHA) * filteredMotionData.accelY;
  filteredMotionData.accelZ  = FILTER_ALPHA * motionData.accelZ  + (1.0f - FILTER_ALPHA) * filteredMotionData.accelZ;
  filteredMotionData.gyroX   = FILTER_ALPHA * motionData.gyroX   + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroX;
  filteredMotionData.gyroY   = FILTER_ALPHA * motionData.gyroY   + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroY;
  filteredMotionData.gyroZ   = FILTER_ALPHA * motionData.gyroZ   + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroZ;
  filteredMotionData.heading = calculateHeading(motionData.magX, motionData.magY);
}

// Initialize the I2C bus for the Magnetometer and IMU.
void initializeMotionDevices() {
  Wire1.begin(IMU_SDA, IMU_SCL, 400000UL);

  // Initialize the LIS3MDL magnetometer.
  if(myMAG.begin_I2C(LIS3MDL_I2CADDR_DEFAULT, &Wire1)) {
    b_mag_found = true;
    myMAG.setPerformanceMode(LIS3MDL_MEDIUMMODE); // Set performance mode to medium
    myMAG.setOperationMode(LIS3MDL_CONTINUOUSMODE); // Set operation mode to continuous
    myMAG.setDataRate(LIS3MDL_DATARATE_1000_HZ); // Set data rate to 1000Hz
    myMAG.setRange(LIS3MDL_RANGE_4_GAUSS); // Set range to 4 Gauss
    myMAG.setIntThreshold(500); // Set interrupt threshold to 500
    myMAG.configInterrupt(false, false, true, true, false, true); // Configure interrupts
  }

  // Initialize the LSM6DS3TR-C IMU.
  if(myIMU.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1)) {
    b_imu_found = true;
    myIMU.setAccelRange(LSM6DS_ACCEL_RANGE_2_G); // Set accelerometer range to 2G
    myIMU.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS); // Set gyroscope range to 250DPS
    myIMU.setAccelDataRate(LSM6DS_RATE_6_66K_HZ); // Set accelerometer data rate to 6.66KHz
    myIMU.setGyroDataRate(LSM6DS_RATE_6_66K_HZ); // Set gyroscope data rate to 6.66KHz
    myIMU.configInt1(false, false, true); // Enable accelerometer data ready interrupt
    myIMU.configInt2(false, true, false); // Enable gyroscope data ready interrupt
  }
}

// Read the motion sensors and print the data to the debug console (if enabled).
void readMotionSensors() {
  if(b_imu_found && b_mag_found) {
    if(!ms_sensor_delay.isRunning()) {
      ms_sensor_delay.start(i_sensor_delay); // Have the IMU/MAG report every N milliseconds.
    }
    else if(ms_sensor_delay.justFinished()) {
      // Poll the sensors.
      sensors_event_t mag;
      sensors_event_t accel;
      sensors_event_t gyro;
      sensors_event_t temp;
      myMAG.getEvent(&mag);
      myIMU.getEvent(&accel, &gyro, &temp);

      // Update the raw sensor data as a global object.
      motionData.magX = mag.magnetic.x;
      motionData.magY = mag.magnetic.y;
      motionData.magZ = mag.magnetic.z;
      motionData.accelX = accel.acceleration.x;
      motionData.accelY = accel.acceleration.y;
      motionData.accelZ = accel.acceleration.z;
      motionData.gyroX = gyro.gyro.x;
      motionData.gyroY = gyro.gyro.y;
      motionData.gyroZ = gyro.gyro.z;

      // Update heading value based on magnetometer X and Y only.
      motionData.heading = calculateHeading(motionData.magX, motionData.magY);

      // Apply smoothing filter to sensor data.
      updateFilteredMotionData();

      // Print the filtered sensor data to the debug console.
      debug("\t\tFiltered Mag   X: ");
      debug(filteredMotionData.magX);
      debug(" \tY: ");
      debug(filteredMotionData.magY);
      debug(" \tZ: ");
      debug(filteredMotionData.magZ);
      debugln(" uTesla ");
      debug("\t\tFiltered Accel X: ");
      debug(filteredMotionData.accelX);
      debug(" \tY: ");
      debug(filteredMotionData.accelY);
      debug(" \tZ: ");
      debug(filteredMotionData.accelZ);
      debugln(" m/s^2 ");
      debug("\t\tFiltered Gyro  X: ");
      debug(filteredMotionData.gyroX);
      debug(" \tY: ");
      debug(filteredMotionData.gyroY);
      debug(" \tZ: ");
      debug(filteredMotionData.gyroZ);
      debugln(" rads/s ");
      debug("\t\tFiltered Heading: ");
      debug(filteredMotionData.heading);
      debugln(" deg ");
      debugln();
    }
  }
}
