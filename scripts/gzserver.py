#!/usr/bin/env python3
"""
Gzip-Aware HTTP Server with Web Routing
========================================

A simple HTTP server that automatically serves .gz compressed versions of files
when available, with proper Content-Encoding headers for transparent decompression.
Includes URL-to-file mapping and ~130+ mock API endpoints to match the web routing
used in ESP32 device firmware for offline testing.

USAGE:
    Specify directory to serve:
        python3 gzserver.py --dir /path/to/directory

    Specify directory and port:
        python3 gzserver.py --dir /path/to/directory --port 8080

    Using shorthand flags:
        python3 gzserver.py -d /path/to/assets -p 9000

DESCRIPTION:
    This server replicates the complete web routing behavior of ESP32 firmware:
    
    1. Static Files (HTML/CSS/JS/SVG):
       - Maps URLs to files: /settings/pack → pack.html.gz
       - Serves .gz files with Content-Encoding headers
       - Falls back to uncompressed files if .gz doesn't exist
    
    2. Data Endpoints (GET - Returns JSON):
       - /config/device, /status, /wifi/networks, etc.
       - Returns mock JSON from mock/ directory if available
       - Returns empty {} if no mock file exists
    
    3. Action Endpoints (PUT/DELETE):
       - /pack/on, /music/next, /volume/master/set/*, etc.
       - Returns {"success": true} response
    
    4. Body Handlers (POST - Accepts JSON):
       - /config/device/save, /wifi/update, etc.
       - Accepts JSON body, returns success response

API REFERENCE:
    See mock/README.md for complete list of all ~130+ supported endpoints
    with examples, mock data instructions, and testing guide.

MOCK DATA:
    Mock data lookup order (first found wins):
    1. Local: ./mock/ in the assets directory (device-specific overrides)
    2. Centralized: scripts/mock/ directory (shared defaults)
    
    File naming: /endpoint/path → endpoint_path.json
    - /config/device → config_device.json
    - /status → status.json
    - /wifi/networks → wifi_networks.json
    
    See mock/README.md for details on creating mock responses.

EXAMPLES:
    # Serve a specific assets folder on default port 8080
    ./gzserver.py --dir ../source/Attenuator/assets

    # Serve on a custom port
    ./gzserver.py --dir ../source/Attenuator/assets --port 3000

    # Using shorthand flags
    ./gzserver.py -d ./assets -p 9000

    # Test API endpoints
    curl http://localhost:8080/config/device
    curl -X PUT http://localhost:8080/pack/on
    curl -X POST http://localhost:8080/config/device/save -d '{"buzzer":true}'

NOTES:
    - The --dir parameter is required to specify which directory to serve
    - Both compressed (.gz) and uncompressed files can coexist
    - Proper MIME types are automatically detected
    - URL routing matches ESP32 firmware behavior for local testing
    - Press Ctrl+C to stop the server
    - If the server doesn't stop, use: ./killserver.sh or lsof -ti:8080 | xargs kill -9

"""

import http.server
import socketserver
import os
import sys
import json
from pathlib import Path
from urllib.parse import urlparse, parse_qs

DEFAULT_PORT = 8080

# Get the directory where this script lives (for finding centralized mock data)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
MOCK_DIR = os.path.join(SCRIPT_DIR, 'mock')

# ==============================================================================
# ENDPOINT DEFINITIONS - Comprehensive route catalog from all devices
# ==============================================================================

# Data endpoints - GET requests that return JSON responses
DATA_ENDPOINTS = {
    '/config/device',
    '/config/pack',
    '/config/smoke',
    '/config/wand',
    '/config/blaster',
    '/status',
    '/openapi.json',
    '/wifi/settings',
    '/wifi/networks',
    '/wifi/restart',
}

# Action endpoints - PUT/DELETE requests that perform actions (return success JSON)
ACTION_ENDPOINTS = {
    # System control
    ('/restart', 'DELETE'),
    # EEPROM operations
    ('/eeprom/all', 'PUT'),
    ('/eeprom/pack', 'PUT'),
    ('/eeprom/pack', 'DELETE'),
    ('/eeprom/wand', 'PUT'),
    ('/eeprom/wand', 'DELETE'),
    ('/eeprom/blaster', 'PUT'),
    ('/eeprom/blaster/reset', 'PUT'),
    # Pack control
    ('/pack/on', 'PUT'),
    ('/pack/off', 'PUT'),
    ('/pack/attenuate', 'PUT'),
    ('/pack/vent', 'PUT'),
    ('/pack/cable/on', 'PUT'),
    ('/pack/cable/off', 'PUT'),
    ('/pack/lockout', 'PUT'),
    ('/pack/restart', 'PUT'),
    ('/pack/cyclotron/clockwise', 'PUT'),
    ('/pack/cyclotron/counterclockwise', 'PUT'),
    ('/pack/smoke/on', 'PUT'),
    ('/pack/smoke/off', 'PUT'),
    ('/pack/vibration/on', 'PUT'),
    ('/pack/vibration/off', 'PUT'),
    # Stream modes
    ('/pack/stream/proton', 'PUT'),
    ('/pack/stream/stasis', 'PUT'),
    ('/pack/stream/slime', 'PUT'),
    ('/pack/stream/meson', 'PUT'),
    ('/pack/stream/spectral', 'PUT'),
    ('/pack/stream/holiday/halloween', 'PUT'),
    ('/pack/stream/holiday/christmas', 'PUT'),
    ('/pack/stream/spectral/custom', 'PUT'),
    # Themes
    ('/pack/theme/1984', 'PUT'),
    ('/pack/theme/1989', 'PUT'),
    ('/pack/theme/2021', 'PUT'),
    ('/pack/theme/2024', 'PUT'),
    # Trap/Gizmo/Effects control
    ('/selftest/enable', 'PUT'),
    ('/selftest/disable', 'PUT'),
    ('/smoke/enable', 'PUT'),
    ('/smoke/disable', 'PUT'),
    ('/light/on', 'PUT'),
    ('/light/off', 'PUT'),
    # PSTT target control
    ('/pstt/drop', 'PUT'),
    ('/pstt/reset', 'PUT'),
    # Infrared control
    ('/infrared/signal', 'PUT'),
    # Sensor control
    ('/sensors/recenter', 'PUT'),
    ('/sensors/calibrate/gyro', 'PUT'),
    ('/sensors/calibrate/enable', 'PUT'),
    ('/sensors/calibrate/disable', 'PUT'),
    # Volume control
    ('/volume/mute', 'PUT'),
    ('/volume/unmute', 'PUT'),
    ('/volume/master/up', 'PUT'),
    ('/volume/master/down', 'PUT'),
    ('/volume/effects/up', 'PUT'),
    ('/volume/effects/down', 'PUT'),
    ('/volume/music/up', 'PUT'),
    ('/volume/music/down', 'PUT'),
    # Music control
    ('/music/startstop', 'PUT'),
    ('/music/pauseresume', 'PUT'),
    ('/music/next', 'PUT'),
    ('/music/select', 'PUT'),
    ('/music/prev', 'PUT'),
    ('/music/loop/all', 'PUT'),
    ('/music/loop/one', 'PUT'),
    ('/music/shuffle/on', 'PUT'),
    ('/music/shuffle/off', 'PUT'),
}

# Body handler endpoints - POST requests that accept JSON bodies
BODY_HANDLER_ENDPOINTS = {
    '/config/device/save',
    '/config/pack/save',
    '/config/wand/save',
    '/config/smoke/save',
    '/config/blaster/save',
    '/password/update',
    '/wifi/update',
}

# Static file mappings (everything else should try file serving first)
STATIC_EXTENSIONS = {'.html', '.css', '.js', '.svg', '.ico', '.stl', '.gz', '.json', '.png', '.jpg', '.woff', '.woff2', '.ttf'}


class GzipHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """HTTP handler that serves .gz files transparently with proper encoding headers"""

    def map_url_to_file(self, url_path):
        """
        Maps URL paths to actual file paths following the web routing conventions.
        
        Examples:
            /                   -> index.html
            /network            -> network.html
            /password           -> password.html
            /settings/device    -> device.html
            /settings/pack      -> pack.html
            /settings/wand      -> wand.html
            /settings/smoke     -> smoke.html
            /settings/blaster   -> blaster.html
            /swaggerui          -> swaggerui.html
            /pack               -> pack.html.gz
            /wand               -> wand.html.gz
            /device             -> device.html.gz
        """
        # Strip query parameters and leading/trailing slashes
        url_path = url_path.split('?')[0].strip('/')
        
        # Root path maps to index.html
        if url_path == '':
            return 'index.html'
        
        # Handle /settings/* prefix - strip it and map to the base name
        if url_path.startswith('settings/'):
            base_name = url_path.replace('settings/', '')
            return f'{base_name}.html'
        
        # Common page mappings (without settings prefix)
        page_map = {
            'network': 'network.html',
            'password': 'password.html',
            'device': 'device.html',
            'pack': 'pack.html',
            'wand': 'wand.html',
            'smoke': 'smoke.html',
            'blaster': 'blaster.html',
            'swaggerui': 'swaggerui.html',
        }
        
        if url_path in page_map:
            return page_map[url_path]
        
        # For paths that already have extensions, return as-is
        if '.' in url_path:
            return url_path
        
        # Default: assume it might be an HTML page
        return f'{url_path}.html'

    def send_json_response(self, data, status=200):
        """Send a JSON response with proper headers"""
        json_data = json.dumps(data).encode('utf-8')
        self.send_response(status)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(json_data)))
        self.send_header('Cache-Control', 'no-cache')
        self.end_headers()
        self.wfile.write(json_data)

    def load_mock(self, endpoint):
        """
        Load mock JSON data from a file if available.
        First checks for local mock/ directory (device-specific overrides).
        Falls back to centralized scripts/mock/ directory (shared defaults).
        Example: /config/device -> mock/config_device.json or scripts/mock/config_device.json
        """
        # Remove leading slash and replace remaining slashes with underscores
        filename = endpoint.strip('/').replace('/', '_') + '.json'
        
        # Try local mock directory first (device-specific)
        local_mock_file = os.path.join('mock', filename)
        if os.path.exists(local_mock_file):
            try:
                with open(local_mock_file, 'r') as f:
                    return json.load(f)
            except Exception as e:
                print(f"⚠️  Error loading mock data from {local_mock_file}: {e}")
        
        # Fall back to centralized mock directory
        central_mock_file = os.path.join(MOCK_DIR, filename)
        if os.path.exists(central_mock_file):
            try:
                with open(central_mock_file, 'r') as f:
                    return json.load(f)
            except Exception as e:
                print(f"⚠️  Error loading mock data from {central_mock_file}: {e}")
        
        return {}

    def handle_data_endpoint(self, endpoint):
        """Handle DATA_ENDPOINT requests - return JSON (mock or empty)"""
        mock = self.load_mock(endpoint)
        self.send_json_response(mock)
        print(f"✓ DATA {self.path} → {len(mock)} fields")

    def handle_action_endpoint(self, endpoint):
        """Handle ACTION_ENDPOINT requests - return success JSON"""
        response = {"success": True, "endpoint": endpoint}
        self.send_json_response(response)
        print(f"✓ ACTION {self.command} {self.path} → success")

    def handle_body_handler(self, endpoint):
        """Handle BODY_HANDLER POST requests - accept JSON body, return success"""
        content_length = int(self.headers.get('Content-Length', 0))
        body = {}
        
        if content_length > 0:
            try:
                body_data = self.rfile.read(content_length)
                body = json.loads(body_data.decode('utf-8'))
            except Exception as e:
                print(f"⚠️  Error parsing body: {e}")
        
        response = {"success": True, "received": len(body)}
        self.send_json_response(response)
        print(f"✓ POST {self.path} → {len(body)} fields received")

    def matches_wildcard_pattern(self, path, pattern):
        """Check if path matches a wildcard pattern like /volume/master/set/*"""
        if '*' not in pattern:
            return path == pattern
        
        # Convert pattern to regex-like matching
        parts = pattern.split('*')
        if len(parts) == 2:
            prefix, suffix = parts
            return path.startswith(prefix) and path.endswith(suffix)
        return False

    def do_GET(self):
        # Parse the URL path (without query params)
        url_path = self.path.split('?')[0]
        
        # Check if this is a data endpoint
        if url_path in DATA_ENDPOINTS:
            self.handle_data_endpoint(url_path)
            return
        
        # Otherwise, try to serve as a static file
        # Map the URL to the actual file path
        mapped_file = self.map_url_to_file(url_path)
        
        # Get the full file system path
        requested_path = self.translate_path('/' + mapped_file)
        
        # Check if a .gz version exists
        gz_path = requested_path + '.gz'

        if os.path.isfile(gz_path) and not self.path.endswith('.gz'):
            # Serve the .gz version with Content-Encoding header
            try:
                with open(gz_path, 'rb') as f:
                    content = f.read()

                self.send_response(200)

                # Set Content-Encoding to tell browser it's gzipped
                self.send_header('Content-Encoding', 'gzip')

                # Determine and send Content-Type based on original filename
                self.send_header('Content-Type', self.guess_type(requested_path))
                self.send_header('Content-Length', str(len(content)))

                # No caching for development/testing
                self.send_header('Cache-Control', 'no-cache')

                self.end_headers()
                self.wfile.write(content)

                print(f"✓ Served {os.path.basename(gz_path)} for {self.path}")

            except Exception as e:
                print(f"✗ Error serving {gz_path}: {e}")
                self.send_error(500, f"Error reading file: {e}")
        elif os.path.isfile(requested_path):
            # Serve uncompressed file if .gz doesn't exist but file does
            super().do_GET()
        else:
            # If mapped file doesn't exist, try the original path as-is
            original_requested = self.translate_path(url_path)
            original_gz = original_requested + '.gz'
            
            if os.path.isfile(original_gz) and not self.path.endswith('.gz'):
                try:
                    with open(original_gz, 'rb') as f:
                        content = f.read()
                    
                    self.send_response(200)
                    self.send_header('Content-Encoding', 'gzip')
                    self.send_header('Content-Type', self.guess_type(original_requested))
                    self.send_header('Content-Length', str(len(content)))
                    self.send_header('Cache-Control', 'no-cache')
                    self.end_headers()
                    self.wfile.write(content)
                    
                    print(f"✓ Served {os.path.basename(original_gz)} for {self.path}")
                except Exception as e:
                    print(f"✗ Error serving {original_gz}: {e}")
                    self.send_error(500, f"Error reading file: {e}")
            else:
                # Fall back to default behavior (will likely 404)
                super().do_GET()

    def do_PUT(self):
        """Handle PUT requests - action endpoints"""
        url_path = self.path.split('?')[0]
        
        # Check for exact path matches
        if (url_path, 'PUT') in ACTION_ENDPOINTS:
            self.handle_action_endpoint(url_path)
            return
        
        # Check for wildcard matches (e.g., /volume/master/set/*)
        for endpoint, method in ACTION_ENDPOINTS:
            if method == 'PUT' and self.matches_wildcard_pattern(url_path, endpoint):
                self.handle_action_endpoint(url_path)
                return
        
        # Not found
        self.send_error(404, f"PUT endpoint not found: {url_path}")

    def do_POST(self):
        """Handle POST requests - body handler endpoints"""
        url_path = self.path.split('?')[0]
        
        if url_path in BODY_HANDLER_ENDPOINTS:
            self.handle_body_handler(url_path)
            return
        
        # Not found
        self.send_error(404, f"POST endpoint not found: {url_path}")

    def do_DELETE(self):
        """Handle DELETE requests - action endpoints"""
        url_path = self.path.split('?')[0]
        
        # Check for exact path matches
        if (url_path, 'DELETE') in ACTION_ENDPOINTS:
            self.handle_action_endpoint(url_path)
            return
        
        # Check for wildcard matches (e.g., /wifi/network/*)
        for endpoint, method in ACTION_ENDPOINTS:
            if method == 'DELETE' and self.matches_wildcard_pattern(url_path, endpoint):
                self.handle_action_endpoint(url_path)
                return
        
        # Special case for /wifi/network/* deletion
        if url_path.startswith('/wifi/network/'):
            self.handle_action_endpoint(url_path)
            return
        
        # Not found
        self.send_error(404, f"DELETE endpoint not found: {url_path}")

    def log_message(self, format, *args):
        """Override to provide cleaner log messages"""
        # Uncomment the line below for verbose logging
        # print(f"[{self.address_string()}] {format % args}")
        pass


def print_usage():
    """Print usage information"""
    print(__doc__)


def parse_arguments():
    """Parse command line arguments"""
    directory = None
    port = DEFAULT_PORT

    args = sys.argv[1:]

    if '--help' in args or '-h' in args or len(args) == 0:
        print_usage()
        sys.exit(0)

    # Check for directory flag (required)
    if '--dir' in args or '-d' in args:
        try:
            flag = '--dir' if '--dir' in args else '-d'
            idx = args.index(flag)
            directory = args[idx + 1]
            # Remove directory flag and value from args
            args = args[:idx] + args[idx + 2:]
        except (IndexError, ValueError):
            print("Error: --dir/-d requires a directory path")
            print("")
            print_usage()
            sys.exit(1)
    else:
        print("Error: --dir parameter is required")
        print("")
        print_usage()
        sys.exit(1)

    # Check for port flag (optional)
    if '--port' in args or '-p' in args:
        try:
            flag = '--port' if '--port' in args else '-p'
            idx = args.index(flag)
            port = int(args[idx + 1])
        except (IndexError, ValueError):
            print("Error: --port/-p requires a valid port number")
            sys.exit(1)

    # Verify directory exists
    if not os.path.isdir(directory):
        print(f"Error: Directory '{directory}' does not exist")
        sys.exit(1)

    return directory, port


if __name__ == '__main__':
    directory, port = parse_arguments()

    # Change to the target directory
    os.chdir(directory)
    directory = os.getcwd()  # Get absolute path

    Handler = GzipHTTPRequestHandler

    try:
        with socketserver.TCPServer(("", port), Handler) as httpd:
            # Check for mock data sources
            local_mock = os.path.exists('mock')
            central_mock = os.path.exists(MOCK_DIR)
            mock_status = []
            if local_mock:
                mock_status.append("local ./mock/")
            if central_mock:
                mock_status.append(f"central {MOCK_DIR}")
            mock_info = ", ".join(mock_status) if mock_status else "none found (using empty responses)"
            
            print("=" * 70)
            print("Gzip-Aware HTTP Server with Web Routing")
            print("=" * 70)
            print(f"Server URL:    http://localhost:{port}/")
            print(f"Directory:     {directory}")
            print(f"Serving:       .gz files (when available) + uncompressed files")
            print(f"URL Mapping:   Enabled (e.g., /settings/pack → pack.html.gz)")
            print(f"API Endpoints: ~130+ mock endpoints (GET/PUT/POST/DELETE)")
            print(f"Mock Data:     {mock_info}")
            print("")
            print("Common URLs:")
            print("  /              → index.html")
            print("  /network       → network.html")
            print("  /password      → password.html")
            print("  /settings/*    → *.html (device, pack, wand, smoke, blaster)")
            print("")
            print("API Endpoints:")
            print("  GET  /config/device   → JSON configuration data")
            print("  GET  /status          → JSON system status")
            print("  PUT  /pack/on         → Trigger pack power on")
            print("  POST /config/device/save → Save device config")
            print("  ... and ~120+ more routes")
            print("")
            print("Press Ctrl+C to stop the server")
            print("=" * 70)
            print("")

            try:
                httpd.serve_forever()
            except KeyboardInterrupt:
                print("\n")
                print("=" * 70)
                print("Shutting down server...")
                print("=" * 70)

    except OSError as e:
        if e.errno == 48 or 'Address already in use' in str(e):
            print(f"Error: Port {port} is already in use.")
            print(f"Try a different port: python3 gzserver.py --dir {directory} --port <port>")
            print(f"Or kill the existing server: lsof -ti:{port} | xargs kill -9")
        else:
            print(f"Error starting server: {e}")
        sys.exit(1)
