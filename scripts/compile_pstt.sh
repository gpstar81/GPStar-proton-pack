#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Stream Target Trainer.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/PSTT"

mkdir -p ${BINDIR}/pstt/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Create or update BuildInfo.h with the new build date
BUILDINFO_FILE="${PROJECT_DIR}/include/BuildInfo.h"
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
cat <<EOF > "${BUILDINFO_FILE}"
#pragma once
// Reflect the last build date for this binary.
const char* build_date = "${MJVER}_${TIMESTAMP}";
EOF

echo ""

# Proton Stream Target Trainer (ESP32)
echo "Proton Stream Target Trainer [ESP32] - Building..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Proton Stream Target Trainer Binary [ESP32] - Build succeeded!"
else
  echo "Proton Stream Target Trainer Binary [ESP32] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.bin ${BINDIR}/pstt/PSTT.bin
  echo "Firmware copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/pstt/extras/PSTT-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/pstt/extras/PSTT-Partitions.bin
  echo "Partitions copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
