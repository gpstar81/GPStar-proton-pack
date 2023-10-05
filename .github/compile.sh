#!/bin/bash

BINDIR="../binaries"
SRCDIR="../source"

mkdir -p ${BINDIR}/attenuator
mkdir -p ${BINDIR}/pack
mkdir -p ${BINDIR}/wand

# Proton Pack - Mega
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/ProtonPack/ProtonPack.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/ProtonPack.ino.hex ${BINDIR}/pack/ProtonPack_Mega.hex

# Neutrona Wand - Mega
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand_Mega.hex

# Neutrona Wand - Nano
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:nano -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand_Nano.hex

# Attenuator - Nano
arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:nano -e ${SRCDIR}/Attenuator/Attenuator.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/Attenuator.ino.hex ${BINDIR}/attenuator/Attenuator_Nano.hex

# CUSTOM BINARIES

# Neutrona Wand - Mega - Frutto Vent Light
sed -i -e 's/\/\/#define FRUTTO_VENT_LIGHT/#define FRUTTO_VENT_LIGHT/g' ${SRCDIR}/NeutronaWand/Configuration.h

arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand_Mega_FruttoVentLight.hex

sed -i -e 's/#define FRUTTO_VENT_LIGHT/\/\/#define FRUTTO_VENT_LIGHT/g' ${SRCDIR}/NeutronaWand/Configuration.h

# Neutrona Wand - Mega - Quick Vent
sed -i -e 's/b_quick_vent = false/b_quick_vent = true/g' ${SRCDIR}/NeutronaWand/Configuration.h

arduino-cli compile --output-dir ${BINDIR} -b arduino:avr:mega -e ${SRCDIR}/NeutronaWand/NeutronaWand.ino

rm -f ${BINDIR}/*.bin
rm -f ${BINDIR}/*.eep
rm -f ${BINDIR}/*.elf
rm -f ${BINDIR}/*bootloader.hex

mv ${BINDIR}/NeutronaWand.ino.hex ${BINDIR}/wand/NeutronaWand_Mega_QuickVent.hex

sed -i -e 's/b_quick_vent = true/b_quick_vent = false/g' ${SRCDIR}/NeutronaWand/Configuration.h
