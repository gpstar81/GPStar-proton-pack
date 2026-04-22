#!/bin/bash

echo ""

# Common directories
SRCDIR="../source"
SHARED_DIR="$SRCDIR/SharedLib"

# Current build timestamp and major version to be reflected in the builds for ESP32.
MJVER="V6"
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

echo "Running sanity tests for $SHARED_DIR/DeviceState..."

# Clean build files
pio run --project-dir "$SHARED_DIR/DeviceState" --target clean

# Run unit tests
pio test --project-dir "$SHARED_DIR/DeviceState" -v

# Clean build files
pio run --project-dir "$SHARED_DIR/MagCalibration" --target clean

# Run unit tests
pio test --project-dir "$SHARED_DIR/MagCalibration" -v