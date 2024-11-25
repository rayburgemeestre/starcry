#pragma once

#include <chrono>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Benchmark {
public:
  Benchmark();
  ~Benchmark();

  std::chrono::high_resolution_clock::time_point measure(const std::string& label);

  const int N(const std::string& label) const;            // Fast
  const double mean(const std::string& label) const;      // Implemented with a naive running implementation
  const double median(const std::string& label) const;    // Fast
  const double variance(const std::string& label) const;  // Slow
  const double stddev(const std::string& label) const;    // Implemented with a naive running implementation
  const double stderr_(const std::string& label) const;   // Implemented with a naive running implementation

  void store(const std::string& label, std::chrono::high_resolution_clock::time_point start);

  void finalize();
  void writeResults();

  // User options
  void setOutput(std::string output);
  void setDescription(std::string description);
  void includeRawMeasures(bool flag);
  void startHistogramAtZero(bool flag);

protected:
  void writeBenchmark(const std::string& label, std::ofstream& resultsFile);
  void writeStatistics(const std::string& label, std::ofstream& resultsFile);
  void writeFrequencies(const std::string& label, std::ofstream& resultsFile);
  void writeRawMeasures(const std::string& label, std::ofstream& resultsFile);

private:
  std::string output_;
  std::string description_;

  std::map<std::string, std::chrono::high_resolution_clock::time_point> beginTimer_;
  std::map<std::string, std::chrono::high_resolution_clock::time_point> currentTimer_;

  mutable std::map<std::string, std::vector<double>> measures_;

  bool includeRawMeasures_;
  bool startHistogramAtZero_;

  // running mean & stddev stuff, see http://blog.cppse.nl/smart-calculation-mean
  mutable std::map<std::string, int> N_;
  mutable std::map<std::string, double> mean_;
  mutable std::map<std::string, double> frac1, frac2;
  mutable std::map<std::string, double> x_sum, x2_sum;
  mutable std::map<std::string, double> minValue_, maxValue_;

  // MeasureInterval
  mutable std::map<std::string, int64_t> counts_;
  mutable std::map<std::string, double> average_;
  mutable std::map<std::string, double> averageTotal_;
  std::mutex mut_;
};