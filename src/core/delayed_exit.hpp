/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

class delayed_exit {
public:
  delayed_exit(int timeout_seconds) : timeout_seconds(timeout_seconds), job_done(false) {
    exit_thread = std::thread(&delayed_exit::wait_and_exit, this);
  }

  ~delayed_exit() {
    {
      std::lock_guard<std::mutex> lock(mtx);
      job_done = true;
    }
    cv.notify_one();
    if (exit_thread.joinable()) {
      exit_thread.join();
    }
  }

  void wait_and_exit() {
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::seconds(timeout_seconds), [this]() { return job_done; })) {
      std::exit(2);
    }
  }

private:
  std::thread exit_thread;
  std::mutex mtx;
  std::condition_variable cv;
  int timeout_seconds;
  bool job_done;
};
