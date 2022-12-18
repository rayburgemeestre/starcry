/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fstream>
#include <memory>
#include <mutex>

using std::chrono::system_clock;

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "starcry/metrics.h"

static std::recursive_mutex logger_mut__;

enum LogLevel { DEBUG, INFO, WARNING, ERROR, FATAL, _NONE };

class metrics;
extern metrics *_metrics;
extern bool _stdout;

void set_metrics(metrics *ptr);

class logger {
  std::stringstream ss_;

public:
  explicit logger(std::ostream &os, std::stringstream &ss) : _holder(std::make_shared<holder>(os, ss)) {}
  explicit logger(LogLevel level) : _holder(std::make_shared<holder>(level, fi, ss_)) {}

  friend logger operator<<(const logger &l, std::ostream &(*fun)(std::ostream &)) {
    bool line_ended = false;
#ifndef SC_CLIENT
#ifndef EMSCRIPTEN
    if (_metrics) {
      l._holder->_ss << "\n";
      line_ended = true;
      const auto line = l._holder->_ss.str();
      _metrics->log_callback((int)l._holder->_level, line);
    }
#endif
#endif
    if (_stdout) {
      if (!line_ended) l._holder->_ss << "\n";
      const auto line = l._holder->_ss.str();
      std::cout << line << std::flush;
    }
    l._holder->_os << std::endl;
    return logger(l._holder->_os, l._holder->_ss);
  }

  template <class T>
  friend logger operator<<(const logger &l, const T &t) {
    system_clock::time_point tp = system_clock::now();
    time_t raw_time = system_clock::to_time_t(tp);
    struct tm *timeinfo = std::localtime(&raw_time);
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::string milliseconds_str = std::to_string(ms.count() % 1000);
    if (milliseconds_str.length() < 3) {
      milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;
    }
    std::stringstream ss;
    if (l._holder->_level != _NONE) {
      ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S.") << milliseconds_str;
      switch (l._holder->_level) {
        case DEBUG:
          ss << " DEBUG ";
          break;
        case INFO:
          ss << "  INFO ";
          break;
        case WARNING:
          ss << "  WARN ";
          break;
        case ERROR:
          ss << " ERROR ";
          break;
        case FATAL:
          ss << " FATAL ";
          break;
        default:
          break;
      }
    }
    ss << t;
    const auto line = ss.str();
    l._holder->_ss << line;
    return logger(l._holder->_os << line, l._holder->_ss);  //._holder->_os;
  }

private:
  struct holder {
    explicit holder(LogLevel level, std::ostream &os, std::stringstream &ss)
        : _level(level), _os(os), _ss(ss), _lock(logger_mut__) {}
    explicit holder(std::ostream &os, std::stringstream &ss)
        : _level(LogLevel::_NONE), _os(os), _ss(ss), _lock(logger_mut__) {}
    LogLevel _level = LogLevel::DEBUG;
    std::ostream &_os;
    std::stringstream &_ss;
    std::lock_guard<std::recursive_mutex> _lock;
  };

  static std::ofstream fi;
  mutable std::shared_ptr<holder> _holder;
};