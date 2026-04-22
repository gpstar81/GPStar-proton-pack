---
description: 'Project structure, device constraints, processor-specific considerations, and memory management for ATMega2560 and ESP32-based systems'
applyTo: '**/*.{cpp,h,hpp,ino,ini,json}'
---

# Architecture

This project is structured for development with VScode with the PlatformIO extension, using the Arduino framework, and multiple environments using ATMega2560 and ESP32-based microcontrollers (aka. boards). The key components of the available projects include:

- **Devices**: Device-specific directories exist in the `sources` directory and represent a distinct sub-project.
- **Shared Libraries**: A `SharedLib` directory contains shared resources (headers and/or classes) for multiple devices.
- **Source Code**: The main application logic should reside in `src/main.cpp` of each sub-project.
- **Headers**: Shared declarations and utilities are located in each sub-project's `include/` directory.
- **Libraries**: External libraries are managed in the sub-project's `libdeps/` directory.
- **Board Configurations**: Board-specific configurations are stored in the sub-project's `boards/` directory.
- **Partition Tables**: Custom partition tables are defined in the sub-project's `partitions/` directory.
- **PlatformIO Configuration**: The `platformio.ini` file within each sub-project defines the build environment and dependencies.

## Device Constraints

1. **ESP32**:
   - Devices should include a partition definition file in CSV format and must include the standard nvs and coredump partitions.
   - Partitions should include space for Over The Air (OTA) updates, providing the otadata, app0, and app1 partitions necessary.
   - Assume a 4MB flash partition if not specified and maximize the app0 and app1 as equal-sized partitions whenever possible.
   - Any remaining free space can be used for a secondary nvs partition. Example:

   ````cpp
   # Name,Type,SubType,Offset,Size,Flags
   nvs,data,nvs,0x9000,0x5000,
   otadata,data,ota,0xE000,0x2000,
   app0,app,ota_0,0x10000,0x1E0000,
   app1,app,ota_1,0x1F0000,0x1E0000,
   nvs2,data,nvs,0x3D0000,0x20000,
   coredump,data,coredump,0x3F0000,0x10000,

2. **ATMega2560**:
   - These devices will not have a partitions directory as they cannot be partitioned.

3. **Mixed Processors**:
   - Some devices will need support multiple board environments and allowing compilation against the ATMega2560 or ESP32 type hardware.
   - Be mindful to make suggestions based on both processor types when necessary in the event a feature exists exclusive to one architecture or the other.
   - Use preprocessor conditions where necessary to provide hardware-specific options, suggesting or highlighting the more maintainable option when possible.

4. **All Devices**:
   - Be mindful of memory space for every device, utilizing PROGMEM if necessary and possible.
   - Attempt to keep all variable types as small as necessary, for instance avoid `int` if a simple `uint8_t` would effectively hold the expected value.
   - For integer types use an explicit size instead of a common name, for instance use `uint8_t` instead of `byte` to maintain compatibility.
   - Try to account for situations where an integer value may become negative and choose an appropriate type, for instance change to a signed integer.
