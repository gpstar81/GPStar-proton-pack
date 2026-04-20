#!/bin/bash
#
# Start Gzip-Aware Web Server
# ===========================
#
# Starts a local web server that serves this assets directory.
# Automatically serves .gz compressed files when available for better performance.
# Includes URL-to-file mapping to match the device's web routing behavior.
#
# USAGE:
#   From this directory:
#       ./startserver.sh
#
#   Or from anywhere:
#       bash /path/to/startserver.sh
#
# The server will start at http://localhost:8080/
# Press Ctrl+C to stop the server.
#
# URL MAPPINGS:
#   The server replicates the device's URL routing:
#   - http://localhost:8080/              → index.html
#   - http://localhost:8080/network       → network.html
#   - http://localhost:8080/settings/wand → wand.html
#   - http://localhost:8080/wand          → wand.html.gz
#   - etc.
## ADVANCED USAGE:
#   To use a custom port (e.g., 9000):
#       python3 ../../scripts/gzserver.py --dir "$SCRIPT_DIR" --port 9000
## NOTE: This script uses the common gzserver.py from the scripts/ folder.
#

# Get the directory where this script lives (the assets folder)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Path to the common gzserver.py (relative to project root)
GZSERVER_PATH="$SCRIPT_DIR/../../../scripts/gzserver.py"

# Check if gzserver.py exists
if [ ! -f "$GZSERVER_PATH" ]; then
    echo "Error: gzserver.py not found at: $GZSERVER_PATH"
    echo "Make sure the scripts folder contains gzserver.py"
    exit 1
fi

# Start the server, passing the current assets directory
echo "Starting web server for: $SCRIPT_DIR"
python3 "$GZSERVER_PATH" --dir "$SCRIPT_DIR"
