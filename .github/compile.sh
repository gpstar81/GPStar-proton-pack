#!/bin/bash

# Note: All warnings are set to none to ensure builds run.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand/extras

# Proton Pack
echo "Building Proton Pack Binary..."
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --warnings none --export-binaries ${SRCDIR}/ProtonPack/ProtonPack.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex
echo "Done."

# Neutrona Wand
echo "Building Neutrona Wand Binary..."
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --warnings none --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex
echo "Done."

# Attenuator (Arduino)
echo "Building Attenuator Binary (Arduino)..."
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:nano --warnings none --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator-Nano.hex
echo "Done."

# Attenuator (ESP32)
echo "Building Attenuator Binary (ESP32)..."
arduino-cli compile --output-dir ${BINDIR} --fqbn esp32:esp32:esp32 --warnings none --export-binaries ${SRCDIR}/Attenuator/Attenuator.ino

# Keep any .bin files
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*.map
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
echo "Done."

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary..."
arduino-cli compile --output-dir ${BINDIR} --fqbn arduino:avr:mega --warnings none --export-binaries ${SRCDIR}/NeutronaWand/NeutronaWand.ino

# Change flag(s) for compilation
sed -i -e 's/b_gpstar_benchtest = false/b_gpstar_benchtest = true/g' ${SRCDIR}/NeutronaWand/Configuration.h

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/extras/NeutronaWand-BenchTest.hex

# Restore flag(s) from compilation
sed -i -e 's/b_gpstar_benchtest = true/b_gpstar_benchtest = false/g' ${SRCDIR}/NeutronaWand/Configuration.h

rm -f ${SRCDIR}/NeutronaWand/*.h-e

echo "Done."
