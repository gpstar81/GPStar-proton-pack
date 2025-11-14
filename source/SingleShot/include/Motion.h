/**
 *   GPStar Single Shot Blaster - Ghostbusters Proton Pack & Neutrona Wand.
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
 * The Big TODO List:
 * - Implement accelerometer calibration via 6-direction acknowledgement.
 * - Implement automatic orientation direction detection via above calibration.
 */

/**
 * The mockup below represents the installation of the sensors and their registration mark for purposes of orientation.
 * In both orientations the USB-C port is at the top of the device and the terminal blocks are represented on the side as
 * appropriate for the orientation. For both views the Y axis runs top to bottom, with the Y+ direction being bottom/South.
 * The X axis runs left to right, with the X+ direction being relative to the device direction (up West, down East),
 * and the Z axis is always toward you or away from you in these views (Up/Down). Because this may differ by
 * installer we must read the sensor data on whichever axis gives us the correct spatial orientation.
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
 * For magnetic readings we will still use NED orientation though we have specific expectations for readings.
 * There is also a geographic component which will alter the readings based on your location on Earth.
 * With the device laying flat on a table and oriented North, we expect:
 *   X: Should be positive and largest (horizontal, forward, points toward magnetic north)
 *   Y: Smaller value than X, can be positive or negative (horizontal, right, depends on local declination)
 *   Z: Positive, typically largest (vertical, downward, toward Earth)
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

extern Task motionTask; // Declare reference to the motion task used by the scheduler.

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
millisDelay ms_gyro_calibration;
const uint8_t i_sensor_samples = 50; // Sets count of samples to take for averaging offsets.
const float f_gravity = 9.80665f; // Constant for converting m/s^2 to Gs.
uint32_t i_gyro_calibration_duration; // Time in milliseconds to run a gyroscope calibration (ms_gyro_calibration).
Adafruit_Mahony ahrs_filter; // Create a filter object for sensor fusion (AHRS); Mahony better suited for human motion.

// Current state of the motion sensors and target for telemetry.
enum SENSOR_READ_TARGETS { NOT_INITIALIZED, MAG_CALIBRATION, GYRO_CALIBRATION, OFFSETS, TELEMETRY };
enum SENSOR_READ_TARGETS SENSOR_READ_TARGET = NOT_INITIALIZED;

// Orientation positions expected by mounting for final installation (eg. as held by the user).
// This will be set by user preference in the web interface and saved to "device" preferences.
enum INSTALL_ORIENTATIONS : uint8_t {
  COMPONENTS_NOT_ORIENTED = 0,
  COMPONENTS_UP_USB_FRONT = 1,
  COMPONENTS_UP_USB_REAR = 2,
  COMPONENTS_DOWN_USB_FRONT = 3,
  COMPONENTS_DOWN_USB_REAR = 4,
  COMPONENTS_LEFT_USB_FRONT = 5,
  COMPONENTS_LEFT_USB_REAR = 6,
  COMPONENTS_RIGHT_USB_FRONT = 7,
  COMPONENTS_RIGHT_USB_REAR = 8,
  COMPONENTS_FACTORY_DEFAULT = 9
};
enum INSTALL_ORIENTATIONS INSTALL_ORIENTATION = COMPONENTS_NOT_ORIENTED; // Default until preferences are restored.

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

// Global object to hold magnetic calibration data.
CalibrationData magCalData;

/**
 * Struct: OrientedSensorData
 * Purpose: Holds raw sensor readings that have been oriented according to installation but not yet calibrated or filtered.
 * Attributes:
 *   - magX, magY, magZ: Oriented magnetometer readings (uTesla)
 *   - accelX, accelY, accelZ: Oriented accelerometer readings (m/s^2)
 *   - gyroX, gyroY, gyroZ: Oriented gyroscope readings (deg/s)
 */
struct OrientedSensorData {
  float magX = 0.0f;
  float magY = 0.0f;
  float magZ = 0.0f;
  float accelX = 0.0f;
  float accelY = 0.0f;
  float accelZ = 0.0f;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
};

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
  uint16_t samples = 0;
  float accelX = 0.0f;
  float accelY = 0.0f;
  float accelZ = 0.0f;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
};

// Global object to hold the calibration readings.
MotionOffsets calibratedOffsets, quickOffsets;

// Generic 3-axis container usable for gyro or accel offsets/storage.
struct Axis3F {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};
Axis3F accelOffsets; // For acceleration offsets.
Axis3F gyroOffsets; // For gyroscope offsets.

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
void collectQuickMotionOffsets();
void processMotionData();
void readRawSensorData();
void averageCalibrationData();
void reportCalibrationData();
void resetAllMotionData(bool b_calibrate);
void notifyWSClients(); // From Webhandler.h
void sendGyroCalData(); // From Webhandler.h
void sendMagCalData(bool b_update_points); // From Webhandler.h
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

  #if DEBUG == 1
    // Print information about the sensors.
    accelerometer->printSensorDetails();
    gyroscope->printSensorDetails();
    magnetometer->printSensorDetails();
  #endif

  return true;
#endif
}

/**
 * Struct: MagSelfTestResult
 * Purpose: Holds results of LIS3MDL self-test including baseline, self-test, delta, and pass/fail status.
 * Members:
 *   - baselineX, baselineY, baselineZ: Baseline readings before self-test (µT)
 *   - selfTestX, selfTestY, selfTestZ: Readings during self-test (µT)
 *   - deltaX, deltaY, deltaZ: Difference between self-test and baseline (µT)
 *   - passX, passY, passZ: Pass/fail status for each axis
 */
struct MagSelfTestResult {
  float baselineX;
  float baselineY;
  float baselineZ;
  float selfTestX;
  float selfTestY;
  float selfTestZ;
  float deltaX;
  float deltaY;
  float deltaZ;
  bool passX;
  bool passY;
  bool passZ;
} magSelfTest;

/**
 * Function: runMagSelfTest
 * Purpose: Runs LIS3MDL self-test and returns results in a struct.
 * Inputs:
 *   - Adafruit_LIS3MDL &mag: Reference to LIS3MDL sensor object.
 * Outputs:
 *   - MagSelfTestResult: Struct containing all relevant self-test data.
 */
MagSelfTestResult runMagSelfTest() {
#ifdef MOTION_SENSORS
  MagSelfTestResult result;

  // 1. Record baseline readings
  sensors_event_t baseline;
  magSensor.getEvent(&baseline);
  result.baselineX = baseline.magnetic.x;
  result.baselineY = baseline.magnetic.y;
  result.baselineZ = baseline.magnetic.z;

  // 2. Enable self-test
  magSensor.selfTest(true);
  delay(10); // Wait for self-test to take effect

  // 3. Read self-test values
  sensors_event_t selfTest;
  magSensor.getEvent(&selfTest);
  result.selfTestX = selfTest.magnetic.x;
  result.selfTestY = selfTest.magnetic.y;
  result.selfTestZ = selfTest.magnetic.z;

  // 4. Disable self-test
  magSensor.selfTest(false);

  // 5. Calculate deltas
  result.deltaX = result.selfTestX - result.baselineX;
  result.deltaY = result.selfTestY - result.baselineY;
  result.deltaZ = result.selfTestZ - result.baselineZ;

  // 6. Pass/fail logic for ±12 gauss (Table 2: X/Y 100–300 µT, Z 10–100 µT)
  result.passX = (result.deltaX >= 100.0f && result.deltaX <= 300.0f);
  result.passY = (result.deltaY >= 100.0f && result.deltaY <= 300.0f);
  result.passZ = (fabs(result.deltaZ) >= 10.0f && fabs(result.deltaZ) <= 100.0f);

  return result;
#endif
}

// Helper to read one register via I2C.
uint8_t readRegister( uint8_t reg) {
  Wire1.beginTransmission(LIS3MDL_I2CADDR_DEFAULT);
  Wire1.write(reg);
  // send restart (false) so we can requestFrom immediately
  if(Wire1.endTransmission(false) != 0) {
    // transmission error / device NACK
    return 0xFF;
  }
  if(Wire1.requestFrom(LIS3MDL_I2CADDR_DEFAULT, (uint8_t)1) != 1) {
    return 0xFF;
  }
  return Wire1.read();
}

// Struct: MagConfigInfo
// Purpose: Holds raw LIS3MDL register values and decoded configuration options.
// Members:
//   - rawRegisters: Array of register values (indexed by register address)
//   - performanceMode, dataRate, range, operationMode: Decoded config values (as enums or strings)
struct MagConfigInfo {
  struct RegisterValue {
    const char* name;
    uint8_t address;
    uint8_t value;
  } rawRegisters[25];
  const char* performanceMode;
  const char* dataRate;
  const char* range;
  const char* operationMode;
} magConfigInfo;

/**
 * Function: readMagConfig
 * Purpose: Returns raw register values and current config options.
 * Inputs:
 *   - Adafruit_LIS3MDL &mag: Reference to LIS3MDL sensor object.
 *   - TwoWire &wire: Reference to I2C bus (default Wire1).
 *   - uint8_t i2c_addr: I2C address of the LIS3MDL (default 0x1E).
 * Outputs:
 *   - MagConfigInfo: Struct containing all relevant config data.
 */
MagConfigInfo readMagConfig() {
#ifdef MOTION_SENSORS
  MagConfigInfo info;

  struct {
    const char *name;
    uint8_t reg;
  } regs[] = {
    {"OFFSET_X_REG_L_M", 0x05},
    {"OFFSET_X_REG_H_M", 0x06},
    {"OFFSET_Y_REG_L_M", 0x07},
    {"OFFSET_Y_REG_H_M", 0x08},
    {"OFFSET_Z_REG_L_M", 0x09},
    {"OFFSET_Z_REG_H_M", 0x0A},
    {"WHO_AM_I",   0x0F},
    {"CTRL_REG1",  0x20},
    {"CTRL_REG2",  0x21},
    {"CTRL_REG3",  0x22},
    {"CTRL_REG4",  0x23},
    {"CTRL_REG5",  0x24},
    {"STATUS_REG", 0x27},
    {"OUT_X_L",    0x28},
    {"OUT_X_H",    0x29},
    {"OUT_Y_L",    0x2A},
    {"OUT_Y_H",    0x2B},
    {"OUT_Z_L",    0x2C},
    {"OUT_Z_H",    0x2D},
    {"TEMP_OUT_L", 0x2E},
    {"TEMP_OUT_H", 0x2F},
    {"INT_CFG",    0x30},
    {"INT_SRC",    0x31},
    {"INT_THS_L",  0x32},
    {"INT_THS_H",  0x33}
  };

  if(!b_mag_found) {
    // Sensor not initialized, fill with error values
    for(size_t i = 0; i < sizeof(regs)/sizeof(regs[0]); ++i) {
      info.rawRegisters[i].name = regs[i].name;
      info.rawRegisters[i].address = regs[i].reg;
      info.rawRegisters[i].value = 0xFF;
    }
    info.performanceMode = "Sensor Not Found";
    info.dataRate = "Sensor Not Found";
    info.range = "Sensor Not Found";
    info.operationMode = "Sensor Not Found";
    return info;
  }

  // Populate raw register values (ensure array size matches)
  const size_t numRegs = sizeof(regs) / sizeof(regs[0]);
  static_assert(numRegs == sizeof(info.rawRegisters) / sizeof(info.rawRegisters[0]), "rawRegisters size mismatch");
  for(size_t i = 0; i < numRegs; ++i) {
    info.rawRegisters[i].name = regs[i].name;
    info.rawRegisters[i].address = regs[i].reg;
    info.rawRegisters[i].value = readRegister(regs[i].reg);
  }

  // Decode performance mode
  switch (magSensor.getPerformanceMode()) {
    case LIS3MDL_LOWPOWERMODE:    info.performanceMode = "Low Power Mode"; break;
    case LIS3MDL_MEDIUMMODE:      info.performanceMode = "Medium Performance Mode"; break;
    case LIS3MDL_HIGHMODE:        info.performanceMode = "High Performance Mode"; break;
    case LIS3MDL_ULTRAHIGHMODE:   info.performanceMode = "Ultra-High Performance Mode"; break;
    default:                      info.performanceMode = "Unknown / Other"; break;
  }

  // Decode data rate
  switch (magSensor.getDataRate()) {
    case LIS3MDL_DATARATE_0_625_HZ:   info.dataRate = "0.625 Hz"; break;
    case LIS3MDL_DATARATE_1_25_HZ:    info.dataRate = "1.25 Hz"; break;
    case LIS3MDL_DATARATE_2_5_HZ:     info.dataRate = "2.5 Hz"; break;
    case LIS3MDL_DATARATE_5_HZ:       info.dataRate = "5 Hz"; break;
    case LIS3MDL_DATARATE_10_HZ:      info.dataRate = "10 Hz"; break;
    case LIS3MDL_DATARATE_20_HZ:      info.dataRate = "20 Hz"; break;
    case LIS3MDL_DATARATE_40_HZ:      info.dataRate = "40 Hz"; break;
    case LIS3MDL_DATARATE_80_HZ:      info.dataRate = "80 Hz"; break;
    case LIS3MDL_DATARATE_155_HZ:     info.dataRate = "155 Hz Ultra High"; break;
    case LIS3MDL_DATARATE_300_HZ:     info.dataRate = "300 Hz High"; break;
    case LIS3MDL_DATARATE_560_HZ:     info.dataRate = "560 Hz Medium"; break;
    case LIS3MDL_DATARATE_1000_HZ:    info.dataRate = "1000 Hz Low Power"; break;
    default:                          info.dataRate = "Unknown / Other"; break;
  }

  // Decode range
  switch (magSensor.getRange()) {
    case LIS3MDL_RANGE_4_GAUSS:   info.range = "±4 gauss"; break;
    case LIS3MDL_RANGE_8_GAUSS:   info.range = "±8 gauss"; break;
    case LIS3MDL_RANGE_12_GAUSS:  info.range = "±12 gauss"; break;
    case LIS3MDL_RANGE_16_GAUSS:  info.range = "±16 gauss"; break;
    default:                      info.range = "Unknown / Other"; break;
  }

  // Decode operation mode
  switch(magSensor.getOperationMode()) {
    case LIS3MDL_CONTINUOUSMODE:  info.operationMode = "Continuous-conversion"; break;
    case LIS3MDL_SINGLEMODE:      info.operationMode = "Single-conversion"; break;
    case LIS3MDL_POWERDOWNMODE:   info.operationMode = "Power-down"; break;
    default:                      info.operationMode = "Unknown / Other"; break;
  }

  return info;
#endif
}

/**
 * Function: configureSensors
 * Purpose: Configures the motion sensors.
 */
void configureSensors() {
#ifdef MOTION_SENSORS
  debugln(F("Configuring motion sensors..."));

  if(b_mag_found && b_imu_found) {
    /**
     * Purpose: Sets the LIS3MDL magnetometer's measurement mode.
     * Options:
     *   - LIS3MDL_CONTINUOUSMODE: Continuous measurement mode (recommended for real-time applications).
     *   - LIS3MDL_SINGLEMODE: Single-shot measurement mode (lower power, not suitable for streaming).
     *   - LIS3MDL_POWERDOWNMODE: Power-down mode (sensor is off).
     *
     *   - Note that magSensor.begin_I2C() defaults to LIS3MDL_CONTINUOUSMODE.
     */
    magSensor.setOperationMode(LIS3MDL_CONTINUOUSMODE);

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
     *   - Setting data rate to 155/300/560/1000 implicitly calls setPerformanceMode and sets accordingly.
     */
    magSensor.setDataRate(LIS3MDL_DATARATE_80_HZ);

    /**
     * Purpose: Sets the LIS3MDL magnetometer's measurement range (sensitivity).
     * Options:
     *   - LIS3MDL_RANGE_4_GAUSS: ±4 Gauss [6842 LSB/g, 0.146 µT/LSB] (Default: highest sensitivity, lowest max field).
     *   - LIS3MDL_RANGE_8_GAUSS: ±8 Gauss [3421 LSB/g, 0.292 µT/LSB] (1/2: mid sensitivity, mid max field).
     *   - LIS3MDL_RANGE_12_GAUSS: ±12 Gauss [3421 LSB/g, 0.292 µT/LSB] (1/3: low-mid sensitivity, high-mid max field).
     *   - LIS3MDL_RANGE_16_GAUSS: ±16 Gauss [1711 LSB/g, 0.584 µT/LSB] (1/4: lowest sensitivity, highest max field).
     *
     *   - Note that magSensor.begin_I2C() defaults to LIS3MDL_RANGE_4_GAUSS.
     */
    magSensor.setRange(LIS3MDL_RANGE_8_GAUSS);

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
    magSensor.configInterrupt(false, false, false, // Enable one or more axes
                              true, // Polarity active high
                              false, // Don't latch (pulse)
                              false); // Disable the interrupt

    // Dump all LIS3MDL registers for debugging and perform a self-test.
    magConfigInfo = readMagConfig();
    magSelfTest = runMagSelfTest();

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
    float f_sample_freq = (1000.0f / motionTask.getInterval());
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
  resetSpatialData(spatialData);

  if(b_calibrate) {
    debugln(F("Reset all motion data, performing quick offset collection..."));
    SENSOR_READ_TARGET = OFFSETS; // Set target to collect offsets after reset.
    resetMotionOffsets(quickOffsets); // Clear previous offsets set/collected.
    collectQuickMotionOffsets(); // Calibrate IMU offsets with X samples.
  }
}

/**
 * Function: applySensorOrientation
 * Purpose: Maps raw sensor readings to the correct axes based on the installation orientation.
 *          This function only handles coordinate system transformation, no calibration or filtering.
 * Inputs:
 *   - const sensors_event_t& mag_event: Raw magnetometer event data
 *   - const sensors_event_t& accel_event: Raw accelerometer event data
 *   - const sensors_event_t& gyro_event: Raw gyroscope event data
 * Outputs:
 *   - OrientedSensorData: Sensor readings mapped to the correct coordinate system
 */
OrientedSensorData applySensorOrientation(const sensors_event_t& mag_event,
                                          const sensors_event_t& accel_event,
                                          const sensors_event_t& gyro_event) {
  OrientedSensorData oriented;

  // Map the sensor readings to the correct axes based on the installation orientation
  switch(INSTALL_ORIENTATION) {
    case COMPONENTS_UP_USB_FRONT:
      // Magnetometer values
      oriented.magX = mag_event.magnetic.y * -1;
      oriented.magY = mag_event.magnetic.x * -1;
      oriented.magZ = mag_event.magnetic.z * -1;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y;
      oriented.accelY = accel_event.acceleration.x;
      oriented.accelZ = accel_event.acceleration.z;
      oriented.gyroX = gyro_event.gyro.y * -1;
      oriented.gyroY = gyro_event.gyro.x * -1;
      oriented.gyroZ = gyro_event.gyro.z * -1;
    break;

    case COMPONENTS_UP_USB_REAR:
      // Magnetometer values
      oriented.magX = mag_event.magnetic.y;
      oriented.magY = mag_event.magnetic.x;
      oriented.magZ = mag_event.magnetic.z * -1;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y * -1;
      oriented.accelY = accel_event.acceleration.x * -1;
      oriented.accelZ = accel_event.acceleration.z;
      oriented.gyroX = gyro_event.gyro.y;
      oriented.gyroY = gyro_event.gyro.x;
      oriented.gyroZ = gyro_event.gyro.z * -1;
    break;

    case COMPONENTS_DOWN_USB_FRONT:
      // Default Hasbro installation orientation

      // Magnetometer values
      oriented.magX = mag_event.magnetic.y * -1;
      oriented.magY = mag_event.magnetic.x;
      oriented.magZ = mag_event.magnetic.z;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y;
      oriented.accelY = accel_event.acceleration.x * -1;
      oriented.accelZ = accel_event.acceleration.z * -1;
      oriented.gyroX = gyro_event.gyro.y * -1;
      oriented.gyroY = gyro_event.gyro.x;
      oriented.gyroZ = gyro_event.gyro.z;
    break;

    case COMPONENTS_DOWN_USB_REAR:
      // Magnetometer values
      oriented.magX = mag_event.magnetic.y;
      oriented.magY = mag_event.magnetic.x * -1;
      oriented.magZ = mag_event.magnetic.z;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y * -1;
      oriented.accelY = accel_event.acceleration.x;
      oriented.accelZ = accel_event.acceleration.z * -1;
      oriented.gyroX = gyro_event.gyro.y;
      oriented.gyroY = gyro_event.gyro.x * -1;
      oriented.gyroZ = gyro_event.gyro.z;
    break;

    case COMPONENTS_LEFT_USB_FRONT:
      // Magnetometer values
      oriented.magX = mag_event.magnetic.y * -1;
      oriented.magY = mag_event.magnetic.z * -1;
      oriented.magZ = mag_event.magnetic.x;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y;
      oriented.accelY = accel_event.acceleration.z;
      oriented.accelZ = accel_event.acceleration.x * -1;
      oriented.gyroX = gyro_event.gyro.y * -1;
      oriented.gyroY = gyro_event.gyro.z * -1;
      oriented.gyroZ = gyro_event.gyro.x;
    break;

    case COMPONENTS_LEFT_USB_REAR:
      // Magnetometer values
      oriented.magX = mag_event.magnetic.y;
      oriented.magY = mag_event.magnetic.z * -1;
      oriented.magZ = mag_event.magnetic.x * -1;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y * -1;
      oriented.accelY = accel_event.acceleration.z;
      oriented.accelZ = accel_event.acceleration.x;
      oriented.gyroX = gyro_event.gyro.y;
      oriented.gyroY = gyro_event.gyro.z * -1;
      oriented.gyroZ = gyro_event.gyro.x * -1;
    break;

    case COMPONENTS_RIGHT_USB_FRONT:
      // Default Mack's Factory installation orientation

      // Magnetometer values
      oriented.magX = mag_event.magnetic.y * -1;
      oriented.magY = mag_event.magnetic.z;
      oriented.magZ = mag_event.magnetic.x * -1;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y;
      oriented.accelY = accel_event.acceleration.z * -1;
      oriented.accelZ = accel_event.acceleration.x;
      oriented.gyroX = gyro_event.gyro.y * -1;
      oriented.gyroY = gyro_event.gyro.z;
      oriented.gyroZ = gyro_event.gyro.x * -1;
    break;

    case COMPONENTS_RIGHT_USB_REAR:
      // Magnetometer values
      oriented.magX = mag_event.magnetic.y;
      oriented.magY = mag_event.magnetic.z;
      oriented.magZ = mag_event.magnetic.x;

      // Acceleration and gyroscope values
      oriented.accelX = accel_event.acceleration.y * -1;
      oriented.accelY = accel_event.acceleration.z * -1;
      oriented.accelZ = accel_event.acceleration.x * -1;
      oriented.gyroX = gyro_event.gyro.y;
      oriented.gyroY = gyro_event.gyro.z;
      oriented.gyroZ = gyro_event.gyro.x;
    break;

    case COMPONENTS_FACTORY_DEFAULT:
      // If the orientation is unknown, do not modify the sensor readings.
      oriented.magX = mag_event.magnetic.x;
      oriented.magY = mag_event.magnetic.y;
      oriented.magZ = mag_event.magnetic.z;
      oriented.accelX = accel_event.acceleration.x;
      oriented.accelY = accel_event.acceleration.y;
      oriented.accelZ = accel_event.acceleration.z;
      oriented.gyroX = gyro_event.gyro.x;
      oriented.gyroY = gyro_event.gyro.y;
      oriented.gyroZ = gyro_event.gyro.z;
    break;
  }

  return oriented;
}

/**
 * Function: readRawSensorData
 * Purpose: Reads all sensor data directly from the magnetometer and IMU, applies calibration corrections and orientation mapping.
 *          IMPORTANT: Only read the raw values from the sensors, do not apply any localized offsets or filtering here!
 * Inputs: None (uses global sensor objects)
 * Outputs: None (updates global sensor objects)
 */
void readRawSensorData() {
#ifdef MOTION_SENSORS
  if(b_imu_found && b_mag_found) {
    // Poll the sensors for raw data
    magnetometer->getEvent(&mag_event);
    gyroscope->getEvent(&gyro_event);
    accelerometer->getEvent(&accel_event);

    // Apply orientation mapping to all sensor data
    OrientedSensorData oriented = applySensorOrientation(mag_event, accel_event, gyro_event);

    // Apply hard iron corrections to magnetic readings (post-orientation).
    float mx = oriented.magX - magCalData.mag_hardiron[0];
    float my = oriented.magY - magCalData.mag_hardiron[1];
    float mz = oriented.magZ - magCalData.mag_hardiron[2];

    // Apply soft iron corrections to magnetic readings (post-orientation).
    motionData.magX = mx * magCalData.mag_softiron[0] + my * magCalData.mag_softiron[1] + mz * magCalData.mag_softiron[2];
    motionData.magY = mx * magCalData.mag_softiron[3] + my * magCalData.mag_softiron[4] + mz * magCalData.mag_softiron[5];
    motionData.magZ = mx * magCalData.mag_softiron[6] + my * magCalData.mag_softiron[7] + mz * magCalData.mag_softiron[8];

    // Store the oriented values in global motionData struct for access.
    // Converts gyroscope from rad/s to deg/s as expected by AHRS library.
    motionData.accelX = oriented.accelX;
    motionData.accelY = oriented.accelY;
    motionData.accelZ = oriented.accelZ;
    motionData.gyroX = oriented.gyroX * SENSORS_RADS_TO_DPS;
    motionData.gyroY = oriented.gyroY * SENSORS_RADS_TO_DPS;
    motionData.gyroZ = oriented.gyroZ * SENSORS_RADS_TO_DPS;
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
  if(filteredMotionData.gForce > GFORCE_SHAKE_THRESHOLD &&
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
  ) / f_gravity; // 1g = 9.80665 m/s^2
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
   * Fusion expects gyroscope in deg/s, accelerometer in m/s^2, magnetometer in uT.
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
  sprintf(buf, "%c%s%.2f", (value >= 0 ? '+' : '-'), pad, fabsf(value));
  return String(buf);
}

/**
 * Function: checkMotionSensors
 * Purpose: Checks the timer to know when to read the latest motion sensor data and prints the data to the debug console (if enabled).
 * NOTICE: For this device no timer is used to trigger the sensor reads; this is called by the task scheduler and its set interval.
 */
void checkMotionSensors() {
#ifdef MOTION_SENSORS
  if(b_imu_found && b_mag_found) {
    // Read the latest data, using it for calibration or telemetry processing.
    processMotionData();

  #if defined(DEBUG_TELEMETRY_DATA)
    // Print the filtered sensor data to the debug console.
    debug("\t\tOff Accel X: ");
    debug(formatSignedFloat(calibratedOffsets.accelX));
    debug(" \tY: ");
    debug(formatSignedFloat(calibratedOffsets.accelY));
    debug(" \tZ: ");
    debug(formatSignedFloat(calibratedOffsets.accelZ));
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
    debug(formatSignedFloat(calibratedOffsets.gyroX));
    debug(" \tY: ");
    debug(formatSignedFloat(calibratedOffsets.gyroY));
    debug(" \tZ: ");
    debug(formatSignedFloat(calibratedOffsets.gyroZ));
    debugln(" deg/s ");

    debug("\t\tRaw Gyro  X: ");
    debug(formatSignedFloat(motionData.gyroX));
    debug(" \tY: ");
    debug(formatSignedFloat(motionData.gyroY));
    debug(" \tZ: ");
    debug(formatSignedFloat(motionData.gyroZ));
    debugln(" deg/s ");

    debug("\t\tAvg Gyro  X: ");
    debug(formatSignedFloat(filteredMotionData.gyroX));
    debug(" \tY: ");
    debug(formatSignedFloat(filteredMotionData.gyroY));
    debug(" \tZ: ");
    debug(formatSignedFloat(filteredMotionData.gyroZ));
    debugln(" deg/s ");
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

    if(SENSOR_READ_TARGET == TELEMETRY) {
      // Send telemetry data to connected clients via server-side events.
      sendTelemetryData();
    }
  }
#endif
}

// Helper: Returns true when a MotionOffsets instance appears to be default/empty.
inline bool isMotionOffsetsDefault(const MotionOffsets &m) {
  // Explicit field checks are preferred over raw byte checks to avoid issues with padding/NaN.
  return (m.samples == 0) &&
         (m.accelX == 0.0f) && (m.accelY == 0.0f) && (m.accelZ == 0.0f) &&
         (m.gyroX  == 0.0f) && (m.gyroY  == 0.0f) && (m.gyroZ  == 0.0f);
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

    case GYRO_CALIBRATION:
      averageCalibrationData(); // Send raw data to console (USB) output for external capture.
    break;

    case MAG_CALIBRATION:
      reportCalibrationData(); // Send raw data to console (USB) output for external capture.
    break;

    case OFFSETS:
      collectQuickMotionOffsets(); // Collect sensor data and calculate IMU offsets of N samples.
    break;

    case TELEMETRY:
    default:
      // Read the raw sensor data with orientation corrections and update the motionData object.
      readRawSensorData();

      // Calculate the magnitude of the raw angular velocity vector (deg/s).
      motionData.angVel = calculateAngularVelocity(motionData);

      // Calculate the magnitude of the raw acceleration vector (g-force).
      motionData.gForce = calculateGForce(motionData);

      // Apply offsets to IMU readings only after we know the installation orientation.
      if(INSTALL_ORIENTATION != COMPONENTS_FACTORY_DEFAULT) {
        // Choose Offsets: Prefer calibratedOffsets, but use quickOffsets when calibrated offsets are default/empty.
        const MotionOffsets *usedOffsets = &calibratedOffsets;
        if(isMotionOffsetsDefault(calibratedOffsets)) {
          usedOffsets = &quickOffsets;

          #if defined(DEBUG_TELEMETRY_DATA)
            debugln(F("No calibrated offsets present; using quickOffsets for runtime corrections."));
          #endif
        }

        // Apply chosen offsets
        motionData.accelX -= usedOffsets->accelX;
        motionData.accelY -= usedOffsets->accelY;
        motionData.accelZ -= usedOffsets->accelZ;
        motionData.gyroX  -= usedOffsets->gyroX;
        motionData.gyroY  -= usedOffsets->gyroY;
        motionData.gyroZ  -= usedOffsets->gyroZ;
      }

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
 * Function: collectQuickMotionOffsets
 * Purpose: Samples the IMU while stationary to determine and set baseline offsets for accelerometer and gyroscope.
 * Inputs: None (uses global motionData, quickOffsets, and samples count).
 * Outputs: None (updates global quickOffsets struct)
 */
void collectQuickMotionOffsets() {
#if defined(MOTION_SENSORS) && defined(MOTION_OFFSETS)
  if(quickOffsets.samples < i_sensor_samples) {
    quickOffsets.samples++; // Increment the sample count.
    #if defined(DEBUG_SEND_TO_CONSOLE)
      debugln("Calibrating motion offsets... Sample " + String(quickOffsets.samples) + " of " + String(i_sensor_samples));
    #endif

    readRawSensorData(); // Read the raw sensor data and place the latest values in the motionData object.

    // Collect using running sums to avoid possible overflows.
    quickOffsets.sumAccelX += motionData.accelX;
    quickOffsets.sumAccelY += motionData.accelY;
    quickOffsets.sumAccelZ += motionData.accelZ;
    quickOffsets.sumGyroX += motionData.gyroX;
    quickOffsets.sumGyroY += motionData.gyroY;
    quickOffsets.sumGyroZ += motionData.gyroZ;

    // Live averages for telemetry / debug, avoids large buffer storage.
    uint32_t i_samples = (quickOffsets.samples == 0) ? 1 : quickOffsets.samples;
    float f_inv = 1.0f / (float)i_samples; // For floating-point division (reciprocal ).
    quickOffsets.accelX = quickOffsets.sumAccelX * f_inv;
    quickOffsets.accelY = quickOffsets.sumAccelY * f_inv;
    quickOffsets.accelZ = (quickOffsets.sumAccelZ * f_inv) - f_gravity; // Get offset from gravity for Z axis (9.81 m/s^2)
    quickOffsets.gyroX = quickOffsets.sumGyroX * f_inv;
    quickOffsets.gyroY = quickOffsets.sumGyroY * f_inv;
    quickOffsets.gyroZ = quickOffsets.sumGyroZ * f_inv;
  }
  else {
    debugln(F("Quick offsets collected, switching to standard telemetry collection mode."));
    SENSOR_READ_TARGET = TELEMETRY; // Set target to telemetry after calibration.
    notifyWSClients(); // Send a special notification after offsets are loaded.
  }
#endif
}

/**
 * Function: beginGyroCalibration
 * Purpose: Puts the motion system into gyroscope calibration mode to collect and average data for 30 seconds.
 */
void beginGyroCalibration(uint8_t i_duration_seconds) {
#ifdef MOTION_SENSORS
  debugln(F("Starting gyroscope calibration mode..."));
  resetAllMotionData(false); // Clear but don't perform quick calibration.
  resetMotionOffsets(calibratedOffsets); // Clear the calibrated offsets.
  SENSOR_READ_TARGET = GYRO_CALIBRATION; // Set target to gyro calibration.
  i_gyro_calibration_duration = (uint32_t)i_duration_seconds * 1000UL; // Convert seconds to milliseconds.
  ms_gyro_calibration.start(i_gyro_calibration_duration); // Start the calibration timer.
#endif
}

/**
 * Function: averageCalibrationData
 * Purpose: Collects the current calibration data from the motion sensors with proper orientation mapping.
 *          This ensures calibration tools receive data in the device's coordinate system, not the raw chip coordinates.
 *          Data is collected internally for purposes of averaging results.
 */
void averageCalibrationData() {
#ifdef MOTION_SENSORS
  readRawSensorData(); // Read the raw sensor data and place the latest values in the motionData object.

  // Guard sample count from wrapping.
  if(calibratedOffsets.samples < UINT16_MAX) {
    calibratedOffsets.samples++;
  }

  // Collect using running sums to avoid possible overflows.
  calibratedOffsets.sumAccelX += motionData.accelX;
  calibratedOffsets.sumAccelY += motionData.accelY;
  calibratedOffsets.sumAccelZ += motionData.accelZ;
  calibratedOffsets.sumGyroX += motionData.gyroX;
  calibratedOffsets.sumGyroY += motionData.gyroY;
  calibratedOffsets.sumGyroZ += motionData.gyroZ;

  // Live averages for telemetry / debug, avoids large buffer storage.
  uint32_t i_samples = (calibratedOffsets.samples == 0) ? 1 : calibratedOffsets.samples;
  float f_inv = 1.0f / (float)i_samples; // For floating-point division (reciprocal ).
  calibratedOffsets.accelX = calibratedOffsets.sumAccelX * f_inv;
  calibratedOffsets.accelY = calibratedOffsets.sumAccelY * f_inv;
  calibratedOffsets.accelZ = (calibratedOffsets.sumAccelZ * f_inv) - f_gravity; // Get offset from gravity for Z axis (9.81 m/s^2)
  calibratedOffsets.gyroX = calibratedOffsets.sumGyroX * f_inv;
  calibratedOffsets.gyroY = calibratedOffsets.sumGyroY * f_inv;
  calibratedOffsets.gyroZ = calibratedOffsets.sumGyroZ * f_inv;

  // Provide audio feedback via beep every ~1 seconds during calibration.
  static int16_t i_last_beep_interval = -1;
  int16_t i_interval = (int16_t)(ms_gyro_calibration.remaining() / 1000);
  if(i_interval != i_last_beep_interval) {
    i_last_beep_interval = i_interval;
    playEffect(S_BEEPS_ALT);
  }

  sendGyroCalData(); // Report the latest timer value.

  // Stop collection once the calibration timer has finished.
  if(ms_gyro_calibration.justFinished()) {
    i_gyro_calibration_duration = 0; // Reset the timer duration.
    debugln(F("Gyro calibration complete; offsets computed."));
    playEffect(S_BEEPS);

    // Save only accel offsets as a triplet.
    accelOffsets.x = calibratedOffsets.accelX;
    accelOffsets.y = calibratedOffsets.accelY;
    accelOffsets.z = calibratedOffsets.accelZ;

    // Save only gyro offsets as a triplet.
    gyroOffsets.x = calibratedOffsets.gyroX;
    gyroOffsets.y = calibratedOffsets.gyroY;
    gyroOffsets.z = calibratedOffsets.gyroZ;

    // Report the final calibration results.
    debugln(F("Final calibration summary:"));
    debug(F("\tSamples: "));
    debugln(calibratedOffsets.samples);
    debug(F("\tAccel Offsets (m/s^2): X="));
    debug(formatSignedFloat(accelOffsets.x));
    debug(F(" Y="));
    debug(formatSignedFloat(accelOffsets.y));
    debug(F(" Z="));
    debugln(String(accelOffsets.z) + " m/s^2");
    debug(F("\tGyro Offsets (deg/s):  X="));
    debug(formatSignedFloat(gyroOffsets.x));
    debug(F(" Y="));
    debug(formatSignedFloat(gyroOffsets.y));
    debug(F(" Z="));
    debugln(String(gyroOffsets.z) + " deg/s");

    // Reset the counters and summation fields.
    calibratedOffsets.sumAccelX = 0.0f;
    calibratedOffsets.sumAccelY = 0.0f;
    calibratedOffsets.sumAccelZ = 0.0f;
    calibratedOffsets.sumGyroX = 0.0f;
    calibratedOffsets.sumGyroY = 0.0f;
    calibratedOffsets.sumGyroZ = 0.0f;
    calibratedOffsets.samples = 0;

    // Create Preferences object to handle non-volatile storage (NVS).
    Preferences preferences;

    // Save the offset data (as an object) to preferences.
    if(preferences.begin("device", false)) {
      preferences.putBytes("accel_cal", &accelOffsets, sizeof(accelOffsets));
      preferences.putBytes("gyro_cal", &gyroOffsets, sizeof(gyroOffsets));
      preferences.end();
    }

    // Switch back to telemetry and notify clients
    SENSOR_READ_TARGET = TELEMETRY;
    notifyWSClients();
    return;
  }
#endif
}

/**
 * Function: reportCalibrationData
 * Purpose: Reports the current calibration data from the motion sensors with proper orientation mapping.
 *          This ensures calibration tools receive data in the device's coordinate system, not the raw chip coordinates.
 *          Data is output direct to serial console (USB) for capture by external tools, if desired.
 */
void reportCalibrationData() {
#ifdef MOTION_SENSORS
  // Begin by reading the raw sensor data.
  magnetometer->getEvent(&mag_event);
  gyroscope->getEvent(&gyro_event);
  accelerometer->getEvent(&accel_event);

  // Uncomment to force the device's raw orientation for calibration reporting, when necessary.
  // INSTALL_ORIENTATION = COMPONENTS_FACTORY_DEFAULT;

  // Apply orientation mapping to get data in the device's intended coordinate system (NED).
  // This ensures all calibration offsets will be stored relative to the XYZ axis intended.
  OrientedSensorData oriented = applySensorOrientation(mag_event, accel_event, gyro_event);

  // 'Raw' values to match expectation of MotionCal (using oriented data)
  Serial.print("Raw:");
  Serial.print(int(oriented.accelX * 8192 / 9.8)); Serial.print(",");
  Serial.print(int(oriented.accelY * 8192 / 9.8)); Serial.print(",");
  Serial.print(int(oriented.accelZ * 8192 / 9.8)); Serial.print(",");
  Serial.print(int(oriented.gyroX * SENSORS_RADS_TO_DPS * 16)); Serial.print(",");
  Serial.print(int(oriented.gyroY * SENSORS_RADS_TO_DPS * 16)); Serial.print(",");
  Serial.print(int(oriented.gyroZ * SENSORS_RADS_TO_DPS * 16)); Serial.print(",");
  Serial.print(int(oriented.magX * 10)); Serial.print(",");
  Serial.print(int(oriented.magY * 10)); Serial.print(",");
  Serial.print(int(oriented.magZ * 10)); Serial.println("");

  // 'Uni' values to match expectation of MotionCal (using oriented data)
  Serial.print("Uni:");
  Serial.print(oriented.accelX); Serial.print(",");
  Serial.print(oriented.accelY); Serial.print(",");
  Serial.print(oriented.accelZ); Serial.print(",");
  Serial.print(oriented.gyroX, 4); Serial.print(",");
  Serial.print(oriented.gyroY, 4); Serial.print(",");
  Serial.print(oriented.gyroZ, 4); Serial.print(",");
  Serial.print(oriented.magX); Serial.print(",");
  Serial.print(oriented.magY); Serial.print(",");
  Serial.print(oriented.magZ); Serial.println("");

  // While reporting is running we send the oriented magnetometer data to the MagCal
  // logic for collection into bins. This ensures the mag calibration is performed
  // relative in the device's intended coordinate system and displayed to the user.
  bool b_point_added = magCal.addSample(oriented.magX, oriented.magY, oriented.magZ);
  sendMagCalData(b_point_added);
#endif
}
