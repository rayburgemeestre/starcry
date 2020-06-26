/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "render_threads.h"

void render_threads::run(size_t worker_num, std::unique_ptr<std::thread> thread) {
  {
    const std::lock_guard<std::mutex> lock(running_mut);
    running[worker_num] = true;
  }
  const std::lock_guard<std::mutex> lock(threads_mut);
  threads[worker_num] = std::move(thread);
}

bool render_threads::keep_running(size_t worker_num) {
  const std::lock_guard<std::mutex> lock(running_mut);
  return running[worker_num];
}

size_t render_threads::num_queued(size_t worker_num) {
  const std::lock_guard<std::mutex> lock(jobs_mut);
  return jobs[worker_num].size();
}

void render_threads::shutdown() {
  std::vector<size_t> workers;
  {
    const std::lock_guard<std::mutex> lock(running_mut);
    for (const auto &pair : running) {
      workers.push_back(pair.first);
    }
  }
  for (const auto &worker : workers) {
    const std::lock_guard<std::mutex> lock(running_mut);
    running[worker] = false;
  }
  for (const auto &worker : workers) {
    const std::lock_guard<std::mutex> lock(threads_mut);
    threads[worker]->join();
  }
}

std::pair<data::job, bool> render_threads::job(size_t worker_num) {
  const std::lock_guard<std::mutex> lock(jobs_mut);
  auto job_pair = *jobs[worker_num].cbegin();
  jobs[worker_num].erase(job_pair);
  return job_pair;
}

void render_threads::add_result(size_t worker_num, const data::job &job, const data::pixel_data2 &dat) {
  const std::lock_guard<std::mutex> lock(result_mut);
  results[worker_num].push_back(std::make_pair(job, dat));
}

void render_threads::for_each_and_clear(size_t worker_num,
                                        std::function<void(const data::job &, const data::pixel_data2 &)> callback) {
  const std::lock_guard<std::mutex> lock(result_mut);
  for (const auto &pair : results[worker_num]) {
    callback(pair.first, pair.second);
  }
  results[worker_num].clear();
}

void render_threads::add_job(size_t worker_num, const data::job &job, bool to_file) {
  const std::lock_guard<std::mutex> lock(jobs_mut);
  jobs[worker_num].insert(std::make_pair(job, to_file));
}
