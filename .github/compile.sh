#!/bin/bash

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand

# Proton Pack
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/ProtonPack/ProtonPack.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack.hex

# Neutrona Wand
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand.hex

# Attenuator
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:nano -e ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator.hex
