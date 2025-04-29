# Copilot Instructions

This document provides guidelines and instructions for using GitHub Copilot effectively in this project.

## Project Overview

This project is structured for development with PlatformIO, using the Arduino framework, and ATMega2560 and ESP32-based boards. The key components of the project include:

- **Devices**: Device-specific directories exist in the `sources` directory and represent a distinct sub-project.
- **Source Code**: The main application logic resides in `src/main.cpp` of each sub-project.
- **Headers**: Shared declarations and utilities are located in each sub-project's `include/` directory.
- **Libraries**: External libraries are managed in the sub-project's `libdeps/` directory.
- **Board Configurations**: Board-specific configurations are stored in the sub-project's `boards/` directory.
- **Partition Tables**: Custom partition tables are defined in the sub-project's `partitions/` directory.
- **PlatformIO Configuration**: The `platformio.ini` file within each sub-project defines the build environment and dependencies.

## Coding Guidelines

1. **File Organization**:
   - Place all implementation code in `src/`.
   - Use the `include/` directory for header files that define shared constants, classes, and functions.
   - Each sub-project should be self-contained with any libraries, includes, and supporting files.

2. **Naming Conventions**:
   - Use `PascalCase` for class names (e.g., `Device`).
   - Use `camelCase` for variables and functions (e.g., `initializeWiFi`).
   - Use `UPPER_SNAKE_CASE` for constants (e.g., `MAX_CONNECTIONS`).

3. **PlatformIO-Specific Practices**:
   - Use the `platformio.ini` file to manage dependencies and build configurations.
   - Keep board-specific settings in the `boards/` directory.

4. **Code Style**:
   - Follow consistent indentation (e.g., 2 spaces per level, 2 spaces per tab).
   - Use comments to explain complex logic or hardware-specific code.
   - Avoid hardcoding values; use constants or configuration files instead.

## Development Patterns

1. **WiFi Configuration**:
   - Use the `ExtWiFi.h` header for WiFi-related functionality.
   - Centralize WiFi credentials in `Configuration.h`.

2. **Web Server**:
   - Use `Webhandler.h` to define routes and handle HTTP requests.
   - Serve static files (e.g., `Index.h`, `Style.h`) for the web interface.

3. **System Utilities**:
   - Use `System.h` for system-level utilities and initialization.

4. **Time Management**:
   - Any action which requires a delay should use the millisDelay() object and avoid the blocking delay() statement whenever possible.
   - For hardware such as ESP32 where RTOS is built-in and available attempt to follow patterns using a task scheduling approach.

5. **Testing**:
   - Add unit tests in the `test/` directory.
   - Use PlatformIO's built-in testing framework to run tests.

## Device Constraints

1. **ESP32**:
   - Devices should include a partition definition file in CSV format and must include the standard nvs and coredump partitions.
   - Partitions should include space for Over The Air (OTA) updates, providing the otadata, app0, and app1, partitions.
   - Assume a 4MB flash partition if not specified and maximize the app0 and app1 partitions whenever possible.
   - Any remaining space can be used for a secondary nvs partition. Example:

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

3. **All Devices**:
   - Be mindful of memory space for every device.
   - Attempt to keep all variable types as small as necessary, for instance avoid `int` if a simple `uint8_t` would effectively hold the expected value.
   - For integer types use a specific size instead of a common name, for instance use `uint8_t` instead of `byte`.
   - Try to account for situations where an integer value may become negative and choose an appropriate type, for instance change to a signed integer.

## Commenting and Editing Styles

1. **Thought Process and Plan of Action**:
   - When making recommendations or edits, clearly lay out the thought process and the plan of action.
   - Provide a "before" and "after" comparison for edits, allowing for review and approval before changes are finalized.

2. **Commenting Code**:
   - Functions and methods should include comments explaining their purpose, inputs, outputs, and any side effects.
   - Objects and classes should be documented to describe their role in the system and their key attributes or methods.
   - For code involving complex math or algorithms, provide detailed comments explaining the logic and purpose of the calculations. Example:

   ````cpp
   // Function: calculateTrajectory
   // Purpose: Computes the trajectory of a projectile based on initial velocity and angle.
   // Inputs:
   //   - float velocity: The initial velocity of the projectile (m/s).
   //   - float angle: The launch angle of the projectile (degrees).
   // Outputs:
   //   - float: The computed trajectory distance (meters).
   float calculateTrajectory(float velocity, float angle) {
       // Convert angle to radians for trigonometric calculations
       float angleRadians = angle * (PI / 180.0);

       // Use the physics formula: distance = (velocity^2 * sin(2 * angle)) / gravity
       float distance = (pow(velocity, 2) * sin(2 * angleRadians)) / GRAVITY;

       return distance;
   }
