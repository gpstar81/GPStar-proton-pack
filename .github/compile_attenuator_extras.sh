#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.
#
# This script compiles only the extra ESP32 binaries.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras

# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# COMPILE ALL ATTENUATOR VARIANTS

# Attenuator (Arduino - Normal)
echo "Building Attenuator Binary (Arduino - Normal)..."

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/Attenuator.ino.hex ]; then
  mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator-Nano.hex
fi
echo "Done."
echo ""

# Attenuator (Arduino - Standalone)
echo "Building Attenuator Binary (Arduino - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/Attenuator.ino.hex ]; then
  mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator-Nano-Standalone.hex
fi

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${SRCDIR}/Attenuator/Configuration.h

rm -f ${SRCDIR}/Attenuator/*.h-e

echo "Done."
echo ""

# Attenuator (ESP32 - Standalone)
echo "Building Attenuator Binary (ESP32 - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin
rm -f ${BINDIR}/*bootloader.*
rm -f ${BINDIR}/*partitions.*

if [ -f ${BINDIR}/Attenuator.ino.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32-Standalone.bin
fi

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${SRCDIR}/Attenuator/Configuration.h

rm -f ${SRCDIR}/Attenuator/*.h-e

echo "Done."
echo ""

# Attenuator (ESP32 - WiFi Reset)
echo "Building Attenuator Binary (ESP32 - WiFi Reset)..."

# Change flag(s) for compilation
sed -i -e 's/\/\/\#define DEBUG_WIRELESS_SETUP/\#define DEBUG_WIRELESS_SETUP/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\/\/\#define DEBUG_SEND_TO_CONSOLE/\#define DEBUG_SEND_TO_CONSOLE/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\/\/\#define RESET_AP_SETTINGS/\#define RESET_AP_SETTINGS/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin
rm -f ${BINDIR}/*bootloader.*
rm -f ${BINDIR}/*partitions.*

if [ -f ${BINDIR}/Attenuator.ino.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Reset.bin
fi

# Restore flag(s) from compilation
sed -i -e 's/\#define DEBUG_WIRELESS_SETUP/\/\/\#define DEBUG_WIRELESS_SETUP/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\#define DEBUG_SEND_TO_CONSOLE/\/\/\#define DEBUG_SEND_TO_CONSOLE/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\#define RESET_AP_SETTINGS/\/\/\#define RESET_AP_SETTINGS/' ${SRCDIR}/Attenuator/Configuration.h

rm -f ${SRCDIR}/Attenuator/*.h-e

echo "Done."
echo ""
