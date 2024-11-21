#include "util/benchmark.h"

#include <stdexcept>

std::unique_ptr<AbstractTimer> TimerFactory::factory(TimerFactory::Type type) {
  switch (type) {
    case Type::BoostTimerImpl:
      return std::unique_ptr<AbstractTimer>(new BoostTimer());
    case Type::BoostChronoTimerImpl:
      return std::unique_ptr<AbstractTimer>(new BoostChronoTimer());
#ifdef WIN32
    case Type::WindowsHRTimerImpl:
      return std::unique_ptr<AbstractTimer>(new WindowsHRTimer());
#else
    case Type::WindowsHRTimerImpl:
      return nullptr;  // should not happen, added to prevent: warning: enumeration value 'WindowsHRTimerImpl' not
                       // handled in switch [-Wswitch]
#endif
  }
  throw std::runtime_error("factory for given type not implemented");
}

std::unique_ptr<AbstractTimer> TimerFactory::factory(const std::string &timerFactoryTypeName) {
  if (!timerFactoryTypeName.compare("BoostTimerImpl")) {
    return std::unique_ptr<AbstractTimer>(new BoostTimer());
  } else if (!timerFactoryTypeName.compare("BoostChronoTimerImpl")) {
    return std::unique_ptr<AbstractTimer>(new BoostChronoTimer());
  }
#ifdef WIN32
  else if (!timerFactoryTypeName.compare("WindowsHRTimer")) {
    return std::unique_ptr<AbstractTimer>(new WindowsHRTimer());
  }
#endif
  throw std::runtime_error("factory for given type not implemented");
}
