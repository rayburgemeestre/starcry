/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include "data/job.hpp"
#include "data/pixels.hpp"

class render_threads {
public:
  void run(size_t worker_num, std::function<void()> &&func);
  bool keep_running(size_t worker_num);
  size_t num_queued(size_t worker_num);
  void shutdown();
  std::pair<data::job, bool> job(size_t worker_num);
  void add_result(size_t worker_num, const data::job &job, const data::pixel_data2 &dat);
  void for_each_and_clear(size_t worker_num,
                          std::function<void(const data::job &, const data::pixel_data2 &)> callback);
  void add_job(size_t worker_num, const data::job &job, bool to_file);

private:
  std::map<size_t, std::thread *> threads;
  std::mutex threads_mut;
  std::map<size_t, bool> running;
  std::mutex running_mut;
  std::map<size_t, std::vector<std::pair<data::job, data::pixel_data2>>> results;
  std::mutex result_mut;
  std::map<size_t, std::set<std::pair<data::job, bool>>> jobs;
  std::mutex jobs_mut;
};
