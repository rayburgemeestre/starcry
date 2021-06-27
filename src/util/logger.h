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

static std::mutex logger_mut__;

enum LogLevel { DEBUG, INFO, WARNING, ERROR, FATAL };

class logger {
public:
  explicit logger(LogLevel level) : _holder(std::make_shared<holder>(level, fi)) {}

  template <class T>
  friend std::ostream &operator<<(const logger &l, const T &t) {
    system_clock::time_point tp = system_clock::now();
    time_t raw_time = system_clock::to_time_t(tp);
    struct tm *timeinfo = std::localtime(&raw_time);
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    std::string milliseconds_str = std::to_string(ms.count() % 1000);
    if (milliseconds_str.length() < 3) {
      milliseconds_str = std::string(3 - milliseconds_str.length(), '0') + milliseconds_str;
    }
    l._holder->_os << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S.") << milliseconds_str;
    switch (l._holder->_level) {
      case DEBUG:
        l._holder->_os << " DEBUG ";
        break;
      case INFO:
        l._holder->_os << "  INFO ";
        break;
      case WARNING:
        l._holder->_os << "  WARN ";
        break;
      case ERROR:
        l._holder->_os << " ERROR ";
        break;
      case FATAL:
        l._holder->_os << " FATAL ";
        break;
      default:
        break;
    }
    return l._holder->_os << t;
  }

private:
  struct holder {
    explicit holder(LogLevel level, std::ostream &os) : _level(level), _os(os), _lock(logger_mut__) {}
    LogLevel _level = LogLevel::DEBUG;
    std::ostream &_os;
    std::lock_guard<std::mutex> _lock;
  };

  static std::ofstream fi;
  mutable std::shared_ptr<holder> _holder;
};