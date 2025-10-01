/**
 *   GPStar Supporting Library.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
 *                         & Nomake Wan <nomake_wan@yahoo.co.jp>
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
    // Constants for binning and buffer sizes.
    static constexpr int NUM_AZIMUTH_BINS = 18;
    static constexpr int NUM_ELEVATION_BINS = 9;
    static constexpr int MAX_POINTS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS;
    static constexpr int MAX_SAMPLES = MAX_POINTS * 2;

    // Internal buffers for samples and visualization.
    float xSamples[MAX_SAMPLES];
    float ySamples[MAX_SAMPLES];
    float zSamples[MAX_SAMPLES];
    int sampleCount;

    float visX[MAX_POINTS];
    float visY[MAX_POINTS];
    float visZ[MAX_POINTS];
    int visCount;

    // Bin filled flags.
    bool bins[MAX_POINTS];

    // Helper: Jacobi eigen-decomposition (symmetric 3x3).
    void jacobiEigen3(float A[3][3], float V[3][3], float w[3]) const;

    // Helper: Solve small linear system Ax = b.
    bool solveLinearSystem(int n, float A[], float b[], float x[]) const;

    // Helper: 3x3 inverse (returns false if singular).
    bool invert3x3(const float A_in[3][3], float A_out[3][3]) const;

    // Helper: Round float to 3 decimal places.
    float roundFloat3(float val) const;
};
