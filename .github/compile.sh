#!/bin/bash

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand

# Proton Pack
echo "Building Proton Pack Binary..."
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/ProtonPack/ProtonPack.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex
echo "Done."

# Neutrona Wand
echo "Building Neutrona Wand Binary..."
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex
echo "Done."

# Attenuator
echo "Building Attenuator Binary..."
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:nano -e ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator.hex
echo "Done."

# Neutrona Wand (Bench Test)
echo "Building Neutrona Wand (Bench Test) Binary..."
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

# Change flag(s) for compilation
sed -i -e 's/b_gpstar_benchtest = false/b_gpstar_benchtest = true/g' ${SRCDIR}/NeutronaWand/Configuration.h

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand-BenchTest.hex

# Restore flag(s) from compilation
sed -i -e 's/b_gpstar_benchtest = true/b_gpstar_benchtest = false/g' ${SRCDIR}/NeutronaWand/Configuration.h

echo "Done."
