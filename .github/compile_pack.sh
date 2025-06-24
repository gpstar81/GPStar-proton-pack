#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/pack

echo ""

# Proton Pack
echo "Building Proton Pack Binary..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/ProtonPack"

#if [ -f ${BINDIR}/ProtonPack.ino.hex ]; then
#  mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex
#fi

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/pack/ProtonPack.hex
fi
echo "Done."
echo ""
