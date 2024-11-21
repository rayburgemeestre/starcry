#include "util/benchmark.h"

// In case you are using VS100 libs with VS120, define this: BOOST_ALL_NO_LIB, and manually add the correct
//  library, because it will not detect the version as Visual studio I guess, and try to auto link the wrong
//  library (one with 'lib' prefix, as is common in unix.

#include <boost/date_time/posix_time/posix_time.hpp>

class BoostTimerImpl {
private:
  BoostTimerImpl();

  boost::posix_time::ptime time_;

  double value_;

  friend class BoostTimer;
};

BoostTimerImpl::BoostTimerImpl() : time_(boost::posix_time::microsec_clock::local_time()), value_(0) {}

BoostTimer::BoostTimer() : impl_(std::unique_ptr<BoostTimerImpl>(new BoostTimerImpl())) {}

void BoostTimer::start() {
  impl_->time_ = boost::posix_time::microsec_clock::local_time();
}

const double& BoostTimer::end() {
  auto now = boost::posix_time::microsec_clock::local_time();

  boost::posix_time::time_duration diff = now - impl_->time_;

  // Unfortunately precision appears (on my windows & linux box) not to be so awesome if you look at individual
  // measures, (In boost the implementation is in see date_time\filetime_functions.hpp &
  // date_time\microsec_time_clock.hpp) If you look at the end result it seems that the timings are however correct.
  // Perhaps some rounding/buffering occurs. I've also created a boost::chrono based timer, that one doesn't seem to
  // have this issue. For Windows I also made a platform specific timer that uses frequencies for better precision.

  impl_->value_ = static_cast<double>(diff.total_microseconds() / 1000.0);

  return impl_->value_;
}