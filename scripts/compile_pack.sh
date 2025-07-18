#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/ProtonPack"

mkdir -p ${BINDIR}/pack/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Update date of compilation
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

echo ""

# Proton Pack
echo "Proton Pack Binary [ATMega] - Building..."

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Proton Pack Binary [ATMega] - Build succeeded!"
else
  echo "Proton Pack Binary [ATMega] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/pack/ProtonPack.hex
  echo "Firmware copy completed."
fi
echo ""

echo "Proton Pack Binary [ESP32] - Building..."

# Clean the project before building
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Proton Pack Binary [ESP32] - Build succeeded!"
else
  echo "Proton Pack Binary [ESP32] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ${BINDIR}/pack/ProtonPackII.bin
  echo "Firmware copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/pack/extras/ProtonPackII-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/pack/extras/ProtonPackII-Partitions.bin
  echo "Partitions copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
