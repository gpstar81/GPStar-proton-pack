#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles all extra Attenuator ESP32 binaries.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/AttenuatorESP32"

mkdir -p ${BINDIR}/attenuator/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Update date of compilation
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

echo ""

# Attenuator (ESP32 - Standalone)
echo "Attenuator Binary (ESP32 - Standalone) - Building..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${PROJECT_DIR}/include/Configuration.h

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${PROJECT_DIR}/include/Configuration.h

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Attenuator Binary (ESP32 - Standalone) - Build succeeded!"
else
  echo "Attenuator Binary (ESP32 - Standalone) - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/attenuator/Attenuator-Standalone.bin
  echo "Firmware copy completed."
fi
echo ""

# Attenuator (ESP32 - WiFi Reset)
echo "Attenuator Binary (ESP32 - WiFi Reset) - Building..."

# Change flag(s) for compilation
sed -i -e 's/\/\/\#define DEBUG_WIRELESS_SETUP/\#define DEBUG_WIRELESS_SETUP/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/\#define DEBUG_SEND_TO_CONSOLE/\#define DEBUG_SEND_TO_CONSOLE/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/\#define RESET_AP_SETTINGS/\#define RESET_AP_SETTINGS/' ${PROJECT_DIR}/include/Configuration.h

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

# Restore flag(s) from compilation
sed -i -e 's/\#define DEBUG_WIRELESS_SETUP/\/\/\#define DEBUG_WIRELESS_SETUP/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\#define DEBUG_SEND_TO_CONSOLE/\/\/\#define DEBUG_SEND_TO_CONSOLE/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\#define RESET_AP_SETTINGS/\/\/\#define RESET_AP_SETTINGS/' ${PROJECT_DIR}/include/Configuration.h

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Attenuator Binary (ESP32 - WiFi Reset) - Build succeeded!"
else
  echo "Attenuator Binary (ESP32 - WiFi Reset) - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/attenuator/extras/Attenuator-Reset.bin
  echo "Firmware copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
