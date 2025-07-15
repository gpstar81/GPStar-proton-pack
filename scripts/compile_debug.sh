#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the ESP32-S3 debug code.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/DebugESP"

mkdir -p ${BINDIR}/DebugESP/extras

echo ""

# DebugESP
echo "Debug [ESP32-S3] - Building..."

# Clean the project before building
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --target clean

# Compile the PlatformIO project
pio run -e esp32s3 --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Debug [ESP32-S3] - Build succeeded!"
else
  echo "Debug [ESP32-S3] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/firmware.elf ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/firmware.elf ${BINDIR}/DebugESP/DebugESP-ESP32.bin
  echo "Firmware copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/bootloader.bin ${BINDIR}/DebugESP/extras/DebugESP-ESP32-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32s3/partitions.bin ${BINDIR}/DebugESP/extras/DebugESP-ESP32-Partitions.bin
  echo "Partitions copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
