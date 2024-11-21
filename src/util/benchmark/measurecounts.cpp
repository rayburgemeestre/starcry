#include "util/benchmark.h"

#include <iomanip>

MeasureCounts::MeasureCounts(TimerFactory::Type timerImplType)
    : Benchmark(timerImplType),
      counts_(0),
      average_(0),
      averageTotal_(0),
      batchSize_(1000),
      batchCounts_(0),
      batchCountIndex_(0),
      batchMilliseconds_(0) {}

void MeasureCounts::reset() {
  counts_ = 0;
  average_ = 0;
  averageTotal_ = 0;

  batchGroupSize_ = 0;

  batchCounts_ = 0;
  batchCountIndex_ = 0;
  batchMilliseconds_ = 0;

  reset_();
}

MeasureCounts::~MeasureCounts() {
  if (!measuresComplete_) writeResults();
}

void MeasureCounts::setBatchSize(__int32 batchSize) {
  batchSize_ = batchSize;
}

void MeasureCounts::setBatchGroupSize(__int32 batchGroupSize) {
  batchGroupSize_ = batchGroupSize;
}

void MeasureCounts::measure() {
  if (!counts_++) {
    beginTimer_->start();
    currentTimer_->start();
    return;
  }

  const double &diff = currentTimer_->end();

  if (diff >= batchGroupSize_) {
    batchCounts_ = static_cast<int>(counts_ - batchCountIndex_);
    batchCountIndex_ = static_cast<int>(counts_);
    batchMilliseconds_ = static_cast<int>(diff);

    // reset batch
    currentTimer_->start();

    // store value for group
    store(avg());
  }
}

const __int64 &MeasureCounts::counts() {
  return counts_;
}

const double &MeasureCounts::avg() {
  if (batchMilliseconds_ == 0) return average_;

  average_ = (batchCounts_ / static_cast<double>(batchMilliseconds_)) * batchSize_;
  return average_;
}

const double &MeasureCounts::totalavg() {
  averageTotal_ = (counts_ / static_cast<double>(beginTimer_->end())) * batchSize_;
  return averageTotal_;
}

void MeasureCounts::writeBenchmark(std::ofstream &resultsFile) {
  resultsFile << "@@@ BENCHMARK @@@" << std::endl << std::endl;

  resultsFile << "profile \"" << output_ << "\"." << std::endl << std::endl;

  resultsFile << "measure \"" << description_ << "\" / "
              << "" << batchSize_ << " ms." << std::endl
              << std::endl;

  resultsFile << "measure interval = " << batchGroupSize_ << " ms." << std::endl << std::endl << std::endl;
}
