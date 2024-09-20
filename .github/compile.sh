#!/bin/bash

# Perform a full compile of all binaries using the Arduino-CLI and any boards/libraries
# already installed as part of the ArduinoIDE on a local Mac/PC development environment.
# For PC/Windows users, a Cygwin environment may be used to execute this build script.

BINDIR="../binaries"
SRCDIR="../source"


# Current build timestamp to be reflected in the Attenuator build for ESP32.
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo ""

# COMPILE ALL ATMEGA DEVICES

source ./compile_pack.sh

source ./compile_wand.sh

source ./compile_blaster.sh

# COMPILE ATTENUATOR AND VARIANTS

source ./compile_attenuator_nano.sh

source ./compile_attenuator_esp.sh
