/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stack>
#include <thread>

namespace sw {
namespace redis {
class Redis;
}
}  // namespace sw
class starcry;
class job_message;

class redis_server {
private:
  std::atomic<bool> running = true;
  std::thread runner;
  std::thread job_timeout_runner;
  std::thread job_waiter;
  std::unique_ptr<sw::redis::Redis> redis;
  std::set<std::string> known_workers;
  std::stack<std::string> waiting_workers;
  std::mutex waiting_workers_mut;
  std::mutex dispatch_job_mut_;

  std::string host;
  starcry& sc;

  // TODO: these make outstanding_jobs variable redundant
  using job_id_t = std::pair<uint32_t, uint32_t>;
  size_t outstanding_jobs = 0;
  std::map<job_id_t, std::pair<std::chrono::high_resolution_clock::time_point, std::shared_ptr<job_message>>>
      outstanding_jobs2;                                               // waiting (for confirmation)
  std::map<job_id_t, std::shared_ptr<job_message>> outstanding_jobs3;  // in progress (confirmed)

  bool recv_last = false;
  bool send_last = false;
  std::string my_id_;

public:
  explicit redis_server(const std::string& host,
                        starcry& starcry /*std::shared_ptr<queue> source, std::shared_ptr<queue> dest*/);
  ~redis_server();

  void run();

private:
  void dispatch_job_from_queue();
  bool dispatch_job(std::shared_ptr<job_message> job);
  bool workers_available();
  std::string pop_worker();
};