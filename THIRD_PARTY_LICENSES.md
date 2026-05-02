# Third-Party Licenses and Attributions

This document lists all third-party libraries and dependencies used in the GPStar Proton Pack project, along with their respective licenses and copyright notices.

The GPStar Proton Pack software is licensed under the GNU General Public License v3.0 or later (GPLv3+). All third-party components listed below are compatible with this license.

---

## Table of Contents

- [Embedded Assets](#embedded-assets)
- [Arduino/PlatformIO Libraries](#arduinoplatformio-libraries)
- [LGPL Libraries - Special Notice](#lgpl-libraries---special-notice)
- [ShuffleMusic Library - Special Notice](#shufflemusic-library---special-notice)
- [Build Tools and Frameworks](#build-tools-and-frameworks)

---

## Embedded Assets

### Three.js

**Copyright:** Copyright © 2010-2023 Three.js Authors  
**License:** MIT License  
**Website:** https://threejs.org/  
**Usage:** 3D visualization library used in the web interface for magnetometer calibration visualization  
**Files:** `source/NeutronaWand/assets/three.min.js`, `source/SingleShot/assets/three.min.js`

```
The MIT License

Copyright © 2010-2023 three.js authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```

---

## Arduino/PlatformIO Libraries

The following libraries are automatically downloaded and managed by PlatformIO during the build process. These are specified in the various `platformio.ini` files throughout the project.

### FastLED

**Copyright:** Copyright (c) 2013 FastLED  
**License:** MIT License  
**Repository:** https://github.com/FastLED/FastLED  
**Version:** 3.10.3 or later  
**Usage:** LED control and effects across all devices

### ArduinoJson

**Copyright:** Copyright © 2014-2024, Benoit BLANCHON  
**License:** MIT License  
**Repository:** https://github.com/bblanchon/ArduinoJson  
**Version:** 7.4.3 or later  
**Usage:** JSON parsing and serialization for web APIs (ESP32 devices)

### SafeString

**Copyright:** Copyright (c) 2019, PowerBroker2  
**License:** MIT License  
**Repository:** https://github.com/PowerBroker2/SafeString  
**Version:** 4.1.42 or later  
**Usage:** Safe string manipulation across all devices

### ezButton

**Copyright:** Copyright (c) 2019, ArduinoGetStarted.com  
**License:** MIT License  
**Repository:** https://github.com/ArduinoGetStarted/button  
**Version:** 1.0.6 or later  
**Usage:** Button debouncing and state management

### SerialTransfer

**Copyright:** Copyright (c) 2019, PowerBroker2  
**License:** MIT License  
**Repository:** https://github.com/PowerBroker2/SerialTransfer  
**Version:** 3.1.5 or later  
**Usage:** Serial communication between devices

### GPStar Audio Serial Library

**Copyright:** Copyright (c) 2021-2024, GPStar Technologies  
**License:** MIT License  
**Repository:** https://github.com/gpstar81/GPStarAudio-Serial-Library  
**Version:** 1.3.5 or later  
**Usage:** Communication with GPStar Audio hardware

### CRC32

**Copyright:** Copyright (c) 2014, Christopher Baker  
**License:** MIT License  
**Repository:** https://github.com/bakercp/CRC32  
**Version:** 2.0.1 or later  
**Usage:** CRC32 checksum calculations

### ElegantOTA

**Copyright:** Copyright (c) 2020, Ayush Sharma  
**License:** MIT License  
**Repository:** https://github.com/ayushsharma82/ElegantOTA  
**Version:** 3.1.7 or later  
**Usage:** Over-the-air firmware updates for ESP32 devices

### Arduino-IRremote

**Copyright:** Copyright (c) 2020, Arduino-IRremote  
**License:** MIT License  
**Repository:** https://github.com/Arduino-IRremote/Arduino-IRremote  
**Version:** 4.7.1 or later  
**Usage:** Infrared remote control support (NeutronaWand)

### ArduinoINA219

**Copyright:** Copyright (c) 2019, Flavius Bindea  
**License:** MIT License  
**Repository:** https://github.com/flav1972/ArduinoINA219  
**Version:** 1.1.1 or later  
**Usage:** INA219 current/voltage sensor support (ProtonPack)

### Simple HT16K33 Library

**Copyright:** Copyright (c) 2019, lpaseen  
**License:** MIT License  
**Repository:** https://github.com/lpaseen/ht16k33  
**Version:** 1.0.3 or later  
**Usage:** HT16K33 LED driver support

### avdweb_Switch

**Copyright:** Copyright (c) 2016, Albert van Dalen  
**License:** MIT License  
**Repository:** https://github.com/avdwebLibraries/avdweb_Switch  
**Version:** 1.2.9 or later  
**Usage:** Advanced button/switch handling (NeutronaWand)

### Adafruit LIS3MDL

**Copyright:** Copyright (c) 2020, Adafruit Industries  
**License:** BSD License (GPL-compatible)  
**Repository:** https://github.com/adafruit/Adafruit_LIS3MDL  
**Version:** 1.2.5 or later  
**Usage:** LIS3MDL magnetometer support (NeutronaWand)

### Adafruit LSM6DS

**Copyright:** Copyright (c) 2020, Adafruit Industries  
**License:** BSD License (GPL-compatible)  
**Repository:** https://github.com/adafruit/Adafruit_LSM6DS  
**Version:** 4.7.4 or later  
**Usage:** LSM6DS accelerometer/gyroscope support (NeutronaWand)

### Adafruit AHRS

**Copyright:** Copyright (c) 2020, Adafruit Industries  
**License:** BSD License (GPL-compatible)  
**Repository:** https://github.com/adafruit/Adafruit_AHRS  
**Version:** 2.4.0 or later  
**Usage:** Attitude and heading reference system calculations (NeutronaWand)

### GuL TI Humidity HDC10XX

**Copyright:** Copyright (c) 2020, boeserfrosch  
**License:** MIT License  
**Repository:** https://github.com/boeserfrosch/GuL_TI_Humidity  
**Version:** 1.1.3 or later  
**Usage:** HDC1080 temperature/humidity sensor support (ProtonPack)

### arduinoWebSockets

**Copyright:** Copyright (c) 2016, Links2004  
**License:** GPLv2+ (GPL-compatible)  
**Repository:** https://github.com/Links2004/arduinoWebSockets  
**Version:** 2.7.3 or later  
**Usage:** WebSocket communication (GhostTrapBase)

---

## LGPL Libraries - Special Notice

The following libraries are licensed under the GNU Lesser General Public License (LGPL), which is compatible with GPLv3 but has special requirements for embedded systems.

### digitalWriteFast

**Copyright:** Copyright (c) 2015-2020, Watterott electronic & Armin Joachimsmeyer  
**License:** LGPL 2.1 or later  
**Repository:** https://github.com/ArminJo/digitalWriteFast  
**Version:** 1.3.1 or later  
**Usage:** Fast digital I/O operations across all Arduino-based devices

### Ramp

**Copyright:** Copyright (c) 2015, Sylvain Garnavault  
**License:** LGPL 3.0 or later  
**Repository:** https://github.com/siteswapjuggler/RAMP  
**Version:** 0.7.0 or later  
**Usage:** Ramping/interpolation functions for smooth animations

### ESPAsyncWebServer

**Copyright:** Copyright (c) 2016, ESP Async Web Server Contributors  
**License:** LGPL 3.0 or later  
**Repository:** https://github.com/ESP32Async/ESPAsyncWebServer  
**Version:** 3.10.3 or later  
**Usage:** Asynchronous web server for ESP32 devices

#### LGPL Compliance Notes

The LGPL libraries listed above are used in accordance with LGPL terms. When distributing firmware that includes these libraries:

1. **Source Availability:** All source code for LGPL components is available through their respective repositories listed above, and build instructions are provided in this project's documentation.

2. **Modification and Replacement:** Users can modify and rebuild the firmware with alternative or modified versions of these LGPL libraries by following the build instructions in [docs/VSCODE.md](docs/VSCODE.md) and using the provided PlatformIO configuration files.

3. **Combined Work:** Under LGPL terms, this GPLv3-licensed project may incorporate LGPL libraries. The entire combined work is distributed under GPLv3, which is compatible with LGPL requirements.

4. **Installation Information:** Complete build and installation instructions are provided in the [docs/](docs/) directory, satisfying LGPL's requirement to enable users to modify and reinstall the software.

---

## ShuffleMusic Library - Special Notice

### ShuffleMusic (EA SEED Shuffle)

**Copyright:** Copyright (c) 2023 Electronic Arts Inc.  
**License:** BSD 3-Clause License with Additional Restrictions  
**Location:** `source/SharedLib/ShuffleMusic/`  
**License File:** [source/SharedLib/ShuffleMusic/LICENSE.txt](source/SharedLib/ShuffleMusic/LICENSE.txt)  
**Usage:** Music shuffling algorithm for randomized playback

#### Important Notes:

This library includes a BSD 3-Clause license with an additional clause (Clause 4) regarding the use of EA's trademarks and logos. The full license text is available in the library's directory.

**Clause 4 Summary:** EA's marks or logos included with this software are for demonstration purposes only and may not be displayed or shared except as part of redistributing this software without modification.

**GPL Compatibility:** The core BSD 3-Clause license is fully GPL-compatible.

**Verification Note:** This project has been audited and confirmed to contain **no EA marks, logos, or SEED branding** in the source code, compiled firmware, web interface, or documentation. The trademark restriction clause (Clause 4) does not apply to this distribution as no EA branding materials are present. Only the algorithmic code adapted from EA's open-source repository is used.

---

## Build Tools and Frameworks

### Arduino Framework

**License:** LGPL 2.1 or later  
**Website:** https://www.arduino.cc/  
**Usage:** Core framework for all microcontroller firmware

### ESP-IDF (Espressif IoT Development Framework)

**License:** Apache License 2.0 (GPL-compatible)  
**Website:** https://github.com/espressif/esp-idf  
**Usage:** Core framework for ESP32 development

### PlatformIO

**License:** Apache License 2.0 (GPL-compatible)  
**Website:** https://platformio.org/  
**Usage:** Build system and development platform

---

## License Compatibility Summary

All libraries listed in this document are used in accordance with their respective licenses and are compatible with the GPStar Proton Pack project's GPLv3 license. Key compatibility points:

- **MIT/BSD/Apache Licenses:** These are permissive licenses explicitly compatible with GPLv3. Code under these licenses can be incorporated into GPLv3 projects.

- **LGPL (Lesser GPL):** Specifically designed to be compatible with GPL. The LGPL allows linking with proprietary or differently-licensed code, and when combined with GPLv3 code, the entire work can be distributed under GPLv3 terms while respecting LGPL's provisions for library modification.

- **GPL v2+:** Libraries licensed as "GPL v2 or later" are compatible with GPLv3 per FSF guidance.

---

## How to Rebuild with Modified Libraries

To rebuild the firmware with modified versions of any third-party library:

1. Install development tools following [docs/VSCODE.md](docs/VSCODE.md)
2. Modify the library version or source in the relevant `platformio.ini` file
3. For LGPL libraries specifically, you may:
   - Replace the library reference with a local path to your modified version
   - Modify the library's source in the PlatformIO library cache
   - Use PlatformIO's `lib_deps` with a git repository URL pointing to your fork
4. Run the build scripts in the `scripts/` directory or use PlatformIO directly
5. Flash the resulting firmware to your device

Detailed build instructions are available in the project documentation:
- [docs/VSCODE.md](docs/VSCODE.md) - VSCode + PlatformIO setup
- [docs/COMPILING_FLASHING.md](docs/COMPILING_FLASHING.md) - General compilation guide
- [docs/DIY_FLASHING.md](docs/DIY_FLASHING.md) - DIY build instructions

---

## Contact and Updates

For questions about third-party licenses or to report licensing issues:
- GitHub Issues: https://github.com/gpstar81/GPStar-proton-pack/issues
- Project Documentation: https://gpstar81.github.io/GPStar-proton-pack/

This document is maintained alongside the source code and updated when dependencies change. Last updated: April 2026.

---

## Acknowledgments

The GPStar Proton Pack project gratefully acknowledges all library authors and contributors for making their work available under open-source licenses. Their contributions make projects like this possible.

Special thanks to the Arduino, PlatformIO, and ESP32 communities for their ongoing development and support of embedded development tools and frameworks.
