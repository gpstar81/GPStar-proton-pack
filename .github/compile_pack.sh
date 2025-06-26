#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/pack/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Update date of compilation
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

echo ""

# Proton Pack
echo "Building Proton Pack Binary [ATMega]..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/ProtonPack"

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Build succeeded!"
else
  echo "Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/pack/ProtonPack.hex
fi
echo "Done."
echo ""

echo "Building Proton Pack Binary [ESP32]..."

# Clean the project before building
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e esp32s3 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.elf ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.elf ${BINDIR}/pack/ProtonPack-ESP32.bin
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/pack/extras/ProtonPack-ESP32-Bootloader.bin
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/pack/extras/ProtonPack-ESP32-Partitions.bin
fi
echo "Done."
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
