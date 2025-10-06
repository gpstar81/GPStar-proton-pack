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

#pragma once

/**
 * This code was co-authored by ChatGPT and GitHub Copilot.
 */

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

    // Begin a new calibration session by clearing buffers and coverage.
    void beginCalibration();

    // Add a raw magnetometer sample, only stores if it expands coverage.
    // Returns true if sample was added, false if ignored (duplicate bin or max samples reached).
    bool addSample(float x, float y, float z);

    // Get coverage % (0..100) based on filled bins.
    float getCoveragePercent() const;

    // Get usable points for visualization.
    // Outputs: pointers to internal arrays and count.
    int getVisPoints(const float*& outX, const float*& outY, const float*& outZ) const;

    // Compute calibration parameters from collected samples.
    CalibrationData computeCalibration() const;

  private:
    /**
     * SPHERICAL COORDINATE BINNING SYSTEM:
     * 
     * This class divides the magnetometer's 3D measurement space into a grid of spherical bins.
     * Since magnetometer readings represent magnetic field vectors, we normalize them to unit 
     * vectors on a sphere surface and categorize them by direction.
     * 
     * COORDINATE SYSTEM:
     * - Input: Raw magnetometer readings (x, y, z) in µT (micro-Tesla)
     * - Normalized: Unit vector (nx, ny, nz) where sqrt(nx² + ny² + nz²) = 1.0
     * - Spherical: (azimuth, elevation) angles in radians
     * 
     * AZIMUTH (Horizontal Rotation):
     * - Range: -π to +π radians (-180° to +180°) = 360° total coverage
     * - Physical meaning: Rotation around the Z-axis (like compass heading)
     * - Bins: 18 bins × 20° each = 360° total coverage
     * - Formula: azimuth = atan2(ny, nx)
     * 
     * ELEVATION (Vertical Tilt):
     * - Range: -π/2 to +π/2 radians (-90° to +90°) = 180° total coverage
     * - Physical meaning: Tilt up/down from horizontal plane
     * - Bins: 9 bins × 20° each = 180° total coverage
     * - Formula: elevation = asin(nz)
     * 
     * TOTAL COVERAGE SPACE:
     * - Total bins: 36 × 18 = 648 discrete orientation regions
     * - Each bin represents a 10° × 10° "patch" on the unit sphere
     * - 100% coverage = all 648 bins filled with at least one sample
     * 
     * COVERAGE REQUIREMENTS:
     * For good calibration, the magnetometer should be oriented through many 
     * different directions. The user should move the device not only around
     * and up/down, but also rotate it throughout the motions to pick up all
     * spatial orientations.
     */
    static constexpr int NUM_AZIMUTH_BINS = 36; // Horizontal, around the Z axis (0 to 360 degrees).
    static constexpr int NUM_ELEVATION_BINS = 18; // Vertical, from -90 to +90 degrees (up/down).
    static constexpr int MAX_POINTS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS; // aka. Bin total.
    static constexpr int MAX_SAMPLES = MAX_POINTS * 2; // Ensure sufficient samples for fitting.

    // Internal buffers for samples using double precision.
    double xSamples[MAX_SAMPLES];
    double ySamples[MAX_SAMPLES];
    double zSamples[MAX_SAMPLES];
    int sampleCount; // Common tracker for samples taken.

    // Internal buffers for visualization data.
    float visX[MAX_POINTS];
    float visY[MAX_POINTS];
    float visZ[MAX_POINTS];
    int visCount; // Common tracker for points available.

    // Bin filled flags.
    bool bins[MAX_POINTS];

    // Helper: Jacobi eigen-decomposition (symmetric 3x3).
    void jacobiEigen3(float A[3][3], float V[3][3], float w[3]) const;

    // Helper: Solve small linear system Ax = b.
    bool solveLinearSystem(int n, double A[], double b[], float x[]) const;

    // Helper: 3x3 inverse (returns false if singular).
    bool invert3x3(const float A_in[3][3], float A_out[3][3]) const;

    // Helper: Round float to 3 decimal places.
    float roundFloat3(float val) const;
};
