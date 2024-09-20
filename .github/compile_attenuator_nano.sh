#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.
#
# This script compiles only the extra ESP32 binaries.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras

echo ""

# COMPILE ALL ATTENUATOR VARIANTS

# Attenuator (Arduino - Normal)
echo "Building Attenuator Binary (Arduino - Normal)..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/AttenuatorNano"

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --export-binaries ${PROJECT_DIR}/AttenuatorNano.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/AttenuatorNano.ino.hex ]; then
  mv ${BINDIR}/AttenuatorNano.ino.hex ${BINDIR}/attenuator/Attenuator-Nano.hex
fi
echo "Done."
echo ""

# Attenuator (Arduino - Standalone)
echo "Building Attenuator Binary (Arduino - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${PROJECT_DIR}/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --export-binaries ${PROJECT_DIR}/AttenuatorNano.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/AttenuatorNano.ino.hex ]; then
  mv ${BINDIR}/AttenuatorNano.ino.hex ${BINDIR}/attenuator/Attenuator-Nano-Standalone.hex
fi

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${PROJECT_DIR}/Configuration.h

rm -f ${PROJECT_DIR}/*.h-e

echo "Done."
echo ""
