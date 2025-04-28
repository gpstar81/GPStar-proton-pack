#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the main ESP32 binary.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/stream/extras

# Current build timestamp to be reflected in the build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/StreamEffects"

# Update date of compilation
echo "Updating Build Timestamp:" $TIMESTAMP
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"$TIMESTAMP"'\2/' ${PROJECT_DIR}/include/Configuration.h

# StreamEffects (ESP32 - Normal)
echo "Building StreamEffects Binary (ESP32)..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/stream/StreamEffects-ESP32.bin
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ${BINDIR}/stream/extras/StreamEffects-ESP32-Bootloader.bin
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ${BINDIR}/stream/extras/StreamEffects-ESP32-Partitions.bin
fi
echo "Done."
echo ""
