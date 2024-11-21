#include "util/benchmark.h"
#include <cmath>

Benchmark::Benchmark(TimerFactory::Type timerImplType)
    : output_("DEFAULT"),
      description_("unknown"),
      beginTimer_(TimerFactory::factory(timerImplType)),
      currentTimer_(TimerFactory::factory(timerImplType)),
      batchNumber_(0),
      requireNumberOfSets_(0),
      requireMinimumStandardErrorOf_(0),
      minValue_(std::numeric_limits<__int32>::max()),
      maxValue_(std::numeric_limits<__int32>::min()),
      measuresComplete_(false),
      includeRawMeasures_(false),
      startHistogramAtZero_(false),
      N_(0),
      mean_(0),
      frac1(0),
      frac2(0),
      x_sum(0),
      x2_sum(0) {}

Benchmark::~Benchmark() {}

void Benchmark::reset_() {
  batchNumber_ = 0;
  requireNumberOfSets_ = 0;
  requireMinimumStandardErrorOf_ = 0;
  minValue_ = std::numeric_limits<__int32>::max();
  maxValue_ = std::numeric_limits<__int32>::min();
  measuresComplete_ = false;
  includeRawMeasures_ = false;
  startHistogramAtZero_ = false;
  measures_.clear();
  N_ = 0;
  mean_ = 0;
  frac1 = 0;
  frac2 = 0;
  x_sum = 0;
  x2_sum = 0;

  beginTimer_->start();
  currentTimer_->start();
}

void Benchmark::setOutput(std::string output) {
  output_ = output;
}
void Benchmark::setDescription(std::string description) {
  description_ = description;
}

void Benchmark::require(__int32 numberOfSets) {
  requireNumberOfSets_ = numberOfSets;
  measures_.reserve(requireNumberOfSets_);
}

void Benchmark::require_standard_error(double minimum) {
  requireMinimumStandardErrorOf_ = minimum;
}

void Benchmark::store(const double &measure) {
  if (measuresComplete_) return;

  if (measure < minValue_) minValue_ = measure;

  if (measure > maxValue_) maxValue_ = measure;

  measures_.push_back(measure);

  // @see http://blog.cppse.nl/smart-calculation-mean
  // TODO: maybe replace this naive approach with something from
  // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance (i.e., "Computing Shifted Data" ?)
  N_++;
  double frac1 = 1.0 / static_cast<double>(N_ + 1);
  double frac2 = 1.0 - frac1;
  mean_ *= frac2;
  mean_ += measure * frac1;
  x_sum += measure;
  x2_sum += measure * measure;

  if (N_ == 1) return;

  if (requireMinimumStandardErrorOf_ > 0.0 && stderr() < requireMinimumStandardErrorOf_) {
    measuresComplete_ = true;
    writeResults();
  }
  if (requireNumberOfSets_ > 0 && measures_.size() >= static_cast<size_t>(requireNumberOfSets_)) {
    measuresComplete_ = true;
    writeResults();
  }
}

const bool &Benchmark::complete() {
  return measuresComplete_;
}

void Benchmark::writeResults() {
  if (measures_.empty()) return;

  std::ofstream resultsFile;
  std::stringstream ss;
  ss << "RESULT_" << output_ << ".TXT";
  resultsFile.open(ss.str());

  writeBenchmark(resultsFile);
  writeStatistics(resultsFile);
  writeFrequencies(resultsFile);
  writeRawMeasures(resultsFile);

  resultsFile.close();
}

#include <iomanip>
void Benchmark::writeStatistics(std::ofstream &resultsFile) {
  resultsFile << "@@@ STATISTICS @@@" << std::endl << std::endl;

  const int N = this->N();
  const double mean = this->mean();
  const double median = this->median();
  const double variance = this->variance();
  const double stddev = this->stddev();
  const double semean = this->stderr();

  resultsFile << std::fixed << std::setprecision(6);
  resultsFile << "N          " << N << std::endl;
  resultsFile << "Mean       " << mean << std::endl;
  resultsFile << "S.E. Mean  " << semean << std::endl;
  resultsFile << "Std. Dev   " << stddev << std::endl;
  resultsFile << "Variance   " << variance << std::endl;
  resultsFile << "Minimum    " << minValue_ << std::endl;
  resultsFile << "Maximum    " << maxValue_ << std::endl;
  resultsFile << "Median     " << median << std::endl;
}

const int Benchmark::N() const {
  return static_cast<int>(measures_.size());
}

const double Benchmark::mean() const {
  return mean_;  // or sum_x / N_
}

const double Benchmark::median() const {
  size_t medianIndex = static_cast<size_t>(measures_.size() / 2.0);
  return measures_[medianIndex];
}

const double Benchmark::variance() const {
  double variance = 0;
  const int N = this->N();
  const double mean = this->mean();
  for (int i = 0; i < N; i++) {
    double squared = (measures_[i] - mean) * (measures_[i] - mean);
    variance += squared;
  }
  return variance;
}

const double Benchmark::stddev() const {
  // return sqrt(variance() / (measures_.size() - 1));
  return sqrt((x2_sum / N_) - (mean_ * mean_));
}

const double Benchmark::stderr() const {
  return stddev() / sqrt(N());
}

void Benchmark::writeFrequencies(std::ofstream &resultsFile) {
  resultsFile << std::endl << std::endl << "@@@ HISTOGRAM @@@" << std::endl << std::endl;

  double useMinValue = minValue_;

  if (startHistogramAtZero_) useMinValue = 0;

  double buckets = 10;
  double bucketWidth = (maxValue_ - useMinValue) / (buckets - 1);
  std::vector<int> histogram(static_cast<unsigned int>(buckets), 0);

  int maxMeasure = 0;
  // std::for_each(std::begin(measures_), std::end(measures_), [&](double measure) {
  std::for_each(measures_.begin(), measures_.end(), [&](double measure) {
    int bucket = static_cast<int>((measure - useMinValue) / bucketWidth);

    histogram[bucket]++;

    if (histogram[bucket] > maxMeasure) maxMeasure = histogram[bucket];
  });

  double plotHeight = 10;
  for (int h = static_cast<int>(plotHeight); h >= 0; h--) {
    for (int i = 0; i < buckets; i++) {
      double normalized = (histogram[i] / static_cast<double>(maxMeasure)) * plotHeight;
      resultsFile << ((static_cast<int>(normalized) >= h) ? " # " : "   ");
    }
    resultsFile << std::endl;
  }
  resultsFile << "-";
  for (int i = 0; i < buckets; i++) resultsFile << "---";
  resultsFile << std::endl;

  for (int i = 0; i < buckets; i++) {
    if (i < 10)
      resultsFile << " " << i << ".";
    else
      resultsFile << " " << i << "";
  }

  resultsFile << std::endl << std::endl;

  for (int i = 0; i < buckets; i++) {
    double a = useMinValue + (i * bucketWidth);
    double b = a + bucketWidth;

    resultsFile << std::fixed << std::setprecision(1) << i << ".  " << std::setprecision(4) << a << " <> " << b
                << "  =  " << std::setprecision(2) << histogram[i] << std::endl;
  }
}

void Benchmark::writeRawMeasures(std::ofstream &resultsFile) {
  if (!includeRawMeasures_) return;

  resultsFile << std::endl << std::endl << "@@@ RAW VALUES @@@" << std::endl << std::endl;

  // std::for_each(std::begin(measures_), std::end(measures_), [&](double measure) {
  std::for_each(measures_.begin(), measures_.end(), [&](double measure) {
    resultsFile << std::setprecision(4) << measure << std::endl;
  });
}

void Benchmark::includeRawMeasures(bool flag) {
  includeRawMeasures_ = flag;
}
void Benchmark::startHistogramAtZero(bool flag) {
  startHistogramAtZero_ = flag;
}
