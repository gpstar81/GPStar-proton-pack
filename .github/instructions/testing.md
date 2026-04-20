---
description: 'Unit testing practices using googletest framework for pure C/C++ components in embedded systems'
applyTo: '**/test/**/*.{cpp,h,hpp}'
---

# Testing Guidelines

## Testing Infrastructure

- All unit tests should reside in the `test/` directory of each project
- Most projects cannot be tested as they require the Arduino framework or special hardware
- Only projects which are pure C/C++ code can, and ideally should, be tested
- We will use and follow the `googletest` framework to run tests when possible
- Create both a positive and negative test for scenarios when possible

## Test Structure

### Test File Organization
- Test files should mirror the source structure: `src/Component.cpp` → `test/ComponentTest.cpp`
- Use descriptive test names that explain what is being tested
- Group related tests using `TEST_F` fixtures for shared setup

### Test Patterns
```cpp
// Arrange - set up test data
// Act - execute the code under test
// Assert - verify the results

TEST(ComponentTest, ShouldReturnExpectedValue_WhenValidInput) {
    // Arrange
    Component component;
    int expectedValue = 42;
    
    // Act
    int result = component.calculate(10, 32);
    
    // Assert
    EXPECT_EQ(expectedValue, result);
}
```

## Hardware Constraints in Testing
- Mock hardware dependencies using dependency injection
- Test business logic separately from hardware interactions
- Use preprocessor definitions to enable test-only code paths
- Consider testing on host system for pure C++ components

## Memory Testing
- Test for memory leaks in dynamic allocation scenarios
- Verify stack usage doesn't exceed available memory
- Test boundary conditions for buffer operations