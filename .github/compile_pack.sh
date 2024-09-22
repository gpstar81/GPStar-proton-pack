#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.
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

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${PROJECT_DIR}/ProtonPack.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/ProtonPack.ino.hex ]; then
  mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex
fi

# Clean the project before building
#pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
#pio run --project-dir "$PROJECT_DIR"

#if [ -f ${BINDIR}/.pio/build/megaatmega2560/firmware.hex ]; then
#  mv ${BINDIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/pack/ProtonPack.hex
#fi
echo "Done."
echo ""
