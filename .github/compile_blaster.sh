#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.
#
# This script compiles only the Single-Shot Blaster.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/blaster

# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# Single-Shot Blaster
echo "Building Single-Shot Blaster Binary..."

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/SingleShot/SingleShot.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/SingleShot.ino.hex ]; then
  mv ${BINDIR}/SingleShot.ino.hex ${BINDIR}/blaster/SingleShot.hex
fi
echo "Done."
echo ""
