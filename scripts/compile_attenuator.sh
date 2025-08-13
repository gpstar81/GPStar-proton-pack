#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the main Attenuator ESP32 binary.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/Attenuator"

mkdir -p ${BINDIR}/attenuator/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Update date of compilation
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

echo ""

# Attenuator (ESP32 - Normal)
echo "Attenuator Binary (ESP32 - Normal) - Building..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Building Attenuator Binary (ESP32 - Normal) - Build succeeded!"
else
  echo "Building Attenuator Binary (ESP32 - Normal) - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/attenuator/Attenuator.bin
  echo "Firmware copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ${BINDIR}/attenuator/extras/Attenuator-Partitions.bin
  echo "Partitions copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
