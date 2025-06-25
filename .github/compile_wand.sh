#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Proton Pack.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/wand/extras

echo ""

# Neutrona Wand
echo "Building Neutrona Wand Binary [ATMega]..."

# Set the project directory based on the source folder
PROJECT_DIR="$SRCDIR/NeutronaWand"

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/wand/NeutronaWand.hex
fi
echo "Done."
echo ""

echo "Building Neutrona Wand Binary [ESP32]..."

# Clean the project before building
#pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
#pio run -e esp32s3 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.hex ${BINDIR}/wand/NeutronaWand-ESP32.hex
fi
echo "Done."
echo ""

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary [ATMega]..."

# Change flag(s) for compilation
sed -i -e 's/bool b_gpstar_benchtest = false/const bool b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/b_gpstar_benchtest = true/\/\/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h

# Clean the project before building
pio run -e atmega2560 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e atmega2560 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/atmega2560/firmware.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex
fi
echo "Done."
echo ""

echo "Building Neutrona Wand (Bench Test) Binary [ESP32]..."

# Clean the project before building
#pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
#pio run -e esp32s3 --project-dir "$PROJECT_DIR" | grep -iv Retrieved

rm -f ${PROJECT_DIR}/include/*.h-e

if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.hex ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest-ESP32.hex
fi
echo "Done."
echo ""

# Restore flag(s) from compilation
sed -i -e 's/const bool b_gpstar_benchtest = true/bool b_gpstar_benchtest = false/' ${PROJECT_DIR}/include/Configuration.h
sed -i -e 's/\/\/b_gpstar_benchtest = true/b_gpstar_benchtest = true/' ${PROJECT_DIR}/include/Serial.h
