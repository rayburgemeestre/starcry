#include "util/benchmark.h"

#include <cmath>
#include <iomanip>
#include <mutex>

const double end_time(std::chrono::high_resolution_clock::time_point start__) {
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::microseconds ms = std::chrono::duration_cast<std::chrono::microseconds>(now - start__);
  return static_cast<double>(ms.count() / 1000.0);
}

Benchmark::Benchmark()
    : output_("DEFAULT"), description_("unknown"), includeRawMeasures_(false), startHistogramAtZero_(false) {}

Benchmark::~Benchmark() {}

std::chrono::high_resolution_clock::time_point Benchmark::measure(const std::string& label) {
  std::scoped_lock lock(mut_);

  if (!counts_.contains(label)) {
    measures_[label].clear();
    N_[label] = 0;
    mean_[label] = 0;
    frac1[label] = 0;
    frac2[label] = 0;
    x_sum[label] = 0;
    x2_sum[label] = 0;
    minValue_[label] = std::numeric_limits<int32_t>::max();
    maxValue_[label] = std::numeric_limits<int32_t>::min();
    beginTimer_[label] = std::chrono::high_resolution_clock::now();
    currentTimer_[label] = std::chrono::high_resolution_clock::now();
  }

  counts_[label]++;
  currentTimer_[label] = std::chrono::high_resolution_clock::now();
  return currentTimer_[label];
}

void Benchmark::setOutput(std::string output) {
  output_ = output;
}

void Benchmark::setDescription(std::string description) {
  description_ = description;
}

void Benchmark::store(const std::string& label, std::chrono::high_resolution_clock::time_point start) {
  std::scoped_lock lock(mut_);
  const auto measure = end_time(currentTimer_[label]);

  if (measure < minValue_[label]) minValue_[label] = measure;
  if (measure > maxValue_[label]) maxValue_[label] = measure;

  measures_[label].push_back(measure);

  // @see http://blog.cppse.nl/smart-calculation-mean
  // TODO: maybe replace this naive approach with something from
  // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance (i.e., "Computing Shifted Data" ?)
  N_[label]++;
  double frac1 = 1.0 / static_cast<double>(N_[label] + 1);
  double frac2 = 1.0 - frac1;
  mean_[label] *= frac2;
  mean_[label] += measure * frac1;
  x_sum[label] += measure;
  x2_sum[label] += measure * measure;
  if (N_[label] == 1) return;
}

void Benchmark::finalize() {
  for (auto& timer : beginTimer_) {
    measure(timer.first);
  }
}

void Benchmark::writeResults() {
  std::scoped_lock lock(mut_);
  if (measures_.empty()) return;

  std::ofstream resultsFile;
  std::stringstream ss;
  ss << "RESULT_" << output_ << ".TXT";
  resultsFile.open(ss.str());

  for (const auto& measure : measures_) {
    writeBenchmark(measure.first, resultsFile);
    writeStatistics(measure.first, resultsFile);
    writeFrequencies(measure.first, resultsFile);
    writeRawMeasures(measure.first, resultsFile);
    resultsFile << std::endl << std::endl;
  }

  resultsFile.close();
}

void Benchmark::writeBenchmark(const std::string& label, std::ofstream& resultsFile) {
  resultsFile << "@@@ BENCHMARK @@@" << std::endl << std::endl;

  resultsFile << "label \"" << label << "\"." << std::endl << std::endl;

  resultsFile << "profile \"" << output_ << "\"." << std::endl << std::endl;

  resultsFile << "measure \"" << description_ << "\"." << std::endl << std::endl << std::endl;

  // resultsFile << "measure interval = " << batchGroupSize_ << " ms." << std::endl << std::endl << std::endl;
}
void Benchmark::writeStatistics(const std::string& label, std::ofstream& resultsFile) {
  resultsFile << "@@@ STATISTICS @@@" << std::endl << std::endl;

  const int N = this->N(label);
  const double mean = this->mean(label);
  const double median = this->median(label);
  const double variance = this->variance(label);
  const double stddev = this->stddev(label);
  const double semean = this->stderr_(label);

  resultsFile << std::fixed << std::setprecision(6);
  resultsFile << "N          " << N << std::endl;
  resultsFile << "Mean       " << mean << std::endl;
  resultsFile << "S.E. Mean  " << semean << std::endl;
  resultsFile << "Std. Dev   " << stddev << std::endl;
  resultsFile << "Variance   " << variance << std::endl;
  resultsFile << "Minimum    " << minValue_[label] << std::endl;
  resultsFile << "Maximum    " << maxValue_[label] << std::endl;
  resultsFile << "Median     " << median << std::endl;
}

const int Benchmark::N(const std::string& label) const {
  return static_cast<int>(measures_[label].size());
}

const double Benchmark::mean(const std::string& label) const {
  return mean_[label];  // or sum_x / N_
}

const double Benchmark::median(const std::string& label) const {
  size_t medianIndex = static_cast<size_t>(measures_[label].size() / 2.0);
  return measures_[label][medianIndex];
}

const double Benchmark::variance(const std::string& label) const {
  double variance = 0;
  const int N = this->N(label);
  const double mean = this->mean(label);
  for (int i = 0; i < N; i++) {
    double squared = (measures_[label][i] - mean) * (measures_[label][i] - mean);
    variance += squared;
  }
  return variance;
}

const double Benchmark::stddev(const std::string& label) const {
  // return sqrt(variance() / (measures_.size() - 1));
  return sqrt((x2_sum[label] / N_[label]) - (mean_[label] * mean_[label]));
}

const double Benchmark::stderr_(const std::string& label) const {
  return stddev(label) / sqrt(N(label));
}

void Benchmark::writeFrequencies(const std::string& label, std::ofstream& resultsFile) {
  resultsFile << std::endl << std::endl << "@@@ HISTOGRAM @@@" << std::endl << std::endl;

  double useMinValue = minValue_[label];

  if (startHistogramAtZero_) useMinValue = 0;

  double buckets = 10;
  double bucketWidth = (maxValue_[label] - useMinValue) / (buckets - 1);
  std::vector<int> histogram(static_cast<unsigned int>(buckets), 0);

  int maxMeasure = 0;
  // std::for_each(std::begin(measures_), std::end(measures_), [&](double measure) {
  std::for_each(measures_[label].begin(), measures_[label].end(), [&](double measure) {
    if (bucketWidth == 0) {
      bucketWidth = 1;
    }
    int bucket = static_cast<int>((measure - useMinValue) / bucketWidth);
    if (bucket >= buckets) bucket = static_cast<int>(buckets - 1);
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

void Benchmark::writeRawMeasures(const std::string& label, std::ofstream& resultsFile) {
  if (!includeRawMeasures_) return;

  resultsFile << std::endl << std::endl << "@@@ RAW VALUES @@@" << std::endl << std::endl;

  // std::for_each(std::begin(measures_), std::end(measures_), [&](double measure) {
  std::for_each(measures_[label].begin(), measures_[label].end(), [&](double measure) {
    resultsFile << std::setprecision(4) << measure << std::endl;
  });
}

void Benchmark::includeRawMeasures(bool flag) {
  includeRawMeasures_ = flag;
}
void Benchmark::startHistogramAtZero(bool flag) {
  startHistogramAtZero_ = flag;
}
