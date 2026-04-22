// This file forces the linker to include the class implementation
#include "../src/MagCalibration.cpp"

// Include the Google Test framework
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
