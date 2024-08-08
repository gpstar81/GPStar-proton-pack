#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand/extras

# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# Proton Pack
echo "Building Proton Pack Binary..."

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/ProtonPack/ProtonPack.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/ProtonPack.ino.hex ]; then
  mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex
fi
echo "Done."
echo ""

# Neutrona Wand
echo "Building Neutrona Wand Binary..."

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/NeutronaWand.ino.hex ]; then
  mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex
fi
echo "Done."
echo ""

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary..."

# Change flag(s) for compilation
sed -i -e 's/bool b_gpstar_benchtest = false/const bool b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Configuration.h
sed -i -e 's/b_gpstar_benchtest = true/\/\/b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Serial.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/NeutronaWand.ino.hex ]; then
  mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex
fi

# Restore flag(s) from compilation
sed -i -e 's/const bool b_gpstar_benchtest = true/bool b_gpstar_benchtest = false/' ${SRCDIR}/NeutronaWand/Configuration.h
sed -i -e 's/\/\/b_gpstar_benchtest = true/b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Serial.h

rm -f ${SRCDIR}/NeutronaWand/*.h-e

echo "Done."
echo ""

# Attenuator (Arduino)
echo "Building Attenuator Binary (Arduino)..."

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/Attenuator.ino.hex ]; then
  mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator-Nano.hex
fi
echo "Done."
echo ""

# Attenuator (Arduino - Standalone)
echo "Building Attenuator Binary (Arduino - Standalone)..."

# Change flag(s) for compilation
sed -i -e 's/b_wait_for_pack = true/b_wait_for_pack = false/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

if [ -f ${BINDIR}/Attenuator.ino.hex ]; then
  mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator-Nano-Standalone.hex
fi

# Restore flag(s) from compilation
sed -i -e 's/b_wait_for_pack = false/b_wait_for_pack = true/' ${SRCDIR}/Attenuator/Configuration.h

rm -f ${SRCDIR}/Attenuator/*.h-e

echo "Done."
echo ""

# Attenuator (ESP32)
echo "Building Attenuator Binary (ESP32)..."

# Update date of compilation
echo "Updating Build Timestamp:" $TIMESTAMP
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"$TIMESTAMP"'\2/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin

if [ -f ${BINDIR}/Attenuator.ino.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
fi
if [ -f ${BINDIR}/Attenuator.ino.bootloader.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Bootloader.bin
fi
if [ -f ${BINDIR}/Attenuator.ino.partitions.bin ]; then
  mv ${BINDIR}/Attenuator.ino.partitions.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Partitions.bin
fi
echo "Done."
echo ""

# Attenuator (ESP32 - WiFi Reset)
echo "Building Attenuator Binary (ESP32 - WiFi Reset)..."

# Change flag(s) for compilation
sed -i -e 's/\/\/\#define DEBUG_WIRELESS_SETUP/\#define DEBUG_WIRELESS_SETUP/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\/\/\#define DEBUG_SEND_TO_CONSOLE/\#define DEBUG_SEND_TO_CONSOLE/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\/\/\#define RESET_AP_SETTINGS/\#define RESET_AP_SETTINGS/' ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*.merged.bin
rm -f ${BINDIR}/*bootloader.*
rm -f ${BINDIR}/*partitions.*

if [ -f ${BINDIR}/Attenuator.ino.bin ]; then
  mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Reset.bin
fi

# Restore flag(s) from compilation
sed -i -e 's/\#define DEBUG_WIRELESS_SETUP/\/\/\#define DEBUG_WIRELESS_SETUP/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\#define DEBUG_SEND_TO_CONSOLE/\/\/\#define DEBUG_SEND_TO_CONSOLE/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\#define RESET_AP_SETTINGS/\/\/\#define RESET_AP_SETTINGS/' ${SRCDIR}/Attenuator/Configuration.h

rm -f ${SRCDIR}/Attenuator/*.h-e

echo "Done."
echo ""
