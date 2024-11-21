#include "util/benchmark.h"

MeasureInterval::MeasureInterval(TimerFactory::Type timerImplType)
    : Benchmark(timerImplType), counts_(0), average_(0.0), averageTotal_(0.0) {}

MeasureInterval::~MeasureInterval() {
  if (measures_.empty()) return;

  if (!measuresComplete_) writeResults();
}

void MeasureInterval::measure() {
  if (!counts_++) {
    currentTimer_->start();
    return;
  }

  store(currentTimer_->end());

  currentTimer_->start();
}

const __int64 &MeasureInterval::counts() {
  return counts_;
}

const double &MeasureInterval::avg() {
  return average_;
}

const double &MeasureInterval::totalavg() {
  return averageTotal_;
}

void MeasureInterval::writeBenchmark(std::ofstream &resultsFile) {
  resultsFile << "@@@ BENCHMARK @@@" << std::endl << std::endl;

  resultsFile << "profile \"" << output_ << "\"." << std::endl << std::endl;

  resultsFile << "measure \"" << description_ << "\"." << std::endl << std::endl << std::endl;

  // resultsFile << "measure interval = " << batchGroupSize_ << " ms." << std::endl << std::endl << std::endl;
}