#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Arduino Nano binaries.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras

echo ""

# COMPILE ALL ATTENUATOR VARIANTS

# Attenuator (Arduino - Normal)
echo "Building Attenuator Binary (Arduino - Normal)..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/AttenuatorNano"

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

if [ -f ${PROJECT_DIR}/.pio/build/nanoatmega328/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/nanoatmega328/firmware.hex ${BINDIR}/attenuator/Attenuator-Nano.hex
fi
echo "Done."
echo ""

# Attenuator (Arduino - Standalone)
echo "Building Attenuator Binary (Arduino - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${PROJECT_DIR}/include/Configuration.h

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

if [ -f ${PROJECT_DIR}/.pio/build/nanoatmega328/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/nanoatmega328/firmware.hex ${BINDIR}/attenuator/Attenuator-Nano-Standalone.hex
fi

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${PROJECT_DIR}/include/Configuration.h

rm -f ${PROJECT_DIR}/include/*.h-e

echo "Done."
echo ""
