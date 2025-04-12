#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/wand/extras

echo ""

# Neutrona Wand
echo "Building Neutrona Wand Binary..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/NeutronaWand"

#if [ -f ${BINDIR}/NeutronaWand.ino.hex ]; then
#  mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex
#fi

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

if [ -f ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/wand/NeutronaWand.hex
fi
echo "Done."
echo ""

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary..."

# Change flag(s) for compilation
sed -i -e 's/bool b_gpstar_benchtest = false/const bool b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/b_gpstar_benchtest = true/\/\/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR"

# Restore flag(s) from compilation
sed -i -e 's/const bool b_gpstar_benchtest = true/bool b_gpstar_benchtest = false/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/b_gpstar_benchtest = true/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/megaatmega2560/firmware.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex
fi
echo "Done."
echo ""
