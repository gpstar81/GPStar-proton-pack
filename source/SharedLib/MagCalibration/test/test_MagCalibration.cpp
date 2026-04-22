/**
 * Test suite for magnetometer calibration.
 */

#include <gtest/gtest.h>
#include "MagCalibration.h"
#include <set>
#include <cmath>  // For NAN, INFINITY, cosf, sinf

// Test fixture for MagCalibration tests
class MagCalibrationDefaultFixture : public ::testing::Test {
protected:
    // This member variable is available in every test using this fixture.
    // It is default-constructed when the fixture is created for each test.
    MagCalibration calibration;

    // SetUp() is called before each test.
    // Use this to set up any common initialization, if needed.
    void SetUp() override {
    }
};

TEST_F(MagCalibrationDefaultFixture, CanInstantiate) {
    SUCCEED(); // If we reach this point, instantiation succeeded.
}

// Confirms the expected defaults are set for this mode.
TEST_F(MagCalibrationDefaultFixture, ConstructorDefaults) {
    // Check that the default calibration is as expected
    CalibrationData defaults = calibration.getDefaultCalibration();
    EXPECT_FLOAT_EQ(defaults.mag_hardiron[0], 0.0f);
    EXPECT_FLOAT_EQ(defaults.mag_hardiron[1], 0.0f);
    EXPECT_FLOAT_EQ(defaults.mag_hardiron[2], 0.0f);
    EXPECT_FLOAT_EQ(defaults.mag_softiron[0], 1.0f);
    EXPECT_FLOAT_EQ(defaults.mag_softiron[4], 1.0f);
    EXPECT_FLOAT_EQ(defaults.mag_softiron[8], 1.0f);
    EXPECT_FLOAT_EQ(defaults.mag_field, 50.0f);
}

TEST_F(MagCalibrationDefaultFixture, BeginCalibrationResetsState) {
    calibration.addSample(10.0f, 0.0f, 0.0f);
    calibration.beginCalibration();
    EXPECT_FLOAT_EQ(calibration.getCoveragePercent(), 0.0f);
}

TEST_F(MagCalibrationDefaultFixture, AddSampleStoresValidSample) {
    bool added = calibration.addSample(10.0f, 0.0f, 0.0f);
    EXPECT_TRUE(added);
    MagData last = calibration.getLastSample();
    EXPECT_FLOAT_EQ(last.x, 10.0f);
    EXPECT_FLOAT_EQ(last.y, 0.0f);
    EXPECT_FLOAT_EQ(last.z, 0.0f);
}

TEST_F(MagCalibrationDefaultFixture, AddSampleRejectsDuplicateOrientation) {
    calibration.beginCalibration();
    bool added1 = calibration.addSample(10.0f, 0.0f, 0.0f);
    bool added2 = calibration.addSample(20.0f, 0.0f, 0.0f); // Same direction
    EXPECT_TRUE(added1);
    EXPECT_FALSE(added2);
}

TEST_F(MagCalibrationDefaultFixture, CoveragePercentIncreasesWithSamples) {
    calibration.beginCalibration();
    float before = calibration.getCoveragePercent();
    calibration.addSample(10.0f, 0.0f, 0.0f);
    float after = calibration.getCoveragePercent();
    EXPECT_LT(before, after);
}

TEST_F(MagCalibrationDefaultFixture, GetVisPointsReturnsCorrectCount) {
    calibration.beginCalibration();
    calibration.addSample(10.0f, 0.0f, 0.0f);
    const double *x, *y, *z;
    uint16_t count = calibration.getVisPoints(x, y, z);
    EXPECT_EQ(count, 1);
    EXPECT_DOUBLE_EQ(x[0], 10.0);
    EXPECT_DOUBLE_EQ(y[0], 0.0);
    EXPECT_DOUBLE_EQ(z[0], 0.0);
}

TEST_F(MagCalibrationDefaultFixture, CalculateHardIronOffsetsDefault) {
    calibration.beginCalibration();
    // Not enough samples, should return default offsets
    HardIronOffsets offsets = calibration.calculateHardIronOffsets();
    EXPECT_FLOAT_EQ(offsets.offsets.x, 0.0f);
    EXPECT_FLOAT_EQ(offsets.offsets.y, 0.0f);
    EXPECT_FLOAT_EQ(offsets.offsets.z, 0.0f);
    EXPECT_FALSE(offsets.sufficientSpread);
}

// Test invalid input handling
TEST_F(MagCalibrationDefaultFixture, InvalidInputHandling) {
    calibration.beginCalibration();

    // Test NaN inputs
    EXPECT_FALSE(calibration.addSample(NAN, 0.0f, 0.0f));
    EXPECT_FALSE(calibration.addSample(0.0f, NAN, 0.0f));
    EXPECT_FALSE(calibration.addSample(0.0f, 0.0f, NAN));

    // Test infinite inputs
    EXPECT_FALSE(calibration.addSample(INFINITY, 0.0f, 0.0f));
    EXPECT_FALSE(calibration.addSample(0.0f, INFINITY, 0.0f));
    EXPECT_FALSE(calibration.addSample(0.0f, 0.0f, INFINITY));

    // Test zero magnitude (before offsets applied)
    EXPECT_FALSE(calibration.addSample(0.0f, 0.0f, 0.0f));

    // Valid input should still work
    EXPECT_TRUE(calibration.addSample(10.0f, 20.0f, 30.0f));
}

// Test spherical binning system accuracy
TEST_F(MagCalibrationDefaultFixture, SphericalBinningAccuracy) {
    calibration.beginCalibration();

    // Add samples in different orientations
    calibration.addSample(50.0f, 0.0f, 0.0f);   // +X direction
    calibration.addSample(-50.0f, 0.0f, 0.0f);  // -X direction
    calibration.addSample(0.0f, 50.0f, 0.0f);   // +Y direction
    calibration.addSample(0.0f, -50.0f, 0.0f);  // -Y direction
    calibration.addSample(0.0f, 0.0f, 50.0f);   // +Z direction
    calibration.addSample(0.0f, 0.0f, -50.0f);  // -Z direction

    // Check coverage increased
    EXPECT_GT(calibration.getCoveragePercent(), 0.0f);

    // Check active bin count
    EXPECT_GT(calibration.getActiveBinCount(), 0);
    EXPECT_LE(calibration.getActiveBinCount(), 6); // Should not exceed sample count

    // Test bin distribution functions
    const uint16_t* elevationCounts;
    const uint16_t* azimuthCounts;

    uint8_t elevationBins = calibration.getElevationBinDistribution(elevationCounts);
    uint8_t azimuthBins = calibration.getAzimuthBinDistribution(azimuthCounts);

    EXPECT_GT(elevationBins, 0);
    EXPECT_GT(azimuthBins, 0);
    EXPECT_EQ(elevationBins, 180 / 9); // 180 degrees / 9 degree bins = 20
    EXPECT_EQ(azimuthBins, 360 / 9);   // 360 degrees / 9 degree bins = 40
}

// Test maximum sample capacity
TEST_F(MagCalibrationDefaultFixture, MaximumSampleCapacity) {
    calibration.beginCalibration();

    int samplesAdded = 0;
    bool stillAdding = true;

    // Try to fill up to maximum capacity
    for(int i = 0; i < 1000 && stillAdding; i++) {
        // Create varied orientations to avoid duplicate rejection
        float angle1 = i * 0.1f;
        float angle2 = i * 0.05f;
        float x = 50.0f * cosf(angle1) * sinf(angle2);
        float y = 50.0f * sinf(angle1) * sinf(angle2);
        float z = 50.0f * cosf(angle2);

        if(calibration.addSample(x, y, z)) {
            samplesAdded++;
        } else {
            // Check if we hit capacity or just duplicate orientation
            if(calibration.getCoveragePercent() > 95.0f) {
                stillAdding = false; // Likely hit capacity
            }
        }
    }

    // Should have added a reasonable number of samples
    EXPECT_GT(samplesAdded, 100); // Should get decent coverage
    EXPECT_LT(samplesAdded, 1000); // But not unlimited
}
