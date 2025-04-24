#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles all extra Attenuator ESP32 binaries.

BINDIR="../binaries"
SRCDIR="../source"

# Define the major version for this build
MJVER="V5"

mkdir -p ${BINDIR}/attenuator/extras

# Current build timestamp to be reflected in the build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/AttenuatorESP32"

# Update date of compilation
echo "Updating Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

# Attenuator (ESP32 - Standalone)
echo "Building Attenuator Binary (ESP32 - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${PROJECT_DIR}/include/Configuration.h

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${PROJECT_DIR}/include/Configuration.h

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/attenuator/Attenuator-ESP32-Standalone.bin
fi
echo "Done."
echo ""

# Attenuator (ESP32 - WiFi Reset)
echo "Building Attenuator Binary (ESP32 - WiFi Reset)..."

# Change flag(s) for compilation
sed -i -e 's/\/\/\#define DEBUG_WIRELESS_SETUP/\#define DEBUG_WIRELESS_SETUP/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/\#define DEBUG_SEND_TO_CONSOLE/\#define DEBUG_SEND_TO_CONSOLE/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/\#define RESET_AP_SETTINGS/\#define RESET_AP_SETTINGS/' ${PROJECT_DIR}/include/Configuration.h

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

# Restore flag(s) from compilation
sed -i -e 's/\#define DEBUG_WIRELESS_SETUP/\/\/\#define DEBUG_WIRELESS_SETUP/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\#define DEBUG_SEND_TO_CONSOLE/\/\/\#define DEBUG_SEND_TO_CONSOLE/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\#define RESET_AP_SETTINGS/\/\/\#define RESET_AP_SETTINGS/' ${PROJECT_DIR}/include/Configuration.h

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Reset.bin
fi
echo "Done."
echo ""
