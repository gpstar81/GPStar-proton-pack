#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/pack

echo ""

# Proton Pack
echo "Building Proton Pack Binary [ATMega]..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/ProtonPack"

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/pack/ProtonPack.hex
fi
echo "Done."
echo ""

echo "Building Proton Pack Binary [ESP32]..."

# Clean the project before building
#pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
#pio run -e esp32s3 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/esp32s3dev/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3dev/firmware.hex ${BINDIR}/pack/ProtonPack-ESP32.hex
fi
echo "Done."
echo ""
