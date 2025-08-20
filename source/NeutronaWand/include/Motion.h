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

/**
 * NOTICE! Remember that the PCB for the Neutrona Wand is mounted upside down!
 * For proper orientation hold the device with the components facing downward.
 */

#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <MadgwickAHRS.h>

// Forward function declarations.
void calibrateIMUOffsets(uint8_t numSamples);
void readMotionSensors();
void resetAllMotionData();
void sendTelemetryData(); // From Webhandler.h

/**
 * Magnetometer and IMU
 * Defines all device objects and variables.
 */
Adafruit_LIS3MDL magSensor;
Adafruit_LSM6DS3TRC imuSensor;
bool b_mag_found = false;
bool b_imu_found = false;
millisDelay ms_sensor_read_delay, ms_sensor_report_delay;
const uint16_t i_sensor_read_delay = 20; // Delay between sensor reads in milliseconds (20ms/50Hz).
const uint16_t i_sensor_report_delay = 100; // Delay between telemetry reporting in milliseconds.
Madgwick filter; // Create a global filter object for sensor fusion (AHRS for Roll/Pitch/Yaw).

/**
 * Constant: FILTER_ALPHA
 * Purpose: Controls the smoothing factor for exponential moving average filtering (0 < FILTER_ALPHA <= 1).
 *          Increasing this value makes it more responsive to changes, decreasing smooths out fluctuations.
 *
 * How it works:
 *   - FILTER_ALPHA determines how much weight is given to the newest sensor reading versus the previous filtered value.
 *   - The formula for each update is:
 *       filteredValue = FILTER_ALPHA * newValue + (1 - FILTER_ALPHA) * previousFilteredValue;
 *   - If FILTER_ALPHA is close to 1.0:
 *       - The filter reacts quickly to new data (less smoothing, more responsive).
 *   - If FILTER_ALPHA is close to 0.0:
 *       - The filter reacts slowly (more smoothing, less responsive).
 *
 * Example:
 *   - FILTER_ALPHA = 0.1: Very smooth, but slow to respond to rapid changes.
 *   - FILTER_ALPHA = 0.5: Balanced between smoothness and responsiveness.
 *   - FILTER_ALPHA = 0.9: Very responsive, but less smoothing.
 *
 * Tuning:
 *   - Increase FILTER_ALPHA if you want the sensor data to react faster to changes.
 *   - Decrease FILTER_ALPHA if you want to suppress noise and jitter more.
 */
const float FILTER_ALPHA = 0.5f;

// Thresholds: adjust as needed for the sensor's noise profile.
const float MAG_THRESHOLD = 20.0f; // uTesla
const float ACCEL_THRESHOLD = 2.0f; // m/s^2
const float GYRO_THRESHOLD = 2.0f; // rads/s

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

// Global objects to hold the latest raw or averaged sensor readings.
MotionData motionData, filteredMotionData;

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
 * Function: initializeMotionDevices
 * Purpose: Initializes the I2C bus and configures the Magnetometer and IMU devices.
 */
void initializeMotionDevices() {
#ifdef MOTION_SENSORS
  Wire1.begin(IMU_SDA, IMU_SCL, 400000UL);

  // Initialize the LIS3MDL magnetometer.
  if(magSensor.begin_I2C(LIS3MDL_I2CADDR_DEFAULT, &Wire1)) {
    b_mag_found = true; // Indicate that the magnetometer was found.
    debugln(F("LIS3MDL found at default address"));

    /**
     * Purpose: Sets the LIS3MDL magnetometer's performance mode, balancing power consumption and measurement accuracy.
     * Options:
     *   - LIS3MDL_LOWPOWERMODE: Lowest power, lowest accuracy.
     *   - LIS3MDL_MEDIUMMODE: Balanced power and accuracy (recommended for most uses).
     *   - LIS3MDL_HIGHMODE: Higher accuracy, higher power consumption.
     *   - LIS3MDL_ULTRAHIGHMODE: Maximum accuracy, maximum power consumption.
     */
    magSensor.setPerformanceMode(LIS3MDL_LOWPOWERMODE);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's measurement mode.
     * Options:
     *   - LIS3MDL_CONTINUOUSMODE: Continuous measurement mode (recommended for real-time applications).
     *   - LIS3MDL_SINGLEMODE: Single-shot measurement mode (lower power, not suitable for streaming).
     *   - LIS3MDL_POWERDOWNMODE: Power-down mode (sensor is off).
     */
    magSensor.setOperationMode(LIS3MDL_CONTINUOUSMODE);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's output data rate (ODR).
     * Options:
     *   - LIS3MDL_DATARATE_0_625_HZ: 0.625 Hz (lowest rate, lowest power).
     *   - LIS3MDL_DATARATE_1_25_HZ: 1.25 Hz.
     *   - LIS3MDL_DATARATE_2_5_HZ: 2.5 Hz.
     *   - LIS3MDL_DATARATE_5_HZ: 5 Hz.
     *   - LIS3MDL_DATARATE_10_HZ: 10 Hz.
     *   - LIS3MDL_DATARATE_20_HZ: 20 Hz.
     *   - LIS3MDL_DATARATE_40_HZ: 40 Hz.
     *   - LIS3MDL_DATARATE_80_HZ: 80 Hz (recommended for matching CPU polling).
     */
    magSensor.setDataRate(LIS3MDL_DATARATE_80_HZ);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's measurement range (sensitivity).
     * Options:
     *   - LIS3MDL_RANGE_4_GAUSS: ±4 Gauss (highest sensitivity, lowest max field).
     *   - LIS3MDL_RANGE_8_GAUSS: ±8 Gauss (mid sensitivity, mid max field).
     *   - LIS3MDL_RANGE_12_GAUSS: ±12 Gauss.
     *   - LIS3MDL_RANGE_16_GAUSS: ±16 Gauss (lowest sensitivity, highest max field).
     */
    magSensor.setRange(LIS3MDL_RANGE_16_GAUSS);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's interrupt threshold.
     * Options:
     *   - Any integer value representing the threshold in milliGauss (mG).
     *   - Typical values: 100–1000 (adjust based on noise and application).
     */
    magSensor.setIntThreshold(500);

    /**
     * Purpose: Configures the LIS3MDL magnetometer's interrupt pin behavior.
     * Parameters:
     *   - enableX: Enable interrupt for X axis (true/false).
     *   - enableY: Enable interrupt for Y axis (true/false).
     *   - enableZ: Enable interrupt for Z axis (true/false).
     *   - polarity: Interrupt polarity (true = active high, false = active low).
     *   - latch: Latch interrupt (true = latched until cleared, false = pulse).
     *   - enabled: Enable the interrupt (true/false).
     */
    magSensor.configInterrupt(false, false, false, // Enable one or more axis
                              true, // Polarity active high
                              false, // Don't latch (pulse)
                              false); // Disable the interrupt
  }

  // Initialize the LSM6DS3TR-C IMU.
  if(imuSensor.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1)) {
    b_imu_found = true; // Indicate that the IMU was found.
    debugln(F("LSM6DS3TR-C found at default address"));

    /**
     * Purpose: Sets the LSM6DS3TR-C IMU's accelerometer measurement range.
     * Options:
     *   - LSM6DS_ACCEL_RANGE_2_G: ±2g (highest sensitivity, lowest max acceleration).
     *   - LSM6DS_ACCEL_RANGE_4_G: ±4g.
     *   - LSM6DS_ACCEL_RANGE_8_G: ±8g (mid sensitivity, low max acceleration).
     *   - LSM6DS_ACCEL_RANGE_16_G: ±16g (lowest sensitivity, highest max acceleration).
     */
    imuSensor.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);

    /**
     * Purpose: Sets the LSM6DS3TR-C IMU's gyroscope measurement range.
     * Options:
     *   - LSM6DS_GYRO_RANGE_125_DPS: ±125°/s (highest sensitivity, lowest max rotation).
     *   - LSM6DS_GYRO_RANGE_250_DPS: ±250°/s.
     *   - LSM6DS_GYRO_RANGE_500_DPS: ±500°/s (mid sensitivity, low max rotation).
     *   - LSM6DS_GYRO_RANGE_1000_DPS: ±1000°/s.
     *   - LSM6DS_GYRO_RANGE_2000_DPS: ±2000°/s (lowest sensitivity, highest max rotation).
     */
    imuSensor.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);

    /**
     * Purpose: Sets the LSM6DS3TR-C IMU's accelerometer output data rate (ODR).
     * Options:
     *   - LSM6DS_RATE_POWER_DOWN: Power down (no output).
     *   - LSM6DS_RATE_12_5_HZ: 12.5 Hz.
     *   - LSM6DS_RATE_26_HZ: 26 Hz.
     *   - LSM6DS_RATE_52_HZ: 52 Hz.
     *   - LSM6DS_RATE_104_HZ: 104 Hz.
     *   - LSM6DS_RATE_208_HZ: 208 Hz (recommended for responsive motion tracking).
     *   - LSM6DS_RATE_416_HZ: 416 Hz.
     *   - LSM6DS_RATE_833_HZ: 833 Hz.
     *   - LSM6DS_RATE_1660_HZ: 1660 Hz.
     *   - LSM6DS_RATE_3330_HZ: 3330 Hz.
     *   - LSM6DS_RATE_6660_HZ: 6660 Hz.
     */
    imuSensor.setAccelDataRate(LSM6DS_RATE_208_HZ);
    imuSensor.setGyroDataRate(LSM6DS_RATE_208_HZ);

    /**
     * Purpose: Enables or disables the LSM6DS3TR-C IMU's high-pass filter and sets the divisor.
     * Parameters:
     *   - enable: Enable high-pass filter (true/false).
     *   - divisor: Filter divisor, options:
     *     - LSM6DS_HPF_ODR_DIV_50: ODR/50
     *     - LSM6DS_HPF_ODR_DIV_100: ODR/100 (recommended for most applications)
     *     - LSM6DS_HPF_ODR_DIV_9: ODR/9
     *     - LSM6DS_HPF_ODR_DIV_400: ODR/400
     */
    imuSensor.highPassFilter(true, LSM6DS_HPF_ODR_DIV_100);

    /**
     * Purpose: Configures the LSM6DS3TR-C IMU's INT1 interrupt pin (GYRO_INT1_PIN).
     * Parameters:
     *   - accelReady: Enable accelerometer data ready interrupt (true/false).
     *   - gyroReady: Enable gyroscope data ready interrupt (true/false).
     *   - tempReady: Enable temperature data ready interrupt (true/false).
     */
    imuSensor.configInt1(true, false, false);

    /**
     * Purpose: Configures the LSM6DS3TR-C IMU's INT2 interrupt pin (GYRO_INT2_PIN).
     * Parameters:
     *   - accelReady: Enable accelerometer data ready interrupt (true/false).
     *   - gyroReady: Enable gyroscope data ready interrupt (true/false).
     *   - tempReady: Enable temperature data ready interrupt (true/false).
     */
    imuSensor.configInt2(false, true, false);
  }

  // Set the sample frequency for the Madgwick filter (converting our sensor delay interval from milliseconds to Hz).
  float f_sample_freq = (1000.0f / i_sensor_read_delay);
  filter.begin(f_sample_freq);
#endif

  // Reset all motion data, then perform a read, followed by another reset.
  resetAllMotionData();
  readMotionSensors();
  resetAllMotionData();
}

/**
 * Function: resetAllMotionData
 * Purpose: Resets both global motionData and filteredMotionData objects to zero.
 */
void resetAllMotionData() {
  debugln(F("Resetting all motion data."));
  resetMotionData(motionData);
  resetMotionData(filteredMotionData);
  resetSpatialData(spatialData);
  calibrateIMUOffsets(20); // Calibrate IMU offsets with 20 samples.
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
  float headingRad = atan2(-magY, -magX); // Get heading in radians from atan2 of Y and X (both flipped).
  float headingDeg = headingRad * (180.0f / PI); // Convert radians to degrees (180/pi).

  // Normalize to 0-360°
  while (headingDeg < 0.0f) {
    headingDeg += 360.0f;
  }
  while (headingDeg >= 360.0f) {
    headingDeg -= 360.0f;
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
#ifdef MOTION_SENSORS
  /**
   * Madgwick expects gyroscope in deg/s, accelerometer in g, magnetometer in uT.
   * It also assumes gravity-positive z-axis and right-handed coordinate system.
   * It will use all 9 DoF values to calculate roll (X), pitch (Y), and yaw (Z).
   */

  // Convert gyroscope from rad/s to deg/s by multiplying with (180.0f / PI).
  float gx = filteredMotionData.gyroX * (180.0f / PI);
  float gy = filteredMotionData.gyroY * (180.0f / PI);
  float gz = filteredMotionData.gyroZ * (180.0f / PI);

  // Convert accelerometer from m/s^2 to g.
  float ax = filteredMotionData.accelX / 9.80665f;
  float ay = filteredMotionData.accelY / 9.80665f;
  float az = (filteredMotionData.accelZ -9.80665f) / 9.80665f; // Always subtract gravity for Z axis (9.81 m/s^2)

  // Update the filter, using the calculated sample frequency in Hz.
  // Magnetometer is already in micro-Teslas so we just use as-is.
  filter.update(gx, gy, gz, ax, ay, az, filteredMotionData.magX, filteredMotionData.magY, filteredMotionData.magZ);

  // Get Euler angles (degrees) for position in NED space.
  spatialData.roll = filter.getRoll();
  spatialData.pitch = filter.getPitch();
  spatialData.yaw = filter.getYaw();

  // Mirror along Z-axis to match the heading.
  spatialData.yaw = 360.0f - spatialData.yaw;
  if(spatialData.yaw >= 360.0f) {
    spatialData.yaw -= 360.0f;
  }
#endif
}

/**
 * Function: formatSignedFloat
 * Purpose: Formats a float with explicit sign (+/-) and pads with spaces if whole number < 3 digits.
 * Inputs:
 *   - float value: The value to format.
 * Outputs:
 *   - String: Formatted string.
 */
String formatSignedFloat(float value) {
  char buf[16];
  int whole = abs((int)value);
  // Determine padding: if whole < 10, pad 2 spaces; < 100, pad 1 space; else no pad
  const char* pad = (whole < 10) ? "  " : (whole < 100) ? " " : "";
  sprintf(buf, "%c%s%.2f", (value >= 0 ? '+' : '-'), pad, abs(value));
  return String(buf);
}

/**
 * Function: isValidReading
 * Purpose: Checks if a reading is valid (not a spurious zero).
 * Inputs:
 *   - float value: axis value.
 * Outputs:
 *   - bool: True if valid, false if likely a glitch.
 */
bool isValidReading(float value) {
  // Accept values not exactly zero or within a small threshold.
  return fabs(value) > 0.01f;
}

/**
 * Function: isOutlier
 * Purpose: Determines if a new sensor reading is an outlier compared to the previous value.
 * Inputs:
 *   - float newValue: The new sensor reading.
 *   - float prevValue: The previous sensor reading.
 *   - float threshold: The maximum allowed change for a valid reading.
 * Outputs:
 *   - bool: True if the new value is an outlier, false otherwise.
 */
bool isOutlier(float newValue, float prevValue, float threshold) {
  return fabs(newValue - prevValue) > threshold;
}

/**
 * Function: readMotionSensors
 * Purpose: Reads the motion sensors and prints the data to the debug console (if enabled).
 */
void checkMotionSensors() {
#ifdef MOTION_SENSORS
  if(b_imu_found && b_mag_found) {
    // Read the IMU/MAG values every N milliseconds.
    if(!ms_sensor_read_delay.isRunning()) {
      ms_sensor_read_delay.start(i_sensor_read_delay);
    }
    else if(ms_sensor_read_delay.justFinished()) {
      readMotionSensors();
    }

    // Report the averaged IMU/MAG values every N milliseconds.
    if(!ms_sensor_report_delay.isRunning()) {
      ms_sensor_report_delay.start(i_sensor_report_delay);
    }
    else if(ms_sensor_report_delay.justFinished()) {
      // Print the filtered sensor data to the debug console.
    #if defined(DEBUG_TELEMETRY_DATA)
      debug("\t\tRaw Accel X: ");
      debug(formatSignedFloat(motionData.accelX));
      debug(" \tY: ");
      debug(formatSignedFloat(motionData.accelY));
      debug(" \tZ: ");
      debug(formatSignedFloat(motionData.accelZ));
      debugln(" m/s^2 ");
      debug("\t\tAvg Accel X: ");
      debug(formatSignedFloat(filteredMotionData.accelX));
      debug(" \tY: ");
      debug(formatSignedFloat(filteredMotionData.accelY));
      debug(" \tZ: ");
      debug(formatSignedFloat(filteredMotionData.accelZ));
      debugln(" m/s^2 ");
      debugln();

      debug("\t\tRaw Gyro  X: ");
      debug(formatSignedFloat(motionData.gyroX));
      debug(" \tY: ");
      debug(formatSignedFloat(motionData.gyroY));
      debug(" \tZ: ");
      debug(formatSignedFloat(motionData.gyroZ));
      debugln(" rads/s ");
      debug("\t\tAvg Gyro  X: ");
      debug(formatSignedFloat(filteredMotionData.gyroX));
      debug(" \tY: ");
      debug(formatSignedFloat(filteredMotionData.gyroY));
      debug(" \tZ: ");
      debug(formatSignedFloat(filteredMotionData.gyroZ));
      debugln(" rads/s ");
      debugln();

      debug("\t\tRaw Mag   X: ");
      debug(formatSignedFloat(motionData.magX));
      debug(" \tY: ");
      debug(formatSignedFloat(motionData.magY));
      debug(" \tZ: ");
      debug(formatSignedFloat(motionData.magZ));
      debugln(" uTesla ");
      debug("\t\tAvg Mag   X: ");
      debug(formatSignedFloat(filteredMotionData.magX));
      debug(" \tY: ");
      debug(formatSignedFloat(filteredMotionData.magY));
      debug(" \tZ: ");
      debug(formatSignedFloat(filteredMotionData.magZ));
      debugln(" uTesla ");
      debugln();

      debug("\t\tRaw Heading: ");
      debug(motionData.heading);
      debugln(" deg ");
      debug("\t\tAvg Heading: ");
      debug(filteredMotionData.heading);
      debugln(" deg ");
      debugln();
    #endif

      // Send telemetry data to connected clients via server-side events.
      sendTelemetryData();
    }
  }
#endif
}

/**
 * Function: readMotionSensors
 * Purpose: Reads the motion sensors and prints the data to the debug console (if enabled).
 * Inputs: None, operates on sensor objects.
 * Outputs: None, operates on global motionData and filteredMotionData.
 */
void readMotionSensors() {
#ifdef MOTION_SENSORS
  if(b_imu_found && b_mag_found) {
    // Poll the sensors.
    sensors_event_t mag, accel, gyro, temp;
    magSensor.getEvent(&mag);
    imuSensor.getEvent(&accel, &gyro, &temp);

    /**
     * Update the raw IMU data in a global object, accounting for the orientation of the magnetometer and IMU sensors relative
     * to the mounted position of the PCB in the wand. In our case, the PCB is mounted upside down, so we need to consider the
     * orientation of the components as looking at the BACK of the PCB with the USB port facing forward (up/north) and the two
     * terminal blocks are on the RIGHT (long edge) of the board. Note that the X/Y orientation of the sensors is based on the
     * robotic coordinate system and mounted face-up so we'll need to adjust for 3D spatial orientation.
     *
     *     |---|
     * |-----------|_
     * |    USB    ||
     * | .G/A      ||  Gyro/Accel Sensor
     * |           |-
     * |           |_
     * |         . ||
     * |        M  ||  Magnetometer
     * |           ||
     * |-----------|-
     *
     * In this orientation both sensors are mounted such that their Y+ is away from the USB port (down), X+ is to the right,
     * and Z+ is toward you (as you look down). However, this does not align with NED (North-East-Down) conventions.
     *
     * We will use the “Aerospace NED Frame” (North–East–Down convention) for positive values on each axis:
     *  +X = Forward (-Backward)
     *  +Y = Right (-Left)
     *  +Z = Down (toward the Earth at +9.81 m/s^2) remaining "gravity positive" for NED orientation.
     */

    // Update the magnetometer data (swapping the X and Y axes due to component's installation).
    motionData.magX = (isValidReading(mag.magnetic.y) && !isOutlier(mag.magnetic.y, motionData.magX, MAG_THRESHOLD)) ? mag.magnetic.y : motionData.magX;
    motionData.magY = (isValidReading(mag.magnetic.x) && !isOutlier(mag.magnetic.x, motionData.magY, MAG_THRESHOLD)) ? mag.magnetic.x : motionData.magY;
    motionData.magZ = (isValidReading(mag.magnetic.z) && !isOutlier(mag.magnetic.z, motionData.magZ, MAG_THRESHOLD)) ? mag.magnetic.z : motionData.magZ;

    // Update heading value based on the raw magnetometer X and Y only.
    motionData.heading = calculateHeading(motionData.magX, motionData.magY);

    // Update the acceleration and gyroscope values (swapping the X and Y axes due to component's installation).
    // Note: We must invert Z because the device is typically installed upside down.
    motionData.accelX = !isOutlier(accel.acceleration.y, motionData.accelX, ACCEL_THRESHOLD) ? accel.acceleration.y : motionData.accelX;
    motionData.accelY = !isOutlier(accel.acceleration.x * -1, motionData.accelX, ACCEL_THRESHOLD) ? accel.acceleration.x * -1 : motionData.accelY;
    motionData.accelZ = !isOutlier(accel.acceleration.z * -1, motionData.accelX, ACCEL_THRESHOLD) ? accel.acceleration.z * -1 : motionData.accelZ;
    motionData.gyroX = !isOutlier(gyro.gyro.y, motionData.gyroX, GYRO_THRESHOLD) ? gyro.gyro.y : motionData.gyroX;
    motionData.gyroY = !isOutlier(gyro.gyro.x * -1, motionData.gyroY, GYRO_THRESHOLD) ? gyro.gyro.x * -1 : motionData.gyroY;
    motionData.gyroZ = !isOutlier(gyro.gyro.z * -1, motionData.gyroZ, GYRO_THRESHOLD) ? gyro.gyro.z * -1 : motionData.gyroZ;

    // Apply offsets to IMU readings.
    motionData.accelX -= motionOffsets.accelX;
    motionData.accelY -= motionOffsets.accelY;
    motionData.accelZ -= motionOffsets.accelZ;
    motionData.gyroX -= motionOffsets.gyroX;
    motionData.gyroY -= motionOffsets.gyroY;
    motionData.gyroZ -= motionOffsets.gyroZ;

    // Apply smoothing filter to sensor data.
    updateFilteredMotionData();

    // Update heading value based on the moving average magnetometer X and Y only.
    filteredMotionData.heading = calculateHeading(filteredMotionData.magX, filteredMotionData.magY);

    // Update the orientation using the filtered data.
    updateOrientation();
  }
#endif
}

/**
 * Function: calibrateIMUOffsets
 * Purpose: Samples the IMU while stationary to determine and set baseline offsets for accelerometer and gyroscope.
 * Inputs:
 *   - uint8_t numSamples: Number of samples to average for calibration.
 * Outputs: None (updates global motionOffsets struct)
 * Side Effects: Updates motionOffsets for more accurate stationary readings.
 * Note: Samples are collected as fast as possible, no delay.
 */
void calibrateIMUOffsets(uint8_t numSamples) {
  // Reset all motion offset data offsets
  motionOffsets.accelX = 0.0f;
  motionOffsets.accelY = 0.0f;
  motionOffsets.accelZ = 0.0f;
  motionOffsets.gyroX = 0.0f;
  motionOffsets.gyroY = 0.0f;
  motionOffsets.gyroZ = 0.0f;

#if defined(MOTION_SENSORS) && defined(MOTION_OFFSETS)
  float axSum = 0.0f, aySum = 0.0f, azSum = 0.0f;
  float gxSum = 0.0f, gySum = 0.0f, gzSum = 0.0f;

  for (uint8_t i = 0; i < numSamples; i++) {
    sensors_event_t accel, gyro, temp;
    imuSensor.getEvent(&accel, &gyro, &temp);

    axSum += accel.acceleration.y; // Swap X and Y axes due to component's installation.
    aySum += accel.acceleration.x * -1; // Invert X for the component's installation.
    azSum += accel.acceleration.z * -1; // Invert Z because we install upside down.
    gxSum += gyro.gyro.y; // Swap X and Y axes due to component's installation.
    gySum += gyro.gyro.x * -1; // Invert X for the component's installation.
    gzSum += gyro.gyro.z * -1; // Invert Z because we install upside down.
  }

  // Calculate average offsets
  motionOffsets.accelX = axSum / numSamples;
  motionOffsets.accelY = aySum / numSamples;
  motionOffsets.accelZ = (azSum / numSamples) - 9.80665f; // Subtract gravity for Z axis (9.81 m/s^2)
  motionOffsets.gyroX = gxSum / numSamples;
  motionOffsets.gyroY = gySum / numSamples;
  motionOffsets.gyroZ = gzSum / numSamples;

  // Print the filtered sensor data to the debug console.
  #if defined(DEBUG_TELEMETRY_DATA)
    debug("\t\tOffset Accel X: ");
    debug(formatSignedFloat(motionOffsets.accelX));
    debug(" \tY: ");
    debug(formatSignedFloat(motionOffsets.accelY));
    debug(" \tZ: ");
    debug(formatSignedFloat(motionOffsets.accelZ));
    debugln(" m/s^2 ");
    debug("\t\tOffset Gyro  X: ");
    debug(formatSignedFloat(motionOffsets.gyroX));
    debug(" \tY: ");
    debug(formatSignedFloat(motionOffsets.gyroY));
    debug(" \tZ: ");
    debug(formatSignedFloat(motionOffsets.gyroZ));
    debugln(" rads/s ");
    debugln();
  #endif
#endif
}