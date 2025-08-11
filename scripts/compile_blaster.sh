#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Single-Shot Blaster.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/SingleShot"

echo ""

# Single-Shot Blaster
echo "Single-Shot Blaster Binary [ATMega] - Building..."

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Building Single-Shot Blaster Binary [ATMega] - Build succeeded!"
else
  echo "Building Single-Shot Blaster Binary [ATMega] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/blaster/SingleShot.hex
fi
echo "Firmware copy completed."
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
