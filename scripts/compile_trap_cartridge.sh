#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the main ATtiny binary.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/GhostTrapCartridge"

mkdir -p ${BINDIR}/trap/extras

echo ""

# GhostTrap Cartridge (ATtiny)
echo "GhostTrap Cartridge Binary (ATtiny) - Building..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "GhostTrap Cartridge Binary (ATtiny) - Build succeeded!"
else
  echo "GhostTrap Cartridge Binary (ATtiny) - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/attiny1616/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/attiny1616/firmware.hex ${BINDIR}/trap/GhostTrap-Cartridge.hex
  echo "Firmware copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e