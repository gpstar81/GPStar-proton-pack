/***************************************************************************
  This is an example for the Adafruit AHRS library
  It will look for a supported magnetometer and output
  PJRC Motion Sensor Calibration Tool-compatible serial data

  PJRC & Adafruit invest time and resources providing this open source code,
  please support PJRC and open-source hardware by purchasing products
  from PJRC!

  Written by PJRC, adapted by Limor Fried for Adafruit Industries.
  https://github.com/adafruit/Adafruit_AHRS/tree/master/examples/calibration
 ***************************************************************************/

#define IMU_SCL 47
#define IMU_SDA 48

#include <Arduino.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Wire.h>

Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer;

#include "LSM6DS_LIS3MDL.h"

sensors_event_t mag_event, gyro_event, accel_event;

int loopcount = 0;

void setup(void) {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  Serial.println(F("Adafruit AHRS - IMU Calibration!"));

  Serial.println("Calibration filesys test");
  Wire1.begin(IMU_SDA, IMU_SCL, 400000UL);

  if (!init_sensors()) {
    Serial.println("Failed to find sensors");
    while (1) delay(10);
  }
  accelerometer->printSensorDetails();
  gyroscope->printSensorDetails();
  magnetometer->printSensorDetails();

  setup_sensors();
}

void loop() {
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

  // unified data
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
  loopcount++;

  delay(10); 
}
