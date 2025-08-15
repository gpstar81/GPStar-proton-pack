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
#include <MadgwickAHRS.h>

// Forward function declarations.
void sendTelemetryData(); // From Webhandler.h

/**
 * Magnetometer and IMU
 * Defines all device objects and variables.
 */
Adafruit_LIS3MDL myMAG;
Adafruit_LSM6DS3TRC myIMU;
bool b_mag_found = false;
bool b_imu_found = false;
millisDelay ms_sensor_delay;
const uint16_t i_sensor_delay = 200; // Delay between sensor reads in milliseconds (also affects telemetry reporting).
const float HEADING_OFFSET_DEG = 270.0f; // Correct magnetometer orientation on the controller PCB.
bool isDeviceInstalled = false; // true: Components DOWN (Installed), false: Components UP (Development)

// Create a global filter object
Madgwick filter;

/**
 * Constant: FILTER_ALPHA
 * Purpose: Controls the smoothing factor for exponential moving average filtering (0 < FILTER_ALPHA <= 1).
 *          Increasing this value makes it more responsive to changes, decreasing smooths out fluctuations.
 */
const float FILTER_ALPHA = 0.2f;

/**
 * Struct: MotionData
 * Purpose: Holds all motion sensor readings for magnetometer, accelerometer, gyroscope, and calculated heading.
 * Attributes:
 *   - magX, magY, magZ: Magnetometer readings (uTesla)
 *   - accelX, accelY, accelZ: Accelerometer readings (m/s^2)
 *   - gyroX, gyroY, gyroZ: Gyroscope readings (rads/s)
 *   - heading: Compass heading in degrees (0-360°), derived from magX and magY
 */
struct MotionData {
  float magX;
  float magY;
  float magZ;
  float heading;
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;
};

// Global object to hold the latest raw sensor readings.
MotionData motionData;

// Global object to hold the latest filtered sensor readings.
MotionData filteredMotionData;

/**
 * Struct: SpatialData
 * Purpose: Holds fused sensor readings for magnetometer, accelerometer, gyroscope, and calculated heading.
 * Attributes:
 *   - roll, pitch, yaw: Euler angles in degrees representing the orientation of the device.
 */
struct SpatialData {
  float roll;
  float pitch;
  float yaw;
};

// Global object to hold the fused sensor readings.
SpatialData spatialData;

/**
 * Function: resetMotionData
 * Purpose: Resets all fields of a MotionData object to zero.
 * Inputs:
 *   - MotionData &data: Reference to the MotionData object to reset.
 * Outputs: None (modifies the object in place)
 */
void resetMotionData(MotionData &data) {
  data.magX = 0.0f;
  data.magY = 0.0f;
  data.magZ = 0.0f;
  data.accelX = 0.0f;
  data.accelY = 0.0f;
  data.accelZ = 0.0f;
  data.gyroX = 0.0f;
  data.gyroY = 0.0f;
  data.gyroZ = 0.0f;
  data.heading = 0.0f;
}

/**
 * Function: resetSpatialData
 * Purpose: Resets all fields of a SpatialData object to zero.
 * Inputs:
 *   - SpatialData &data: Reference to the SpatialData object to reset.
 * Outputs: None (modifies the object in place)
 */
void resetSpatialData(SpatialData &data) {
  data.pitch = 0.0f;
  data.yaw = 0.0f;
  data.roll = 0.0f;
}

/**
 * Struct: MotionOffsets
 * Purpose: Holds baseline offsets for accelerometer and gyroscope to correct sensor drift.
 * Members:
 *   - accelX, accelY, accelZ: Accelerometer offsets (m/s^2)
 *   - gyroX, gyroY, gyroZ: Gyroscope offsets (rads/s)
 */
struct MotionOffsets {
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;
};

// Global object to hold the latest IMU offsets.
MotionOffsets motionOffsets = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

/**
 * Function: calibrateIMUOffsets
 * Purpose: Samples the IMU while stationary to determine and set baseline offsets for accelerometer and gyroscope.
 * Inputs:
 *   - uint8_t numSamples: Number of samples to average for calibration.
 * Outputs: None (updates global motionOffsets struct)
 * Side Effects: Updates motionOffsets for more accurate stationary readings.
 * Note: Samples are collected as fast as possible, no delay.
 */
void calibrateIMUOffsets(uint8_t numSamples = 20) {
  float axSum = 0.0f, aySum = 0.0f, azSum = 0.0f;
  float gxSum = 0.0f, gySum = 0.0f, gzSum = 0.0f;

  for (uint8_t i = 0; i < numSamples; i++) {
    sensors_event_t accel, gyro, temp;
    myIMU.getEvent(&accel, &gyro, &temp);

    axSum += accel.acceleration.x;
    aySum += accel.acceleration.z * -1; // Use z-axis for Y to match physical orientation, invert value
    azSum += accel.acceleration.y; // Use y-axis for Z to match physical orientation
    gxSum += gyro.gyro.x;
    gySum += gyro.gyro.z * -1; // Use z-axis for Y to match physical orientation, invert value
    gzSum += gyro.gyro.y; // Use y-axis for Z to match physical orientation
  }

  // Calculate average offsets
  motionOffsets.accelX = axSum / numSamples;
  motionOffsets.accelY = aySum / numSamples;
  motionOffsets.accelZ = (azSum / numSamples) - 9.80665f; // Subtract gravity for Z axis
  motionOffsets.gyroX = gxSum / numSamples;
  motionOffsets.gyroY = gySum / numSamples;
  motionOffsets.gyroZ = gzSum / numSamples;
}

/**
 * Function: resetAllMotionData
 * Purpose: Resets both global motionData and filteredMotionData objects to zero.
 */
void resetAllMotionData() {
  resetMotionData(motionData);
  resetMotionData(filteredMotionData);
  resetSpatialData(spatialData);
  calibrateIMUOffsets();
}

/**
 * Function: initializeMotionDevices
 * Purpose: Initializes the I2C bus and configures the Magnetometer and IMU devices.
 */
void initializeMotionDevices() {
  Wire1.begin(IMU_SDA, IMU_SCL, 400000UL);

  // Initialize the LIS3MDL magnetometer.
  if(myMAG.begin_I2C(LIS3MDL_I2CADDR_DEFAULT, &Wire1)) {
    b_mag_found = true;
    myMAG.setPerformanceMode(LIS3MDL_MEDIUMMODE); // Set performance mode to medium (balanced power/accuracy)
    myMAG.setOperationMode(LIS3MDL_CONTINUOUSMODE); // Set operation mode to continuous measurements
    myMAG.setDataRate(LIS3MDL_DATARATE_155_HZ); // Set data rate to 155Hz (or LIS3MDL_DATARATE_300_HZ)
    myMAG.setRange(LIS3MDL_RANGE_8_GAUSS); // Set range to 8 Gauss (mid sensitivity, mid max field)
    myMAG.setIntThreshold(500); // Set interrupt threshold to 500
    myMAG.configInterrupt(false, false, true, true, false, true); // Configure interrupts
  }

  // Initialize the LSM6DS3TR-C IMU.
  if(myIMU.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1)) {
    b_imu_found = true;
    myIMU.setAccelRange(LSM6DS_ACCEL_RANGE_4_G); // Set accelerometer range to 4G (high sensitivity, low max acceleration)
    myIMU.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS); // Set gyroscope range to 250DPS (high sensitivity, low max rotation)
    myIMU.setAccelDataRate(LSM6DS_RATE_208_HZ); // Set accelerometer data rate to 208Hz
    myIMU.setGyroDataRate(LSM6DS_RATE_208_HZ); // Set gyroscope data rate to 208Hz
    myIMU.configInt1(false, false, true); // Enable accelerometer data ready interrupt
    myIMU.configInt2(false, true, false); // Enable gyroscope data ready interrupt
  }

  // Reset all motion data.
  resetAllMotionData();
}

/**
 * Function: calculateHeading
 * Purpose: Computes the compass heading (in degrees) from magnetometer X and Y values, applying a device-specific offset and optional inversion for mounting.
 * Inputs:
 *   - float magX: Magnetometer X-axis reading
 *   - float magY: Magnetometer Y-axis reading
 * Outputs:
 *   - float: Compass heading in degrees (0-360°)
 */
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

/**
 * Function: updateFilteredMotionData
 * Purpose: Applies exponential moving average filtering to raw motionData and updates filteredMotionData.
 * Inputs: None (uses global motionData and filteredMotionData)
 * Outputs: None (updates filteredMotionData)
 */
void updateFilteredMotionData() {
  filteredMotionData.magX    = FILTER_ALPHA * motionData.magX    + (1.0f - FILTER_ALPHA) * filteredMotionData.magX;
  filteredMotionData.magY    = FILTER_ALPHA * motionData.magY    + (1.0f - FILTER_ALPHA) * filteredMotionData.magY;
  filteredMotionData.magZ    = FILTER_ALPHA * motionData.magZ    + (1.0f - FILTER_ALPHA) * filteredMotionData.magZ;
  filteredMotionData.heading = FILTER_ALPHA * motionData.heading + (1.0f - FILTER_ALPHA) * filteredMotionData.heading;
  filteredMotionData.accelX  = FILTER_ALPHA * motionData.accelX  + (1.0f - FILTER_ALPHA) * filteredMotionData.accelX;
  filteredMotionData.accelY  = FILTER_ALPHA * motionData.accelY  + (1.0f - FILTER_ALPHA) * filteredMotionData.accelY;
  filteredMotionData.accelZ  = FILTER_ALPHA * motionData.accelZ  + (1.0f - FILTER_ALPHA) * filteredMotionData.accelZ;
  filteredMotionData.gyroX   = FILTER_ALPHA * motionData.gyroX   + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroX;
  filteredMotionData.gyroY   = FILTER_ALPHA * motionData.gyroY   + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroY;
  filteredMotionData.gyroZ   = FILTER_ALPHA * motionData.gyroZ   + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroZ;
}

/**
 * Function: updateOrientation
 * Purpose: Updates the orientation using sensor fusion (Madgwick filter).
 * Inputs: None (uses filteredMotionData)
 * Outputs: None (updates global orientation variables)
 */
void updateOrientation() {
  // Madgwick expects gyroscope in deg/s, accelerometer in g, magnetometer in uT
  // Convert gyroscope from rad/s to deg/s
  float gx = filteredMotionData.gyroX * (180.0f / PI);
  float gy = filteredMotionData.gyroY * (180.0f / PI);
  float gz = filteredMotionData.gyroZ * (180.0f / PI);

  // Convert accelerometer from m/s^2 to g
  float ax = filteredMotionData.accelX / 9.80665f;
  float ay = filteredMotionData.accelY / 9.80665f;
  float az = filteredMotionData.accelZ / 9.80665f;

  // Magnetometer already in uT
  float mx = filteredMotionData.magX;
  float my = filteredMotionData.magY;
  float mz = filteredMotionData.magZ;

  // Update the filter (sample frequency in Hz, e.g., 100)
  filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

  // Get Euler angles (degrees)
  spatialData.roll = filter.getRoll();
  spatialData.pitch = filter.getPitch();
  spatialData.yaw = filter.getYaw();
}

/**
 * Function: readMotionSensors
 * Purpose: Reads the motion sensors and prints the data to the debug console (if enabled).
 */
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

      // Update the magnetometer data.
      motionData.magX = mag.magnetic.x;
      motionData.magY = mag.magnetic.y;
      motionData.magZ = mag.magnetic.z;

      // Update heading value based on magnetometer X and Y only.
      motionData.heading = calculateHeading(motionData.magX, motionData.magY);

      /**
       * Update the raw IMU data in a global object, accounting for the orientation
       * of the IMU sensor relative to the mounted position of the PCB in the wand.
       * 
       * For a user standing at origin would be at (0,0,0).
       *
       * X Rotation (Roll):
       *  Positive Roll: Tilt left.
       *  Negative Roll: Tilt right.
       *
       * X Acceleration (Lateral, Left-Right):
       *  Positive X: To your left.
       *  Negative X: To your right.
       *  Movement: Sidestepping right/left.
       *
       * Y Rotation (Pitch):
       *  Positive Pitch: Look up.
       *  Negative Pitch: Look down.
       *
       * Y Acceleration (Longitudinal, Forward-Backward):
       *  Positive Z: Move backward.
       *  Negative Z: Move forward.
       *  Movement: Walking forward/backward.
       *
       * Z Rotation (Yaw):
       *  Positive Yaw: Turn left.
       *  Negative Yaw: Turn right.
       *
       * Z Acceleration (Vertical, Up-Down):
       *  Positive Y: Upwards.
       *  Negative Y: Downwards.
       *
       * Due to the direction the sensor is mounted on the PCB we need to swap the Z and Y axes.
       * Additionally, the Z axis reads backwards from what is expected so we need to invert Y values.
       * Even further the device is normally mounted upside down, sowe need to use our inversion flag
       * to swap the values for everything when installed.
       */
      // Apply offsets to IMU readings
      motionData.accelX = accel.acceleration.x - motionOffsets.accelX;
      motionData.accelY = (accel.acceleration.z * -1) - motionOffsets.accelY;
      motionData.accelZ = accel.acceleration.y - motionOffsets.accelZ;
      motionData.gyroX = gyro.gyro.x - motionOffsets.gyroX;
      motionData.gyroY = (gyro.gyro.z * -1) - motionOffsets.gyroY;
      motionData.gyroZ = gyro.gyro.y - motionOffsets.gyroZ;

      if (!isDeviceInstalled) {
        // When not installed we need to invert the X/Y values.
        motionData.accelX = motionData.accelX * -1;
        motionData.accelY = motionData.accelY * -1;
        motionData.gyroX = motionData.gyroX * -1;
        motionData.gyroY = motionData.gyroY * -1;
      }

      // Apply smoothing filter to sensor data.
      updateFilteredMotionData();

      // Print the filtered sensor data to the debug console.
    #if defined(DEBUG_SEND_TO_CONSOLE)
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
    #endif

      // Update the orientation using the filtered data.
      updateOrientation();

      // Send telemetry data to connected clients via server-side events.
      sendTelemetryData();
    }
  }
}
