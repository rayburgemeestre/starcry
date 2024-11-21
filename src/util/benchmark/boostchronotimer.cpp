#include "util/benchmark.h"

#include <boost/chrono.hpp>

class BoostChronoTimerImpl {
private:
  BoostChronoTimerImpl();

  boost::chrono::high_resolution_clock::time_point start_;

  double value_;

  friend class BoostChronoTimer;
};

BoostChronoTimerImpl::BoostChronoTimerImpl() : start_(boost::chrono::high_resolution_clock::now()), value_(0) {}

BoostChronoTimer::BoostChronoTimer() : impl_(std::unique_ptr<BoostChronoTimerImpl>(new BoostChronoTimerImpl())) {}

void BoostChronoTimer::start() {
  impl_->start_ = boost::chrono::high_resolution_clock::now();
}

const double& BoostChronoTimer::end() {
  auto now = boost::chrono::high_resolution_clock::now();

  boost::chrono::microseconds ms = boost::chrono::duration_cast<boost::chrono::microseconds>(now - impl_->start_);

  impl_->value_ = static_cast<double>(ms.count() / 1000.0);

  return impl_->value_;
}