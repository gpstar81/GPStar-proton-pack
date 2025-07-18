#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/NeutronaWand"

mkdir -p ${BINDIR}/wand/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Update date of compilation
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

echo ""

# Neutrona Wand
echo "Neutrona Wand Binary [ATMega] - Building..."

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Neutrona Wand Binary [ATMega] - Build succeeded!"
else
  echo "Neutrona Wand Binary [ATMega] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/wand/NeutronaWand.hex
  echo "Firmware copy completed."
fi
echo ""

echo "Neutrona Wand Binary [ESP32] - Building..."

# Clean the project before building
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Neutrona Wand Binary [ESP32] - Build succeeded!"
else
  echo "Neutrona Wand Binary [ESP32] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ${BINDIR}/wand/NeutronaWandII.bin
  echo "Firmware copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/wand/extras/NeutronaWandII-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/wand/extras/NeutronaWandII-Partitions.bin
  echo "Partitions copy completed."
fi
echo ""

# Neutrona Wand (Bench Test)
echo "Neutrona Wand (Bench Test) Binary [ATMega] - Building..."

# Change flag(s) for compilation
sed -i -e 's/bool b_gpstar_benchtest = false/const bool b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/b_gpstar_benchtest = true/\/\/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Neutrona Wand (Bench Test) Binary [ATMega] - Build succeeded!"
else
  # Restore flag(s) from compilation
  sed -i -e 's/const bool b_gpstar_benchtest = true/bool b_gpstar_benchtest = false/' ${PROJECT_DIR}/include/Configuration.h
  sed -i -e 's/\/\/b_gpstar_benchtest = true/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h

  echo "Neutrona Wand (Bench Test) Binary [ATMega] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex
  echo "Firmware copy completed."
fi
echo ""

echo "Neutrona Wand (Bench Test) Binary [ESP32] - Building..."

# Clean the project before building
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --jobs 4

# Restore flag(s) from compilation
sed -i -e 's/const bool b_gpstar_benchtest = true/bool b_gpstar_benchtest = false/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/b_gpstar_benchtest = true/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Neutrona Wand (Bench Test) Binary [ESP32] - Build succeeded!"
else
  echo "Neutrona Wand (Bench Test) Binary [ESP32] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ${BINDIR}/wand/extras/NeutronaWandII-BenchTest.bin
  echo "Firmware copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
