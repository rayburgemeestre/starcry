/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "util/logger.h"

class fps_progress {
  std::chrono::high_resolution_clock::time_point start;
  std::chrono::high_resolution_clock::time_point last_update;
  std::mutex mut;
  std::condition_variable cv;
  std::atomic<bool> active{true};
  std::atomic<size_t> counter{0};
  std::atomic<size_t> counter_current{0};
  std::thread runner;

public:
  fps_progress()
      : start(std::chrono::high_resolution_clock::now()),
        last_update(start),
        runner(&fps_progress::start_fps_monitoring_thread, this) {}

  void start_fps_monitoring_thread() {
    while (active) {
      std::unique_lock<std::mutex> l(mut);
      cv.wait_for(l, std::chrono::milliseconds(1000), [&]() -> bool {
        return !active;
      });
      if (!active) {
        break;
      }
      update_fps();
    }
  }

  void update_fps() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = now - last_update;
    if (diff.count() >= 1000.0) {
      std::chrono::duration<double, std::milli> passed = now - start;
      logger(DEBUG) << "FPS: frame=" << counter
                    << ", average=" << static_cast<double>(counter) / (passed.count() / 1000.0)
                    << ", current=" << static_cast<double>(counter - counter_current) << std::endl;
      counter_current.store(counter);
    }
  }

  void inc() {
    counter++;
  }

  double final() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> idle = now - start;
    return static_cast<double>(counter) / (idle.count() / 1000.0);
  }

  ~fps_progress() {
    active = false;
    cv.notify_one();
    runner.join();
  }
};
