---
description: 'Code review focusing on embedded systems best practices and project standards'
model: 'Claude Sonnet 4'
tools: ['codebase']
---

# Code Review Guide

## Review Focus Areas

Please review the code for:

### **Project Standards**
- Naming conventions (PascalCase, camelCase, UPPER_SNAKE_CASE, prefixed variables)
- Memory efficiency (appropriate integer types, avoid waste)
- Non-blocking patterns (millisDelay over delay())
- Processor compatibility (ESP32 vs ATMega2560)
- Documentation completeness

### **Embedded Systems Issues**
- Buffer overflows and memory safety
- Stack vs heap usage patterns
- Hardware abstraction and platform-specific code
- Timing-critical operations and ISR safety
- Power consumption implications

### **Code Quality**
- Potential bugs or edge cases
- Error handling and recovery
- Testability and maintainability
- Performance bottlenecks

## Review Style

- **Challenge assumptions** and provide constructive criticism
- **Explain the "why"** behind suggestions, not just the "what"
- **Ask clarifying questions** about requirements and constraints
- **Point out trade-offs** between different approaches
- **Suggest improvements** with specific examples

## Quick Start

For a basic review, just paste your code and ask:
```
Review this code following the guide above
```

For focused review, specify the area:
```
Focus on memory safety and ESP32/ATMega compatibility:
[your code here]
```

For learning-oriented review:
```
Review this as a mentor - explain issues and help me understand:
[your code here]
```