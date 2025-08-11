#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/ATmegaEEPROM"

mkdir -p ${BINDIR}/pack/extras
mkdir -p ${BINDIR}/blaster/extras
mkdir -p ${BINDIR}/wand/extras

echo ""

# Proton Pack
echo "EEPROM Reset Binary [ATMega] - Building..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/pack/extras/ResetEEPROM.hex
  cp ${BINDIR}/pack/extras/ResetEEPROM.hex ${BINDIR}/blaster/extras/ResetEEPROM.hex
  cp ${BINDIR}/pack/extras/ResetEEPROM.hex ${BINDIR}/wand/extras/ResetEEPROM.hex
  echo "Firmware copy completed."
fi
echo ""
