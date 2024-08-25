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

# Update date of compilation
echo "Updating Build Timestamp:" $TIMESTAMP
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"$TIMESTAMP"'\2/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# EXPERIMENTAL: Atttempting to compile with an alternative partition table to allow for larger OTA firmware sizes at the expense of a smaller filesystem area.
# We should wait for the next major release of the codebase before we require users to change their partition scheme as this will require a re-flash over USB!
#arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries --board-options PartitionScheme=min_spiffs ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin

if [ -f ${BINDIR}/Attenuator.ino.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
fi
if [ -f ${BINDIR}/Attenuator.ino.bootloader.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Bootloader.bin
fi
if [ -f ${BINDIR}/Attenuator.ino.partitions.bin ]; then
  mv ${BINDIR}/Attenuator.ino.partitions.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Partitions.bin
fi
echo "Done."
echo ""
