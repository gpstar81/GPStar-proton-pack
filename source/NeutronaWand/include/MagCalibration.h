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

  // Configurable constants; increase samples and bins for more precision if necessary.
  // The bin approach ensures even coverage of the sphere by dividing it into discrete sections.
  // Each filled bin becomes a stored sample until all bins are filled (represented as coverage).
  constexpr int NUM_AZIMUTH_BINS = 18;        // 20° horizontal divisions (heading: 1-360°)
  constexpr int NUM_ELEVATION_BINS = 9;       // 20° vertical divisions (-90° to +90°)
  constexpr int MAX_POINTS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS; // Total discrete bins, also used for visualization
  constexpr int MAX_SAMPLES = MAX_POINTS * 2; // Max unique samples stored during calibration

  // Internal buffers
  static float xSamples[MAX_SAMPLES];
  static float ySamples[MAX_SAMPLES];
  static float zSamples[MAX_SAMPLES];
  static int sampleCount = 0;

  static float visX[MAX_POINTS];
  static float visY[MAX_POINTS];
  static float visZ[MAX_POINTS];
  static int visCount = 0;

  // Bin filled flags
  static bool bins[MAX_POINTS] = {false};

  // Begin a new calibration session by clearing buffers and coverage.
  inline void beginCalibration() {
      sampleCount = 0;
      visCount = 0;
      for(int i=0; i<MAX_POINTS; i++) bins[i] = false;
  }

  // Add a raw magnetometer sample, only stores if it expands coverage
  // Returns true if sample was added, false if ignored (duplicate bin or max samples reached)
  inline bool addSample(float x, float y, float z) {
    if (sampleCount >= MAX_SAMPLES) return false; // Max samples reached

    // Normalize vector
    float r = sqrt(x * x + y * y + z * z);
    if (r == 0) return false; // Invalid sample
    float nx = x / r;
    float ny = y / r;
    float nz = z / r;

    // Spherical coordinates
    float az = atan2(ny, nx); // -PI..PI
    float el = asin(nz);      // -PI/2..PI/2

    // Map to bin indices
    int azIndex = (int)((az + M_PI) / (2 * M_PI) * NUM_AZIMUTH_BINS);
    int elIndex = (int)((el + M_PI/2) / M_PI * NUM_ELEVATION_BINS);

    if (azIndex < 0) azIndex = 0;
    if (azIndex >= NUM_AZIMUTH_BINS) azIndex = NUM_AZIMUTH_BINS - 1;
    if (elIndex < 0) elIndex = 0;
    if (elIndex >= NUM_ELEVATION_BINS) elIndex = NUM_ELEVATION_BINS - 1;

    int binIndex = elIndex * NUM_AZIMUTH_BINS + azIndex;

    // Only store if bin is empty
    if (!bins[binIndex]) {
      bins[binIndex] = true;

      // Store in calibration buffer
      xSamples[sampleCount] = x;
      ySamples[sampleCount] = y;
      zSamples[sampleCount] = z;
      sampleCount++;

      // Store in visualization buffer
      if (visCount < MAX_POINTS) {
        visX[visCount] = x;
        visY[visCount] = y;
        visZ[visCount] = z;
        visCount++;
      }

      return true; // new bin filled
    }

    return false; // duplicate bin or max samples reached
  }

  // Get coverage % (0..100) based on filled bins
  inline float getCoveragePercent() {
    int filled = 0;
    for (int i = 0; i < MAX_POINTS; i++) if (bins[i]) filled++;
    return (filled / (float)MAX_POINTS) * 100.0f;
  }

  // Get usable points for visualization
  inline int getVisPoints(const float*& outX, const float*& outY, const float*& outZ) {
    outX = visX;
    outY = visY;
    outZ = visZ;
    return visCount;
  }

  // Helper: compute mean (center) of samples
  inline void computeMeanCenter(float &cx, float &cy, float &cz) {
    if (sampleCount == 0) { cx = cy = cz = 0.0f; return; }
    double sx = 0, sy = 0, sz = 0;
    for (int i = 0; i < sampleCount; ++i) {
      sx += xSamples[i];
      sy += ySamples[i];
      sz += zSamples[i];
    }
    cx = (float)(sx / sampleCount);
    cy = (float)(sy / sampleCount);
    cz = (float)(sz / sampleCount);
  }

  // Helper: compute covariance matrix of centered samples (3x3 symmetric)
  inline void computeCovariance(const float cx, const float cy, const float cz, float cov[3][3]) {
    // zero
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) cov[i][j] = 0.0f;
    if (sampleCount < 2) return;

    // accumulate
    for (int i = 0; i < sampleCount; ++i) {
      float dx = xSamples[i] - cx;
      float dy = ySamples[i] - cy;
      float dz = zSamples[i] - cz;
      cov[0][0] += dx * dx;
      cov[0][1] += dx * dy;
      cov[0][2] += dx * dz;
      cov[1][1] += dy * dy;
      cov[1][2] += dy * dz;
      cov[2][2] += dz * dz;
    }

    // normalize by (N - 1) for sample covariance
    float norm = 1.0f / (float)(sampleCount - 1);
    cov[0][0] *= norm;
    cov[0][1] *= norm; cov[1][0] = cov[0][1];
    cov[0][2] *= norm; cov[2][0] = cov[0][2];
    cov[1][1] *= norm;
    cov[1][2] *= norm; cov[2][1] = cov[1][2];
    cov[2][2] *= norm;
  }

  // Small Jacobi eigen-decomposition for symmetric 3x3 matrix
  // Produces eigenvalues in w[0..2] and orthonormal eigenvectors in V (columns)
  inline void jacobiEigen3(float A[3][3], float V[3][3], float w[3]) {
    // Initialize V to identity, copy A to local
    float a00 = A[0][0], a01 = A[0][1], a02 = A[0][2];
    float a11 = A[1][1], a12 = A[1][2];
    float a22 = A[2][2];

    // V = I
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) V[i][j] = (i==j) ? 1.0f : 0.0f;

    // Using a simple Jacobi rotation loop
    const int MAX_ITER = 50;
    for (int iter = 0; iter < MAX_ITER; ++iter) {
      // find largest off-diagonal element
      float abs01 = fabsf(a01), abs02 = fabsf(a02), abs12 = fabsf(a12);
      int p=0,q=1;
      float maxv = abs01;
      if (abs02 > maxv) { maxv = abs02; p=0; q=2; }
      if (abs12 > maxv) { maxv = abs12; p=1; q=2; }

      if (maxv < 1e-6f) break; // converged

      // Get current a_pq
      float apq = (p==0 && q==1) ? a01 : (p==0 && q==2) ? a02 : a12;
      float app = (p==0) ? a00 : (p==1) ? a11 : a22;
      float aqq = (q==0) ? a00 : (q==1) ? a11 : a22;

      float phi = 0.5f * atan2f(2.0f * apq, aqq - app);
      float c = cosf(phi);
      float s = sinf(phi);

      // Rotation: update A entries (only affected rows/cols)
      // Build rotation matrix elements
      // Compute new diagonal entries
      float app1 = c*c*app - 2.0f*s*c*apq + s*s*aqq;
      float aqq1 = s*s*app + 2.0f*s*c*apq + c*c*aqq;
      float apq1 = 0.0f;

      // other off-diagonals
      float a0 = a00, a1 = a11, a2 = a22;
      float b01 = a01, b02 = a02, b12 = a12;

      // apply rotation depending on which pair (p,q)
      if (p==0 && q==1) {
        a00 = app1; a11 = aqq1; a01 = apq1;
        a02 = c*b02 - s*b12;
        a12 = s*b02 + c*b12;
      } else if (p==0 && q==2) {
        a00 = app1; a22 = aqq1; a02 = apq1;
        a01 = c*b01 - s*b12;
        a12 = s*b01 + c*b12;
      } else { // p==1 && q==2
        a11 = app1; a22 = aqq1; a12 = apq1;
        a01 = c*b01 - s*b02;
        a02 = s*b01 + c*b02;
      }

      // Update eigenvector matrix V = V * R(p,q,phi)
      for (int i=0;i<3;i++) {
        float vip = V[i][p];
        float viq = V[i][q];
        V[i][p] = c*vip - s*viq;
        V[i][q] = s*vip + c*viq;
      }

      // ensure symmetry maintained for a01,a02,a12 already set above
      // loop continues until converge
    }

    // set eigenvalues from diagonals
    w[0] = a00; w[1] = a11; w[2] = a22;

    // NOTE: after Jacobi rotations we may not have eigenvalues sorted. We'll sort them (descending)
    for (int i=0;i<2;i++) {
      int idx = i;
      for (int j=i+1;j<3;j++) if (w[j] > w[idx]) idx = j;
      if (idx != i) {
        float tw = w[i]; w[i] = w[idx]; w[idx] = tw;
        // swap columns of V
        for (int r=0;r<3;r++) {
          float tv = V[r][i]; V[r][i] = V[r][idx]; V[r][idx] = tv;
        }
      }
    }
  }

  // Build soft-iron matrix M that whitens covariance: M = E * diag(1/sqrt(lambda)) * E^T
  inline void buildSoftIronFromCov(const float cov[3][3], float Mout[3][3]) {
    float V[3][3];
    float lambda[3];
    // copy cov into mutable array for jacobi
    float Acopy[3][3];
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) Acopy[i][j] = cov[i][j];

    jacobiEigen3(Acopy, V, lambda);

    // guard: ensure positive eigenvalues
    for (int i=0;i<3;i++) if (lambda[i] <= 1e-9f) lambda[i] = 1e-9f;

    // compute D^{-1/2}
    float DinvSqrt[3];
    for (int i=0;i<3;i++) DinvSqrt[i] = 1.0f / sqrtf(lambda[i]);

    // M = V * diag(DinvSqrt) * V^T  (row-major)
    // compute temp = V * diag(DinvSqrt)
    float temp[3][3];
    for (int r=0;r<3;r++) {
      for (int c=0;c<3;c++) temp[r][c] = V[r][c] * DinvSqrt[c];
    }
    // Mout = temp * V^T
    for (int r=0;r<3;r++) {
      for (int c=0;c<3;c++) {
        float sum = 0.0f;
        for (int k=0;k<3;k++) sum += temp[r][k] * V[c][k]; // V^T element (k,c) -> V[c][k]
        Mout[r][c] = sum;
      }
    }
  }

  // Complete final calibration with centroid + covariance whitening (full 3x3 soft-iron)
  // Note: This is not a full ellipsoid fitting as used by MotionCal.
  inline CalibrationData computeCalibrationComplete() {
    CalibrationData cal;

    if (sampleCount == 0) return cal;

    // 1) compute centroid (hard-iron estimate)
    float cx, cy, cz;
    computeMeanCenter(cx, cy, cz);
    cal.mag_hardiron[0] = cx;
    cal.mag_hardiron[1] = cy;
    cal.mag_hardiron[2] = cz;

    // 2) compute covariance of centered samples
    float cov[3][3];
    computeCovariance(cx, cy, cz, cov);

    // 3) build whitening matrix from covariance (soft-iron matrix)
    float M[3][3];
    buildSoftIronFromCov(cov, M);

    // Optionally scale so that mean corrected magnitude equals typical Earth field.
    // Compute mean radius after applying M to centered samples
    double sumR = 0.0;
    for (int i=0;i<sampleCount;i++) {
      float dx = xSamples[i] - cx;
      float dy = ySamples[i] - cy;
      float dz = zSamples[i] - cz;
      // r' = M * d
      float rx = M[0][0]*dx + M[0][1]*dy + M[0][2]*dz;
      float ry = M[1][0]*dx + M[1][1]*dy + M[1][2]*dz;
      float rz = M[2][0]*dx + M[2][1]*dy + M[2][2]*dz;
      sumR += sqrtf(rx*rx + ry*ry + rz*rz);
    }
    float meanR = (float)(sumR / sampleCount);

    // If you want mag_field to be in microTesla, you can compute scaling factor to map meanR -> measured Earth field.
    // But we will set mag_field = meanR, and leave matrix as whitening (so corrected magnitudes ~1.0)
    cal.mag_field = meanR;

    // Write M into cal.mag_softiron in row-major order
    cal.mag_softiron[0] = M[0][0]; cal.mag_softiron[1] = M[0][1]; cal.mag_softiron[2] = M[0][2];
    cal.mag_softiron[3] = M[1][0]; cal.mag_softiron[4] = M[1][1]; cal.mag_softiron[5] = M[1][2];
    cal.mag_softiron[6] = M[2][0]; cal.mag_softiron[7] = M[2][1]; cal.mag_softiron[8] = M[2][2];

    return cal;
  }

  // Alternative final calibration with only diagonal soft-iron (basic, diagonal scaling only)
  inline CalibrationData computeCalibrationBasic() {
    CalibrationData cal;

    if(sampleCount == 0) return cal; // Nothing to compute

    // Step 1: find min/max per axis
    float minX = xSamples[0], maxX = xSamples[0];
    float minY = ySamples[0], maxY = ySamples[0];
    float minZ = zSamples[0], maxZ = zSamples[0];

    for(int i = 1; i < sampleCount; i++){
      if (xSamples[i] < minX) minX = xSamples[i];
      if (xSamples[i] > maxX) maxX = xSamples[i];
      if (ySamples[i] < minY) minY = ySamples[i];
      if (ySamples[i] > maxY) maxY = ySamples[i];
      if (zSamples[i] < minZ) minZ = zSamples[i];
      if (zSamples[i] > maxZ) maxZ = zSamples[i];
    }

    // Step 2: hard-iron offsets
    cal.mag_hardiron[0] = (maxX+minX) / 2.0f;
    cal.mag_hardiron[1] = (maxY+minY) / 2.0f;
    cal.mag_hardiron[2] = (maxZ+minZ) / 2.0f;

    // Step 3: soft-iron diagonal scaling
    float avgRadius = ((maxX-minX) + (maxY-minY) + (maxZ-minZ)) / 6.0f;
    float scaleX = avgRadius / ((maxX-minX) / 2.0f);
    float scaleY = avgRadius / ((maxY-minY) / 2.0f);
    float scaleZ = avgRadius / ((maxZ-minZ) / 2.0f);

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
    for(int i = 0; i < sampleCount; i++){
      float mx = (xSamples[i] - cal.mag_hardiron[0]) * scaleX;
      float my = (ySamples[i] - cal.mag_hardiron[1]) * scaleY;
      float mz = (zSamples[i] - cal.mag_hardiron[2]) * scaleZ;
      sumB += sqrt(mx * mx + my * my + mz * mz);
    }
    cal.mag_field = sumB / sampleCount;

    return cal;
  }
}
