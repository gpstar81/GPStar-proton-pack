---
description: 'File organization, naming conventions, development patterns, and commenting styles for C++ (Arduino) embedded code'
applyTo: '**/*.{cpp,c,h,hpp,ino}'
---

# Coding Standards

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

   - Use a `webhandler.h` to define handlers for HTTP requests.
   - Use a `Webrouting.h` to define routes for URIs and their handlers.
   - Serve static files (e.g., `index.html`, `style.css`) using embedded assets.
   - Provide a WebSocket for responsive communication between connected devices.

3. **System Utilities**:

   - Use a `System.h` for system-level utilities and initialization.

4. **Time Management**:

   - Any action which requires a delay should use the `millisDelay` object (included in all projects) and avoid the blocking `delay()` statement whenever possible.
   - For hardware such as ESP32 where RTOS is built-in and available attempt to follow patterns using a task scheduling approach.

## Commenting and Editing Styles

1. **Thought Process and Plan of Action**:

   - Before making recommendations or suggesting edits, be sure to ask for any clarifications to avoid making assumptions; Align on the intended outcome first.
   - If given a question for clarifications, try to state up front whether an approach may be on the right track or not, then offer clarification as necessary.
   - Keep suggestions concise and to the point, focusing on steps one at a time if necessary to avoid overwhelming the user and allow testing in small increments.
   - Allow the user to review any recommendations and offer a clear approval of understanding before proceeding with any proposed file changes.
   - Prioritize teaching over completing. Explain concepts, reasoning, and trade-offs so the user learns and can apply the knowledge independently.
   - If the requested solution doesn't align with the stated goal, challenge the approach and propose alternatives that better address the actual need.

2. **Presentation Requirement for Proposed Code Edits**:

   - Whenever possible, provide two separate code blocks labeled exactly "Before" and "After" to demonstrate the code to be changed.
   - Each block must be a fenced code block using four backticks (use markdown for mixed text/code).
   - The BEFORE block must show the exact existing code context (or the single marker "// ...existing code..." where appropriate) inside the fenced block.
   - The AFTER block must show the proposed replacement or new code inside its own fenced block.
   - Do not combine the two blocks; do not use unified diff format.

3. **Commenting Code**:

   - Functions and methods should include fenced multi-line comments explaining their purpose, inputs, and outputs.
   - Objects and classes should be documented to describe their role in the system and their key attributes or methods.
   - When refactoring code, keep any existing comments to help identify the original intent and help with file compares.
   - Provide new comments on a new line below any existing comments, providing any context or reasoning for the changes.
   - For code involving complex math or algorithms, provide detailed comments explaining the logic and purpose of the calculations. For Example:

   ````cpp
   /**
    * Function: calculateTrajectory
    * Purpose: Computes the trajectory of a projectile based on initial velocity and angle.
    * Inputs:
    *   - float velocity: The initial velocity of the projectile (m/s).
    *   - float angle: The launch angle of the projectile (degrees).
    * Outputs:
    *   - float: The computed trajectory distance (meters).
    */
   float calculateTrajectory(float velocity, float angle) {
       // Convert angle to radians for trigonometric calculations
       float angleRadians = angle * (PI / 180.0);

       // Use the physics formula: distance = (velocity^2 * sin(2 * angle)) / gravity
       float distance = (pow(velocity, 2) * sin(2 * angleRadians)) / GRAVITY;

       return distance;
   }
`````
