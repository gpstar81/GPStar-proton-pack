#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/ATmegaEEPROM"

mkdir -p ${BINDIR}/pack/extras

echo ""

# Proton Pack
echo "Building EEPROM Reset Binary..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/pack/extras/ResetEEPROM.hex
fi
echo "Done."
echo ""
