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

echo "Stopping GPStar web server..."
echo ""

# Method 1: Kill by process name (most reliable)
echo "Method 1: Killing by process name..."
if pkill -f gzserver.py 2>/dev/null; then
    echo "✓ Killed gzserver.py processes"
else
    echo "○ No gzserver.py processes found by name"
fi

# Method 2: Kill by port using lsof
echo ""
echo "Method 2: Killing by port ($PORT)..."
if PID=$(lsof -ti:$PORT 2>/dev/null); then
    if [ -n "$PID" ]; then
        kill -9 $PID 2>/dev/null && echo "✓ Killed process on port $PORT (PID: $PID)" || echo "⚠️  Could not kill process $PID"
    fi
else
    echo "○ No process found on port $PORT (lsof method)"
fi

# Method 3: Kill by port using netstat + ps (fallback)
echo ""
echo "Method 3: Verifying with netstat..."
if netstat -an -p tcp 2>/dev/null | grep -q "\.${PORT} .*LISTEN"; then
    echo "⚠️  Warning: Port $PORT may still be in use"
    netstat -an -p tcp | grep "\.${PORT} .*LISTEN"
else
    echo "✓ Port $PORT is now available"
fi

echo ""
echo "Done."
