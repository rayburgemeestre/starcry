/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class delayed_executor {
private:
  std::function<void()> fun;
  std::thread t;
  std::chrono::milliseconds delay;
  std::chrono::high_resolution_clock::time_point time;
  std::condition_variable cv;
  std::mutex mut;
  bool stop;

public:
  explicit delayed_executor(std::chrono::milliseconds delay)
      : delay(delay), time(std::chrono::high_resolution_clock::now()), stop(false) {}
  ~delayed_executor() {
    if (t.joinable()) t.join();
  }

  void run(std::function<void()> f) {
    {
      std::scoped_lock<std::mutex> l(mut);
      if (fun == nullptr && t.joinable()) t.join();
      fun = std::move(f);
    }
    reset();
    if (!t.joinable()) {
      t = std::thread([=]() {
        std::unique_lock<std::mutex> lock(mut);
        while (true) {
          cv.wait_until(lock, time, [=]() {
            return stop;
          });
          if (stop) return;
          if (std::chrono::high_resolution_clock::now() >= time) {
            fun();
            fun = nullptr;
            return;
          }
        }
      });
    }
  }

  void cancel() {
    std::unique_lock<std::mutex> l(mut);
    fun = []() {};
    stop = true;
    l.unlock();
    reset();
  }

private:
  void reset() {
    std::unique_lock<std::mutex> lock(mut);
    time = std::chrono::high_resolution_clock::now() + delay;
  }
};
