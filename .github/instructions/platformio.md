---
description: 'PlatformIO-specific development patterns, build configuration, and library management for embedded systems'
applyTo: '**/platformio.ini, **/library.json, Python/Scripts/*.py'
---

# PlatformIO Development Guidelines

## Project Structure Standards

### Environment Configuration
- Use environment-specific configurations in `platformio.ini`
- Separate build flags for different target boards (ATMega2560 vs ESP32)
- Include board-specific upload protocols and speeds

```ini
[env:protonpack_esp32]
platform = espressif32
board = esp32dev
framework = arduino
build_flags = 
    -DESP32_VERSION
    -DWIFI_ENABLED
lib_deps = 
    WiFi
    WebServer

[env:neutronawand_atmega]
platform = atmelavr
board = megaatmega2560
framework = arduino
build_flags = 
    -DATMEGA_VERSION
    -DLIMITED_MEMORY
```

### Library Management
- Define library dependencies in `platformio.ini` rather than global installation
- Use semantic versioning for library dependencies
- Prefer header-only libraries for memory-constrained environments
- Document library choices and version constraints

```ini
lib_deps = 
    bblanchon/ArduinoJson@^6.21.3
    paulstoffregen/Time@^1.6.1
    = Library Name@^version  ; Exact version
```

### Build Flags Organization
```ini
build_flags = 
    ; Memory optimizations
    -Os
    -ffunction-sections
    -fdata-sections
    -Wl,--gc-sections
    ; Debug flags (development only)
    -DDEBUG_ENABLED
    -DCORE_DEBUG_LEVEL=4
    ; Project-specific defines
    -DPROJECT_VERSION="6.1.0"
```

## Multi-Environment Best Practices

### Conditional Compilation
- Use platform-specific preprocessor guards
- Minimize code duplication between platforms
- Abstract hardware differences into separate modules

```cpp
#ifdef ESP32_VERSION
    #include <WiFi.h>
    #define SERIAL_BAUD 115200
#elif defined(ATMEGA_VERSION)
    #include <SoftwareSerial.h>
    #define SERIAL_BAUD 9600
#endif
```

### Platform-Specific Features
- Wrap ESP32-only features (WiFi, OTA, larger memory) in conditional blocks
- Provide fallback implementations for ATMega2560 where possible
- Document platform limitations in code comments

## Custom Board Definitions

### Board JSON Structure
```json
{
  "build": {
    "core": "arduino",
    "extra_flags": "-DARDUINO_ARCH_AVR",
    "f_cpu": "16000000L",
    "mcu": "atmega2560",
    "variant": "mega"
  },
  "frameworks": ["arduino"],
  "name": "Custom ProtonPack Board",
  "upload": {
    "maximum_ram_size": 8192,
    "maximum_size": 253952,
    "protocol": "stk500v2",
    "speed": 115200
  },
  "vendor": "GPStar"
}
```

### Custom Scripts
- Use Python scripts in `scripts/` folder for pre/post-build tasks
- Automate version number updates
- Generate build information headers
- Validate memory usage and warn if approaching limits

## Memory Optimization Strategies

### Compile-Time Optimizations
```ini
build_flags = 
    -Os                     ; Optimize for size
    -ffunction-sections     ; Place functions in separate sections
    -fdata-sections        ; Place data in separate sections
    -Wl,--gc-sections      ; Remove unused sections
    -flto                  ; Link-time optimization
```

### Monitoring Memory Usage
- Include memory reporting in build process
- Set warning thresholds (80% of available memory)
- Track memory usage trends across builds

## Testing and Validation

### Build Verification
- Ensure all environments build successfully in CI/CD
- Test with different optimization levels
- Validate against memory constraints

### Platform Testing
```ini
[env:test-atmega]
platform = atmelavr
board = megaatmega2560
test_framework = googletest
build_flags = -DUNIT_TESTING

[env:test-esp32]
platform = espressif32
board = esp32dev  
test_framework = googletest
build_flags = -DUNIT_TESTING
```

## Common Issues and Solutions

### Memory Exhaustion
- Move strings to PROGMEM
- Reduce global variable usage
- Implement dynamic allocation carefully
- Use F() macro for string literals

### Build Failures
- Check library compatibility with selected platform
- Verify uploaded library versions
- Clean build cache when switching environments

### Upload Issues
- Verify correct upload_port specification
- Check baud rates and protocols
- Use appropriate upload_flags for custom bootloaders