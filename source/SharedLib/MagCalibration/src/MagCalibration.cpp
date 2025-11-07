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

// Standard library includes for math and string functions
#include <math.h>
#include <string.h>
#include <stdio.h>

// Library Header
#include <MagCalibration.h>

// Define M_PI if not defined
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

/**
 * MagCalibration: Encapsulates calibration data, sampling, and computation.
 *
 * Magnetometer calibration library which should be utilized AFTER the Neutrona Wand PCB has
 * been fully assembled into its final state. This includes any speakers (magnetic sources)
 * so that the calibration process takes these into account in the calculated matrix.
 *
 * Workflow:
 *  1. beginCalibration() - clears buffers and coverage.
 *  2. addSample(x,y,z) - adds a sample if it expands coverage.
 *  3. getCoveragePercent() - percentage of the sphere covered.
 *  4. getVisPoints() - usable points for visualization (via Three.js).
 *  5. computeCalibration() - final CalibrationData for use in sensor reads.
 */

// Constructor: Initializes member variables and clears buffers.
MagCalibration::MagCalibration() {
  beginCalibration();
}

void MagCalibration::resetProvisionalHardIron() {
  provisionalHardIron.offsets.x = 0.0f;
  provisionalHardIron.offsets.y = 0.0f;
  provisionalHardIron.offsets.z = 0.0f;
  provisionalHardIron.range.x = 0.0f;
  provisionalHardIron.range.y = 0.0f;
  provisionalHardIron.range.z = 0.0f;
  provisionalHardIron.sufficientSpread = false;
  provisionalHardIron.offsetsApplied = false;
}

// Private helper: Clears only sample arrays, counters, and bin tracking
void MagCalibration::resetSamples() {
  sampleCount = 0; // Reset count of stored samples

  memset(bins, 0, sizeof(bins)); // Clear coverage tracking (bool array to false)
  memset(xSamples, 0, sizeof(xSamples)); // Clear X samples (double array to 0.0)
  memset(ySamples, 0, sizeof(ySamples)); // Clear Y samples (double array to 0.0)
  memset(zSamples, 0, sizeof(zSamples)); // Clear Z samples (double array to 0.0)

  // Clear bin distribution tracking arrays
  // Purpose: Reset all bin usage counters for a fresh calibration session
  memset(elevationBinCounts, 0, sizeof(elevationBinCounts)); // Clear elevation bin counters
  memset(azimuthBinCounts, 0, sizeof(azimuthBinCounts));     // Clear azimuth bin counters
}

// Clear any existing status message.
void MagCalibration::clearStatusMessage() {
  statusMessage[0] = '\0'; // Clear status message
}

// Begin a new calibration session by clearing all counters, arrays, and flags.
void MagCalibration::beginCalibration() {
  resetProvisionalHardIron();
  resetSamples();
  clearStatusMessage();
}

/**
 * Public Functions
 */

/**
 * Function: addSample
 * Purpose: Add a raw magnetometer sample, only stores if it expands coverage
 * Inputs: float x, float y, float z - raw magnetometer readings in µT (micro-Tesla)
 * Outputs: bool - true if sample was added, false if ignored (duplicate bin or max samples reached).
 *
 * HOW IT WORKS:
 * This function takes a magnetometer reading and decides if it's worth keeping.
 * The goal is to collect samples from many different device orientations to build
 * a complete picture of the magnetic field around the device.
 *
 * WORKFLOW:
 * 1. Check if we have room for more samples
 * 2. Make sure the reading is valid (not zero)
 * 3. Figure out which direction the device is pointing
 * 4. See if we already have a sample from that direction
 * 5. If it's a new direction, keep the sample
 * 6. If we've seen that direction before, ignore it
 */
bool MagCalibration::addSample(float x, float y, float z) {
  // Check if we should calculate and apply hard-iron offset as the Phase 1 of calibration.
  // This is done only once when we have enough samples w/ good spread to make an estimate.
  if(!provisionalHardIron.offsetsApplied) {
    if(sampleCount < HARD_IRON_SAMPLE_THRESHOLD) {
      // Not enough samples to attempt a calculation yet, so instruct the user on what to do.
      snprintf(statusMessage, sizeof(statusMessage), "Move the device in full circular motions in multiple directions. Collected %u of %u samples.", sampleCount, HARD_IRON_SAMPLE_THRESHOLD);
    } else {
      // We have enough samples to attempt a hard-iron offset calculation.
      provisionalHardIron = calculateHardIronOffsets(); // Run the calculation for hard-iron offsets.

      if(provisionalHardIron.sufficientSpread) {
        provisionalHardIron.offsetsApplied = true; // Mark that the hard-iron offset can now be applied.
        resetSamples(); // Clear all previous samples and coverage to begin again with adjusted values.

        // Inform the user that Phase 1 is complete and we're moving to Phase 2 which needs more samples.
        snprintf(statusMessage, sizeof(statusMessage), "Phase 1 calibration complete, collecting for phase 2...");
      } else {
        // Indicates that we performed a calculation but don't have enough spread yet, so keep going.
        snprintf(statusMessage, sizeof(statusMessage), "Preparing for phase 1 calibration, continue moving...");
      }
    }
  }

  // If offset is to be applied, adjust the incoming sample.
  if(provisionalHardIron.offsetsApplied) {
    x -= provisionalHardIron.offsets.x;
    y -= provisionalHardIron.offsets.y;
    z -= provisionalHardIron.offsets.z;
  }

  // Always update lastRawSample with the latest sample values.
  lastRawSample.x = x;
  lastRawSample.y = y;
  lastRawSample.z = z;

  // STEP 1: Check storage capacity
  // We can only store a limited number of samples and if we're full, rejecting any new samples.
  if(sampleCount >= MAX_POINTS) {
    // This should be nigh impossible but just in case let's inform the user.
    snprintf(statusMessage, sizeof(statusMessage), "Maximum samples reached, please end the calibration process.");
    return false; // Max samples reached
  }

  // STEP 2: Validate the magnetometer reading
  // Calculate the strength of the magnetic field reading using double precision
  double dx = (double)x;
  double dy = (double)y;
  double dz = (double)z;
  double r = sqrt(dx * dx + dy * dy + dz * dz); // sqrt() automatically handles double precision

  // Reject samples with unexpected or invalid magnitude (varies by phase of calibration).
  if(provisionalHardIron.offsetsApplied) {
    // After Offsets: reject outliers and sensor errors
    if(r < 10.0 || r > 100.0 || isnan(r) || isinf(r)) {
      return false;
    }
  } else {
    // Before Offsets: only reject zero, NaN, or Inf values
    if(r == 0 || isnan(r) || isinf(r)) {
      return false;
    }
  }

  // Convert to unit vector - we only care about direction, not strength
  // This puts all readings on the same scale for comparison
  double nx = dx / r;
  double ny = dy / r;
  double nz = dz / r;

  // STEP 3: Convert to orientation angles
  // Think of this like getting compass heading and tilt angle
  // az = horizontal rotation (like compass: North, East, South, West)
  // el = vertical tilt (like pitch: up, level, down)
  double az = atan2(ny, nx); // Horizontal orientation (-180° to +180°)

  // Clamp nz to valid range for asin() to prevent NaN from floating-point precision errors
  if(nz > 1.0) {
    nz = 1.0; // Values > 1.0 would produce NaN
  }
  if(nz < -1.0) {
    nz = -1.0; // Values < -1.0 would produce NaN
  }
  double el = asin(nz); // Vertical tilt (-90° to +90°)

  // STEP 4: Figure out which "bin" this orientation belongs to
  // We divide the sphere into regions (using BIN_DEGREES) covering about N° in each direction
  // Note that this is why we use double precision for the math as it ensures we get accurate binning
  int azIndex = (int)((az + M_PI) / (2 * M_PI) * NUM_AZIMUTH_BINS); // Which horizontal slice
  int elIndex = (int)((el + M_PI / 2) / M_PI * NUM_ELEVATION_BINS); // Which vertical slice

  // STEP 5: Make sure we got valid bin numbers
  // Sometimes floating point math can give us numbers slightly outside our range
  if(azIndex < 0) {
    azIndex = 0;
  }
  if(azIndex >= NUM_AZIMUTH_BINS) {
    azIndex = NUM_AZIMUTH_BINS - 1;
  }
  if(elIndex < 0) {
    elIndex = 0;
  }
  if(elIndex >= NUM_ELEVATION_BINS) {
    elIndex = NUM_ELEVATION_BINS - 1;
  }

  // Convert 2D coordinates to a single bin number
  int binIndex = elIndex * NUM_AZIMUTH_BINS + azIndex;

  // STEP 6: Check if this orientation is new
  // We only want one sample per orientation region, otherwise we ignore future samples from that area.
  if(!bins[binIndex]) {
    // This is a new orientation! Mark it as covered.
    bins[binIndex] = true;

    // Store the raw magnetometer reading for calibration calculations.
    xSamples[sampleCount] = dx;
    ySamples[sampleCount] = dy;
    zSamples[sampleCount] = dz;
    sampleCount++;

    // Update bin distribution tracking for real-time monitoring and diagnostics.
    // These counters enable external applications to display coverage patterns.
    elevationBinCounts[elIndex]++; // Increment counter for this elevation region
    azimuthBinCounts[azIndex]++;   // Increment counter for this azimuth region

    return true; // SUCCESS: New orientation covered, sample stored
  }

  // STEP 7: Reject duplicate orientation
  // We already have a sample from this direction, so this one doesn't help
  // The user needs to move the device to a different orientation to make progress
  return false; // IGNORED: Already have sample from this orientation
}

/**
 * Function: getLastSample
 * Purpose: Return the most recent raw sample, regardless of whether it was stored.
 * Inputs: none
 * Outputs: MagData - struct containing x, y, z as floats.
 */
MagData MagCalibration::getLastSample() const {
  return lastRawSample;
}

/**
 * Function: getCoveragePercent
 * Purpose: Get coverage % (0..100) based on filled bins
 * Inputs: none
 * Outputs: float - coverage percentage.
 */
float MagCalibration::getCoveragePercent() const {
  int filled = 0;
  for(uint16_t i = 0; i < MAX_POINTS; i++) {
    if(bins[i]) {
      filled++;
    }
  }
  return (filled / (float)MAX_POINTS) * 100.0f;
}

/**
 * Function: getVisPoints
 * Purpose: Get usable points for visualization
 * Inputs: const float*& outX, const float*& outY, const float*& outZ
 * Outputs: int - number of points, and sets outX, outY, outZ to point to internal arrays.
 */
uint16_t MagCalibration::getVisPoints(const double*& outX, const double*& outY, const double*& outZ) const {
  outX = xSamples;
  outY = ySamples;
  outZ = zSamples;
  return sampleCount;
}

/**
 * Function: calculateHardIronOffsets
 * Purpose: Computes hard-iron offsets (center of min/max) from collected samples.
 * Inputs: none
 * Outputs: MagData - struct containing x, y, z offsets (µT).
 */
HardIronOffsets MagCalibration::calculateHardIronOffsets() {
  // Clear any existing values for the provisional hard-iron offsets.
  resetProvisionalHardIron();

  if(sampleCount < HARD_IRON_SAMPLE_THRESHOLD) {
    return provisionalHardIron; // Too few samples, return defaults.
  }

  // Calculate min/max for each axis using double precision
  double minX = xSamples[0], maxX = xSamples[0];
  double minY = ySamples[0], maxY = ySamples[0];
  double minZ = zSamples[0], maxZ = zSamples[0];

  for(uint16_t i = 1; i < sampleCount; ++i) {
    if(xSamples[i] < minX) minX = xSamples[i];
    if(xSamples[i] > maxX) maxX = xSamples[i];
    if(ySamples[i] < minY) minY = ySamples[i];
    if(ySamples[i] > maxY) maxY = ySamples[i];
    if(zSamples[i] < minZ) minZ = zSamples[i];
    if(zSamples[i] > maxZ) maxZ = zSamples[i];
  }

  // Calculate offsets as the center of min/max.
  provisionalHardIron.offsets.x = static_cast<float>((maxX + minX) / 2.0);
  provisionalHardIron.offsets.y = static_cast<float>((maxY + minY) / 2.0);
  provisionalHardIron.offsets.z = static_cast<float>((maxZ + minZ) / 2.0);

  // Calculate range for each axis.
  provisionalHardIron.range.x = static_cast<float>(maxX - minX);
  provisionalHardIron.range.y = static_cast<float>(maxY - minY);
  provisionalHardIron.range.z = static_cast<float>(maxZ - minZ);

  // Apply a threshold to indicate sufficient spread (e.g., 30 µT per axis)
  provisionalHardIron.sufficientSpread = (provisionalHardIron.range.x > HARD_IRON_SPREAD_THRESHOLD) &&
                                         (provisionalHardIron.range.y > HARD_IRON_SPREAD_THRESHOLD) &&
                                         (provisionalHardIron.range.z > HARD_IRON_SPREAD_THRESHOLD);

  return provisionalHardIron;
}

/**
 * Function: calculateDiagonalFallback
 * Purpose: Computes hard-iron offset and diagonal soft-iron matrix using min/max method.
 * Inputs: none (uses member arrays)
 * Outputs: CalibrationData - struct containing hard-iron offset, soft-iron matrix, and field strength
 *
 * This function centralizes the fallback logic for calibration, ensuring consistent results
 * whenever the full ellipsoid fit cannot be performed. It is used in cases of poor coverage,
 * failed matrix inversion, or invalid fit.
 */
CalibrationData MagCalibration::calculateDiagonalFallback() const {
  CalibrationData cal = {};

  if(sampleCount == 0) {
    // No samples, return defaults
    // Assign default hard-iron offsets (no samples case)
    cal.mag_hardiron[0] = 0.0f;
    cal.mag_hardiron[1] = 0.0f;
    cal.mag_hardiron[2] = 0.0f;

    // Assign default soft-iron matrix (identity for diagonal, zero for off-diagonal)
    cal.mag_softiron[0] = 1.0f; // X scale
    cal.mag_softiron[1] = 0.0f; // XY
    cal.mag_softiron[2] = 0.0f; // XZ
    cal.mag_softiron[3] = 0.0f; // YX
    cal.mag_softiron[4] = 1.0f; // Y scale
    cal.mag_softiron[5] = 0.0f; // YZ
    cal.mag_softiron[6] = 0.0f; // ZX
    cal.mag_softiron[7] = 0.0f; // ZY
    cal.mag_softiron[8] = 1.0f; // Z scale

    cal.mag_field = 50.0f;
    return cal;
  }

  // Find min/max for each axis
  double minX = xSamples[0], maxX = xSamples[0];
  double minY = ySamples[0], maxY = ySamples[0];
  double minZ = zSamples[0], maxZ = zSamples[0];
  for(uint16_t i = 1; i < sampleCount; i++) {
    if(xSamples[i] < minX) minX = xSamples[i];
    if(xSamples[i] > maxX) maxX = xSamples[i];
    if(ySamples[i] < minY) minY = ySamples[i];
    if(ySamples[i] > maxY) maxY = ySamples[i];
    if(zSamples[i] < minZ) minZ = zSamples[i];
    if(zSamples[i] > maxZ) maxZ = zSamples[i];
  }

  // Hard-iron offset: center of min/max
  cal.mag_hardiron[0] = (float)((maxX + minX) / 2.0);
  cal.mag_hardiron[1] = (float)((maxY + minY) / 2.0);
  cal.mag_hardiron[2] = (float)((maxZ + minZ) / 2.0);

  // Soft-iron scaling factors
  double rangeX = maxX - minX;
  double rangeY = maxY - minY;
  double rangeZ = maxZ - minZ;
  double avgRadius = (rangeX + rangeY + rangeZ) / 6.0;

  double scaleX = (rangeX == 0.0) ? 1.0 : avgRadius / (rangeX / 2.0);
  double scaleY = (rangeY == 0.0) ? 1.0 : avgRadius / (rangeY / 2.0);
  double scaleZ = (rangeZ == 0.0) ? 1.0 : avgRadius / (rangeZ / 2.0);

  // Diagonal soft-iron matrix
  cal.mag_softiron[0] = (float)scaleX;
  cal.mag_softiron[1] = 0.0f;
  cal.mag_softiron[2] = 0.0f;
  cal.mag_softiron[3] = 0.0f;
  cal.mag_softiron[4] = (float)scaleY;
  cal.mag_softiron[5] = 0.0f;
  cal.mag_softiron[6] = 0.0f;
  cal.mag_softiron[7] = 0.0f;
  cal.mag_softiron[8] = (float)scaleZ;

  // Mean field magnitude
  double sumB = 0.0;
  for(uint16_t i = 0; i < sampleCount; i++) {
    double mx = (xSamples[i] - cal.mag_hardiron[0]) * scaleX;
    double my = (ySamples[i] - cal.mag_hardiron[1]) * scaleY;
    double mz = (zSamples[i] - cal.mag_hardiron[2]) * scaleZ;
    sumB += sqrt(mx*mx + my*my + mz*mz);
  }
  cal.mag_field = (sampleCount > 0) ? (float)(sumB / sampleCount) : 50.0f;

  // Round all calibration fields to 3 decimal places for consistency
  for(int i = 0; i < 3; ++i) {
    cal.mag_hardiron[i] = roundFloat3(cal.mag_hardiron[i]);
  }
  for(int i = 0; i < 9; ++i) {
    cal.mag_softiron[i] = roundFloat3(cal.mag_softiron[i]);
  }
  cal.mag_field = roundFloat3(cal.mag_field);

  // Add initial hard-iron offset to the fallback offset for final calibration.
  // This ensures both the provisional offset (applied during sample collection)
  // and the fallback offset (from min/max method) are included.
  cal.mag_hardiron[0] = (float)((maxX + minX) / 2.0) + provisionalHardIron.offsets.x;
  cal.mag_hardiron[1] = (float)((maxY + minY) / 2.0) + provisionalHardIron.offsets.y;
  cal.mag_hardiron[2] = (float)((maxZ + minZ) / 2.0) + provisionalHardIron.offsets.z;

  return cal;
}

/**
 * Function: computeCalibration
 * Purpose: Full ellipsoid fit -> center & 3x3 soft-iron matrix
 * Inputs: none
 * Outputs: CalibrationData - the computed calibration data returned in the common struct.
 */
CalibrationData MagCalibration::computeCalibration() const {
  CalibrationData cal;

  // If we have less than 50% coverage, fall back to simple min/max method.
  if(sampleCount < (MAX_POINTS / 2)) {
    // Use fallback logic to create diagonal offsets
    return calculateDiagonalFallback();
  }

  // Build design matrix columns for solving for 9 coefficients (A,B,C,D,E,F,G,H,I) with J = -1.
  // For each sample i: [x^2, y^2, z^2, xy, xz, yz, x, y, z] * coeffs = 1
  // We'll solve normal equations (9x9).
  const int Ncols = 9;
  double ATA[Ncols*Ncols];
  double ATb[Ncols];

  // Initialize with double precision
  for(int i = 0; i < Ncols*Ncols; i++) ATA[i] = 0.0;
  for(int i = 0; i < Ncols; i++) ATb[i] = 0.0;

  // Enhanced precision accumulation
  for(uint16_t s = 0; s < sampleCount; ++s) {
    double x = xSamples[s], y = ySamples[s], z = zSamples[s];
    double row[Ncols];
    row[0] = x*x; row[1] = y*y; row[2] = z*z;
    row[3] = x*y; row[4] = x*z; row[5] = y*z;
    row[6] = x;   row[7] = y;   row[8] = z;

    // Double precision matrix operations
    for(int i = 0; i < Ncols; i++) {
      for(int j = 0; j < Ncols; j++) {
        ATA[i * Ncols + j] += row[i] * row[j];
      }
      ATb[i] += row[i] * 1.0;
    }
  }

  // Solve ATA * coeffs = ATb
  float coeffs[Ncols];
  bool ok = solveLinearSystem(Ncols, ATA, ATb, coeffs);
  if(!ok) {
    // Use fallback logic to create diagonal offsets
    return calculateDiagonalFallback();
  }

  // Map coeffs -> full quadratic form
  float A = coeffs[0];
  float B = coeffs[1];
  float C = coeffs[2];
  float D = coeffs[3];
  float E = coeffs[4];
  float F = coeffs[5];
  float G = coeffs[6];
  float H = coeffs[7];
  float I = coeffs[8];
  float J = -1.0f;

  // build Q matrix (symmetric)
  float Q[3][3];
  Q[0][0] = A;
  Q[0][1] = D * 0.5f;
  Q[0][2] = E * 0.5f;
  Q[1][0] = D * 0.5f;
  Q[1][1] = B;
  Q[1][2] = F * 0.5f;
  Q[2][0] = E * 0.5f;
  Q[2][1] = F * 0.5f;
  Q[2][2] = C;

  // compute center c = -0.5 * Q^{-1} * L where L = [G,H,I]
  float Qinv[3][3];
  bool invok = invert3x3(Q, Qinv);
  if(!invok) {
    // Use fallback logic to create diagonal offsets
    return calculateDiagonalFallback();
  }

  float Lvec[3] = { G, H, I };
  float cx = -0.5f * (Qinv[0][0]*Lvec[0] + Qinv[0][1]*Lvec[1] + Qinv[0][2]*Lvec[2]);
  float cy = -0.5f * (Qinv[1][0]*Lvec[0] + Qinv[1][1]*Lvec[1] + Qinv[1][2]*Lvec[2]);
  float cz = -0.5f * (Qinv[2][0]*Lvec[0] + Qinv[2][1]*Lvec[1] + Qinv[2][2]*Lvec[2]);

  // Add initial hard-iron offset to the computed offset for final calibration.
  // This ensures both the provisional offset (applied during sample collection)
  // and the refined offset (from ellipsoid fit or fallback) are included.
  cal.mag_hardiron[0] = cx + provisionalHardIron.offsets.x;
  cal.mag_hardiron[1] = cy + provisionalHardIron.offsets.y;
  cal.mag_hardiron[2] = cz + provisionalHardIron.offsets.z;

  // compute constant: c^T Q c + L^T c + J
  float cQc = cx*(Q[0][0]*cx + Q[0][1]*cy + Q[0][2]*cz)
            + cy*(Q[1][0]*cx + Q[1][1]*cy + Q[1][2]*cz)
            + cz*(Q[2][0]*cx + Q[2][1]*cy + Q[2][2]*cz);
  float Lc = G*cx + H*cy + I*cz;
  float constant = cQc + Lc + J;
  float R = -constant;
  if(R <= 0.0f) {
    // Use fallback logic to create diagonal offsets
    return calculateDiagonalFallback();
  }

  // Eigen-decompose Q to get V, lambda
  float Qcopy[3][3];
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      Qcopy[r][c] = Q[r][c];
    }
  }
  float V[3][3], lambda[3];
  jacobiEigen3(Qcopy, V, lambda);

  // Ensure positive eigenvalues (guard)
  for(int i = 0; i < 3; i++) {
    if(lambda[i] <= 1e-12f) {
      lambda[i] = 1e-12f;
    }
  }

  // Build M = V * diag(sqrt(lambda)/sqrt(R)) * V^T
  float diag[3];
  float invSqrtR = 1.0f / sqrtf(R);
  for(int i = 0; i < 3; i++) {
    diag[i] = sqrtf(lambda[i]) * invSqrtR;
  }

  float temp[3][3];
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      temp[r][c] = V[r][c] * diag[c];
    }
  }
  float M[3][3];
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      float s = 0.0f;
      for(int k = 0; k < 3; k++) {
        s += temp[r][k] * V[c][k]; // note V^T element is V[c][k]
      }
      M[r][c] = s;
    }
  }

  // Compute mean raw magnitude (centered) to scale M to sensor units (µT)
  double meanRaw = 0.0;
  for(uint16_t i = 0; i < sampleCount; i++) {
    double dx = xSamples[i] - cx;
    double dy = ySamples[i] - cy;
    double dz = zSamples[i] - cz;
    meanRaw += sqrt(dx*dx + dy*dy + dz*dz);
  }
  meanRaw = meanRaw / sampleCount;
  float scaleFactor = (meanRaw > 1e-6) ? (float)meanRaw : 1.0f;

  // final soft-iron matrix in sensor units (row-major)
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      float val = M[r][c] * scaleFactor;
      cal.mag_softiron[r*3 + c] = val;
    }
  }

  // mag_field record: mean corrected magnitude (approx)
  cal.mag_field = (float)meanRaw;

  // At the end of all calculations, round all calibration fields to 3 decimal places
  // This ensures consistent precision for storage and use
  for(int i = 0; i < 3; ++i) {
    cal.mag_hardiron[i] = roundFloat3(cal.mag_hardiron[i]);
  }
  for(int i = 0; i < 9; ++i) {
    cal.mag_softiron[i] = roundFloat3(cal.mag_softiron[i]);
  }
  cal.mag_field = roundFloat3(cal.mag_field);

  return cal;
}

/**
 * Private Functions
 */

// Helper: Common Jacobi eigen-decomposition (symmetric 3x3).
// Outputs V (columns are eigenvectors) and w (eigenvalues).
void MagCalibration::jacobiEigen3(float A[3][3], float V[3][3], float w[3]) const {
  // Initialize identity matrix V where diagonal elements = 1.0f, off-diagonal = 0.0f
  float a00 = A[0][0], a01 = A[0][1], a02 = A[0][2];
  float a11 = A[1][1], a12 = A[1][2], a22 = A[2][2];
  for(int r = 0; r < 3; r++) {
    for(int c = 0; c < 3; c++) {
      V[r][c] = (r == c) ? 1.0f : 0.0f;
    }
  }

  const int MAX_ITER = 60;
  for(int iter = 0; iter < MAX_ITER; ++iter) {
    float abs01 = fabsf(a01), abs02 = fabsf(a02), abs12 = fabsf(a12);
    if(abs01 < 1e-8f && abs02 < 1e-8f && abs12 < 1e-8f) {
      break;
    }
    int p = 0, q = 1; float maxv = abs01;
    if(abs02 > maxv) {
      maxv = abs02;
      p = 0;
      q = 2;
    }
    if(abs12 > maxv) {
      maxv = abs12;
      p = 1;
      q = 2;
    }

    float apq = (p==0 && q==1)?a01 : (p==0 && q==2)?a02 : a12;
    float app = (p==0)?a00 : (p==1)?a11 : a22;
    float aqq = (q==0)?a00 : (q==1)?a11 : a22;
    float phi = 0.5f * atan2f(2.0f*apq, aqq - app);
    float c = cosf(phi), s = sinf(phi);

    // update matrix entries according to p,q
    float b01=a01,b02=a02,b12=a12;
    if(p==0 && q==1) {
      a00 = c*c*app - 2.0f*s*c*apq + s*s*aqq;
      a11 = s*s*app + 2.0f*s*c*apq + c*c*aqq;
      a01 = 0.0f;
      a02 = c*b02 - s*b12;
      a12 = s*b02 + c*b12;
    }
    else if(p==0 && q==2) {
      a00 = c*c*app - 2.0f*s*c*apq + s*s*aqq;
      a22 = s*s*app + 2.0f*s*c*apq + c*c*aqq;
      a02 = 0.0f;
      a01 = c*b01 - s*b12;
      a12 = s*b01 + c*b12;
    }
    else {
      a11 = c*c*app - 2.0f*s*c*apq + s*s*aqq;
      a22 = s*s*app + 2.0f*s*c*apq + c*c*aqq;
      a12 = 0.0f;
      a01 = c*b01 - s*b02;
      a02 = s*b01 + c*b02;
    }

    // update V columns p,q
    for(int r=0;r<3;r++) {
      float vip = V[r][p], viq = V[r][q];
      V[r][p] = c*vip - s*viq;
      V[r][q] = s*vip + c*viq;
    }
  }

  w[0] = a00; w[1] = a11; w[2] = a22;
  // sort descending
  for(int i = 0; i < 2; i++) {
    int idx = i;
    for(int j = i + 1; j < 3; j++) if(w[j] > w[idx]) idx = j;
    if(idx != i) {
      float tw = w[i]; w[i] = w[idx]; w[idx] = tw;
      for(int r = 0; r < 3; r++) {
        float tv = V[r][i];
        V[r][i] = V[r][idx];
        V[r][idx] = tv;
      }
    }
  }
}

// Helper: Solve small linear system Ax = b using Gaussian elimination with partial pivoting.
// n must be <= 10; uses in-place arrays A (n x n), b (n). Result in x[n].
// Returns true on success.
bool MagCalibration::solveLinearSystem(int n, double A[], double b[], float x[]) const {
  // Validate input bounds to prevent stack overflow
  if(n > 10 || n <= 0) {
    return false; // Invalid system size
  }

  // Build augmented matrix of size n x (n+1) in local stack (n<=10 so fine)
  // Use double precision for enhanced accuracy in matrix operations
  double aug[10][11];
  for(int i = 0; i < n; ++i) {
    for(int j = 0; j < n; ++j) aug[i][j] = A[i * n + j];
    aug[i][n] = b[i];
  }

  // Forward elimination with partial pivot using double precision
  for(int col = 0; col < n; ++col) {
    // Find pivot using double precision comparisons
    int piv = col;
    double maxv = fabs(aug[col][col]); // Use fabs() for double precision
    for(int r = col + 1; r < n; ++r) {
      double v = fabs(aug[r][col]); // Use fabs() for double precision
      if(v > maxv) {
        maxv = v;
        piv = r;
      }
    }
    if(maxv < 1e-12) { // Use double precision threshold
      return false; // singular
    }

    // Swap rows if needed
    if(piv != col) {
      for(int c = col; c <= n; ++c) {
        double tmp = aug[col][c];
        aug[col][c] = aug[piv][c];
        aug[piv][c] = tmp;
      }
    }

    // Normalize & eliminate using double precision
    double pivot = aug[col][col];
    for(int c = col; c <= n; ++c) aug[col][c] /= pivot;
    for(int r = 0; r < n; ++r) {
      if(r == col) {
        continue;
      }
      double fac = aug[r][col];
      if(fac == 0.0) { // Use double precision constant
        continue;
      }
      for(int c = col; c <= n; ++c) aug[r][c] -= fac * aug[col][c];
    }
  }

  // Extract solution and convert to float for interface compatibility
  for(int i = 0; i < n; ++i) x[i] = (float)aug[i][n];
  return true;
}

// Helper: 3x3 inverse (returns false if singular). A_in and A_out are row-major 3x3 floats.
bool MagCalibration::invert3x3(const float A_in[3][3], float A_out[3][3]) const {
  float a = A_in[0][0], b = A_in[0][1], c = A_in[0][2];
  float d = A_in[1][0], e = A_in[1][1], f = A_in[1][2];
  float g = A_in[2][0], h = A_in[2][1], i = A_in[2][2];
  float det = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
  if(fabsf(det) < 1e-12f) return false;
  float invdet = 1.0f / det;
  A_out[0][0] =  (e*i - f*h) * invdet;
  A_out[0][1] = -(b*i - c*h) * invdet;
  A_out[0][2] =  (b*f - c*e) * invdet;
  A_out[1][0] = -(d*i - f*g) * invdet;
  A_out[1][1] =  (a*i - c*g) * invdet;
  A_out[1][2] = -(a*f - c*d) * invdet;
  A_out[2][0] =  (d*h - e*g) * invdet;
  A_out[2][1] = -(a*h - b*g) * invdet;
  A_out[2][2] =  (a*e - b*d) * invdet;
  return true;
}

// Helper: Round float to 3 decimal places
float MagCalibration::roundFloat3(float val) const {
  return roundf(val * 1000.0f) / 1000.0f;
}

/**
 * Function: getElevationBinDistribution
 * Purpose: Get elevation bin distribution for coverage analysis and diagnostics
 * Inputs: const uint16_t*& outElevationCounts - reference to pointer for output array
 * Outputs: uint8_t - number of elevation bins, sets outElevationCounts to internal array
 *
 * This function provides access to the elevation coverage distribution, showing how many
 * samples have been collected in each vertical orientation range. This data is useful for:
 * - Real-time calibration progress monitoring
 * - Identifying gaps in vertical coverage
 * - Diagnostic analysis of movement patterns
 * - Visual feedback during calibration process
 */
uint8_t MagCalibration::getElevationBinDistribution(const uint16_t*& outElevationCounts) const {
  outElevationCounts = elevationBinCounts;
  return NUM_ELEVATION_BINS;
}

/**
 * Function: getAzimuthBinDistribution
 * Purpose: Get azimuth bin distribution for coverage analysis and diagnostics
 * Inputs: const uint16_t*& outAzimuthCounts - reference to pointer for output array
 * Outputs: uint8_t - number of azimuth bins, sets outAzimuthCounts to internal array
 *
 * This function provides access to the azimuth coverage distribution, showing how many
 * samples have been collected in each horizontal orientation range. This data is useful for:
 * - Real-time calibration progress monitoring
 * - Identifying gaps in horizontal coverage
 * - Diagnostic analysis of rotation patterns
 * - Visual feedback during calibration process
 */
uint8_t MagCalibration::getAzimuthBinDistribution(const uint16_t*& outAzimuthCounts) const {
  outAzimuthCounts = azimuthBinCounts;
  return NUM_AZIMUTH_BINS;
}

/**
 * Function: getActiveBinCount
 * Purpose: Get total number of bins with at least one sample for quick coverage assessment
 * Inputs: none
 * Outputs: uint16_t - count of bins that contain samples
 *
 * This function provides a quick way to determine how many orientation regions have been
 * covered without needing to process the full bin arrays. Useful for:
 * - Quick coverage assessment
 * - Progress indicators
 * - Calibration completion determination
 * - Performance monitoring
 */
uint16_t MagCalibration::getActiveBinCount() const {
  uint16_t activeBins = 0;
  for(uint16_t i = 0; i < MAX_POINTS; i++) {
    if(bins[i]) {
      activeBins++;
    }
  }
  return activeBins;
}
