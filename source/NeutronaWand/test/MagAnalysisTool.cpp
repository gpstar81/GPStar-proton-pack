/**
 * Magnetometer Calibration Analysis Tool
 * Purpose: Standalone diagnostic tool to analyze prototype vs production magnetometer data
 * 
 * This tool replicates the exact addSample() logic from MagCalibration.cpp to identify
 * why production boards achieve only 10% coverage vs 60% on prototype boards.
 * 
 * Compilation:
 *   g++ -std=c++11 -O2 -o MagAnalysisTool MagAnalysisTool.cpp -lm
 * 
 * Usage:
 *   ./MagAnalysisTool prototype_last3.log production_last3.log
 * 
 * Copyright (C) 2023-2025 Michael Rajotte, Dustin Grau, Nomake Wan
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <iomanip>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

// Constants from MagCalibration.h - replicated for analysis
static constexpr uint8_t BIN_DEGREES = 9;
static constexpr uint8_t NUM_AZIMUTH_BINS = (uint8_t)(360 / BIN_DEGREES);
static constexpr uint8_t NUM_ELEVATION_BINS = (uint8_t)(180 / BIN_DEGREES);
static constexpr uint16_t MAX_POINTS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS;

// Hard-iron calibration thresholds
static constexpr uint16_t HARD_IRON_SAMPLE_THRESHOLD = 40; // Minimum samples before offset check
static constexpr double HARD_IRON_SPREAD_THRESHOLD = 40.0; // Minimum spread per axis (µT)

// Elevation Bias Compensation Configuration
// Purpose: Enable/disable elevation bias compensation for testing production board calibration
// Usage: Set to true to apply an elevation compensation, false to disable
// Note: This simulates the compensation that would be needed in MagCalibration.cpp
static constexpr bool ENABLE_ELEVATION_COMPENSATION = false; // SET TO true TO TEST COMPENSATION

// Elevation compensation constants
// Purpose: Define the compensation values based on analysis of prototype vs production data
// The degree offset was determined from the difference in average elevation readings
static constexpr double ELEVATION_BIAS_DEGREES = 44.0;  // Degrees to compensate
static constexpr double ELEVATION_BIAS_RADIANS = ELEVATION_BIAS_DEGREES * M_PI / 180.0; // Radians equivalent

// Function: applyElevationCompensation  
// Purpose: Apply elevation bias compensation to magnetometer readings for testing
// Inputs: rawElevation - Original elevation angle in radians from magnetometer calculation
// Outputs: Compensated elevation angle in radians
// 
// This function applies a universal elevation compensation to all datasets when enabled.
// The purpose is to test the compensation theory on both prototype and production data
// to validate whether this correction improves coverage for production boards and 
// degrades coverage for prototype boards (confirming the bias direction).
//
// The degree offset was determined from analysis showing production boards read systematically
// lower elevation angles compared to prototype boards for identical physical orientations.
double applyElevationCompensation(double rawElevation) {
  // Apply compensation universally when enabled, regardless of board type
  // Purpose: Test compensation effects on all datasets to validate theory
  if(ENABLE_ELEVATION_COMPENSATION) {
    // Add the bias compensation to shift elevation readings by some degrees
    // This tests whether the systematic bias correction improves production coverage
    double compensatedElevation = rawElevation + ELEVATION_BIAS_RADIANS;
    
    // Ensure the compensated elevation stays within valid range [-π/2, π/2]
    // This prevents mathematical errors in subsequent bin calculations
    // Clamp to valid elevation range to prevent out-of-bounds bin indices
    if(compensatedElevation > M_PI / 2.0) {
      compensatedElevation = M_PI / 2.0;  // Clamp to +90°
    }
    if(compensatedElevation < -M_PI / 2.0) {
      compensatedElevation = -M_PI / 2.0; // Clamp to -90°
    }
    
    return compensatedElevation;
  }
  
  // No compensation applied - return original elevation unchanged
  return rawElevation;
}

// Structure: MagData
// Purpose: Store a single magnetometer reading with analysis results
struct MagData {
  // Raw input data
  double x, y, z;                    // Original magnetometer readings
  uint16_t lineNumber;               // Line number in log file for tracking
  
  // Calculated values (replicated from addSample() logic)
  double magnitude;                  // Magnetic field strength
  double nx, ny, nz;                 // Normalized unit vector
  double azimuth, elevation;         // Spherical coordinates (radians)
  double azimuthDeg, elevationDeg;   // Spherical coordinates (degrees)
  int azIndex, elIndex;              // Bin indices
  int binIndex;                      // Final bin number
  
  // Analysis flags
  bool validSample;                  // Whether sample passed validation
  bool newBin;                       // Whether this created a new bin
  bool wouldBeAccepted;              // Whether addSample() would accept this
};

// Structure: DatasetAnalysis
// Purpose: Complete analysis results for a log file dataset
struct DatasetAnalysis {
  std::string label;                 // "PROTOTYPE" or "PRODUCTION"
  std::string filename;              // Source log file name
  
  // Sample statistics
  uint16_t totalLines;               // Total lines processed from file
  uint16_t validSamples;             // Samples that passed validation
  uint16_t acceptedSamples;          // Samples that would be stored by addSample()
  uint16_t uniqueBins;               // Number of unique bins filled
  float coveragePercent;             // Coverage percentage achieved
  
  // Mathematical statistics
  double avgMagnitude;               // Average magnetic field strength
  double minMagnitude, maxMagnitude; // Magnitude range
  double avgElevation;               // Average elevation angle
  double minElevation, maxElevation; // Elevation range
  double avgAzimuth;                 // Average azimuth angle
  double minAzimuth, maxAzimuth;     // Azimuth range
  
  // Bin tracking arrays
  uint16_t elevationBinCounts[NUM_ELEVATION_BINS]; // Samples per elevation bin
  uint16_t azimuthBinCounts[NUM_AZIMUTH_BINS];     // Samples per azimuth bin
  bool binCoverage[MAX_POINTS];                    // Which bins are filled
  
  // Complete sample data for detailed analysis
  std::vector<MagData> samples;    // All processed samples
};

// Function: processMagData
// Purpose: Replicate the exact addSample() logic for analysis
// Inputs: Raw magnetometer data (x, y, z), line number for tracking
// Outputs: Complete analysis of this sample including bin assignment
// 
// This function performs identical mathematical operations as MagCalibration::addSample()
// but provides comprehensive diagnostic information instead of just true/false.
MagData processMagData(double x, double y, double z, uint16_t lineNumber) {
  MagData sample = {};
  sample.x = x;
  sample.y = y;
  sample.z = z;
  sample.lineNumber = lineNumber;
  
  // STEP 1: Validate input (identical to addSample())
  double dx = x, dy = y, dz = z;
  double r = sqrt(dx * dx + dy * dy + dz * dz);
  
  sample.magnitude = r;
  sample.validSample = (r != 0.0);
  
  if(!sample.validSample) {
    // Invalid sample - would be rejected by addSample()
    sample.wouldBeAccepted = false;
    return sample;
  }
  
  // STEP 2: Normalize to unit vector (identical to addSample())
  sample.nx = dx / r;
  sample.ny = dy / r;
  sample.nz = dz / r;
  
  // STEP 3: Convert to spherical coordinates (identical to addSample())
  sample.azimuth = atan2(sample.ny, sample.nx);
  
  // Clamp nz for asin() (identical to addSample())
  double nz_clamped = sample.nz;
  if(nz_clamped > 1.0) nz_clamped = 1.0;
  if(nz_clamped < -1.0) nz_clamped = -1.0;
  sample.elevation = asin(nz_clamped);
  
  // STEP 3.5: Apply elevation compensation universally if enabled
  // Purpose: Test compensation effects on all datasets to validate theory
  // This applies the same compensation to both prototype and production data
  sample.elevation = applyElevationCompensation(sample.elevation);
  
  // Convert to degrees for easier interpretation
  sample.azimuthDeg = sample.azimuth * 180.0 / M_PI;
  sample.elevationDeg = sample.elevation * 180.0 / M_PI;
  
  // STEP 4: Calculate bin indices using compensated elevation
  sample.azIndex = (int)((sample.azimuth + M_PI) / (2 * M_PI) * NUM_AZIMUTH_BINS);
  sample.elIndex = (int)((sample.elevation + M_PI / 2) / M_PI * NUM_ELEVATION_BINS);
  
  // STEP 5: Apply bounds checking (identical to addSample())
  if(sample.azIndex < 0) sample.azIndex = 0;
  if(sample.azIndex >= NUM_AZIMUTH_BINS) sample.azIndex = NUM_AZIMUTH_BINS - 1;
  if(sample.elIndex < 0) sample.elIndex = 0;
  if(sample.elIndex >= NUM_ELEVATION_BINS) sample.elIndex = NUM_ELEVATION_BINS - 1;
  
  // STEP 6: Calculate final bin index using compensated values
  sample.binIndex = sample.elIndex * NUM_AZIMUTH_BINS + sample.azIndex;
  
  // Mark as valid for bin assignment
  sample.wouldBeAccepted = true; // Will be updated by caller based on bin coverage
  
  return sample;
}

// Function: loadLogFile
// Purpose: Load and parse a magnetometer log file
// Inputs: File path to log file
// Outputs: Vector of parsed magnetometer readings
// 
// Expects CSV format: x,y,z (one reading per line)
std::vector<std::array<double, 3>> loadLogFile(const std::string& filename) {
  std::vector<std::array<double, 3>> readings;
  std::ifstream file(filename);
  
  if(!file.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << std::endl;
    return readings;
  }
  
  std::string line;
  uint16_t lineNumber = 0;
  
  while(std::getline(file, line)) {
    lineNumber++;
    
    // Skip empty lines or lines starting with # (comments)
    if(line.empty() || line[0] == '#') {
      continue;
    }
    
    // Parse CSV format: x,y,z
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    while(std::getline(ss, token, ',')) {
      tokens.push_back(token);
    }
    
    if(tokens.size() == 3) {
      try {
        double x = std::stod(tokens[0]);
        double y = std::stod(tokens[1]);
        double z = std::stod(tokens[2]);
        readings.push_back({x, y, z});
      } catch(const std::exception& e) {
        std::cerr << "Warning: Failed to parse line " << lineNumber << ": " << line << std::endl;
      }
    } else {
      std::cerr << "Warning: Invalid format on line " << lineNumber << ": " << line << std::endl;
    }
  }
  
  file.close();
  std::cout << "Loaded " << readings.size() << " readings from " << filename << std::endl;
  return readings;
}

// Function: analyzeDataset
// Purpose: Perform complete analysis on a dataset, simulating addSample() behavior
// Inputs: Vector of magnetometer readings, label for output, filename for tracking
// Outputs: Complete analysis results structure
DatasetAnalysis analyzeDataset(const std::vector<std::array<double, 3>>& readings, 
                              const std::string& label, 
                              const std::string& filename) {
  DatasetAnalysis analysis = {};
  analysis.label = label;
  analysis.filename = filename;
  analysis.totalLines = readings.size();
  
  // Initialize tracking arrays
  memset(analysis.elevationBinCounts, 0, sizeof(analysis.elevationBinCounts));
  memset(analysis.azimuthBinCounts, 0, sizeof(analysis.azimuthBinCounts));
  memset(analysis.binCoverage, 0, sizeof(analysis.binCoverage));
  
  // Track bin coverage to simulate addSample() behavior exactly
  bool bins[MAX_POINTS] = {};
  
  // Initialize statistics
  analysis.minMagnitude = 1e6;
  analysis.maxMagnitude = 0.0;
  analysis.minElevation = 1e6;
  analysis.maxElevation = -1e6;
  analysis.minAzimuth = 1e6;
  analysis.maxAzimuth = -1e6;
  
  double sumMagnitude = 0.0;
  double sumElevation = 0.0;
  double sumAzimuth = 0.0;
  
  std::cout << "\nProcessing " << label << " dataset..." << std::endl;
  if(ENABLE_ELEVATION_COMPENSATION) {
    std::cout << "  *** UNIVERSAL ELEVATION COMPENSATION ENABLED ***" << std::endl;
  }
  
  // Process each reading
  for(uint16_t i = 0; i < readings.size(); i++) {
    // Add progress reporting for large files
    if(readings.size() > 1000 && i % 1000 == 0) {
      std::cout << "  Processed " << i << " / " << readings.size() << " samples..." << std::endl;
    }
    
    // Apply universal compensation (no board type parameter needed)
    MagData sample = processMagData(readings[i][0], readings[i][1], readings[i][2], i + 1);
    
    if(sample.validSample) {
      analysis.validSamples++;
      
      // Check if this would be a new bin (identical to addSample() logic)
      sample.newBin = !bins[sample.binIndex];
      sample.wouldBeAccepted = sample.newBin;
      
      if(sample.newBin) {
        // Mark bin as covered (identical to addSample() logic)
        bins[sample.binIndex] = true;
        analysis.binCoverage[sample.binIndex] = true;
        analysis.uniqueBins++;
        analysis.acceptedSamples++;
      }
      
      // Update bin counters
      analysis.elevationBinCounts[sample.elIndex]++;
      analysis.azimuthBinCounts[sample.azIndex]++;
      
      // Update statistics
      sumMagnitude += sample.magnitude;
      sumElevation += sample.elevationDeg;
      sumAzimuth += sample.azimuthDeg;
      
      // Track min/max values
      if(sample.magnitude < analysis.minMagnitude) analysis.minMagnitude = sample.magnitude;
      if(sample.magnitude > analysis.maxMagnitude) analysis.maxMagnitude = sample.magnitude;
      if(sample.elevationDeg < analysis.minElevation) analysis.minElevation = sample.elevationDeg;
      if(sample.elevationDeg > analysis.maxElevation) analysis.maxElevation = sample.elevationDeg;
      if(sample.azimuthDeg < analysis.minAzimuth) analysis.minAzimuth = sample.azimuthDeg;
      if(sample.azimuthDeg > analysis.maxAzimuth) analysis.maxAzimuth = sample.azimuthDeg;
      
      // Store sample for detailed analysis
      analysis.samples.push_back(sample);
    }
  }
  
  // Calculate averages
  if(analysis.validSamples > 0) {
    analysis.avgMagnitude = sumMagnitude / analysis.validSamples;
    analysis.avgElevation = sumElevation / analysis.validSamples;
    analysis.avgAzimuth = sumAzimuth / analysis.validSamples;
  }
  
  // Calculate coverage percentage
  analysis.coveragePercent = (analysis.uniqueBins / (float)MAX_POINTS) * 100.0f;
  
  std::cout << "Analysis complete: " << analysis.validSamples << " valid samples, " 
            << analysis.uniqueBins << " unique bins (" << std::fixed << std::setprecision(1) 
            << analysis.coveragePercent << "% coverage)" << std::endl;
  
  return analysis;
}

// Function: writeDetailedSampleBreakdown
// Purpose: Write sample-by-sample analysis to a file with a custom suffix
void writeDetailedSampleBreakdown(const DatasetAnalysis& analysis, const std::string& suffix) {
  std::string outputFilename = analysis.filename;
  size_t lastDot = outputFilename.find_last_of(".");
  if(lastDot != std::string::npos) {
    outputFilename = outputFilename.substr(0, lastDot);
  }
  outputFilename += suffix;

  std::ofstream outFile(outputFilename);
  if(!outFile.is_open()) {
    std::cerr << "Warning: Could not create output file " << outputFilename << std::endl;
    return;
  }

  // Write file header
  outFile << "Magnetometer Calibration Analysis - Detailed Sample Breakdown" << std::endl;
  outFile << "Source File: " << analysis.filename << std::endl;
  outFile << "Dataset Label: " << analysis.label << std::endl;
  outFile << "Generated: " << __DATE__ << " " << __TIME__ << std::endl;
  outFile << "Total Samples: " << analysis.samples.size() << std::endl;
  outFile << "Unique Bins Filled: " << analysis.uniqueBins << " / " << MAX_POINTS 
          << " (" << std::fixed << std::setprecision(2) << analysis.coveragePercent << "% coverage)" << std::endl;
  outFile << std::string(80, '=') << std::endl;

  // Write column headers
  outFile << "Line |     X     |     Y     |     Z     |  Mag  | Az°  | El°  |AzBin|ElBin| Bin |Added" << std::endl;
  outFile << "-----|-----------|-----------|-----------|-------|------|------|-----|-----|-----|-----" << std::endl;

  // Write all sample data
  for(size_t i = 0; i < analysis.samples.size(); i++) {
    const auto& sample = analysis.samples[i];
    outFile << std::setw(4) << sample.lineNumber << " |"
            << std::setw(10) << std::fixed << std::setprecision(2) << sample.x << " |"
            << std::setw(10) << sample.y << " |"
            << std::setw(10) << sample.z << " |"
            << std::setw(6) << std::setprecision(1) << sample.magnitude << " |"
            << std::setw(5) << sample.azimuthDeg << " |"
            << std::setw(5) << sample.elevationDeg << " |"
            << std::setw(4) << sample.azIndex << " |"
            << std::setw(4) << sample.elIndex << " |"
            << std::setw(4) << sample.binIndex << " |"
            << (sample.wouldBeAccepted ? " YES" : " NO ") << std::endl;
  }

  // Write acceptance summary at end of file
  outFile << std::string(80, '=') << std::endl;
  outFile << "SAMPLE ACCEPTANCE SUMMARY" << std::endl;

  uint16_t acceptedCount = 0;
  uint16_t rejectedCount = 0;

  for(const auto& sample : analysis.samples) {
    if(sample.wouldBeAccepted) {
      acceptedCount++;
    } else {
      rejectedCount++;
    }
  }

  outFile << "Total Samples: " << analysis.samples.size() << std::endl;
  outFile << "Accepted (NEW bins): " << acceptedCount << std::endl;
  outFile << "Rejected (duplicate bins): " << rejectedCount << std::endl;
  outFile << "Acceptance Rate: " << std::fixed << std::setprecision(1) 
          << (acceptedCount / (float)analysis.samples.size() * 100.0f) << "%" << std::endl;

  outFile.close();
  std::cout << "  Detailed breakdown written to: " << outputFilename << std::endl;
}

// Function: printDetailedAnalysis
// Purpose: Print comprehensive analysis results
void printDetailedAnalysis(const DatasetAnalysis& analysis) {
  std::cout << "\n=== DETAILED ANALYSIS: " << analysis.label << " ===" << std::endl;
  std::cout << "Source File: " << analysis.filename << std::endl;
  
  // Use "_analysis.txt" for normal, "_fallback.txt" for fallback
  std::string suffix = (analysis.label == "PRODUCTION_FALLBACK") ? "_fallback.txt" : "_analysis.txt";
  writeDetailedSampleBreakdown(analysis, suffix);
  
  // Overall statistics (keep on console)
  std::cout << "\nSample Statistics:" << std::endl;
  std::cout << "  Total Lines: " << analysis.totalLines << std::endl;
  std::cout << "  Valid Samples: " << analysis.validSamples << std::endl;
  std::cout << "  Accepted Samples: " << analysis.acceptedSamples << std::endl;
  std::cout << "  Unique Bins: " << analysis.uniqueBins << " / " << MAX_POINTS << std::endl;
  std::cout << "  Coverage: " << std::fixed << std::setprecision(2) << analysis.coveragePercent << "%" << std::endl;
  std::cout << "  Processing Efficiency: " << std::fixed << std::setprecision(1) 
            << (analysis.validSamples / (float)analysis.totalLines * 100.0f) << "% valid samples" << std::endl;
  
  // Magnitude analysis (keep on console)
  std::cout << "\nMagnitude Analysis:" << std::endl;
  std::cout << "  Average: " << std::fixed << std::setprecision(2) << analysis.avgMagnitude << " µT" << std::endl;
  std::cout << "  Range: " << analysis.minMagnitude << " to " << analysis.maxMagnitude << " µT" << std::endl;
  std::cout << "  Spread: " << (analysis.maxMagnitude - analysis.minMagnitude) << " µT" << std::endl;
  
  // Angular analysis (keep on console)
  std::cout << "\nAngular Analysis:" << std::endl;
  std::cout << "  Azimuth - Avg: " << std::fixed << std::setprecision(1) << analysis.avgAzimuth 
            << "°, Range: " << analysis.minAzimuth << "° to " << analysis.maxAzimuth << "°" << std::endl;
  std::cout << "  Elevation - Avg: " << analysis.avgElevation 
            << "°, Range: " << analysis.minElevation << "° to " << analysis.maxElevation << "°" << std::endl;
  
  // Elevation bin distribution (keep on console)
  std::cout << "\nElevation Bin Distribution:" << std::endl;
  for(int i = 0; i < NUM_ELEVATION_BINS; i++) {
    if(analysis.elevationBinCounts[i] > 0) {
      double binCenter = (i * BIN_DEGREES) - 90.0; // Convert bin to degrees
      std::cout << "  Bin " << std::setw(2) << i << " (" << std::setw(6) << std::fixed 
                << std::setprecision(1) << binCenter << "°): " << std::setw(4) 
                << analysis.elevationBinCounts[i] << " samples" << std::endl;
    }
  }
  
  // Active azimuth bins (keep on console)
  std::cout << "\nActive Azimuth Bins: ";
  int activeBins = 0;
  for(int i = 0; i < NUM_AZIMUTH_BINS; i++) {
    if(analysis.azimuthBinCounts[i] > 0) {
      std::cout << i << " ";
      activeBins++;
      if(activeBins % 15 == 0) std::cout << "\n                     "; // Line wrap
    }
  }
  std::cout << std::endl;
  
  // Calculate and show acceptance summary on console
  uint16_t acceptedCount = 0;
  for(const auto& sample : analysis.samples) {
    if(sample.wouldBeAccepted) acceptedCount++;
  }
  
  std::cout << "\nAcceptance Summary:" << std::endl;
  std::cout << "  Accepted: " << acceptedCount << " / " << analysis.samples.size() 
            << " (" << std::fixed << std::setprecision(1) 
            << (acceptedCount / (float)analysis.samples.size() * 100.0f) << "%)" << std::endl;
}

// Function: compareAnalysis
// Purpose: Print side-by-side comparison of two datasets
void compareAnalysis(const DatasetAnalysis& prototypeAnalysis, const DatasetAnalysis& productionAnalysis) {
  std::cout << "\n=== COMPARATIVE ANALYSIS ===" << std::endl;

  std::cout << std::left << std::setw(25) << "Metric" << " | " 
            << std::setw(15) << "Prototype" << " | " 
            << std::setw(15) << "Production" << " | " 
            << std::setw(12) << "Ratio (P/Pr)" << std::endl;
  std::cout << std::string(70, '-') << std::endl;

  // Sample statistics comparison
  std::cout << std::setw(25) << "Valid Samples" << " | "
            << std::setw(15) << prototypeAnalysis.validSamples << " | "
            << std::setw(15) << productionAnalysis.validSamples << " | "
            << std::setw(12) << std::fixed << std::setprecision(2) 
            << (float)productionAnalysis.validSamples / prototypeAnalysis.validSamples << std::endl;

  std::cout << std::setw(25) << "Unique Bins" << " | "
            << std::setw(15) << prototypeAnalysis.uniqueBins << " | "
            << std::setw(15) << productionAnalysis.uniqueBins << " | "
            << std::setw(12) << (float)productionAnalysis.uniqueBins / prototypeAnalysis.uniqueBins << std::endl;

  std::cout << std::setw(25) << "Coverage %" << " | "
            << std::setw(15) << std::setprecision(1) << prototypeAnalysis.coveragePercent << " | "
            << std::setw(15) << productionAnalysis.coveragePercent << " | "
            << std::setw(12) << std::setprecision(2) << productionAnalysis.coveragePercent / prototypeAnalysis.coveragePercent << std::endl;

  std::cout << std::setw(25) << "Avg Magnitude (µT)" << " | "
            << std::setw(15) << std::setprecision(1) << prototypeAnalysis.avgMagnitude << " | "
            << std::setw(15) << productionAnalysis.avgMagnitude << " | "
            << std::setw(12) << std::setprecision(2) << productionAnalysis.avgMagnitude / prototypeAnalysis.avgMagnitude << std::endl;

  std::cout << std::setw(25) << "Avg Elevation (°)" << " | "
            << std::setw(15) << prototypeAnalysis.avgElevation << " | "
            << std::setw(15) << productionAnalysis.avgElevation << " | "
            << std::setw(12) << productionAnalysis.avgElevation / prototypeAnalysis.avgElevation << std::endl;

  std::cout << std::setw(25) << "Avg Azimuth (°)" << " | "
            << std::setw(15) << prototypeAnalysis.avgAzimuth << " | "
            << std::setw(15) << productionAnalysis.avgAzimuth << " | "
            << std::setw(12) << productionAnalysis.avgAzimuth / prototypeAnalysis.avgAzimuth << std::endl;

  // Key findings analysis
  std::cout << "\n=== KEY FINDINGS ===" << std::endl;

  // Magnitude analysis
  double magRatio = productionAnalysis.avgMagnitude / prototypeAnalysis.avgMagnitude;
  std::cout << "1. Magnitude Difference: Production readings are " 
            << std::fixed << std::setprecision(1) << magRatio << "x stronger" << std::endl;
  if(magRatio > 1.5) {
    std::cout << "   -> SIGNIFICANT: Trace removal eliminated magnetic damping" << std::endl;
  }

  // Coverage analysis
  double coverageRatio = productionAnalysis.coveragePercent / prototypeAnalysis.coveragePercent;
  std::cout << "2. Coverage Difference: Production achieves " 
            << coverageRatio << "x the coverage" << std::endl;
  if(coverageRatio < 0.3) {
    std::cout << "   -> CRITICAL: Severe coverage reduction detected" << std::endl;
  }

  // Elevation analysis
  double elevationDiff = productionAnalysis.avgElevation - prototypeAnalysis.avgElevation;
  std::cout << "3. Elevation Shift: Production reads " 
            << elevationDiff << "° different elevation" << std::endl;
  if(fabs(elevationDiff) > 10.0) {
    std::cout << "   -> SIGNIFICANT: Large elevation bias detected" << std::endl;
  }

  // Bin overlap analysis
  int sharedBins = 0;
  for(int i = 0; i < MAX_POINTS; i++) {
    if(prototypeAnalysis.binCoverage[i] && productionAnalysis.binCoverage[i]) {
      sharedBins++;
    }
  }
  float overlapPercent = 0.0f;
  int totalUniqueBins = prototypeAnalysis.uniqueBins + productionAnalysis.uniqueBins - sharedBins;
  if(totalUniqueBins > 0) {
    overlapPercent = (sharedBins / (float)totalUniqueBins) * 100.0f;
  }
  std::cout << "4. Bin Overlap: " << std::fixed << std::setprecision(1) 
            << overlapPercent << "% of bins are shared between datasets" << std::endl;
  if(overlapPercent < 50.0f) {
    std::cout << "   -> CRITICAL: Low bin overlap suggests systematic bias" << std::endl;
  }
}

// Function: printConfigurationInfo
// Purpose: Display the current binning configuration settings being used for analysis
// Inputs: None (uses global constants)
// Outputs: Console output showing bin configuration details
//
// This function provides transparency about the analysis parameters, making it clear
// what bin size and total bin count is being used for the current analysis run.
void printConfigurationInfo() {
  std::cout << "\n=== ANALYSIS CONFIGURATION ===" << std::endl;
  std::cout << "Bin Size: " << (uint16_t)BIN_DEGREES << "° per bin" << std::endl;
  std::cout << "Grid Dimensions: " << (uint16_t)NUM_AZIMUTH_BINS << " azimuth bins × " 
            << (uint16_t)NUM_ELEVATION_BINS << " elevation bins" << std::endl;
  std::cout << "Total Bins Available: " << MAX_POINTS << " bins" << std::endl;
  std::cout << "Azimuth Range: 0° to 360° (coverage: " << (uint16_t)NUM_AZIMUTH_BINS << " bins)" << std::endl;
  std::cout << "Elevation Range: -90° to +90° (coverage: " << (uint16_t)NUM_ELEVATION_BINS << " bins)" << std::endl;
  std::cout << "Coverage Resolution: Each bin represents " << (uint16_t)BIN_DEGREES 
            << "° × " << (uint16_t)BIN_DEGREES << "° area" << std::endl;
  
  // Update compensation status display for universal application
  std::cout << "Elevation Compensation: " << (ENABLE_ELEVATION_COMPENSATION ? "ENABLED" : "DISABLED");
  if(ENABLE_ELEVATION_COMPENSATION) {
    std::cout << " (" << ELEVATION_BIAS_DEGREES << "° applied to ALL datasets)";
  }
  std::cout << std::endl;
}

// Function: writeCompleteAnalysisToFile
// Purpose: Write all analysis results to a comprehensive summary file
// Inputs: Analysis results for both datasets
// Outputs: Creates analysis.txt file with complete summary
//
// This function captures all the analysis output that would normally appear on console
// and writes it to a single comprehensive file for easy review and sharing.
void writeCompleteAnalysisToFile(const DatasetAnalysis& prototypeAnalysis, 
                                const DatasetAnalysis& productionAnalysis,
                                const std::string& prototypeDebug,
                                const std::string& productionDebug) {
  std::ofstream outFile("analysis.txt");
  if(!outFile.is_open()) {
    std::cerr << "Warning: Could not create analysis.txt output file" << std::endl;
    return;
  }
  
  // Write header and configuration
  outFile << "Magnetometer Calibration Analysis Tool" << std::endl;
  outFile << "Purpose: Analyze prototype vs production binning behavior" << std::endl;
  outFile << "=========================================" << std::endl;
  
  // Configuration section
  outFile << "\n=== ANALYSIS CONFIGURATION ===" << std::endl;
  outFile << "Bin Size: " << (uint16_t)BIN_DEGREES << "° per bin" << std::endl;
  outFile << "Grid Dimensions: " << (uint16_t)NUM_AZIMUTH_BINS << " azimuth bins × " 
          << (uint16_t)NUM_ELEVATION_BINS << " elevation bins" << std::endl;
  outFile << "Total Bins Available: " << MAX_POINTS << " bins" << std::endl;
  outFile << "Azimuth Range: 0° to 360° (coverage: " << (uint16_t)NUM_AZIMUTH_BINS << " bins)" << std::endl;
  outFile << "Elevation Range: -90° to +90° (coverage: " << (uint16_t)NUM_ELEVATION_BINS << " bins)" << std::endl;
  outFile << "Coverage Resolution: Each bin represents " << (uint16_t)BIN_DEGREES 
          << "° × " << (uint16_t)BIN_DEGREES << "° area" << std::endl;
  
  outFile << "Elevation Compensation: " << (ENABLE_ELEVATION_COMPENSATION ? "ENABLED" : "DISABLED");
  if(ENABLE_ELEVATION_COMPENSATION) {
    outFile << " (" << ELEVATION_BIAS_DEGREES << "° applied to ALL datasets)";
  }
  outFile << std::endl;
  
  // File loading summary
  outFile << "\nLoading log files..." << std::endl;
  outFile << "Loaded " << prototypeAnalysis.totalLines << " readings from " << prototypeAnalysis.filename << std::endl;
  outFile << "Loaded " << productionAnalysis.totalLines << " readings from " << productionAnalysis.filename << std::endl;
  
  // Processing results
  outFile << "\nProcessing " << prototypeAnalysis.label << " dataset..." << std::endl;
  if(ENABLE_ELEVATION_COMPENSATION) {
    outFile << "  *** UNIVERSAL ELEVATION COMPENSATION ENABLED ***" << std::endl;
  }
  outFile << "Analysis complete: " << prototypeAnalysis.validSamples << " valid samples, " 
          << prototypeAnalysis.uniqueBins << " unique bins (" << std::fixed << std::setprecision(1) 
          << prototypeAnalysis.coveragePercent << "% coverage)" << std::endl;
  
  outFile << "\nProcessing " << productionAnalysis.label << " dataset..." << std::endl;
  if(ENABLE_ELEVATION_COMPENSATION) {
    outFile << "  *** UNIVERSAL ELEVATION COMPENSATION ENABLED ***" << std::endl;
  }
  outFile << "Analysis complete: " << productionAnalysis.validSamples << " valid samples, " 
          << productionAnalysis.uniqueBins << " unique bins (" << std::fixed << std::setprecision(1) 
          << productionAnalysis.coveragePercent << "% coverage)" << std::endl;
  
  // Write detailed analysis for prototype
  outFile << "\n=== DETAILED ANALYSIS: " << prototypeAnalysis.label << " ===" << std::endl;
  outFile << "Source File: " << prototypeAnalysis.filename << std::endl;
  outFile << "  Detailed breakdown written to: " << prototypeAnalysis.filename.substr(0, prototypeAnalysis.filename.find_last_of(".")) << "_analysis.txt" << std::endl;
  
  // Prototype statistics
  outFile << "\nSample Statistics:" << std::endl;
  outFile << "  Total Lines: " << prototypeAnalysis.totalLines << std::endl;
  outFile << "  Valid Samples: " << prototypeAnalysis.validSamples << std::endl;
  outFile << "  Accepted Samples: " << prototypeAnalysis.acceptedSamples << std::endl;
  outFile << "  Unique Bins: " << prototypeAnalysis.uniqueBins << " / " << MAX_POINTS << std::endl;
  outFile << "  Coverage: " << std::fixed << std::setprecision(2) << prototypeAnalysis.coveragePercent << "%" << std::endl;
  outFile << "  Processing Efficiency: " << std::fixed << std::setprecision(1) 
          << (prototypeAnalysis.validSamples / (float)prototypeAnalysis.totalLines * 100.0f) << "% valid samples" << std::endl;
  
  outFile << "\nMagnitude Analysis:" << std::endl;
  outFile << "  Average: " << std::fixed << std::setprecision(2) << prototypeAnalysis.avgMagnitude << " µT" << std::endl;
  outFile << "  Range: " << prototypeAnalysis.minMagnitude << " to " << prototypeAnalysis.maxMagnitude << " µT" << std::endl;
  outFile << "  Spread: " << (prototypeAnalysis.maxMagnitude - prototypeAnalysis.minMagnitude) << " µT" << std::endl;
  
  outFile << "\nAngular Analysis:" << std::endl;
  outFile << "  Azimuth - Avg: " << std::fixed << std::setprecision(1) << prototypeAnalysis.avgAzimuth 
          << "°, Range: " << prototypeAnalysis.minAzimuth << "° to " << prototypeAnalysis.maxAzimuth << "°" << std::endl;
  outFile << "  Elevation - Avg: " << prototypeAnalysis.avgElevation 
          << "°, Range: " << prototypeAnalysis.minElevation << "° to " << prototypeAnalysis.maxElevation << "°" << std::endl;
  
  // Prototype elevation bin distribution
  outFile << "\nElevation Bin Distribution:" << std::endl;
  for(int i = 0; i < NUM_ELEVATION_BINS; i++) {
    if(prototypeAnalysis.elevationBinCounts[i] > 0) {
      double binCenter = (i * BIN_DEGREES) - 90.0;
      outFile << "  Bin " << std::setw(2) << i << " (" << std::setw(6) << std::fixed 
              << std::setprecision(1) << binCenter << "°): " << std::setw(4) 
              << prototypeAnalysis.elevationBinCounts[i] << " samples" << std::endl;
    }
  }
  
  // Prototype active azimuth bins
  outFile << "\nActive Azimuth Bins: ";
  int prototypeBins = 0;
  for(int i = 0; i < NUM_AZIMUTH_BINS; i++) {
    if(prototypeAnalysis.azimuthBinCounts[i] > 0) {
      outFile << i << " ";
      prototypeBins++;
      if(prototypeBins % 15 == 0) outFile << "\n                     ";
    }
  }
  outFile << std::endl;
  
  // Prototype acceptance summary
  uint16_t prototypeAccepted = 0;
  for(const auto& sample : prototypeAnalysis.samples) {
    if(sample.wouldBeAccepted) prototypeAccepted++;
  }
  outFile << "\nAcceptance Summary:" << std::endl;
  outFile << "  Accepted: " << prototypeAccepted << " / " << prototypeAnalysis.samples.size() 
          << " (" << std::fixed << std::setprecision(1) 
          << (prototypeAccepted / (float)prototypeAnalysis.samples.size() * 100.0f) << "%)" << std::endl;
  
  // Write detailed analysis for production
  outFile << "\n=== DETAILED ANALYSIS: " << productionAnalysis.label << " ===" << std::endl;
  outFile << "Source File: " << productionAnalysis.filename << std::endl;
  outFile << "  Detailed breakdown written to: " << productionAnalysis.filename.substr(0, productionAnalysis.filename.find_last_of(".")) << "_analysis.txt" << std::endl;
  
  // Production statistics
  outFile << "\nSample Statistics:" << std::endl;
  outFile << "  Total Lines: " << productionAnalysis.totalLines << std::endl;
  outFile << "  Valid Samples: " << productionAnalysis.validSamples << std::endl;
  outFile << "  Accepted Samples: " << productionAnalysis.acceptedSamples << std::endl;
  outFile << "  Unique Bins: " << productionAnalysis.uniqueBins << " / " << MAX_POINTS << std::endl;
  outFile << "  Coverage: " << std::fixed << std::setprecision(2) << productionAnalysis.coveragePercent << "%" << std::endl;
  outFile << "  Processing Efficiency: " << std::fixed << std::setprecision(1) 
          << (productionAnalysis.validSamples / (float)productionAnalysis.totalLines * 100.0f) << "% valid samples" << std::endl;
  
  outFile << "\nMagnitude Analysis:" << std::endl;
  outFile << "  Average: " << std::fixed << std::setprecision(2) << productionAnalysis.avgMagnitude << " µT" << std::endl;
  outFile << "  Range: " << productionAnalysis.minMagnitude << " to " << productionAnalysis.maxMagnitude << " µT" << std::endl;
  outFile << "  Spread: " << (productionAnalysis.maxMagnitude - productionAnalysis.minMagnitude) << " µT" << std::endl;
  
  outFile << "\nAngular Analysis:" << std::endl;
  outFile << "  Azimuth - Avg: " << std::fixed << std::setprecision(1) << productionAnalysis.avgAzimuth 
          << "°, Range: " << productionAnalysis.minAzimuth << "° to " << productionAnalysis.maxAzimuth << "°" << std::endl;
  outFile << "  Elevation - Avg: " << productionAnalysis.avgElevation 
          << "°, Range: " << productionAnalysis.minElevation << "° to " << productionAnalysis.maxElevation << "°" << std::endl;
  
  // Production elevation bin distribution
  outFile << "\nElevation Bin Distribution:" << std::endl;
  for(int i = 0; i < NUM_ELEVATION_BINS; i++) {
    if(productionAnalysis.elevationBinCounts[i] > 0) {
      double binCenter = (i * BIN_DEGREES) - 90.0;
      outFile << "  Bin " << std::setw(2) << i << " (" << std::setw(6) << std::fixed 
              << std::setprecision(1) << binCenter << "°): " << std::setw(4) 
              << productionAnalysis.elevationBinCounts[i] << " samples" << std::endl;
    }
  }
  
  // Production active azimuth bins
  outFile << "\nActive Azimuth Bins: ";
  int productionBins = 0;
  for(int i = 0; i < NUM_AZIMUTH_BINS; i++) {
    if(productionAnalysis.azimuthBinCounts[i] > 0) {
      outFile << i << " ";
      productionBins++;
      if(productionBins % 15 == 0) outFile << "\n                     ";
    }
  }
  outFile << std::endl;
  
  // Production acceptance summary
  uint16_t productionAccepted = 0;
  for(const auto& sample : productionAnalysis.samples) {
    if(sample.wouldBeAccepted) productionAccepted++;
  }
  outFile << "\nAcceptance Summary:" << std::endl;
  outFile << "  Accepted: " << productionAccepted << " / " << productionAnalysis.samples.size() 
          << " (" << std::fixed << std::setprecision(1) 
          << (productionAccepted / (float)productionAnalysis.samples.size() * 100.0f) << "%)" << std::endl;
  
  // Comparative analysis section
  outFile << "\n=== COMPARATIVE ANALYSIS ===" << std::endl;
  
  outFile << std::left << std::setw(25) << "Metric" << " | " 
          << std::setw(15) << "Prototype" << " | " 
          << std::setw(15) << "Production" << " | " 
          << std::setw(12) << "Ratio (P/Pr)" << std::endl;
  outFile << std::string(70, '-') << std::endl;
  
  // Sample statistics comparison
  outFile << std::setw(25) << "Valid Samples" << " | "
          << std::setw(15) << prototypeAnalysis.validSamples << " | "
          << std::setw(15) << productionAnalysis.validSamples << " | "
          << std::setw(12) << std::fixed << std::setprecision(2) 
          << (float)productionAnalysis.validSamples / prototypeAnalysis.validSamples << std::endl;

  outFile << std::setw(25) << "Unique Bins" << " | "
          << std::setw(15) << prototypeAnalysis.uniqueBins << " | "
          << std::setw(15) << productionAnalysis.uniqueBins << " | "
          << std::setw(12) << (float)productionAnalysis.uniqueBins / prototypeAnalysis.uniqueBins << std::endl;

  outFile << std::setw(25) << "Coverage %" << " | "
          << std::setw(15) << std::setprecision(1) << prototypeAnalysis.coveragePercent << " | "
          << std::setw(15) << productionAnalysis.coveragePercent << " | "
          << std::setw(12) << std::setprecision(2) << productionAnalysis.coveragePercent / prototypeAnalysis.coveragePercent << std::endl;

  outFile << std::setw(25) << "Avg Magnitude (µT)" << " | "
          << std::setw(15) << std::setprecision(1) << prototypeAnalysis.avgMagnitude << " | "
          << std::setw(15) << productionAnalysis.avgMagnitude << " | "
          << std::setw(12) << std::setprecision(2) << productionAnalysis.avgMagnitude / prototypeAnalysis.avgMagnitude << std::endl;

  outFile << std::setw(25) << "Avg Elevation (°)" << " | "
          << std::setw(15) << prototypeAnalysis.avgElevation << " | "
          << std::setw(15) << productionAnalysis.avgElevation << " | "
          << std::setw(12) << productionAnalysis.avgElevation / prototypeAnalysis.avgElevation << std::endl;

  outFile << std::setw(25) << "Avg Azimuth (°)" << " | "
          << std::setw(15) << prototypeAnalysis.avgAzimuth << " | "
          << std::setw(15) << productionAnalysis.avgAzimuth << " | "
          << std::setw(12) << productionAnalysis.avgAzimuth / prototypeAnalysis.avgAzimuth << std::endl;
  
  // Key findings analysis
  outFile << "\n=== KEY FINDINGS ===" << std::endl;
  
  // Magnitude analysis
  double magRatio = productionAnalysis.avgMagnitude / prototypeAnalysis.avgMagnitude;
  outFile << "1. Magnitude Difference: Production readings are " 
          << std::fixed << std::setprecision(1) << magRatio << "x stronger" << std::endl;
  if(magRatio > 1.5) {
    outFile << "   -> SIGNIFICANT: Trace removal eliminated magnetic damping" << std::endl;
  }
  
  // Coverage analysis
  double coverageRatio = productionAnalysis.coveragePercent / prototypeAnalysis.coveragePercent;
  outFile << "2. Coverage Difference: Production achieves " 
          << coverageRatio << "x the coverage" << std::endl;
  if(coverageRatio < 0.3) {
    outFile << "   -> CRITICAL: Severe coverage reduction detected" << std::endl;
  }
  
  // Elevation analysis
  double elevationDiff = productionAnalysis.avgElevation - prototypeAnalysis.avgElevation;
  outFile << "3. Elevation Shift: Production reads " 
          << elevationDiff << "° different elevation" << std::endl;
  if(fabs(elevationDiff) > 10.0) {
    outFile << "   -> SIGNIFICANT: Large elevation bias detected" << std::endl;
  }
  
  // Bin overlap analysis
  int sharedBins = 0;
  for(int i = 0; i < MAX_POINTS; i++) {
    if(prototypeAnalysis.binCoverage[i] && productionAnalysis.binCoverage[i]) {
      sharedBins++;
    }
  }
  float overlapPercent = 0.0f;
  int totalUniqueBins = prototypeAnalysis.uniqueBins + productionAnalysis.uniqueBins - sharedBins;
  if(totalUniqueBins > 0) {
    overlapPercent = (sharedBins / (float)totalUniqueBins) * 100.0f;
  }
  outFile << "4. Bin Overlap: " << std::fixed << std::setprecision(1) 
          << overlapPercent << "% of bins are shared between datasets" << std::endl;
  if(overlapPercent < 50.0f) {
    outFile << "   -> CRITICAL: Low bin overlap suggests systematic bias" << std::endl;
  }
  
  outFile.close();
  std::cout << "Complete analysis written to: analysis.txt" << std::endl;
}

// Structure: HardIronOffsets
struct HardIronOffsets {
  double x, y, z;
  double rangeX, rangeY, rangeZ;
  bool sufficientSpread;
};

// Function: calculateHardIronOffsets
HardIronOffsets calculateHardIronOffsets(const std::vector<MagData>& samples) {
  HardIronOffsets result = {};
  if(samples.empty()) return result;
  double minX = samples[0].x, maxX = samples[0].x;
  double minY = samples[0].y, maxY = samples[0].y;
  double minZ = samples[0].z, maxZ = samples[0].z;
  for(const auto& s : samples) {
    if(s.x < minX) minX = s.x;
    if(s.x > maxX) maxX = s.x;
    if(s.y < minY) minY = s.y;
    if(s.y > maxY) maxY = s.y;
    if(s.z < minZ) minZ = s.z;
    if(s.z > maxZ) maxZ = s.z;
  }
  result.x = (maxX + minX) / 2.0;
  result.y = (maxY + minY) / 2.0;
  result.z = (maxZ + minZ) / 2.0;
  result.rangeX = maxX - minX;
  result.rangeY = maxY - minY;
  result.rangeZ = maxZ - minZ;
  result.sufficientSpread = (result.rangeX > HARD_IRON_SPREAD_THRESHOLD) &&
                            (result.rangeY > HARD_IRON_SPREAD_THRESHOLD) &&
                            (result.rangeZ > HARD_IRON_SPREAD_THRESHOLD);
  return result;
}

// Update analyzeDatasetWithHardIron to collect debug output
struct AnalysisResult {
  DatasetAnalysis analysis;
  std::string debugLog;
};

AnalysisResult analyzeDatasetWithHardIron(const std::vector<std::array<double, 3>>& readings,
                                          const std::string& label,
                                          const std::string& filename) {
  DatasetAnalysis analysis = {};
  analysis.label = label;
  analysis.filename = filename;
  analysis.totalLines = readings.size();

  std::stringstream debug;
  debug << "\n[DEBUG] Calibration sample count: " << readings.size() << std::endl;

  // First pass: collect samples for offset calculation
  std::vector<MagData> calibrationSamples;
  for(uint16_t i = 0; i < readings.size(); i++) {
    MagData sample = processMagData(readings[i][0], readings[i][1], readings[i][2], i + 1);
    if(sample.validSample) {
      calibrationSamples.push_back(sample);
      if(calibrationSamples.size() == HARD_IRON_SAMPLE_THRESHOLD) break;
    }
  }

  // Calculate hard-iron offset
  HardIronOffsets hardIronOffset = calculateHardIronOffsets(calibrationSamples);
  bool hardIronOffsetApplied = hardIronOffset.sufficientSpread;

  debug << "[DEBUG] Calibration sample count: " << calibrationSamples.size() << std::endl;
  debug << "[DEBUG] Hard-iron spread X: " << hardIronOffset.rangeX << " (threshold: " << HARD_IRON_SPREAD_THRESHOLD << ")" << std::endl;
  debug << "[DEBUG] Hard-iron spread Y: " << hardIronOffset.rangeY << " (threshold: " << HARD_IRON_SPREAD_THRESHOLD << ")" << std::endl;
  debug << "[DEBUG] Hard-iron spread Z: " << hardIronOffset.rangeZ << " (threshold: " << HARD_IRON_SPREAD_THRESHOLD << ")" << std::endl;

  if(hardIronOffsetApplied) {
    debug << "[DEBUG] Hard-iron offset applied after " << HARD_IRON_SAMPLE_THRESHOLD << " samples: "
          << "X=" << hardIronOffset.x << " Y=" << hardIronOffset.y << " Z=" << hardIronOffset.z << std::endl;
    debug << "[DEBUG] Resetting sample counters and bin coverage..." << std::endl;
    debug << "[DEBUG] Re-processing input file with hard-iron offset applied to all samples..." << std::endl;
  } else {
    debug << "[DEBUG] Hard-iron offset NOT applied. Reason: ";
    if(calibrationSamples.size() < HARD_IRON_SAMPLE_THRESHOLD) {
      debug << "Insufficient valid samples (" << calibrationSamples.size() << " < " << HARD_IRON_SAMPLE_THRESHOLD << ")." << std::endl;
    } else {
      debug << "Spread insufficient (X: " << hardIronOffset.rangeX << ", Y: " << hardIronOffset.rangeY << ", Z: " << hardIronOffset.rangeZ << ")." << std::endl;
    }
  }

  // After calculating hard-iron offset
  if(hardIronOffsetApplied) {
    std::cout << "Hard-iron offset applied after " << HARD_IRON_SAMPLE_THRESHOLD << " samples: "
              << "X=" << hardIronOffset.x << " Y=" << hardIronOffset.y << " Z=" << hardIronOffset.z << std::endl;
    std::cout << "Resetting sample counters and bin coverage..." << std::endl;
    std::cout << "Re-processing input file with hard-iron offset applied to all samples..." << std::endl;
  }

  // Clear counters and bins
  memset(analysis.elevationBinCounts, 0, sizeof(analysis.elevationBinCounts));
  memset(analysis.azimuthBinCounts, 0, sizeof(analysis.azimuthBinCounts));
  memset(analysis.binCoverage, 0, sizeof(analysis.binCoverage));
  bool bins[MAX_POINTS] = {};
  analysis.uniqueBins = 0;
  analysis.acceptedSamples = 0;
  analysis.validSamples = 0;
  analysis.samples.clear();

  // Second pass: process all samples with offset applied if needed
  double sumMagnitude = 0.0, sumElevation = 0.0, sumAzimuth = 0.0;
  analysis.minMagnitude = 1e6; analysis.maxMagnitude = 0.0;
  analysis.minElevation = 1e6; analysis.maxElevation = -1e6;
  analysis.minAzimuth = 1e6; analysis.maxAzimuth = -1e6;

  for(uint16_t i = 0; i < readings.size(); i++) {
    double x = readings[i][0];
    double y = readings[i][1];
    double z = readings[i][2];
    if(hardIronOffsetApplied) {
      x -= hardIronOffset.x;
      y -= hardIronOffset.y;
      z -= hardIronOffset.z;
    }
    MagData sample = processMagData(x, y, z, i + 1);
    if(sample.validSample) {
      analysis.validSamples++;
      sample.newBin = !bins[sample.binIndex];
      sample.wouldBeAccepted = sample.newBin;
      if(sample.newBin) {
        bins[sample.binIndex] = true;
        analysis.binCoverage[sample.binIndex] = true;
        analysis.uniqueBins++;
        analysis.acceptedSamples++;
      }
      analysis.elevationBinCounts[sample.elIndex]++;
      analysis.azimuthBinCounts[sample.azIndex]++;
      sumMagnitude += sample.magnitude;
      sumElevation += sample.elevationDeg;
      sumAzimuth += sample.azimuthDeg;
      if(sample.magnitude < analysis.minMagnitude) analysis.minMagnitude = sample.magnitude;
      if(sample.magnitude > analysis.maxMagnitude) analysis.maxMagnitude = sample.magnitude;
      if(sample.elevationDeg < analysis.minElevation) analysis.minElevation = sample.elevationDeg;
      if(sample.elevationDeg > analysis.maxElevation) analysis.maxElevation = sample.elevationDeg;
      if(sample.azimuthDeg < analysis.minAzimuth) analysis.minAzimuth = sample.azimuthDeg;
      if(sample.azimuthDeg > analysis.maxAzimuth) analysis.maxAzimuth = sample.azimuthDeg;
      analysis.samples.push_back(sample);
    }
  }

  // Calculate averages
  if(analysis.validSamples > 0) {
    analysis.avgMagnitude = sumMagnitude / analysis.validSamples;
    analysis.avgElevation = sumElevation / analysis.validSamples;
    analysis.avgAzimuth = sumAzimuth / analysis.validSamples;
  }
  analysis.coveragePercent = (analysis.uniqueBins / (float)MAX_POINTS) * 100.0f;

  AnalysisResult result;
  result.analysis = analysis;
  result.debugLog = debug.str();
  return result;
}

// Modify the main function around line 750:
// Add call to write complete analysis after comparative analysis
int main(int argc, char* argv[]) {
  std::cout << "Magnetometer Calibration Analysis Tool" << std::endl;
  std::cout << "Purpose: Analyze prototype vs production binning behavior" << std::endl;
  std::cout << "=========================================" << std::endl;

  printConfigurationInfo();

  if(argc != 3) {
    std::cout << "\nUsage: " << argv[0] << " <prototype_log> <production_log>" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << argv[0] << " prototype_last3.log production_last3.log" << std::endl;
    return 1;
  }

  std::string prototypeFile = argv[1];
  std::string productionFile = argv[2];

  auto prototypeReadings = loadLogFile(prototypeFile);
  auto productionReadings = loadLogFile(productionFile);

  if(prototypeReadings.empty() || productionReadings.empty()) {
    std::cerr << "Error: Failed to load log files or files are empty" << std::endl;
    return 1;
  }

  // Use new analysis function
  AnalysisResult prototypeResult = analyzeDatasetWithHardIron(prototypeReadings, "PROTOTYPE", prototypeFile);
  AnalysisResult productionResult = analyzeDatasetWithHardIron(productionReadings, "PRODUCTION", productionFile);

  printDetailedAnalysis(prototypeResult.analysis);
  printDetailedAnalysis(productionResult.analysis);
  compareAnalysis(prototypeResult.analysis, productionResult.analysis);
  writeCompleteAnalysisToFile(prototypeResult.analysis, productionResult.analysis,
                             prototypeResult.debugLog, productionResult.debugLog);

  // Fallback: If hard-iron offset was NOT applied, use all valid samples for offset and re-run
  if(productionResult.debugLog.find("Hard-iron offset applied") == std::string::npos) {
    std::cout << "\n[INFO] Fallback: Calculating hard-iron offsets using all available production samples..." << std::endl;

    // Collect all valid samples from the production file
    std::vector<MagData> allValidSamples;
    for(uint16_t i = 0; i < productionReadings.size(); i++) {
      MagData sample = processMagData(productionReadings[i][0], productionReadings[i][1], productionReadings[i][2], i + 1);
      if(sample.validSample) {
        allValidSamples.push_back(sample);
      }
    }

    // Calculate offsets using all valid samples
    HardIronOffsets fallbackOffset = calculateHardIronOffsets(allValidSamples);

    std::stringstream fallbackDebug;
    fallbackDebug << "\n[FALLBACK DEBUG] Used all valid samples (" << allValidSamples.size() << ") for hard-iron offset calculation.\n";
    fallbackDebug << "[FALLBACK DEBUG] Spread X: " << fallbackOffset.rangeX << " (threshold: " << HARD_IRON_SPREAD_THRESHOLD << ")\n";
    fallbackDebug << "[FALLBACK DEBUG] Spread Y: " << fallbackOffset.rangeY << " (threshold: " << HARD_IRON_SPREAD_THRESHOLD << ")\n";
    fallbackDebug << "[FALLBACK DEBUG] Spread Z: " << fallbackOffset.rangeZ << " (threshold: " << HARD_IRON_SPREAD_THRESHOLD << ")\n";
    fallbackDebug << "[FALLBACK DEBUG] Hard-iron offset applied: X=" << fallbackOffset.x << " Y=" << fallbackOffset.y << " Z=" << fallbackOffset.z << "\n";
    fallbackDebug << "[FALLBACK DEBUG] Re-processing production file with fallback offset...\n";

    // Re-run analysis with fallback offset applied
    DatasetAnalysis fallbackAnalysis = {};
    fallbackAnalysis.label = "PRODUCTION_FALLBACK";
    fallbackAnalysis.filename = productionFile;
    fallbackAnalysis.totalLines = productionReadings.size();

    memset(fallbackAnalysis.elevationBinCounts, 0, sizeof(fallbackAnalysis.elevationBinCounts));
    memset(fallbackAnalysis.azimuthBinCounts, 0, sizeof(fallbackAnalysis.azimuthBinCounts));
    memset(fallbackAnalysis.binCoverage, 0, sizeof(fallbackAnalysis.binCoverage));
    bool bins[MAX_POINTS] = {};
    fallbackAnalysis.uniqueBins = 0;
    fallbackAnalysis.acceptedSamples = 0;
    fallbackAnalysis.validSamples = 0;
    fallbackAnalysis.samples.clear();

    double sumMagnitude = 0.0, sumElevation = 0.0, sumAzimuth = 0.0;
    fallbackAnalysis.minMagnitude = 1e6; fallbackAnalysis.maxMagnitude = 0.0;
    fallbackAnalysis.minElevation = 1e6; fallbackAnalysis.maxElevation = -1e6;
    fallbackAnalysis.minAzimuth = 1e6; fallbackAnalysis.maxAzimuth = -1e6;

    for(uint16_t i = 0; i < productionReadings.size(); i++) {
      double x = productionReadings[i][0] - fallbackOffset.x;
      double y = productionReadings[i][1] - fallbackOffset.y;
      double z = productionReadings[i][2] - fallbackOffset.z;
      MagData sample = processMagData(x, y, z, i + 1);
      if(sample.validSample) {
        fallbackAnalysis.validSamples++;
        sample.newBin = !bins[sample.binIndex];
        sample.wouldBeAccepted = sample.newBin;
        if(sample.newBin) {
          bins[sample.binIndex] = true;
          fallbackAnalysis.binCoverage[sample.binIndex] = true;
          fallbackAnalysis.uniqueBins++;
          fallbackAnalysis.acceptedSamples++;
        }
        fallbackAnalysis.elevationBinCounts[sample.elIndex]++;
        fallbackAnalysis.azimuthBinCounts[sample.azIndex]++;
        sumMagnitude += sample.magnitude;
        sumElevation += sample.elevationDeg;
        sumAzimuth += sample.azimuthDeg;
        if(sample.magnitude < fallbackAnalysis.minMagnitude) fallbackAnalysis.minMagnitude = sample.magnitude;
        if(sample.magnitude > fallbackAnalysis.maxMagnitude) fallbackAnalysis.maxMagnitude = sample.magnitude;
        if(sample.elevationDeg < fallbackAnalysis.minElevation) fallbackAnalysis.minElevation = sample.elevationDeg;
        if(sample.elevationDeg > fallbackAnalysis.maxElevation) fallbackAnalysis.maxElevation = sample.elevationDeg;
        if(sample.azimuthDeg < fallbackAnalysis.minAzimuth) fallbackAnalysis.minAzimuth = sample.azimuthDeg;
        if(sample.azimuthDeg > fallbackAnalysis.maxAzimuth) fallbackAnalysis.maxAzimuth = sample.azimuthDeg;
        fallbackAnalysis.samples.push_back(sample);
      }
    }

    if(fallbackAnalysis.validSamples > 0) {
      fallbackAnalysis.avgMagnitude = sumMagnitude / fallbackAnalysis.validSamples;
      fallbackAnalysis.avgElevation = sumElevation / fallbackAnalysis.validSamples;
      fallbackAnalysis.avgAzimuth = sumAzimuth / fallbackAnalysis.validSamples;
    }
    fallbackAnalysis.coveragePercent = (fallbackAnalysis.uniqueBins / (float)MAX_POINTS) * 100.0f;
    writeDetailedSampleBreakdown(fallbackAnalysis, "_fallback.txt");

    // After writing the main analysis.txt file:
    std::ofstream analysisFile("analysis.txt", std::ios::app); // Open for appending
    if(analysisFile.is_open()) {
      analysisFile << "\n=== FALLBACK ANALYSIS SUMMARY ===\n";
      analysisFile << fallbackDebug.str();
      analysisFile << "\nFallback Coverage: " << std::fixed << std::setprecision(2)
                   << fallbackAnalysis.coveragePercent << "% (" << fallbackAnalysis.uniqueBins << " bins)\n";
      analysisFile << "Fallback Avg Magnitude: " << fallbackAnalysis.avgMagnitude << " µT\n";
      analysisFile << "Fallback Avg Elevation: " << fallbackAnalysis.avgElevation << "°\n";
      analysisFile << "Fallback Avg Azimuth: " << fallbackAnalysis.avgAzimuth << "°\n";
      analysisFile << "Fallback Accepted Samples: " << fallbackAnalysis.acceptedSamples << "\n";
      analysisFile << "Fallback Valid Samples: " << fallbackAnalysis.validSamples << "\n";
      analysisFile << "Fallback Min/Max Magnitude: " << fallbackAnalysis.minMagnitude << " / " << fallbackAnalysis.maxMagnitude << " µT\n";
      analysisFile << "Fallback Min/Max Elevation: " << fallbackAnalysis.minElevation << " / " << fallbackAnalysis.maxElevation << "°\n";
      analysisFile << "Fallback Min/Max Azimuth: " << fallbackAnalysis.minAzimuth << " / " << fallbackAnalysis.maxAzimuth << "°\n";
      analysisFile << "\n";
      analysisFile.close();
    }
  }

  std::cout << "\nAnalysis complete!" << std::endl;
  return 0;
}