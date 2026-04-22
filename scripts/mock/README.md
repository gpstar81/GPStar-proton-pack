# gzserver.py Mock Data & API Reference

This directory contains centralized JSON responses for API endpoints used by the gzserver.py mock server for offline testing of ESP32 device web interfaces.

**Mock data lookup hierarchy:**
1. **Local override**: `./mock/` in each device's assets folder (checked first)
2. **Centralized default**: `scripts/mock/` directory (this location, used as fallback)

This allows device-specific customizations while maintaining shared defaults.

## Quick Start

```bash
# Start the server from any device's assets folder
cd source/Attenuator/assets
./startserver.sh

# Test in another terminal
curl http://localhost:8080/config/device
curl -X PUT http://localhost:8080/pack/on
curl -X POST http://localhost:8080/config/device/save -H "Content-Type: application/json" -d '{"buzzer":true}'
```

## How Mock Data Works

The server uses a hierarchical lookup for mock JSON files:

1. **Naming Convention**: Replace slashes in endpoint paths with underscores, add `.json`
2. **Lookup Order**:
   - First checks: `./mock/` (local to the device's assets folder)
   - Then checks: `scripts/mock/` (this centralized directory)
   - If neither exists: Returns empty object `{}`
3. **Live Reload**: Changes to JSON files are immediately visible (no server restart needed)

### Use Cases:
- **Shared defaults**: Place common mock data in `scripts/mock/` 
- **Device-specific overrides**: Create `mock/` folder in a device's assets directory with custom responses
- **Example**: All devices use `scripts/mock/status.json`, but Pack has custom `source/ProtonPack/assets/mock/status.json`

### Examples:

| Endpoint | Mock File |
|----------|-----------|
| `/config/device` | `config_device.json` |
| `/config/pack` | `config_pack.json` |
| `/status` | `status.json` |
| `/wifi/settings` | `wifi_settings.json` |
| `/wifi/networks` | `wifi_networks.json` |

---

## API Endpoint Reference

### Data Endpoints (GET - Returns JSON)

These endpoints return JSON data from mock files:

| Endpoint | Description | Mock File |
|----------|-------------|-----------|
| `GET /config/device` | Device configuration | `config_device.json` |
| `GET /config/pack` | Pack configuration | `config_pack.json` |
| `GET /config/smoke` | Smoke settings | `config_smoke.json` |
| `GET /config/wand` | Wand configuration | `config_wand.json` |
| `GET /config/blaster` | Blaster configuration | `config_blaster.json` |
| `GET /status` | System status | `status.json` |
| `GET /openapi.json` | OpenAPI spec | `openapi.json` |
| `GET /wifi/settings` | WiFi configuration | `wifi_settings.json` |
| `GET /wifi/networks` | Available networks | `wifi_networks.json` |
| `GET /wifi/restart` | Restart WiFi (returns settings) | `wifi_restart.json` |

### Action Endpoints (PUT/DELETE)

These endpoints return `{"success": true}` to simulate successful actions:

#### System Control
- `DELETE /restart` - Restart device

#### EEPROM Operations
- `PUT /eeprom/all` - Save all settings
- `PUT /eeprom/pack` - Save pack settings
- `DELETE /eeprom/pack` - Reset pack to defaults
- `PUT /eeprom/wand` - Save wand settings
- `DELETE /eeprom/wand` - Reset wand to defaults
- `PUT /eeprom/blaster` - Save blaster settings
- `PUT /eeprom/blaster/reset` - Reset blaster to defaults

#### Pack Control
- `PUT /pack/on` - Turn pack on
- `PUT /pack/off` - Turn pack off
- `PUT /pack/attenuate` - Cancel overheat warning
- `PUT /pack/vent` - Trigger manual vent
- `PUT /pack/cable/on` - Cancel ribbon cable alarm
- `PUT /pack/cable/off` - Activate ribbon cable alarm
- `PUT /pack/lockout` - Trigger button mash lockout
- `PUT /pack/restart` - Cancel lockout
- `PUT /pack/cyclotron/clockwise` - Rotate clockwise
- `PUT /pack/cyclotron/counterclockwise` - Rotate counterclockwise
- `PUT /pack/smoke/on` - Enable smoke
- `PUT /pack/smoke/off` - Disable smoke
- `PUT /pack/vibration/on` - Enable vibration
- `PUT /pack/vibration/off` - Disable vibration

#### Stream Modes
- `PUT /pack/stream/proton` - Proton stream
- `PUT /pack/stream/stasis` - Stasis stream
- `PUT /pack/stream/slime` - Slime stream
- `PUT /pack/stream/meson` - Meson stream
- `PUT /pack/stream/spectral` - Spectral stream
- `PUT /pack/stream/holiday/halloween` - Halloween stream
- `PUT /pack/stream/holiday/christmas` - Christmas stream
- `PUT /pack/stream/spectral/custom` - Custom spectral

#### Themes
- `PUT /pack/theme/1984` - Ghostbusters (1984)
- `PUT /pack/theme/1989` - Ghostbusters II (1989)
- `PUT /pack/theme/2021` - Afterlife (2021)
- `PUT /pack/theme/2024` - Frozen Empire (2024)

#### Device Control
- `PUT /selftest/enable` - Enable self-test
- `PUT /selftest/disable` - Disable self-test
- `PUT /smoke/enable` - Enable smoke
- `PUT /smoke/disable` - Disable smoke
- `PUT /light/on` - Lights on
- `PUT /light/off` - Lights off
- `PUT /pstt/drop` - Target down
- `PUT /pstt/reset` - Target up
- `PUT /infrared/signal` - Send IR signal

#### Sensors
- `PUT /sensors/recenter` - Recenter orientation
- `PUT /sensors/calibrate/gyro` - Calibrate gyroscope
- `PUT /sensors/calibrate/enable` - Enable magnetometer calibration
- `PUT /sensors/calibrate/disable` - Disable magnetometer calibration

#### Volume Control
- `PUT /volume/mute` - Mute audio
- `PUT /volume/unmute` - Unmute audio
- `PUT /volume/master/up` - Master volume up
- `PUT /volume/master/down` - Master volume down
- `PUT /volume/master/set/{0-100}` - Set master volume (e.g., `/volume/master/set/75`)
- `PUT /volume/effects/up` - Effects volume up
- `PUT /volume/effects/down` - Effects volume down
- `PUT /volume/music/up` - Music volume up
- `PUT /volume/music/down` - Music volume down

#### Music Control
- `PUT /music/startstop` - Toggle playback
- `PUT /music/pauseresume` - Toggle pause
- `PUT /music/next` - Next track
- `PUT /music/prev` - Previous track
- `PUT /music/select?track={n}` - Select track (e.g., `/music/select?track=3`)
- `PUT /music/loop/all` - Loop all
- `PUT /music/loop/one` - Loop one
- `PUT /music/shuffle/on` - Shuffle on
- `PUT /music/shuffle/off` - Shuffle off

#### WiFi Management
- `DELETE /wifi/network/{index}` - Delete saved network (e.g., `/wifi/network/0`)

### Body Handler Endpoints (POST)

These endpoints accept JSON body data and return `{"success": true, "received": <field_count>}`:

| Endpoint | Description | Example Body |
|----------|-------------|--------------|
| `POST /config/device/save` | Save device config | `{"buzzer": true, "vibration": false}` |
| `POST /config/pack/save` | Save pack config | `{"ledCount": 40, "brightness": 255}` |
| `POST /config/wand/save` | Save wand config | `{"barrelLEDs": 5, "effectsEnabled": true}` |
| `POST /config/smoke/save` | Save smoke config | `{"overheatDuration": 30, "continuous": false}` |
| `POST /config/blaster/save` | Save blaster config | `{"blasterMode": 1, "fireRate": 2}` |
| `POST /password/update` | Update WiFi password | `{"password": "newpassword123"}` |
| `POST /wifi/update` | Update WiFi settings | `{"active": {"ssid": "MyWiFi", "password": "pass"}}` |

### Static Files (Automatic .gz Support)

All HTML, CSS, JS, and asset files are served automatically with gzip compression when `.gz` versions exist:

| URL | File Served |
|-----|-------------|
| `/` | `index.html.gz` or `index.html` |
| `/network` | `network.html.gz` or `network.html` |
| `/settings/device` | `device.html.gz` or `device.html` |
| `/settings/pack` | `pack.html.gz` or `pack.html` |
| `/common.js` | `common.js.gz` or `common.js` |
| `/style.css` | `style.css.gz` or `style.css` |

---

## Example Mock Files

### config_device.json
```json
{
  "invertRotation": false,
  "invertLeftToggle": false,
  "invertRightToggle": false,
  "invertLEDs": false,
  "buzzer": true,
  "vibration": true,
  "bargraph": true,
  "deviceLEDs": true,
  "wifiName": "GPStar Attenuator",
  "buildDate": "2026-04-02",
  "audioVersion": 3
}
```

### status.json
```json
{
  "mode": "Upgraded",
  "modeID": 1,
  "theme": "1984",
  "themeID": 2,
  "vgMode": false,
  "smoke": true,
  "vibration": true,
  "direction": true,
  "switch": "Ready",
  "pack": "Idle",
  "ramping": false,
  "power": "1",
  "safety": "Safety On",
  "wand": "Connected",
  "wandPower": "Idle",
  "wandMode": "Proton Stream",
  "firing": "Idle",
  "crossedStreams": false,
  "cable": "Connected",
  "cyclotron": "Normal",
  "cyclotronLid": false,
  "temperature": "Normal",
  "musicPlaying": false,
  "musicPaused": false,
  "musicLooping": false,
  "musicShuffled": false,
  "musicCurrent": 501,
  "musicStart": 500,
  "musicEnd": 520,
  "volMuted": false,
  "volMaster": 75,
  "volEffects": 80,
  "volMusic": 70,
  "battVoltage": 4.85,
  "packTempC": 22.5,
  "packTempF": 72.5,
  "wandAmps": 0.0,
  "apClients": 1,
  "wsClients": 0,
  "canChangeStream": true
}
```

### wifi_networks.json
```json
{
  "networks": [
    {"ssid": "HomeNetwork", "rssi": -45, "encryption": 3},
    {"ssid": "OfficeWiFi", "rssi": -67, "encryption": 3},
    {"ssid": "PublicHotspot", "rssi": -80, "encryption": 0}
  ]
}
```

---

## Creating Mock Data

### From a Live Device

Capture real responses from your device to create accurate mock data:

```bash
# Save to centralized defaults (shared by all devices)
curl http://[device-ip]/config/device > scripts/mock/config_device.json
curl http://[device-ip]/status > scripts/mock/status.json
curl http://[device-ip]/wifi/settings > scripts/mock/wifi_settings.json
curl http://[device-ip]/wifi/networks > scripts/mock/wifi_networks.json

# Or save to device-specific overrides
mkdir -p source/Attenuator/assets/mock
curl http://[device-ip]/status > source/Attenuator/assets/mock/status.json
```

### Manual Creation

**Option 1: Centralized (shared by all devices)**
```bash
# Add shared mock responses
echo '{"mode": "Upgraded", "theme": "1984", "pack": "Idle"}' > scripts/mock/status.json

# Or edit existing files
nano scripts/mock/config_device.json
```

**Option 2: Device-Specific (overrides centralized)**
```bash
# Create device-specific mock directory
mkdir -p source/ProtonPack/assets/mock

# Add custom responses for this device only
echo '{"packSpecific": "data"}' > source/ProtonPack/assets/mock/config_pack.json
```

---

## Testing Examples

### Test GET endpoint with mock data
```bash
curl http://localhost:8080/config/device
# Returns mock data from config_device.json
```

### Test GET endpoint without mock file
```bash
curl http://localhost:8080/config/smoke
# Returns: {} (if no config_smoke.json exists)
```

### Test PUT action endpoint
```bash
curl -X PUT http://localhost:8080/pack/on
# Returns: {"success": true, "endpoint": "/pack/on"}
```

### Test POST endpoint with JSON body
```bash
curl -X POST http://localhost:8080/config/device/save \
  -H "Content-Type: application/json" \
  -d '{"buzzer": true, "vibration": false, "wifiName": "My Device"}'
# Returns: {"success": true, "received": 3}
```

### Test DELETE endpoint
```bash
curl -X DELETE http://localhost:8080/wifi/network/0
# Returns: {"success": true, "endpoint": "/wifi/network/0"}
```

### Test wildcard path parameters
```bash
curl -X PUT http://localhost:8080/volume/master/set/85
# Returns: {"success": true, "endpoint": "/volume/master/set/85"}
```

### Test query parameters
```bash
curl -X PUT http://localhost:8080/music/select?track=5
# Returns: {"success": true, "endpoint": "/music/select?track=5"}
```

---

## Advanced Usage

### Custom Port

```bash
# Start server on port 9000
python3 ../../scripts/gzserver.py --dir . --port 9000
```

### Multiple Instances

Run multiple device servers simultaneously on different ports:

```bash
# Terminal 1: Attenuator on 8080
cd source/Attenuator/assets && ./startserver.sh

# Terminal 2: Pack on 8081
cd source/ProtonPack/assets && python3 ../../../scripts/gzserver.py --dir . --port 8081

# Terminal 3: Wand on 8082
cd source/NeutronaWand/assets && python3 ../../../scripts/gzserver.py --dir . --port 8082
```

### Stopping the Server

Press `Ctrl+C` to stop, or use the helper script:

```bash
# Kill process on port 8080
../../scripts/killserver.sh

# Kill process on custom port
../../scripts/killserver.sh 9000
```

## Notes

- Mock data uses hierarchical lookup: local `./mock/` first, then centralized `scripts/mock/`
- GET endpoints (data) return your mock JSON from the first location found
- PUT/POST/DELETE endpoints (actions) always return `{"success": true}`
- The server logs which endpoints are hit and whether mock data was loaded
- JSON files must be valid JSON (use a validator if needed)
- Changes to mock JSON files are immediately visible (no server restart needed)
- Local overrides allow device-specific testing without affecting shared defaults
