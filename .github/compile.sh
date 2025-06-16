#!/bin/bash

# Perform a compile of all binaries using their respective architectures.

echo ""

# Current build timestamp and major version to be reflected in the builds for ESP32.
MJVER="V6"
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

# COMPILE PACK/WAND COMPONENTS

source ./compile_reset.sh
source ./compile_pack.sh
source ./compile_wand.sh

# COMPILE ATTENUATOR AND VARIANTS

source ./compile_attenuator_esp.sh
source ./compile_attenuator_esp_extras.sh

# COMPILE STANDALONE/ADD-ON DEVICES

source ./compile_blaster.sh
source ./compile_gizmo.sh
source ./compile_stream.sh
