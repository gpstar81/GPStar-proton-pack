/**
 *   MagCalibration - Magnetometer calibration class for GPStar devices.
 *   Handles magnetometer data collection and calibration calculations.
 *   Copyright (C) 2023-2025 Michael Rajotte, Dustin Grau, Nomake Wan
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

 /**
 * This code was co-authored through both ChatGPT and GitHub Copilot.
 * All efforts were made to ensure correctness and clarity throughout
 * and follow standard practices for performing calculations for the
 * necessary offsets. Any similarities to existing projects should be
 * considered coincidental unless otherwise noted.
 */

#pragma once

// Standard library includes for integer type definitions
#include <stdint.h>  // Provides uint8_t, uint16_t, etc.
#include <stdbool.h> // Provides bool type definition.

/**
 * Struct: MagData
 * Purpose: Holds a single triplet of XYZ values as floats.
 */
struct MagData {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

 /**
 * Struct: HardIronOffsets
 * Purpose: Holds offsets and spread info from hard-iron calculation B1.
 * Used to hold the results of provisional samples for hard-iron offset
 * calculation, and will be applied to samples for purposes of creating
 * a revised hard-iron offset (B2) and soft-iron matrix. Final offsets
 * for hard-iron will be computed as B1 + B2.
 */
struct HardIronOffsets {
  MagData offsets;
  MagData range;
  bool sufficientSpread = false;
  bool offsetsApplied = false;
};

/**
 * Magnetometer calibration struct.
 * - mag_hardiron: x/y/z offsets to remove permanent magnet biases
 * - mag_softiron: 3x3 diagonal matrix to correct axis scaling
 * - mag_field: average magnitude of corrected samples (optional)
 */
struct CalibrationData {
  float mag_hardiron[3] = {0.0f, 0.0f, 0.0f};
  float mag_softiron[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  };
  float mag_field = 50.0f;
};

/**
 * Class: MagCalibration
 * Purpose: Encapsulates calibration data, sampling, and computation for a magnetometer.
 * Usage:
 *   MagCalibration magCal;
 *   magCal.beginCalibration();
 *   magCal.addSample(x, y, z);
 *   float percent = magCal.getCoveragePercent();
 *   CalibrationData data = magCal.computeCalibration();
 */
class MagCalibration {
  public:
    MagCalibration();

    // Returns the last status message for debugging purposes.
    const char* getStatusMessage() const { return statusMessage; }

    // Returns a CalibrationData struct with default values.
    CalibrationData getDefaultCalibration() const {
      CalibrationData defaults;

      // Assign default hard-iron offsets
      defaults.mag_hardiron[0] = 0.0f;
      defaults.mag_hardiron[1] = 0.0f;
      defaults.mag_hardiron[2] = 0.0f;

      // Assign default soft-iron matrix (identity for diagonal, zero for off-diagonal)
      defaults.mag_softiron[0] = 1.0f; // X scale
      defaults.mag_softiron[1] = 0.0f; // XY
      defaults.mag_softiron[2] = 0.0f; // XZ
      defaults.mag_softiron[3] = 0.0f; // YX
      defaults.mag_softiron[4] = 1.0f; // Y scale
      defaults.mag_softiron[5] = 0.0f; // YZ
      defaults.mag_softiron[6] = 0.0f; // ZX
      defaults.mag_softiron[7] = 0.0f; // ZY
      defaults.mag_softiron[8] = 1.0f; // Z scale

      // mag_field: typical Earth field strength
      defaults.mag_field = 50.0f;

      return defaults;
    }

    // Begin a new calibration session by clearing buffers and coverage.
    void beginCalibration();

    // Add a raw magnetometer sample, only stores if it expands coverage.
    // Returns true if sample was added, false if ignored (duplicate bin or max samples reached).
    bool addSample(float x, float y, float z);

    // Return the raw magnetometer data of the last raw sample taken.
    MagData getLastSample() const;

    // Get coverage % (0..100) based on filled bins.
    float getCoveragePercent() const;

    // Get usable points for visualization.
    // Outputs: pointers to internal arrays and count.
    uint16_t getVisPoints(const double*& outX, const double*& outY, const double*& outZ) const;

    // Compute hard-iron offsets from collected samples.
    HardIronOffsets calculateHardIronOffsets();

    // Compute calibration parameters from collected samples.
    CalibrationData computeCalibration() const;

    // Get elevation bin distribution for coverage analysis.
    // Outputs: pointer to internal elevation bin count array and bin count.
    uint8_t getElevationBinDistribution(const uint16_t*& outElevationCounts) const;

    // Get active azimuth bins for coverage analysis.
    // Outputs: pointer to internal azimuth bin count array and bin count.
    uint8_t getAzimuthBinDistribution(const uint16_t*& outAzimuthCounts) const;

    // Get total number of active bins (bins with at least one sample).
    uint16_t getActiveBinCount() const;

  private:
    // Minimum samples before allowing hard-iron calculations.
    static constexpr uint16_t HARD_IRON_SAMPLE_THRESHOLD = 30;

    // Minimum range in µT for sufficient hard-iron calculations.
    static constexpr float HARD_IRON_SPREAD_THRESHOLD = 40.0f;

    // Hard-iron offset to apply to incoming samples, once calculated.
    HardIronOffsets provisionalHardIron;
    MagData lastRawSample; // Always holds the most recent raw sample.
    char statusMessage[128]; // Holds last status message for debugging.

    /**
     * SPHERICAL COORDINATE BINNING SYSTEM:
     *
     * This class divides the magnetometer's 3D measurement space into a grid of spherical bins.
     * Since magnetometer readings represent magnetic field vectors, we normalize them to unit
     * vectors on a sphere surface and categorize them by direction. The use of bins allows us
     * to control the size of the data structures necessary for storing coverage information.
     *
     * COORDINATE SYSTEM:
     * - Input: Raw magnetometer readings (x, y, z) in µT (micro-Tesla)
     * - Normalized: Unit vector (nx, ny, nz) where sqrt(nx² + ny² + nz²) = 1.0
     * - Spherical: (azimuth, elevation) angles in radians
     *
     * AZIMUTH (Horizontal Rotation):
     * - Range: -π to +π radians (-180° to +180°) = 360° total coverage
     * - Physical meaning: Rotation around the Z-axis (like compass heading)
     * - Bins: 360° total coverage / degrees per bin = total bins
     * - Formula: azimuth = atan2(ny, nx)
     *
     * ELEVATION (Vertical Tilt):
     * - Range: -π/2 to +π/2 radians (-90° to +90°) = 180° total coverage
     * - Physical meaning: Tilt up/down from horizontal plane
     * - Bins: 90° total coverage / degrees per bin = total bins
     * - Formula: elevation = asin(nz)
     *
     * TOTAL COVERAGE SPACE:
     * - Each bin represents a N° × N° "patch" on the unit sphere
     * - 100% coverage = all bins filled with at least one sample
     * - Ideally we want at least 60% coverage of bins for good calibration
     * - Examples:
     *    - 10° bins = 36 azimuth × 18 elevation = 648 total bins
     *    - 5° bins = 72 azimuth × 36 elevation = 2,592 total bins
     *
     * COVERAGE REQUIREMENTS:
     * For good calibration, the magnetometer should be oriented through many
     * different directions. The user should move the device not only around
     * and up/down, but also rotate it throughout the motions to pick up all
     * spatial orientations.
     */
    static constexpr uint8_t BIN_DEGREES = 9; // Bin size in whole degrees, used to calculate number of bins.

    // Calculate bin counts from resolution (360° azimuth coverage, 180° elevation coverage)
    // Azimuth bins cover full horizontal rotation (compass directions)
    // Elevation bins cover vertical tilt range (from looking down to looking up)
    static constexpr uint8_t NUM_AZIMUTH_BINS = (uint8_t)(360 / BIN_DEGREES);   // Horizontal orientation bins
    static constexpr uint8_t NUM_ELEVATION_BINS = (uint8_t)(180 / BIN_DEGREES); // Vertical tilt bins

    // Total orientation regions to cover for complete calibration
    // Memory usage scales quadratically with resolution (4x memory for half the degrees)
    static constexpr uint16_t MAX_POINTS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS;

    // Internal buffers for samples using double precision.
    double xSamples[MAX_POINTS];
    double ySamples[MAX_POINTS];
    double zSamples[MAX_POINTS];
    uint16_t sampleCount; // Common tracker for samples taken.

    // Bin filled flags.
    bool bins[MAX_POINTS];

    // Bin usage tracking for distribution analysis
    // Purpose: Track how many samples have been collected in each orientation region
    // These arrays enable real-time monitoring of calibration coverage distribution
    uint16_t elevationBinCounts[NUM_ELEVATION_BINS]; // Samples per elevation bin (vertical coverage)
    uint16_t azimuthBinCounts[NUM_AZIMUTH_BINS];     // Samples per azimuth bin (horizontal coverage)

    // Helper: Calculate calibration using diagonal fallback method.
    CalibrationData calculateDiagonalFallback() const;

    // Resets provisionalHardIron to default values.
    void resetProvisionalHardIron();

    // Helper: Clear all samples and reset state for collection.
    void resetSamples();

    // Helper: Clear any existing status message.
    void clearStatusMessage();

    // Helper: Jacobi eigen-decomposition (symmetric 3x3).
    void jacobiEigen3(float A[3][3], float V[3][3], float w[3]) const;

    // Helper: Solve small linear system Ax = b.
    bool solveLinearSystem(int n, double A[], double b[], float x[]) const;

    // Helper: 3x3 inverse (returns false if singular).
    bool invert3x3(const float A_in[3][3], float A_out[3][3]) const;

    // Helper: Round float to 3 decimal places.
    float roundFloat3(float val) const;
};
