#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#ifndef DISABLE_BOOST_CHRONO
#include <boost/chrono.hpp>
#endif

#include <cstdint>
#include <memory>

#ifndef _WIN32
typedef int32_t __int32;
typedef int64_t __int64;
#endif

class AbstractTimer {
protected:
  AbstractTimer();

public:
  virtual ~AbstractTimer();

  virtual void start() = 0;
  virtual const double &end() = 0;
};

class BoostTimerImpl;
class WindowsHRTimerImpl;
#ifndef DISABLE_BOOST_CHRONO
class BoostChronoTimerImpl;
#endif

class BoostTimer : public AbstractTimer {
public:
  BoostTimer();

  void start();
  const double &end();

private:
  std::unique_ptr<BoostTimerImpl> impl_;
};

#ifndef DISABLE_BOOST_CHRONO
class BoostChronoTimer : public AbstractTimer {
public:
  BoostChronoTimer();

  void start();
  const double &end();

private:
  std::unique_ptr<BoostChronoTimerImpl> impl_;
};
#endif

#ifdef _WIN32
class WindowsHRTimer : public AbstractTimer {
public:
  WindowsHRTimer();

  void start();
  const double &end();

private:
  const double frequency() const;

  std::unique_ptr<WindowsHRTimerImpl> impl_;
};
#endif

#include <string>
class TimerFactory {
public:
#ifdef __NVCC__
  /**
   * The nVidia nvcc compiler complains with strongly typed enums
   */
  enum Type {};
#else
  enum class Type { BoostTimerImpl, WindowsHRTimerImpl, BoostChronoTimerImpl };
#endif

  static std::unique_ptr<AbstractTimer> factory(TimerFactory::Type);

  /**
   * This is for when strongly typed enums aren't supported
   */
  static std::unique_ptr<AbstractTimer> factory(const std::string &timerFactoryTypeName);

private:
  TimerFactory();
};

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

class Benchmark {
private:
  Benchmark(TimerFactory::Type timerImplType);

public:
  virtual ~Benchmark();

  void setOutput(std::string output);
  void setDescription(std::string description);

  virtual void measure() = 0;

  virtual const __int64 &counts() = 0;
  virtual const double &avg() = 0;
  virtual const double &totalavg() = 0;
  const int N() const;            // Fast
  const double mean() const;      // Implemented with a naive running implementation
  const double median() const;    // Fast
  const double variance() const;  // Slow
  const double stddev() const;    // Implemented with a naive running implementation
  const double stderr() const;    // Implemented with a naive running implementation

  void require(__int32 numberOfSets);
  void require_standard_error(double minimum);
  void store(const double &measure);

  const bool &complete();

  void writeResults();

  void includeRawMeasures(bool flag);
  void startHistogramAtZero(bool flag);

protected:
  virtual void writeBenchmark(std::ofstream &resultsFile) = 0;
  virtual void writeStatistics(std::ofstream &resultsFile);
  virtual void writeFrequencies(std::ofstream &resultsFile);
  virtual void writeRawMeasures(std::ofstream &resultsFile);

  void reset_();

private:
  std::string output_;
  std::string description_;

  std::unique_ptr<AbstractTimer> beginTimer_;
  std::unique_ptr<AbstractTimer> currentTimer_;

  friend class MeasureCounts;
  friend class MeasureInterval;

  __int64 batchNumber_;
  __int32 requireNumberOfSets_;
  double requireMinimumStandardErrorOf_;

  std::vector<double> measures_;
  double minValue_;
  double maxValue_;

  bool measuresComplete_;
  bool includeRawMeasures_;
  bool startHistogramAtZero_;

  // running mean & stddev stuff, see http://blog.cppse.nl/smart-calculation-mean
  int N_;
  double mean_;
  double frac1, frac2;
  double x_sum, x2_sum;
};

class MeasureCounts : public Benchmark {
public:
  MeasureCounts(TimerFactory::Type timerImplType);
  ~MeasureCounts();

  void reset();

  void setBatchSize(__int32 batchSize);
  void setBatchGroupSize(__int32 batchGroupSize);
  void measure();

  const __int64 &counts();
  const double &avg();
  const double &totalavg();

  virtual void writeBenchmark(std::ofstream &myfile);

private:
  __int64 counts_;
  double average_;
  double averageTotal_;

  __int32 batchSize_;
  __int32 batchGroupSize_;

  __int32 batchCounts_;
  __int32 batchCountIndex_;
  __int32 batchMilliseconds_;
};

class MeasureInterval : public Benchmark {
public:
  MeasureInterval(TimerFactory::Type timerImplType);
  ~MeasureInterval();

  void measure();

  const __int64 &counts();
  const double &avg();
  const double &totalavg();

  virtual void writeBenchmark(std::ofstream &myfile);

private:
  __int64 counts_;
  double average_;
  double averageTotal_;
};

#endif  // __BENCHMARK_H__
