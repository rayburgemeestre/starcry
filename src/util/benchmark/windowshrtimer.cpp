#ifdef _WIN32
#include "benchmark.h"

#include <windows.h>
#include <memory>
#include <stdexcept>

class WindowsHRTimerImpl {
private:
  WindowsHRTimerImpl(double frequency);

  LARGE_INTEGER start_;

  LARGE_INTEGER stop_;

  double frequency_;

  double value_;

  friend class WindowsHRTimer;
};
WindowsHRTimerImpl::WindowsHRTimerImpl(double frequency) : start_(), stop_(), frequency_(frequency), value_(0) {}

WindowsHRTimer::WindowsHRTimer() : impl_(std::unique_ptr<WindowsHRTimerImpl>(new WindowsHRTimerImpl(frequency()))) {
  start();
}

const double WindowsHRTimer::frequency() const {
  LARGE_INTEGER proc_freq;

  if (!::QueryPerformanceFrequency(&proc_freq)) throw new std::runtime_error("QueryPerformanceFrequency() failed");

  return static_cast<const double>(proc_freq.QuadPart);
}

void WindowsHRTimer::start() {
  DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

  ::QueryPerformanceCounter(&impl_->start_);

  ::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
}

const double& WindowsHRTimer::end() {
  DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

  ::QueryPerformanceCounter(&impl_->stop_);

  ::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

  impl_->value_ = ((impl_->stop_.QuadPart - impl_->start_.QuadPart) / impl_->frequency_) * 1000.0 /**millisecs*/;

  return impl_->value_;
}
#endif  // #ifdef _WIN32
