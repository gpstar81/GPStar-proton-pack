#!/bin/bash

# Perform a full compile of all binaries using PlatformIO (pio).
#
# This script compiles only the Stream Effects ESP32 binary.

BINDIR="../binaries"
SRCDIR="../source"
PROJECT_DIR="$SRCDIR/StreamEffects"
ASSETS_DIR="$PROJECT_DIR/assets"
GZIP_EXTS="svg,ico,stl,min.js"

mkdir -p ${BINDIR}/stream/extras

# Current build timestamp and major version to be reflected in the build for ESP32.
MJVER="${MJVER:="V6"}"
TIMESTAMP="${TIMESTAMP:=$(date +"%Y%m%d%H%M%S")}"

# Update date of compilation
echo "Setting Build Timestamp: ${MJVER}_${TIMESTAMP}"
sed -i -e 's/\(String build_date = "\)[^"]*\(";\)/\1'"${MJVER}_${TIMESTAMP}"'\2/' ${PROJECT_DIR}/include/Configuration.h

echo ""

# StreamEffects (ESP32 - Normal)
echo "StreamEffects Binary [ESP32] - Building..."

# Clean the project before building
pio run --project-dir "$PROJECT_DIR" --target clean

# GZIP selected web assets prior to compilation
# Split the comma-delimited GZIP_EXTS into an array (exts) and iterate.
IFS=',' read -r -a exts <<< "$GZIP_EXTS"

for ext in "${exts[@]}"; do
  find "${ASSETS_DIR}" -type f -name "*.${ext}" -print0 \
    | while IFS= read -r -d '' file; do
        # Compress if .gz missing or original is newer than the .gz
        if [ ! -f "${file}.gz" ] || [ "${file}" -nt "${file}.gz" ]; then
          printf 'Compressing: %s\n' "$file"
          # Quietly compress at maximum, force overwrite, keep original
          gzip -9 -fkq -- "$file"
        fi
      done
done

# Remove any stale .gz files whose source no longer exists
find "${ASSETS_DIR}" -name '*.gz' -print0 \
  | while IFS= read -r -d '' gz; do
      orig="${gz%.gz}"
      [ ! -f "$orig" ] && rm -f -- "$gz"
    done

# Compile the PlatformIO project
pio run --project-dir "$PROJECT_DIR" --jobs 4

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "StreamEffects Binary [ESP32] - Build succeeded!"
else
  echo "StreamEffects Binary [ESP32] - Build failed!"
  exit 1
fi

# Copy the new firmware to the expected binaries directory
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/firmware.bin ${BINDIR}/stream/StreamEffects.bin
  echo "Firmware copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/bootloader.bin ${BINDIR}/stream/extras/StreamEffects-Bootloader.bin
  echo "Bootloader copy completed."
fi
if [ -f ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ]; then
  mv ${PROJECT_DIR}/.pio/build/esp32dev/partitions.bin ${BINDIR}/stream/extras/StreamEffects-Partitions.bin
  echo "Partitions copy completed."
fi
echo ""

rm -f ${PROJECT_DIR}/include/*.h-e
