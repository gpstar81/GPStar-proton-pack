#!/bin/bash
# burn_efuses_esp32s3.sh
# Purpose: Batch-burn required eFuses for ESP32-S3-WROOM to enable GPIO39-44 usage.
#
# ---
# Installation instructions for espefuse.py:
#
# 1. If you do not have pip for Python 3:
#    On macOS:
#      brew install python3
#      brew install pipx
#      pipx ensurepath
#      <restart terminal>
#      pipx install esptool
#      esptool.py --version
#    Or (universal):
#      curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
#      python3 get-pip.py --user
#
# 2. Install espefuse.py (via esptool):
#      python3 -m pip install --user esptool
#
# 3. Make sure ~/.local/bin is in your PATH:
#      export PATH="$HOME/.local/bin:$PATH"
#
# 4. Test espefuse.py:
#      espefuse.py --help
# ---
#
# Usage:
#   bash burn_efuses_esp32s3.sh [esptool_port]
#
# If [esptool_port] is not specified, the script will attempt to auto-detect the port by probing /dev/cu.usb* and /dev/ttyUSB* devices.
# If multiple possible ports are found, you will be prompted to specify the correct one.
# Run with -h or --help to print usage instructions.

set -e

usage() {
  echo "Usage: $0 [esptool_port]"
  echo "  [esptool_port]: Optional. Serial port for ESP32-S3. If not specified, script attempts auto-detection."
  echo "  Example: $0 /dev/cu.usbserial-0001"
  echo "  Use -h or --help to print this message."
  echo "\nAuto-detection:"
  echo "  The script probes all /dev/cu.usb* and /dev/ttyUSB* devices using espefuse.py to find a compatible ESP32 device."
  echo "  If multiple possible ports are found, you will be prompted to specify the correct one."
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
  usage
  exit 0
fi

# Constants
ESPEFUSE_BIN="espefuse.py"

# Check for espefuse.py or espefuse before anything else
if command -v espefuse >/dev/null 2>&1; then
  ESPEFUSE_BIN="espefuse"
  echo "Found 'espefuse' script at: $(command -v espefuse)"
elif command -v espefuse.py >/dev/null 2>&1; then
  ESPEFUSE_BIN="espefuse.py"
  echo "Found 'espefuse.py' script at: $(command -v espefuse.py)"
else
  echo "Error: espefuse (or espefuse.py) not found in PATH. Please install with 'python3 -m pip install --user esptool' and ensure ~/.local/bin is in your PATH."
  exit 1
fi

# Auto-detect port if not provided
if [[ -z "$1" ]]; then
  # Find all candidate serial ports (macOS and Linux, including /dev/cu.usbmodem* and /dev/cu.usbserial*)
  PORT_CANDIDATES=(/dev/cu.usb* /dev/ttyUSB*)
  VALID_PORTS=()
  for dev in "${PORT_CANDIDATES[@]}"; do
    if [[ -e "$dev" ]]; then
      # Try to probe with espefuse.py summary (suppress output)
      if "$ESPEFUSE_BIN" --port "$dev" summary >/dev/null 2>&1; then
        VALID_PORTS+=("$dev")
      fi
    fi
  done

  if [[ ${#VALID_PORTS[@]} -eq 0 ]]; then
    echo "Error: No compatible ESP32 device found on /dev/cu.usb*, /dev/cu.*, or /dev/ttyUSB*. Is your device connected and in bootloader mode?"
    exit 1
  elif [[ ${#VALID_PORTS[@]} -eq 1 ]]; then
    PORT="${VALID_PORTS[0]}"
    echo "Auto-detected ESP32 device on port: $PORT"
  else
    echo "Multiple compatible ESP32 devices found:"
    for p in "${VALID_PORTS[@]}"; do
      echo "  $p"
    done
    echo "Please specify the correct port as an argument."
    exit 1
  fi
else
  PORT="$1"
fi

echo "Using Device Port: $PORT"

# Function to print current state of the two efuses
print_efuse_status() {
  echo "Current eFuse values for UART_PRINT_CONTROL and DIS_PAD_JTAG:"
  $ESPEFUSE_BIN --port "$PORT" summary | grep -E 'UART_PRINT_CONTROL|DIS_PAD_JTAG' || echo "  (Could not parse efuse values)"
}

print_efuse_status

# Improved check: Only skip if both eFuses are set to the correct value
summary_output="$($ESPEFUSE_BIN --port "$PORT" summary)"
UART_VAL=$(echo "$summary_output" | grep 'UART_PRINT_CONTROL' | awk '{print $NF}' | tr -d '()')
JTAG_VAL=$(echo "$summary_output" | grep 'DIS_PAD_JTAG' | awk '{print $NF}' | tr -d '()')

if [[ "$UART_VAL" =~ ^(3|0b11)$ ]] && [[ "$JTAG_VAL" =~ ^(1|0b1|BURNED)$ ]]; then
  echo "The eFuses are already set (UART_PRINT_CONTROL=$UART_VAL, DIS_PAD_JTAG=$JTAG_VAL). Skipping burn."
  exit 0
fi

echo "This script will permanently burn the following eFuses on your ESP32-S3-WROOM:"
echo "  - UART_PRINT_CONTROL = 3 (disables UART0 boot debug, enables GPIO43/44)"
echo "  - DIS_PAD_JTAG = 1 (disables JTAG on GPIO39-42, enables those pins for GPIO)"
echo "WARNING: THIS OPERATION IS IRREVERSIBLE!"
read -p "Are you sure you want to continue? (yes/NO): " CONFIRM
if [[ "$CONFIRM" != "yes" ]]; then
  echo "Aborted. No changes made."
  exit 0
fi

# Burn UART_PRINT_CONTROL
BURN_CMD=("$ESPEFUSE_BIN" --chip esp32s3 --do-not-confirm --port "$PORT" burn-efuse UART_PRINT_CONTROL 3 DIS_PAD_JTAG 1)
echo "Burning UART_PRINT_CONTROL using: ${BURN_CMD[*]}"
"${BURN_CMD[@]}"

echo "Done. New eFuse summary:"
print_efuse_status

echo "All done. Please power-cycle your ESP32-S3 for changes to take effect."
