#!/bin/bash

# Note: All warnings are set to none to ensure builds run.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand/extras

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

mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex
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

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex
echo "Done."
echo ""

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary..."

# Change flag(s) for compilation
sed -i -e 's/b_gpstar_benchtest = false/b_gpstar_benchtest = true/' ${SRCDIR}/NeutronaWand/Configuration.h
# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex

# Restore flag(s) from compilation
sed -i -e 's/b_gpstar_benchtest = true/b_gpstar_benchtest = false/' ${SRCDIR}/NeutronaWand/Configuration.h

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

mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator-Nano.hex
echo "Done."
echo ""

# Attenuator (ESP32)
echo "Building Attenuator Binary (ESP32)..."

# Update date of compilation
sed -i -E "s/(String build_date = \").*(\";)/\1$TIMESTAMP\2/" ${SRCDIR}/Attenuator/Configuration.h

# --warnings none
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map

mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
mv ${BINDIR}/Attenuator.ino.bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Bootloader.bin
mv ${BINDIR}/Attenuator.ino.partitions.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Partitions.bin
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
rm -f ${BINDIR}/*bootloader.*
rm -f ${BINDIR}/*partitions.*

mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Reset.bin

# Restore flag(s) from compilation
sed -i -e 's/\#define DEBUG_WIRELESS_SETUP/\/\/\#define DEBUG_WIRELESS_SETUP/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\#define DEBUG_SEND_TO_CONSOLE/\/\/\#define DEBUG_SEND_TO_CONSOLE/' ${SRCDIR}/Attenuator/Configuration.h
sed -i -e 's/\#define RESET_AP_SETTINGS/\/\/\#define RESET_AP_SETTINGS/' ${SRCDIR}/Attenuator/Configuration.h

rm -f ${SRCDIR}/Attenuator/*.h-e

echo "Done."
echo ""
