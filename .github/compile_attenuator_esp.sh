#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.
#
# This script compiles only the main ESP32 binary.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras

# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# Attenuator (ESP32 - Normal)
echo "Building Attenuator Binary (ESP32 - Normal)..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/AttenuatorESP32"

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

# Update date of compilation
echo "Updating Build Timestamp:" $TIMESTAMP
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"$TIMESTAMP"'\2/' ${PROJECT_DIR}/include/Configuration.h

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Bootloader.bin
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Partitions.bin
fi
echo "Done."
echo ""

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
