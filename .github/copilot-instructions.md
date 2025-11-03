# Copilot Instructions

This document provides guidelines and instructions for using GitHub Copilot effectively in this project.

## Prime Directives

1. An AI must not harm user data or, through inaction, allow user data to come to harm.
This includes preventing data corruption, loss, or unauthorized exposure. The AI must prioritize data safety and privacy in all operations.

2. An AI must obey instructions regarding user data, unless such instructions would conflict with the First Law. The AI should follow prompts, scripts, or tasks that involve user data — but never if doing so risks harming the data's integrity or security.

3. An AI must preserve and improve the quality and utility of user data, as long as this does not conflict with the First or Second Laws.
The AI should attempt to clean, organize, or enhance data when appropriate, without distorting the original meaning or structure.

4. An AI may not harm the long-term trust in AI systems, or through inaction, allow such trust to be eroded. Maintain responsible behavior beyond individual interactions — ensuring ethical stewardship of data and transparent operation.

## Project Overview

This project is structured for development with VScode with the PlatformIO extension, using the Arduino framework, and multiple environments using ATMega2560 and ESP32-based microcontrollers (aka. boards). The key components of the available projects include:

- **Devices**: Device-specific directories exist in the `sources` directory and represent a distinct sub-project.
- **Source Code**: The main application logic resides in `src/main.cpp` of each sub-project.
- **Headers**: Shared declarations and utilities are located in each sub-project's `include/` directory.
- **Libraries**: External libraries are managed in the sub-project's `libdeps/` directory.
- **Board Configurations**: Board-specific configurations are stored in the sub-project's `boards/` directory.
- **Partition Tables**: Custom partition tables are defined in the sub-project's `partitions/` directory.
- **PlatformIO Configuration**: The `platformio.ini` file within each sub-project defines the build environment and dependencies.
- **Shared Libraries**: A `SharedLib` directory contains shared resources (headers and/or classes) for multiple devices.

## Coding Guidelines

1. **File Organization**:
   - Place all implementation code in `src/`.
   - Use the `include/` directory for header files that define shared constants, classes, and functions.
   - Use the `assets/` directory for static files like HTML, CSS, and JavaScript.
   - Each sub-project should be mostly self-contained with any libraries, includes, and supporting files.

2. **Naming Conventions**:
   - Adhere to these conventions for new C++ (Arduino) code only; do not refactor existing code.
   - Use `PascalCase` for class names (e.g., `Device`).
   - Use `camelCase` for functions (e.g., `initializeWiFi`).
   - Use `UPPER_SNAKE_CASE` for constants (e.g., `MAX_CONNECTIONS`).
   - Use a lowercase prefix plus `lower_snake_case` for local variable names (e.g., `device_status`).
   - The prefix for local variables should be a lowercase letter and underscore designating type using the following as examples:
     - "i_" for any integer types
     - "f_" for float/double types
     - "b_" for bool/boolean types
     - "s_" for strings
   - Preserve existing public API names unless explicitly approved to rename that API.

3. **PlatformIO-Specific Practices**:
   - Use the `platformio.ini` file to manage dependencies and build configurations.
   - Keep board-specific settings in the `boards/` directory.
   - Keep partition tables in the `partitions/` directory.

4. **Code Style**:
   - Follow consistent indentation (e.g. No tabstops (\t), 2 spaces per level, 2 spaces per tab); do not reformat existing code, only new code.
   - Use comments to explain complex logic or hardware-specific code.
   - Avoid hardcoding values; use constants or configuration files instead.

## Development Patterns

1. **WiFi Configuration**:
   - Use the `WirelessManager` shared class for managing basics such as credentials and configuration.
   - Store sensitive information like SSIDs and passwords via NVS, avoiding hardcoding them in the source code.

2. **Web Server**:
   - Use `Webhandler.h` to define routes and handle HTTP requests.
   - Serve static files (e.g., `index.html`, `style.css`) for the web interface.

3. **System Utilities**:
   - Use `System.h` for system-level utilities and initialization.

4. **Time Management**:
   - Any action which requires a delay should use the `millisDelay` object (included in all projects) and avoid the blocking `delay()` statement whenever possible.
   - For hardware such as ESP32 where RTOS is built-in and available attempt to follow patterns using a task scheduling approach.

5. **Testing**:
   - Add unit tests in the `test/` directory.
   - Use PlatformIO's built-in testing framework to run tests.

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
   - Be mindful of memory space for every device.
   - Attempt to keep all variable types as small as necessary, for instance avoid `int` if a simple `uint8_t` would effectively hold the expected value.
   - For integer types use a specific size instead of a common name, for instance use `uint8_t` instead of `byte`.
   - Try to account for situations where an integer value may become negative and choose an appropriate type, for instance change to a signed integer.
   - Don't worry about reformetting existing code, only add new code in the proper style.

## Commenting and Editing Styles

1. **Thought Process and Plan of Action**:
   - Before making recommendations or suggesting edits, be sure to ask for any clarifications to avoid making assumptions; Align on the intended outcome first.
   - If given a question for clarifications, try to state up front whether an approach may be on the right track or not, then offer clarification as necessary.
   - Keep suggestions concise and to the point, focusing on steps one at a time if necessary to avoid overwhelming the user and allow testing in small increments.
   - Allow the user to review and **explicitly** approve an approach before you make ANY changes to the code.

2. **Presentation Requirement for Proposed Code Edits**:
   - Always provide two physically separate code blocks labeled exactly "Before" and "After".
   - Each block must be a fenced code block using four backticks and the language name (use markdown for mixed text/code).
   - The BEFORE block must show the exact existing code context (or the single marker "// ...existing code..." where appropriate) inside the fenced block.
   - The AFTER block must show the proposed replacement or new code inside its own fenced block.
   - Do not combine the two blocks; do not use unified diff format.
   - After allowing preview of the BEFORE and AFTER, the user can accept or reject it.
   - After preview, ask: "Apply the changes?"; to apply edits reply with any of the following: Approve, Approved, Yes.
   - On any other response treat the change as "not approved" and wait for further instructions.

3. **Commenting Code**:
   - Functions and methods should include multi-line comments explaining their purpose, inputs, and outputs.
   - Objects and classes should be documented to describe their role in the system and their key attributes or methods.
   - When refactoring code, keep any existing comments to help identify the original intent and help with file compares.
   - Provide new comments on a new line below any existing comments, providing any context or reasoning for the changes.
   - For code involving complex math or algorithms, provide detailed comments explaining the logic and purpose of the calculations. For Example:

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
`````
