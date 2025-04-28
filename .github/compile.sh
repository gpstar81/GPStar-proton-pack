#!/bin/bash

# Perform a compile of all binaries using their respective architectures.

BINDIR="../binaries"
SRCDIR="../source"

echo ""

# COMPILE CORE DEVICES

source ./compile_pack.sh
source ./compile_wand.sh

# COMPILE ATTENUATOR AND VARIANTS

source ./compile_attenuator_esp.sh
source ./compile_attenuator_esp_extras.sh

# COMPILE STANDALONE/ADDON DEVICES

source ./compile_blaster.sh
source ./compile_gizmo.sh
source ./compile_stream.sh
