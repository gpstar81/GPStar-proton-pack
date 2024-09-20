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

# Update date of compilation
echo "Updating Build Timestamp:" $TIMESTAMP
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"$TIMESTAMP"'\2/' ${PROJECT_DIR}/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${PROJECT_DIR}/AttenuatorESP32.ino

# EXPERIMENTAL: Atttempting to compile with an alternative partition table to allow for larger OTA firmware sizes at the expense of a smaller filesystem area.
# We should wait for the next major release of the codebase before we require users to change their partition scheme as this will require a re-flash over USB!
#arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries --board-options PartitionScheme=min_spiffs ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin

if [ -f ${BINDIR}/AttenuatorESP32.ino.bin ]; then
  mv ${BINDIR}/AttenuatorESP32.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
fi
echo "Done."
echo ""

# Attenuator (ESP32 - Standalone)
echo "Building Attenuator Binary (ESP32 - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${PROJECT_DIR}/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${PROJECT_DIR}/AttenuatorESP32.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin
rm -f ${BINDIR}/*bootloader.*
rm -f ${BINDIR}/*partitions.*

if [ -f ${BINDIR}/AttenuatorESP32.ino.bin ]; then
  mv ${BINDIR}/AttenuatorESP32.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32-Standalone.bin
fi

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${PROJECT_DIR}/Configuration.h

rm -f ${PROJECT_DIR}/*.h-e

echo "Done."
echo ""

# Attenuator (ESP32 - WiFi Reset)
echo "Building Attenuator Binary (ESP32 - WiFi Reset)..."

# Change flag(s) for compilation
sed -i -e 's/\/\/\#define DEBUG_WIRELESS_SETUP/\#define DEBUG_WIRELESS_SETUP/' ${PROJECT_DIR}/Configuration.h
sed -i -e 's/\/\/\#define DEBUG_SEND_TO_CONSOLE/\#define DEBUG_SEND_TO_CONSOLE/' ${PROJECT_DIR}/Configuration.h
sed -i -e 's/\/\/\#define RESET_AP_SETTINGS/\#define RESET_AP_SETTINGS/' ${PROJECT_DIR}/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${PROJECT_DIR}/AttenuatorESP32.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin
rm -f ${BINDIR}/*bootloader.*
rm -f ${BINDIR}/*partitions.*

if [ -f ${BINDIR}/AttenuatorESP32.ino.bin ]; then
  mv ${BINDIR}/AttenuatorESP32.ino.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Reset.bin
fi

# Restore flag(s) from compilation
sed -i -e 's/\#define DEBUG_WIRELESS_SETUP/\/\/\#define DEBUG_WIRELESS_SETUP/' ${PROJECT_DIR}/Configuration.h
sed -i -e 's/\#define DEBUG_SEND_TO_CONSOLE/\/\/\#define DEBUG_SEND_TO_CONSOLE/' ${PROJECT_DIR}/Configuration.h
sed -i -e 's/\#define RESET_AP_SETTINGS/\/\/\#define RESET_AP_SETTINGS/' ${PROJECT_DIR}//Configuration.h

rm -f ${PROJECT_DIR}/*.h-e

echo "Done."
echo ""
if [ -f ${BINDIR}/AttenuatorESP32.ino.bootloader.bin ]; then
  mv ${BINDIR}/AttenuatorESP32.ino.bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Bootloader.bin
fi
if [ -f ${BINDIR}/AttenuatorESP32.ino.partitions.bin ]; then
  mv ${BINDIR}/AttenuatorESP32.ino.partitions.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Partitions.bin
fi
echo "Done."
echo ""
