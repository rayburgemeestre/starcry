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

class periodic_executor {
private:
  std::function<void()> fun;
  std::thread t;
  std::chrono::milliseconds delay;
  std::condition_variable cv;
  std::mutex mut;
  bool stop;

public:
  explicit periodic_executor(std::chrono::milliseconds delay) : fun(nullptr), delay(delay), stop(false) {}
  ~periodic_executor() {
    std::unique_lock<std::mutex> l(mut);
    stop = true;
    l.unlock();
    if (t.joinable()) t.join();
  }

  void run(std::function<void()> f) {
    {
      std::scoped_lock<std::mutex> l(mut);
      if (fun == nullptr) {
        if (t.joinable()) t.join();
        fun = std::move(f);
        stop = false;
      }
    }
    if (!t.joinable()) {
      t = std::thread([=, this]() {
        while (!stop) {
          std::unique_lock<std::mutex> lock(mut);
          fun();
          cv.wait_until(lock, std::chrono::high_resolution_clock::now() + delay, [=, this]() {
            return stop;
          });
        }
        fun = nullptr;
        if (stop) return;
      });
    }
  }

  void cancel() {
    std::unique_lock<std::mutex> l(mut);
    fun = []() {};
    stop = true;
    l.unlock();
    if (t.joinable()) {
      t.join();
    }
  }
};
