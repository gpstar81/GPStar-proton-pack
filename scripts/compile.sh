#!/bin/bash

# Perform a compile of all binaries using their respective architectures.

echo ""

# Current build timestamp and major version to be reflected in the builds for ESP32.
MJVER="V6"
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

# ENVIRONMENT INFORMATION
echo "============================================================"
echo "Build Environment Information"
echo "============================================================"
echo "PlatformIO Command: $(which pio)"
if command -v pio &> /dev/null; then
  echo "Python Interpreter: $(head -1 $(which pio) 2>/dev/null || echo 'N/A')"
  echo "Python Version: $(python3 --version 2>&1)"
  echo ""
  echo "PlatformIO System Info:"
  pio system info
else
  echo "ERROR: PlatformIO (pio) not found in PATH"
  exit 1
fi
echo "============================================================"
echo ""

echo "Pausing for 5 seconds to allow review of environment information..."
sleep 5 # Pause for readability before starting the compile process

# PRE-COMPILE TESTS

source ./run_tests.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

# COMPILE PACK/WAND COMPONENTS

source ./compile_pack.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
source ./compile_wand.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

# COMPILE ATTENUATOR AND VARIANTS

source ./compile_attenuator.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
source ./compile_attenuator_extras.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

# COMPILE STANDALONE/ADD-ON DEVICES

source ./compile_blaster.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
source ./compile_gizmo.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
source ./compile_stream.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
source ./compile_pstt.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

# COMPILE GHOST TRAP COMPONENTS

source ./compile_trap_base.sh
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
source ./compile_trap_cartridge.sh
