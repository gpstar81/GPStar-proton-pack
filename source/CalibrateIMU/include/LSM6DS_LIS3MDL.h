#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3mdl;

#include <Adafruit_LSM6DS3TRC.h>
Adafruit_LSM6DS3TRC lsm6ds;

bool init_sensors(void) {
  if (!lsm6ds.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1) || !lis3mdl.begin_I2C(LIS3MDL_I2CADDR_DEFAULT, &Wire1)) {
    return false;
  }
  accelerometer = lsm6ds.getAccelerometerSensor();
  gyroscope = lsm6ds.getGyroSensor();
  magnetometer = &lis3mdl;

  return true;
}

void setup_sensors(void) {
  // set lowest range
  lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);

  // set maximum refresh rate
  lis3mdl.setDataRate(LIS3MDL_DATARATE_1000_HZ);
}
