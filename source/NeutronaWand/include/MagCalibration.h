/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Magnetometer calibration library which should be utilized AFTER the Neutrona Wand PCB has
 * been fully assembled into it's final state. This includes any speakers (with magnets) so
 * that the calibration data takes these into account.
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
  float mag_field = 50.0f; // not required for heading calculations
};

/**
 * MagCal: Encapsulates calibration data, sampling, and computation.
 *
 * Workflow:
 *  1. beginCalibration() - clears buffers and coverage.
 *  2. addSample(x,y,z) - adds a sample if it expands coverage.
 *  3. getCoveragePercent() - percentage of the sphere covered.
 *  4. getVisPoints() - usable points for visualization (Three.js).
 *  5. computeCalibration() - final CalibrationData for use in sensor reads.
 */
namespace MagCal {

  // Configurable constants
  // For higher precision: increase MAX_POINTS or implement finer buckets
  constexpr int MAX_SAMPLES = 500;       // Max raw samples stored during calibration
  constexpr int MAX_POINTS = 200;        // Max points for visualization
  constexpr int NUM_AZIMUTH_BINS = 36;   // 10° horizontal divisions
  constexpr int NUM_ELEVATION_BINS = 18; // 10° vertical divisions
  constexpr int TOTAL_BUCKETS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS;

  // Internal buffers
  static float xSamples[MAX_SAMPLES];
  static float ySamples[MAX_SAMPLES];
  static float zSamples[MAX_SAMPLES];
  static int sampleCount = 0;

  static float visX[MAX_POINTS];
  static float visY[MAX_POINTS];
  static float visZ[MAX_POINTS];
  static int visCount = 0;

  // Bucket filled flags
  static bool buckets[TOTAL_BUCKETS] = {false};

  // Begin a new calibration session by clearing buffers and coverage.
  inline void beginCalibration() {
      sampleCount = 0;
      visCount = 0;
      for(int i=0;i<TOTAL_BUCKETS;i++) buckets[i] = false;
  }

  // Add a raw magnetometer sample, only stores if it expands coverage
  inline void addSample(float x, float y, float z) {
    if(sampleCount >= MAX_SAMPLES) return;

    // Normalize vector
    float r = sqrt(x*x + y*y + z*z);
    if(r == 0) return;
    float nx = x / r;
    float ny = y / r;
    float nz = z / r;

    // Spherical coordinates
    float az = atan2(ny, nx); // -PI..PI
    float el = asin(nz);      // -PI/2..PI/2

    // Map to bucket indices
    int azIndex = (int)((az + M_PI) / (2 * M_PI) * NUM_AZIMUTH_BINS);
    int elIndex = (int)((el + M_PI/2) / M_PI * NUM_ELEVATION_BINS);

    if(azIndex < 0) azIndex = 0;
    if(azIndex >= NUM_AZIMUTH_BINS) azIndex = NUM_AZIMUTH_BINS-1;
    if(elIndex < 0) elIndex = 0;
    if(elIndex >= NUM_ELEVATION_BINS) elIndex = NUM_ELEVATION_BINS-1;

    int bucketIndex = elIndex * NUM_AZIMUTH_BINS + azIndex;

    // Only store if bucket is empty
    if(!buckets[bucketIndex]) {
      buckets[bucketIndex] = true;

      // Store in calibration buffer
      xSamples[sampleCount] = x;
      ySamples[sampleCount] = y;
      zSamples[sampleCount] = z;
      sampleCount++;

      // Store in visualization buffer
      if(visCount < MAX_POINTS) {
          visX[visCount] = x;
          visY[visCount] = y;
          visZ[visCount] = z;
          visCount++;
      }
    }
  }

  // Get coverage % (0..100)
  inline float getCoveragePercent() {
    int filled = 0;
    for(int i=0;i<TOTAL_BUCKETS;i++) if(buckets[i]) filled++;
    return (filled / (float)TOTAL_BUCKETS) * 100.0f;
  }

  // Get usable points for visualization
  inline int getVisPoints(const float*& outX, const float*& outY, const float*& outZ) {
    outX = visX;
    outY = visY;
    outZ = visZ;
    return visCount;
  }

  // Compute final calibration with ellipsoid fitting
  inline CalibrationData computeCalibration() {
    CalibrationData cal;

    if(sampleCount == 0) return cal; // Nothing to compute

    // Step 1: find min/max per axis
    float minX=xSamples[0], maxX=xSamples[0];
    float minY=ySamples[0], maxY=ySamples[0];
    float minZ=zSamples[0], maxZ=zSamples[0];

    for(int i=1;i<sampleCount;i++){
        if (xSamples[i]<minX) minX=xSamples[i];
        if (xSamples[i]>maxX) maxX=xSamples[i];
        if (ySamples[i]<minY) minY=ySamples[i];
        if (ySamples[i]>maxY) maxY=ySamples[i];
        if (zSamples[i]<minZ) minZ=zSamples[i];
        if (zSamples[i]>maxZ) maxZ=zSamples[i];
    }

    // Step 2: hard-iron offsets
    cal.mag_hardiron[0] = (maxX+minX)/2.0f;
    cal.mag_hardiron[1] = (maxY+minY)/2.0f;
    cal.mag_hardiron[2] = (maxZ+minZ)/2.0f;

    // Step 3: soft-iron diagonal scaling
    float avgRadius = ((maxX-minX)+(maxY-minY)+(maxZ-minZ))/6.0f;
    float scaleX = avgRadius / ((maxX-minX)/2.0f);
    float scaleY = avgRadius / ((maxY-minY)/2.0f);
    float scaleZ = avgRadius / ((maxZ-minZ)/2.0f);

    cal.mag_softiron[0] = scaleX;
    cal.mag_softiron[1] = 0;
    cal.mag_softiron[2] = 0;
    cal.mag_softiron[3] = 0;
    cal.mag_softiron[4] = scaleY;
    cal.mag_softiron[5] = 0;
    cal.mag_softiron[6] = 0;
    cal.mag_softiron[7] = 0;
    cal.mag_softiron[8] = scaleZ;

    // Step 4: average field magnitude
    float sumB = 0;
    for(int i=0;i<sampleCount;i++){
        float mx = (xSamples[i]-cal.mag_hardiron[0])*scaleX;
        float my = (ySamples[i]-cal.mag_hardiron[1])*scaleY;
        float mz = (zSamples[i]-cal.mag_hardiron[2])*scaleZ;
        sumB += sqrt(mx*mx + my*my + mz*mz);
    }
    cal.mag_field = sumB / sampleCount;

    return cal;
  }
}
