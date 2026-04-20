#!/bin/bash
#
# Kill Web Server on Port 8080
# =============================
#
# Kills any process running on port 8080 (the default gzserver.py port).
# Useful if Ctrl-C doesn't properly stop the server.
#
# USAGE:
#   ./killserver.sh [port]
#
# EXAMPLES:
#   ./killserver.sh           # Kill server on default port 8080
#   ./killserver.sh 9000      # Kill server on port 9000
#

PORT=${1:-8080}

echo "Looking for processes on port $PORT..."

# Check if anything is using the port
if lsof -ti:$PORT > /dev/null 2>&1; then
    echo "Killing process(es) on port $PORT..."
    lsof -ti:$PORT | xargs kill -9 2>/dev/null
    
    # Verify it was killed
    if lsof -ti:$PORT > /dev/null 2>&1; then
        echo "⚠️  Warning: Process may still be running on port $PORT"
        lsof -i:$PORT
    else
        echo "✓ Successfully killed server on port $PORT"
    fi
else
    echo "No process found on port $PORT"
fi
