#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Single-Shot Blaster.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/blaster

echo ""

# Single-Shot Blaster
echo "Building Single-Shot Blaster Binary..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/SingleShot"

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" | grep -iv Retrieved

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Build succeeded!"
else
  echo "Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/blaster/SingleShot.hex
fi
echo "Done."
echo ""
