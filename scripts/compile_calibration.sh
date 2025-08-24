#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Neutrona Wand II calibration program.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/CalibrateIMU"

mkdir -p ${BINDIR}/wand/extras
mkdir -p ${BINDIR}/blaster/extras

echo ""

echo "IMU Calibration Program [ESP32-S3] - Building..."

# Clean the project before building
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "IMU Calibration Program [ESP32-S3] - Build succeeded!"
else
  echo "IMU Calibration Program [ESP32-S3] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ]; then
  cp ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/blaster/extras/SingleShotII-Bootloader.bin
  mv ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/wand/extras/NeutronaWandII-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ]; then
  cp ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/blaster/extras/SingleShotII-Partitions.bin
  mv ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/wand/extras/NeutronaWandII-Partitions.bin
  echo "Partitions copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ]; then
  cp ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ${BINDIR}/blaster/extras/SingleShotII-Calibration.bin
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ${BINDIR}/wand/extras/NeutronaWandII-Calibration.bin
  echo "Firmware copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
