#!/bin/bash

# Compile only the main ESP32 binary. Useful for a quick sanity test during development.

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator/extras
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand/extras

# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

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

mv ${BINDIR}/Attenuator.ino.bin ${BINDIR}/attenuator/Attenuator-ESP32.bin
mv ${BINDIR}/Attenuator.ino.bootloader.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Bootloader.bin
mv ${BINDIR}/Attenuator.ino.partitions.bin ${BINDIR}/attenuator/extras/Attenuator-ESP32-Partitions.bin
echo "Done."
echo ""
