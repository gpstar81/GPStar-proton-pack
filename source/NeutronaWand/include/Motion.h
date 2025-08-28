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
 * The mockup below represents the installation of the sensors and their registration mark for purposes of orientation.
 * In both orientations the USB-C port is at the top of the device and the terminal blocks are represented on the side as
 * appropriate for the orientation. For both views the Y axis runs top to bottom, with the Y+ direction being bottom/South.
 * The X axis runs left to right, with the X+ direction being relative to the device direction (up West, down East),
 * and the Z axis is always toward you or away from you in these views (Up/Down). Because this may differ by Neutrona Wand maker we
 * must read the sensor data on whichever axis gives us the correct spatial orientation.
 *
 *     Components Up                   Back of PCB
 *         |---|                          |---|
 *    _|-----------|                  |-----------|_
 *    ||    USB    |                  |    USB    ||
 *    ||      G/A. | Gyro/Accel (IMU) | .G/A      ||
 *    -|           |                  |           |-
 *    _|           |                  |           |_
 *    || .         |                  |         . ||
 *    ||  M        |   Magnetometer   |        M  ||
 *    ||           |                  |           ||
 *    -|-----------|                  |-----------|-
 *
 * We will use the “Aerospace NED Frame” (North–East–Down convention) for positive values on each axis:
 *  +X = Forward (-X Backward)
 *  +Y = Right (-Y Left)
 *  +Z = Down (toward the Earth at +9.81 m/s^2) remaining "gravity positive" for NED orientation.
 *
 * In NED orientation, positive and negative gyro values are handled thusly:
 *  +X = Clockwise Roll (-X Counterclockwise Roll)
 *  +Y = Pitch Up (-Y Pitch Down)
 *  +Z = Clockwise Yaw (-Z Counterclockwise Yaw)
 *
 * Since we use a "gravity positive" convention, we should be able to obtain a consistent acceleration value when
 * the device is laid FLAT on a single axis. If placed on the component or PCB side any axis with a positive value
 * closest to +9.8 would be considered the active axis for that orientation. Simply laying the device on 2/3 axes
 * will help identify the installation orientation and we can easily determine the appropriate axis mappings.
 *
 * This convention gives us the readings expected for sensor fusion in an Altitude Heading Reference System (AHRS).
 * That will take care of producing the roll (X), pitch (Y), and yaw (Z) as necessary for 3D representation later.
 */

/**
 * 3rd-Party Libraries
 * Utilizes the unified sensor libraries for Adafruit which work together for reading and sensor fusion.
 */
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_AHRS.h>

/**
 * Magnetometer and IMU
 * Defines all device objects and variables.
 */
Adafruit_LIS3MDL magSensor;
Adafruit_LSM6DS3TRC imuSensor;
Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer;
sensors_event_t mag_event, gyro_event, accel_event;
bool b_mag_found = false;
bool b_imu_found = false;
millisDelay ms_sensor_read_delay, ms_sensor_report_delay;
const uint8_t i_sensor_samples = 50; // Sets count of samples to take for averaging offsets.
const uint16_t i_sensor_read_delay = 20; // Delay between sensor reads in milliseconds (20ms = 50Hz).
const uint16_t i_sensor_report_delay = 50; // Delay between telemetry reporting (via console/web) in milliseconds.
Adafruit_Mahony ahrs_filter; // Create a filter object for sensor fusion (AHRS); Mahony better suited for human motion.

// Current state of the motion sensors and target for telemetry.
enum SENSOR_READ_TARGETS { NOT_INITIALIZED, CALIBRATION, OFFSETS, TELEMETRY };
enum SENSOR_READ_TARGETS SENSOR_READ_TARGET = NOT_INITIALIZED;

// Orientation positions expected by mounting positions.
enum INSTALL_ORIENTATIONS {
  COMPONENTS_UP_USB_FRONT,
  COMPONENTS_UP_USB_REAR,
  COMPONENTS_DOWN_USB_FRONT,
  COMPONENTS_DOWN_USB_REAR,
  COMPONENTS_LEFT_USB_FRONT,
  COMPONENTS_LEFT_USB_REAR,
  COMPONENTS_RIGHT_USB_FRONT,
  COMPONENTS_RIGHT_USB_REAR
};
enum INSTALL_ORIENTATIONS INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_FRONT; // Default for Hasbro installations.

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
const float FILTER_ALPHA = 0.4f;

/**
 * Magnetometer calibration variables obtained through using MotionCal and USB console output.
 * This should be performed near the end of an installation of the Neutrona Wand when the PCB
 * is fully assembled and in its final orientation with any speakers and their magnets nearby.
 * Default values come from a typical calibration session where the PCB is not yet installed.
 * See: https://www.pjrc.com/store/prop_shield.html for MotionCal usage.
 */
struct CalibrationData {
  float mag_hardiron[3] = {-32.05, 21.13, -3.21};
  float mag_softiron[9] = {1.011, 0.051, -0.012, 0.051, 0.988, -0.005, -0.012, -0.005, 1.004};
  float mag_field = 41.75;
};

// Global objects to hold the latest raw or filtered sensor readings.
CalibrationData magCalData;

/**
 * Struct: MotionData
 * Purpose: Holds all motion sensor readings from the magnetometer, accelerometer, and gyroscope.
 * Attributes:
 *   - magX, magY, magZ: Magnetometer readings (uTesla)
 *   - accelX, accelY, accelZ: Accelerometer readings (m/s^2)
 *   - gyroX, gyroY, gyroZ: Gyroscope readings (deg/s)
 */
struct MotionData {
  // Magnetometer readings (uTesla)
  float magX = 0.0f;
  float magY = 0.0f;
  float magZ = 0.0f;
  // Accelerometer readings (m/s^2)
  float accelX = 0.0f;
  float accelY = 0.0f;
  float accelZ = 0.0f;
  // Gyroscope readings (deg/s)
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
  // Calculated g-force (unit: g)
  float gForce = 0.0f;
  // Calculated angular velocity (unit: deg/s)
  float angVel = 0.0f;
  // Indicator for sudden movement (using calculated values)
  bool shaken = false;
};

// Global objects to hold the latest raw or filtered sensor readings.
MotionData motionData, filteredMotionData;

/**
 * Struct: MotionOffsets
 * Purpose: Holds baseline offsets for accelerometer and gyroscope to correct sensor drift.
 * This data is calculated on every reset of telemetry data and acts as a point of reference
 * for future movement. Effectively, this resets the center of the sensor's coordinate system.
 * Members:
 *   - accelX, accelY, accelZ: Accelerometer offsets (m/s^2)
 *   - gyroX, gyroY, gyroZ: Gyroscope offsets (deg/s)
 */
struct MotionOffsets {
  float sumAccelX = 0.0f;
  float sumAccelY = 0.0f;
  float sumAccelZ = 0.0f;
  float sumGyroX = 0.0f;
  float sumGyroY = 0.0f;
  float sumGyroZ = 0.0f;
  uint8_t samples = 0;
  float accelX = 0.0f;
  float accelY = 0.0f;
  float accelZ = 0.0f;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
};

// Global object to hold the calibration readings.
MotionOffsets motionOffsets;

/**
 * Struct: SpatialData
 * Purpose: Holds fused sensor readings from the magnetometer, accelerometer, and gyroscope.
 * Attributes:
 *   - roll, pitch, yaw: Euler angles in degrees representing the orientation of the device.
 *   - quaternion: Quaternion representation for orientation (w, x, y, z).
 */
struct SpatialData {
  float roll = 0.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;
  float quaternion[4] = {1.0f, 0.0f, 0.0f, 0.0f};
};

// Global object to hold the fused sensor readings.
SpatialData spatialData;

// Forward function declarations.
float calculateAngularVelocity(const MotionData& data);
float calculateGForce(const MotionData& data);
void collectMotionOffsets();
void processMotionData();
void readRawSensorData();
void reportCalibrationData();
void resetAllMotionData(bool b_calibrate);
void notifyWSClients(); // From Webhandler.h
void sendTelemetryData(); // From Webhandler.h

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
  data.gForce = 0.0f;
  data.angVel = 0.0f;
  data.shaken = false;
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
  data.quaternion[0] = 1.0f; // w component of quaternion
  data.quaternion[1] = 0.0f; // x component of quaternion
  data.quaternion[2] = 0.0f; // y component of quaternion
  data.quaternion[3] = 0.0f; // z component of quaternion
}

/**
 * Function: resetMotionOffsets
 * Purpose: Resets all fields of a MotionOffsets object to zero.
 * Inputs:
 *   - MotionOffsets &data: Reference to the MotionOffsets object to reset.
 * Outputs: None (modifies the object in place)
 */
void resetMotionOffsets(MotionOffsets &data) {
  data.sumAccelX = 0.0f;
  data.sumAccelY = 0.0f;
  data.sumAccelZ = 0.0f;
  data.sumGyroX = 0.0f;
  data.sumGyroY = 0.0f;
  data.sumGyroZ = 0.0f;
  data.samples = 0;
  data.accelX = 0.0f;
  data.accelY = 0.0f;
  data.accelZ = 0.0f;
  data.gyroX = 0.0f;
  data.gyroY = 0.0f;
  data.gyroZ = 0.0f;
}

/**
 * Function: initializeSensors
 * Purpose: Initializes the Magnetometer and Gyroscope/Accelerometer sensors.
 */
bool initializeSensors() {
#ifdef MOTION_SENSORS
  // Initialize the LIS3MDL magnetometer.
  if(magSensor.begin_I2C(LIS3MDL_I2CADDR_DEFAULT, &Wire1)) {
    b_mag_found = true; // Indicate that the magnetometer was found.
    debugln(F("LIS3MDL found at default address"));
  }

  // Initialize the LSM6DS3TR-C IMU.
  if(imuSensor.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1)) {
    b_imu_found = true; // Indicate that the IMU was found.
    debugln(F("LSM6DS3TR-C found at default address"));
  }

  // Fail if either sensor was not found.
  if(!b_mag_found || !b_imu_found) {
    return false;
  }

  // Register the sensors for use.
  accelerometer = imuSensor.getAccelerometerSensor();
  gyroscope = imuSensor.getGyroSensor();
  magnetometer = &magSensor;

  return true;
#endif
}

/**
 * Function: configureSensors
 * Purpose: Configures the motion sensors.
 */
void configureSensors() {
#ifdef MOTION_SENSORS
  if(b_mag_found && b_imu_found) {
    /**
     * Purpose: Sets the LIS3MDL magnetometer's performance mode, balancing power consumption and measurement accuracy.
     * Options:
     *   - LIS3MDL_LOWPOWERMODE: Lowest power, lowest accuracy.
     *   - LIS3MDL_MEDIUMMODE: Balanced power and accuracy.
     *   - LIS3MDL_HIGHMODE: Higher accuracy, higher power consumption.
     *   - LIS3MDL_ULTRAHIGHMODE: Maximum accuracy, maximum power consumption.
     *
     *   - Note that magSensor.begin_I2C() defaults to LIS3MDL_ULTRAHIGHMODE.
     */
    magSensor.setPerformanceMode(LIS3MDL_LOWPOWERMODE);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's measurement mode.
     * Options:
     *   - LIS3MDL_CONTINUOUSMODE: Continuous measurement mode (recommended for real-time applications).
     *   - LIS3MDL_SINGLEMODE: Single-shot measurement mode (lower power, not suitable for streaming).
     *   - LIS3MDL_POWERDOWNMODE: Power-down mode (sensor is off).
     *
     *   - Note that magSensor.begin_I2C() defaults to LIS3MDL_CONTINUOUSMODE.
     */
    //magSensor.setOperationMode(LIS3MDL_CONTINUOUSMODE);

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
     *   - LIS3MDL_DATARATE_80_HZ: 80 Hz.
     *   - LIS3MDL_DATARATE_155_HZ: 155 Hz. Overrides to LIS3MDL_ULTRAHIGHMODE.
     *   - LIS3MDL_DATARATE_300_HZ: 300 Hz. Overrides to LIS3MDL_HIGHMODE.
     *   - LIS3MDL_DATARATE_560_HZ: 560 Hz. Overrides to LIS3MDL_MEDIUMMODE.
     *   - LIS3MDL_DATARATE_1000_HZ: 1000 Hz. Overrides to LIS3MDL_LOWPOWERMODE.
     *
     *   - Note that magSensor.begin_I2C() defaults to LIS3MDL_DATARATE_155_HZ.
     *   - Setting data rate to 155/300/560/1000 implicitly calls setPerformanceMode.
     */
    magSensor.setDataRate(LIS3MDL_DATARATE_80_HZ);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's measurement range (sensitivity).
     * Options:
     *   - LIS3MDL_RANGE_4_GAUSS: ±4 Gauss (highest sensitivity, lowest max field).
     *   - LIS3MDL_RANGE_8_GAUSS: ±8 Gauss (mid sensitivity, mid max field).
     *   - LIS3MDL_RANGE_12_GAUSS: ±12 Gauss (low-mid sensitivity, high-mid max field).
     *   - LIS3MDL_RANGE_16_GAUSS: ±16 Gauss (lowest sensitivity, highest max field).
     *
     *   - Note that magSensor.begin_I2C() defaults to LIS3MDL_RANGE_4_GAUSS.
     */
    //magSensor.setRange(LIS3MDL_RANGE_4_GAUSS);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's interrupt threshold.
     * Options:
     *   - Any integer value representing the threshold in milliGauss (mG).
     *   - Typical values: 100–1000 (adjust based on noise and application).
     *
     *   - Note this is only required if you are using the INT pins.
     */
    //magSensor.setIntThreshold(500);

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


    /**
     * Purpose: Sets the LSM6DS3TR-C IMU's accelerometer measurement range.
     * Options:
     *   - LSM6DS_ACCEL_RANGE_2_G: ±2g (highest sensitivity, lowest max acceleration).
     *   - LSM6DS_ACCEL_RANGE_4_G: ±4g.
     *   - LSM6DS_ACCEL_RANGE_8_G: ±8g (mid sensitivity, low max acceleration).
     *   - LSM6DS_ACCEL_RANGE_16_G: ±16g (lowest sensitivity, highest max acceleration).
     *
     *   - Note that imuSensor.begin_I2C() defaults to LSM6DS_ACCEL_RANGE_4_G.
     */
    imuSensor.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);

    /**
     * Purpose: Sets the LSM6DS3TR-C IMU's gyroscope measurement range.
     * Options:
     *   - LSM6DS_GYRO_RANGE_125_DPS: ±125°/s (highest sensitivity, lowest max rotation).
     *   - LSM6DS_GYRO_RANGE_250_DPS: ±250°/s.
     *   - LSM6DS_GYRO_RANGE_500_DPS: ±500°/s (mid sensitivity, low max rotation).
     *   - LSM6DS_GYRO_RANGE_1000_DPS: ±1000°/s.
     *   - LSM6DS_GYRO_RANGE_2000_DPS: ±2000°/s (lowest sensitivity, highest max rotation).
     *
     *   - Note that imuSensor.begin_I2C() defaults to LSM6DS_GYRO_RANGE_2000_DPS.
     */
    imuSensor.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);

    /**
     * Purpose: Sets the LSM6DS3TR-C IMU's output data rate (ODR).
     * Options:
     *   - LSM6DS_RATE_SHUTDOWN: Power down (no output).
     *   - LSM6DS_RATE_12_5_HZ: 12.5 Hz.
     *   - LSM6DS_RATE_26_HZ: 26 Hz.
     *   - LSM6DS_RATE_52_HZ: 52 Hz.
     *   - LSM6DS_RATE_104_HZ: 104 Hz.
     *   - LSM6DS_RATE_208_HZ: 208 Hz.
     *   - LSM6DS_RATE_416_HZ: 416 Hz.
     *   - LSM6DS_RATE_833_HZ: 833 Hz.
     *   - LSM6DS_RATE_1_66K_HZ: 1660 Hz.
     *   - LSM6DS_RATE_3_33K_HZ: 3330 Hz.
     *   - LSM6DS_RATE_6_66K_HZ: 6660 Hz.
     *
     *   - Note that imuSensor.begin_I2C() defaults to LSM6DS_RATE_104_HZ for accel and gyro.
     */
    //imuSensor.setAccelDataRate(LSM6DS_RATE_104_HZ);
    //imuSensor.setGyroDataRate(LSM6DS_RATE_104_HZ);

    /**
     * Purpose: Enables or disables the LSM6DS3TR-C IMU's high-pass filter and sets the divisor.
     * Parameters:
     *   - enable: Enable high-pass filter (true/false).
     *   - divisor: Filter divisor, options:
     *     - LSM6DS_HPF_ODR_DIV_9: ODR/9
     *     - LSM6DS_HPF_ODR_DIV_50: ODR/50
     *     - LSM6DS_HPF_ODR_DIV_100: ODR/100
     *     - LSM6DS_HPF_ODR_DIV_400: ODR/400
     *
     *   - Note that imuSensor.begin_I2C() defaults to the HPF disabled.
     */
    //imuSensor.highPassFilter(false, LSM6DS_HPF_ODR_DIV_100);

    /**
     * Purpose: Configures the LSM6DS3TR-C IMU's INT1 interrupt pin (GYRO_INT1_PIN).
     * Parameters:
     *   - accelReady: Enable accelerometer data ready interrupt (true/false).
     *   - gyroReady: Enable gyroscope data ready interrupt (true/false).
     *   - tempReady: Enable temperature data ready interrupt (true/false).
     *
     *   - Note that imuSensor.begin_I2C() defaults to INT1 disabled.
     */
    imuSensor.configInt1(true, false, false);

    /**
     * Purpose: Configures the LSM6DS3TR-C IMU's INT2 interrupt pin (GYRO_INT2_PIN).
     * Parameters:
     *   - accelReady: Enable accelerometer data ready interrupt (true/false).
     *   - gyroReady: Enable gyroscope data ready interrupt (true/false).
     *   - tempReady: Enable temperature data ready interrupt (true/false).
     *
     *   - Note that imuSensor.begin_I2C() defaults to INT2 disabled.
     */
    imuSensor.configInt2(false, true, false);

    // Set the sample frequency for the Madgwick filter (converting our sensor delay interval from milliseconds to Hz).
    float f_sample_freq = (1000.0f / i_sensor_read_delay);
    ahrs_filter.begin(f_sample_freq);

    // Set Mahony gain values to adjust responsiveness and stability of the filter.
    ahrs_filter.setKp(3.0f); // Proportional gain: higher = faster response (default: 0.5f)
  }
#endif
}

/**
 * Function: resetAllMotionData
 * Purpose: Resets both global motionData and filteredMotionData objects to zero.
 */
void resetAllMotionData(bool b_calibrate = false) {
  debugln(F("Resetting all motion data."));
  resetMotionData(motionData);
  resetMotionData(filteredMotionData);
  resetMotionOffsets(motionOffsets);
  resetSpatialData(spatialData);

  if(b_calibrate) {
    SENSOR_READ_TARGET = OFFSETS; // Set target to collect offsets after reset.
    collectMotionOffsets(); // Calibrate IMU offsets with X samples.
  }
}

/**
 * Function: readRawSensorData
 * Purpose: Reads all sensor data directly from the magnetometer and IMU, transforming according to the installation orientation.
 *          IMPORTANT: Only read the raw values from the sensors, do not apply any offsets or filtering here!
 * Inputs: None (uses global sensor objects)
 * Outputs: None (updates global sensor objects)
 */
void readRawSensorData() {
#ifdef MOTION_SENSORS
  if(b_imu_found && b_mag_found) {
    // Poll the sensors.
    magnetometer->getEvent(&mag_event);
    gyroscope->getEvent(&gyro_event);
    accelerometer->getEvent(&accel_event);

    // Hard iron corrections.
    float mx = mag_event.magnetic.x - magCalData.mag_hardiron[0];
    float my = mag_event.magnetic.y - magCalData.mag_hardiron[1];
    float mz = mag_event.magnetic.z - magCalData.mag_hardiron[2];

    // Soft iron corrections.
    mag_event.magnetic.x = mx * magCalData.mag_softiron[0] + my * magCalData.mag_softiron[1] + mz * magCalData.mag_softiron[2];
    mag_event.magnetic.y = mx * magCalData.mag_softiron[3] + my * magCalData.mag_softiron[4] + mz * magCalData.mag_softiron[5];
    mag_event.magnetic.z = mx * magCalData.mag_softiron[6] + my * magCalData.mag_softiron[7] + mz * magCalData.mag_softiron[8];

    switch(INSTALL_ORIENTATION) {
      case COMPONENTS_UP_USB_FRONT:
      break;
      case COMPONENTS_UP_USB_REAR:
      break;
      case COMPONENTS_DOWN_USB_FRONT:
      default:
        // Default Hasbro installation orientation.
        // Update the magnetometer data (swapping the X and Y axes due to component's installation).
        // Note: We must invert Y (L-R) and Z (U-D) values because the device is effectively installed upside down.
        motionData.magX = mag_event.magnetic.y;
        motionData.magY = mag_event.magnetic.x * -1;
        motionData.magZ = mag_event.magnetic.z * -1;

        // Update the acceleration and gyroscope values (swapping the X and Y axes due to component's installation).
        // Note: We must invert Y (L-R) and Z (U-D) values because the device is effectively installed upside down.
        motionData.accelX = accel_event.acceleration.y;
        motionData.accelY = accel_event.acceleration.x * -1;
        motionData.accelZ = accel_event.acceleration.z * -1;
        motionData.gyroX = gyro_event.gyro.y;
        motionData.gyroY = gyro_event.gyro.x * -1;
        motionData.gyroZ = gyro_event.gyro.z * -1;
      break;
      case COMPONENTS_DOWN_USB_REAR:
      break;
      case COMPONENTS_LEFT_USB_FRONT:
      break;
      case COMPONENTS_LEFT_USB_REAR:
      break;
      case COMPONENTS_RIGHT_USB_FRONT:
        // Default Mack's Factory installation orientation.
      break;
      case COMPONENTS_RIGHT_USB_REAR:
      break;

      // @TODO: Draw the rest of the f*cking owl. (https://www.reddit.com/r/funny/comments/eccj2/how_to_draw_an_owl/)
    }

    // Lastly, the AHRS library update() function expects deg/s gyro values, so convert accordingly.
    motionData.gyroX *= SENSORS_RADS_TO_DPS;
    motionData.gyroY *= SENSORS_RADS_TO_DPS;
    motionData.gyroZ *= SENSORS_RADS_TO_DPS;
  }
#endif
}

/**
 * Function: detectShakeEvent
 * Purpose: Detects a shake event using gForce and angular velocity thresholds.
 * Inputs: None (uses global filteredMotionData)
 * Outputs: Returns true if a shake is detected, false otherwise.
 */
bool detectShakeEvent() {
  const float GFORCE_SHAKE_THRESHOLD = 1.2f;    // In g, adjust as needed
  const float ANGVEL_SHAKE_THRESHOLD = 180.0f;  // In deg/s, adjust as needed

  // Detect shake if either threshold is exceeded
  if (filteredMotionData.gForce > GFORCE_SHAKE_THRESHOLD &&
      filteredMotionData.angVel > ANGVEL_SHAKE_THRESHOLD) {
  #if defined(DEBUG_TELEMETRY_DATA)
    debug(F("gForce="));
    debug(filteredMotionData.gForce, 3);
    debug(F(" (T="));
    debug(GFORCE_SHAKE_THRESHOLD, 3);
    debug(F("), angVel="));
    debug(filteredMotionData.angVel, 3);
    debug(F(" (T="));
    debug(ANGVEL_SHAKE_THRESHOLD, 1);
    debugln(F(") "));
  #endif
    return true;
  }

  return false;
}

/**
 * Function: calculateAngularVelocity
 * Purpose: Calculates the magnitude of the angular velocity vector (deg/s) from a MotionData struct.
 * Inputs:
 *   - const MotionData& data: Reference to the MotionData object.
 * Outputs:
 *   - float: Calculated angular velocity (unit: deg/s)
 */
float calculateAngularVelocity(const MotionData& data) {
  return sqrt(
    data.gyroX * data.gyroX +
    data.gyroY * data.gyroY +
    data.gyroZ * data.gyroZ
  );
}

/**
 * Function: calculateGForce
 * Purpose: Calculates the magnitude of the acceleration vector (g-force) from a MotionData struct.
 * Inputs:
 *   - const MotionData& data: Reference to the MotionData object.
 * Outputs:
 *   - float: Calculated g-force (unit: g)
 */
float calculateGForce(const MotionData& data) {
  // Use the Euclidean norm for the acceleration vector and convert to g.
  return sqrt(
    data.accelX * data.accelX +
    data.accelY * data.accelY +
    data.accelZ * data.accelZ
  ) / 9.80665f; // 1g = 9.80665 m/s^2
}

/**
 * Function: updateFilteredMotionData
 * Purpose: Applies exponential moving average filtering to raw motionData and updates filteredMotionData.
 * Inputs: None (uses global motionData and filteredMotionData)
 * Outputs: None (updates filteredMotionData)
 */
void updateFilteredMotionData() {
  filteredMotionData.magX   = FILTER_ALPHA * motionData.magX   + (1.0f - FILTER_ALPHA) * filteredMotionData.magX;
  filteredMotionData.magY   = FILTER_ALPHA * motionData.magY   + (1.0f - FILTER_ALPHA) * filteredMotionData.magY;
  filteredMotionData.magZ   = FILTER_ALPHA * motionData.magZ   + (1.0f - FILTER_ALPHA) * filteredMotionData.magZ;
  filteredMotionData.accelX = FILTER_ALPHA * motionData.accelX + (1.0f - FILTER_ALPHA) * filteredMotionData.accelX;
  filteredMotionData.accelY = FILTER_ALPHA * motionData.accelY + (1.0f - FILTER_ALPHA) * filteredMotionData.accelY;
  filteredMotionData.accelZ = FILTER_ALPHA * motionData.accelZ + (1.0f - FILTER_ALPHA) * filteredMotionData.accelZ;
  filteredMotionData.gyroX  = FILTER_ALPHA * motionData.gyroX  + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroX;
  filteredMotionData.gyroY  = FILTER_ALPHA * motionData.gyroY  + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroY;
  filteredMotionData.gyroZ  = FILTER_ALPHA * motionData.gyroZ  + (1.0f - FILTER_ALPHA) * filteredMotionData.gyroZ;
}

/**
 * Function: updateOrientation
 * Purpose: Updates the orientation using sensor fusion (AHRS).
 * Inputs: None (uses filteredMotionData)
 * Outputs: None (updates global orientation variables)
 */
void updateOrientation() {
#ifdef MOTION_SENSORS
  /**
   * Fusion expects gyroscope in deg/s, accelerometer in m/s2, magnetometer in uT.
   * It assumes a gravity-positive z-axis and NED aerospace framing.
   * All 9 DoF values will calculate roll (X), pitch (Y), and yaw (Z).
   * The sample frequency is in Hz and already calculated from the update time in ms.
   */
  ahrs_filter.update(
    motionData.gyroX, motionData.gyroY, motionData.gyroZ,
    motionData.accelX, motionData.accelY, motionData.accelZ,
    motionData.magX, motionData.magY, motionData.magZ
  );

  // Get position in Euler angles (degrees) for orientation in NED space.
  spatialData.roll = ahrs_filter.getRoll();
  spatialData.pitch = ahrs_filter.getPitch();
  spatialData.yaw = ahrs_filter.getYaw();

  // Obtain the quaternion representation for visualization.
  float qw, qx, qy, qz;
  ahrs_filter.getQuaternion(&qw, &qx, &qy, &qz);
  spatialData.quaternion[0] = qw;
  spatialData.quaternion[1] = qx;
  spatialData.quaternion[2] = qy;
  spatialData.quaternion[3] = qz;

  // Mirror along Z-axis to get the correct direction.
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
 * Function: checkMotionSensors
 * Purpose: Checks the timer to know when to read the latest motion sensor data and prints the data to the debug console (if enabled).
 */
void checkMotionSensors() {
#ifdef MOTION_SENSORS
  if(b_imu_found && b_mag_found) {
    // Read the IMU/MAG values every N milliseconds.
    if(!ms_sensor_read_delay.isRunning()) {
      // Start the delay timer if not already running.
      ms_sensor_read_delay.start(i_sensor_read_delay);
    }
    else if(ms_sensor_read_delay.justFinished()) {
      // Read the latest data, using it for calibration or telemetry processing.
      processMotionData();
    }

    // Report the averaged IMU/MAG values every N milliseconds.
    if(!ms_sensor_report_delay.isRunning()) {
      ms_sensor_report_delay.start(i_sensor_report_delay);
    }
    else if(ms_sensor_report_delay.justFinished()) {
      // Print the filtered sensor data to the debug console.
    #if defined(DEBUG_TELEMETRY_DATA)
      debug("\t\tOff Accel X: ");
      debug(formatSignedFloat(motionOffsets.accelX));
      debug(" \tY: ");
      debug(formatSignedFloat(motionOffsets.accelY));
      debug(" \tZ: ");
      debug(formatSignedFloat(motionOffsets.accelZ));
      debugln(" m/s^2 ");

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

      debug("\t\tRaw G-Force: ");
      debug(motionData.gForce);
      debugln("g ");
      debug("\t\tAvg G-Force: ");
      debug(filteredMotionData.gForce);
      debugln("g ");
      debugln();

      debug("\t\tOff Gyro  X: ");
      debug(formatSignedFloat(motionOffsets.gyroX));
      debug(" \tY: ");
      debug(formatSignedFloat(motionOffsets.gyroY));
      debug(" \tZ: ");
      debug(formatSignedFloat(motionOffsets.gyroZ));
      debugln(" rads/s ");

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

      debug("\t\tRoll (x): ");
      debug(formatSignedFloat(spatialData.roll));
      debug("\tPitch (Y): ");
      debug(formatSignedFloat(spatialData.pitch));
      debug("\tYaw (Z): ");
      debug(formatSignedFloat(spatialData.yaw));
      debugln();
      debugln();
    #endif

      // Send telemetry data to connected clients via server-side events.
      sendTelemetryData();
    }
  }
#endif
}

/**
 * Function: processMotionData
 * Purpose: Reads the motion sensors and prints the data to the debug console (if enabled).
 * Inputs: None, operates on sensor objects.
 * Outputs: None, operates on global motionData and filteredMotionData.
 */
void processMotionData() {
#ifdef MOTION_SENSORS
  switch(SENSOR_READ_TARGET) {
    case NOT_INITIALIZED:
      // Can't do anything until the sensors have been initialized and configured.
    break;

    case CALIBRATION:
      reportCalibrationData(); // Send raw data to console (USB) output for external capture.
    break;

    case OFFSETS:
      collectMotionOffsets(); // Collect sensor data and calibrate IMU offsets with N samples.
    break;

    case TELEMETRY:
    default:
      readRawSensorData(); // Read the raw sensor data and place the latest values in the motionData object.

      // Calculate the magnitude of the raw angular velocity vector (deg/s).
      motionData.angVel = calculateAngularVelocity(motionData);

      // Calculate the magnitude of the raw acceleration vector (g-force).
      motionData.gForce = calculateGForce(motionData);

      // Apply offsets to IMU readings (values should be 0 if not calculated).
      motionData.accelX -= motionOffsets.accelX;
      motionData.accelY -= motionOffsets.accelY;
      motionData.accelZ -= motionOffsets.accelZ;
      motionData.gyroX -= motionOffsets.gyroX;
      motionData.gyroY -= motionOffsets.gyroY;
      motionData.gyroZ -= motionOffsets.gyroZ;

      // Update the orientation via sensor fusion.
      updateOrientation();

      // Apply exponential moving average (EMA) smoothing filter to sensor data.
      updateFilteredMotionData();

      // Calculate the magnitude of the raw angular velocity vector (deg/s).
      filteredMotionData.angVel = calculateAngularVelocity(filteredMotionData);

      // Calculate the magnitude of the filtered acceleration vector (g-force).
      filteredMotionData.gForce = calculateGForce(filteredMotionData);

      // Check for shake events which use our calculated values.
      filteredMotionData.shaken = detectShakeEvent();
    break;
  }
#endif
}

/**
 * Function: collectMotionOffsets
 * Purpose: Samples the IMU while stationary to determine and set baseline offsets for accelerometer and gyroscope.
 * Inputs: None (uses global motionOffsets and samples count).
 * Outputs: None (updates global motionOffsets struct)
 */
void collectMotionOffsets() {
#if defined(MOTION_SENSORS) && defined(MOTION_OFFSETS)
  if(motionOffsets.samples < i_sensor_samples) {
    motionOffsets.samples++; // Increment the sample count.
    debugln("Calibrating motion offsets... Sample " + String(motionOffsets.samples) + " of " + String(i_sensor_samples));

    readRawSensorData(); // Read the raw sensor data and place the latest values in the motionData object.

    // Keep a running sum of the accelerometer and gyroscope values per axis.
    motionOffsets.sumAccelX += motionData.accelX; // Accumulate accelerometer X values.
    motionOffsets.sumAccelY += motionData.accelY; // Accumulate accelerometer Y values.
    motionOffsets.sumAccelZ += motionData.accelZ; // Accumulate accelerometer Z values.
    motionOffsets.sumGyroX += motionData.gyroX; // Accumulate gyroscope X values.
    motionOffsets.sumGyroY += motionData.gyroY; // Accumulate gyroscope Y values.
    motionOffsets.sumGyroZ += motionData.gyroZ; // Accumulate gyroscope Z values.

    // Calculate average offsets after each sample for real-time feedback.
    motionOffsets.accelX = motionOffsets.sumAccelX / motionOffsets.samples;
    motionOffsets.accelY = motionOffsets.sumAccelY / motionOffsets.samples;
    motionOffsets.accelZ = (motionOffsets.sumAccelZ / motionOffsets.samples) - 9.80665f; // Get offset from gravity for Z axis (9.81 m/s^2)
    motionOffsets.gyroX = motionOffsets.sumGyroX / motionOffsets.samples;
    motionOffsets.gyroY = motionOffsets.sumGyroY / motionOffsets.samples;
    motionOffsets.gyroZ = motionOffsets.sumGyroZ / motionOffsets.samples;
  }
  else {
    debugln(F("Calibration completed, switching to standard telemetry collection mode."));
    SENSOR_READ_TARGET = TELEMETRY; // Set target to telemetry after calibration.
    notifyWSClients(); // Send a special notification after offsets are loaded.
  }
#endif
}

/**
 * Function: reportCalibrationData
 * Purpose: Reports the current calibration data direct from the motion sensors.
 * This data contains no offsets or axis modifications, it is only the raw data.
 * See: https://www.pjrc.com/store/prop_shield.html for MotionCal downloads.
 */
void reportCalibrationData() {
#ifdef MOTION_SENSORS
  magnetometer->getEvent(&mag_event);
  gyroscope->getEvent(&gyro_event);
  accelerometer->getEvent(&accel_event);

  // 'Raw' values to match expectation of MotionCal
  Serial.print("Raw:");
  Serial.print(int(accel_event.acceleration.x*8192/9.8)); Serial.print(",");
  Serial.print(int(accel_event.acceleration.y*8192/9.8)); Serial.print(",");
  Serial.print(int(accel_event.acceleration.z*8192/9.8)); Serial.print(",");
  Serial.print(int(gyro_event.gyro.x*SENSORS_RADS_TO_DPS*16)); Serial.print(",");
  Serial.print(int(gyro_event.gyro.y*SENSORS_RADS_TO_DPS*16)); Serial.print(",");
  Serial.print(int(gyro_event.gyro.z*SENSORS_RADS_TO_DPS*16)); Serial.print(",");
  Serial.print(int(mag_event.magnetic.x*10)); Serial.print(",");
  Serial.print(int(mag_event.magnetic.y*10)); Serial.print(",");
  Serial.print(int(mag_event.magnetic.z*10)); Serial.println("");

  // 'Uni' values to match expectation of MotionCal
  Serial.print("Uni:");
  Serial.print(accel_event.acceleration.x); Serial.print(",");
  Serial.print(accel_event.acceleration.y); Serial.print(",");
  Serial.print(accel_event.acceleration.z); Serial.print(",");
  Serial.print(gyro_event.gyro.x, 4); Serial.print(",");
  Serial.print(gyro_event.gyro.y, 4); Serial.print(",");
  Serial.print(gyro_event.gyro.z, 4); Serial.print(",");
  Serial.print(mag_event.magnetic.x); Serial.print(",");
  Serial.print(mag_event.magnetic.y); Serial.print(",");
  Serial.print(mag_event.magnetic.z); Serial.println("");
#endif
}