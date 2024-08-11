#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/wand/extras

# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# Neutrona Wand
echo "Building Neutrona Wand Binary..."

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/NeutronaWand.ino.hex ]; then
  mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex
fi
echo "Done."
echo ""

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary..."

# Change flag(s) for compilation
sed -i -e 's/bool b_gpstar_benchtest = false/const bool b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Configuration.h
sed -i -e 's/b_gpstar_benchtest = true/\/\/b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Serial.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/NeutronaWand.ino.hex ]; then
  mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex
fi

# Restore flag(s) from compilation
sed -i -e 's/const bool b_gpstar_benchtest = true/bool b_gpstar_benchtest = false/' ${SRCDIR}/NeutronaWand/Configuration.h
sed -i -e 's/\/\/b_gpstar_benchtest = true/b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Serial.h

rm -f ${SRCDIR}/NeutronaWand/*.h-e

echo "Done."
echo ""
