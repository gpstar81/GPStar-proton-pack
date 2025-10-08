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
static constexpr uint8_t BIN_DEGREES = 9; // ONLY CHANGE THIS VALUE!
static constexpr uint8_t NUM_AZIMUTH_BINS = (uint8_t)(360 / BIN_DEGREES);
static constexpr uint8_t NUM_ELEVATION_BINS = (uint8_t)(180 / BIN_DEGREES);
static constexpr uint16_t MAX_POINTS = NUM_AZIMUTH_BINS * NUM_ELEVATION_BINS;

// Structure: MagSample
// Purpose: Store a single magnetometer reading with analysis results
struct MagSample {
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
  std::vector<MagSample> samples;    // All processed samples
};

// Function: processMagSample
// Purpose: Replicate the exact addSample() logic for analysis
// Inputs: Raw magnetometer data (x, y, z), line number for tracking
// Outputs: Complete analysis of this sample including bin assignment
// 
// This function performs identical mathematical operations as MagCalibration::addSample()
// but provides comprehensive diagnostic information instead of just true/false.
MagSample processMagSample(double x, double y, double z, uint16_t lineNumber) {
  MagSample sample = {};
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
  
  // Convert to degrees for easier interpretation
  sample.azimuthDeg = sample.azimuth * 180.0 / M_PI;
  sample.elevationDeg = sample.elevation * 180.0 / M_PI;
  
  // STEP 4: Calculate bin indices (identical to addSample())
  sample.azIndex = (int)((sample.azimuth + M_PI) / (2 * M_PI) * NUM_AZIMUTH_BINS);
  sample.elIndex = (int)((sample.elevation + M_PI / 2) / M_PI * NUM_ELEVATION_BINS);
  
  // STEP 5: Apply bounds checking (identical to addSample())
  if(sample.azIndex < 0) sample.azIndex = 0;
  if(sample.azIndex >= NUM_AZIMUTH_BINS) sample.azIndex = NUM_AZIMUTH_BINS - 1;
  if(sample.elIndex < 0) sample.elIndex = 0;
  if(sample.elIndex >= NUM_ELEVATION_BINS) sample.elIndex = NUM_ELEVATION_BINS - 1;
  
  // STEP 6: Calculate final bin index (identical to addSample())
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
  
  // Process each reading
  for(uint16_t i = 0; i < readings.size(); i++) {
    // Add progress reporting for large files
    if(readings.size() > 1000 && i % 1000 == 0) {
      std::cout << "  Processed " << i << " / " << readings.size() << " samples..." << std::endl;
    }
    
    MagSample sample = processMagSample(readings[i][0], readings[i][1], readings[i][2], i + 1);
    
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
// Purpose: Write complete sample-by-sample analysis to a dedicated output file
// Inputs: Analysis results structure
// Outputs: Creates a detailed breakdown file for review
void writeDetailedSampleBreakdown(const DatasetAnalysis& analysis) {
  // Generate output filename based on source log file
  std::string outputFilename = analysis.filename;
  
  // Remove .log extension if present and add _analysis.txt
  size_t lastDot = outputFilename.find_last_of(".");
  if(lastDot != std::string::npos) {
    outputFilename = outputFilename.substr(0, lastDot);
  }
  outputFilename += "_analysis.txt";
  
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
  
  // Write detailed sample breakdown to file
  writeDetailedSampleBreakdown(analysis);
  
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
void compareAnalysis(const DatasetAnalysis& prototype, const DatasetAnalysis& production) {
  std::cout << "\n=== COMPARATIVE ANALYSIS ===" << std::endl;
  
  std::cout << std::left << std::setw(25) << "Metric" << " | " 
            << std::setw(15) << "Prototype" << " | " 
            << std::setw(15) << "Production" << " | " 
            << std::setw(12) << "Ratio (P/Pr)" << std::endl;
  std::cout << std::string(70, '-') << std::endl;
  
  // Sample statistics comparison
  std::cout << std::setw(25) << "Valid Samples" << " | "
            << std::setw(15) << prototype.validSamples << " | "
            << std::setw(15) << production.validSamples << " | "
            << std::setw(12) << std::fixed << std::setprecision(2) 
            << (float)production.validSamples / prototype.validSamples << std::endl;
            
  std::cout << std::setw(25) << "Unique Bins" << " | "
            << std::setw(15) << prototype.uniqueBins << " | "
            << std::setw(15) << production.uniqueBins << " | "
            << std::setw(12) << (float)production.uniqueBins / prototype.uniqueBins << std::endl;
            
  std::cout << std::setw(25) << "Coverage %" << " | "
            << std::setw(15) << std::setprecision(1) << prototype.coveragePercent << " | "
            << std::setw(15) << production.coveragePercent << " | "
            << std::setw(12) << std::setprecision(2) << production.coveragePercent / prototype.coveragePercent << std::endl;
            
  std::cout << std::setw(25) << "Avg Magnitude (µT)" << " | "
            << std::setw(15) << std::setprecision(1) << prototype.avgMagnitude << " | "
            << std::setw(15) << production.avgMagnitude << " | "
            << std::setw(12) << std::setprecision(2) << production.avgMagnitude / prototype.avgMagnitude << std::endl;
            
  std::cout << std::setw(25) << "Avg Elevation (°)" << " | "
            << std::setw(15) << prototype.avgElevation << " | "
            << std::setw(15) << production.avgElevation << " | "
            << std::setw(12) << production.avgElevation / prototype.avgElevation << std::endl;
            
  std::cout << std::setw(25) << "Avg Azimuth (°)" << " | "
            << std::setw(15) << prototype.avgAzimuth << " | "
            << std::setw(15) << production.avgAzimuth << " | "
            << std::setw(12) << production.avgAzimuth / prototype.avgAzimuth << std::endl;
  
  // Key findings analysis
  std::cout << "\n=== KEY FINDINGS ===" << std::endl;
  
  // Magnitude analysis
  double magRatio = production.avgMagnitude / prototype.avgMagnitude;
  std::cout << "1. Magnitude Difference: Production readings are " 
            << std::fixed << std::setprecision(1) << magRatio << "x stronger" << std::endl;
  if(magRatio > 1.5) {
    std::cout << "   -> SIGNIFICANT: Trace removal eliminated magnetic damping" << std::endl;
  }
  
  // Coverage analysis
  double coverageRatio = production.coveragePercent / prototype.coveragePercent;
  std::cout << "2. Coverage Difference: Production achieves " 
            << coverageRatio << "x the coverage" << std::endl;
  if(coverageRatio < 0.3) {
    std::cout << "   -> CRITICAL: Severe coverage reduction detected" << std::endl;
  }
  
  // Elevation analysis
  double elevationDiff = production.avgElevation - prototype.avgElevation;
  std::cout << "3. Elevation Shift: Production reads " 
            << elevationDiff << "° different elevation" << std::endl;
  if(fabs(elevationDiff) > 10.0) {
    std::cout << "   -> SIGNIFICANT: Large elevation bias detected" << std::endl;
  }
  
  // Bin overlap analysis
  int sharedBins = 0;
  for(int i = 0; i < MAX_POINTS; i++) {
    if(prototype.binCoverage[i] && production.binCoverage[i]) {
      sharedBins++;
    }
  }
  float overlapPercent = 0.0f;
  int totalUniqueBins = prototype.uniqueBins + production.uniqueBins - sharedBins;
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
  std::cout << "Azimuth Range: 0° to 360° (coverage: " << (360 / BIN_DEGREES) << " bins)" << std::endl;
  std::cout << "Elevation Range: -90° to +90° (coverage: " << (180 / BIN_DEGREES) << " bins)" << std::endl;
  std::cout << "Coverage Resolution: Each bin represents " << BIN_DEGREES << "° × " << BIN_DEGREES << "° area" << std::endl;
}

// Function: main
// Purpose: Main program entry point
int main(int argc, char* argv[]) {
  std::cout << "Magnetometer Calibration Analysis Tool" << std::endl;
  std::cout << "Purpose: Analyze prototype vs production binning behavior" << std::endl;
  std::cout << "=========================================" << std::endl;
  
  // Display current analysis configuration
  printConfigurationInfo();
  
  if(argc != 3) {
    std::cout << "\nUsage: " << argv[0] << " <prototype_log> <production_log>" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << argv[0] << " prototype_last3.log production_last3.log" << std::endl;
    return 1;
  }
  
  std::string prototypeFile = argv[1];
  std::string productionFile = argv[2];
  
  // Load log files
  std::cout << "\nLoading log files..." << std::endl;
  auto prototypeReadings = loadLogFile(prototypeFile);
  auto productionReadings = loadLogFile(productionFile);
  
  if(prototypeReadings.empty() || productionReadings.empty()) {
    std::cerr << "Error: Failed to load log files or files are empty" << std::endl;
    return 1;
  }
  
  // Analyze both datasets
  DatasetAnalysis prototypeAnalysis = analyzeDataset(prototypeReadings, "PROTOTYPE", prototypeFile);
  DatasetAnalysis productionAnalysis = analyzeDataset(productionReadings, "PRODUCTION", productionFile);
  
  // Print detailed analysis for each dataset
  printDetailedAnalysis(prototypeAnalysis);
  printDetailedAnalysis(productionAnalysis);
  
  // Print comparative analysis
  compareAnalysis(prototypeAnalysis, productionAnalysis);
  
  std::cout << "\nAnalysis complete!" << std::endl;
  return 0;
}