/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry/metrics.h"
#include "tvision/tstarcry.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

metrics::metrics(bool notty) : notty(notty) {
  // caller needs to call init, so we can use shared_from_this()
}

void metrics::init() {
  std::cout << "Welcome to Starcry." << std::endl;

  auto *ptr = this;
  curses = std::thread([=]() {
    if (notty) {
      std::cout << "noTTY" << std::endl;
      std::unique_lock<std::mutex> lock(cv_mut);
      cv.wait(lock, [&] {
        return ready;
      });
    } else {
      program = std::make_shared<TStarcry>(ptr);
      program->run();
      program->shutDown();
    }
    std::exit(0);
  });
}

void metrics::notify() {
  if (notty) {
    {
      std::lock_guard<std::mutex> lock(cv_mut);
      ready = true;
    }
    cv.notify_one();
  } else {
    program->exit();
  }
}

std::string metrics::to_json() {
  std::unique_lock<std::mutex> lock(mut);
  json result = {};

  json threads = {};
  for (const auto &thread : threads_) {
    json j{
        {"name", thread.second.name},
        {"state", str(thread.second.state)},
    };
    if (thread.second.state == thread_state::idle) {
      j["seconds_idle"] = time_diff(thread.second.idle_begin, std::chrono::high_resolution_clock::now());
    } else if (thread.second.state == thread_state::busy) {
      j["job"] = json{
          {"job", thread.second.job_number},
          {"chunk", thread.second.chunk},
          {"seconds_busy", time_diff(thread.second.idle_end, std::chrono::high_resolution_clock::now())},
      };
    }
    threads.push_back(j);
  }
  result["threads"] = threads;

  json json_jobs = {};
  // DUPLICATE CODE AHEAD
  if (!jobs_.empty()) {
    auto first_key = jobs_.begin()->first;
    auto last_key = jobs_.rbegin()->first;

    if (jobs_.size() > max_keep_jobs) {
      auto delete_until_key = last_key - max_keep_jobs;
      if (first_key < delete_until_key) {
        for (auto i = first_key; i < delete_until_key; i++) {
          jobs_.erase(i);
        }
      }
    }

    first_key = jobs_.begin()->first;

    for (size_t i = first_key; i <= last_key; i++) {
      const auto &job = jobs_[i];
      int queued = 0;
      int rendering = 0;
      int rendered = 0;
      std::chrono::time_point<std::chrono::high_resolution_clock> render_begin = job.chunks[0].render_begin;
      std::chrono::time_point<std::chrono::high_resolution_clock> render_end = job.chunks[0].render_begin;
      for (const auto &chunk : job.chunks) {
        if (chunk.state == job_state::queued) queued++;
        if (chunk.state == job_state::rendering) rendering++;
        if (chunk.state == job_state::rendered) rendered++;
        render_begin = std::min(render_begin, chunk.render_begin);
        render_end = std::max(render_end, chunk.render_end);
      }
      auto s = job_state::rendering;
      if (rendered > 0) {
        s = job_state::rendered;
      }
      if (queued == job.chunks.size()) {
        s = job_state::queued;
      }
      if (job.skipped) {
        s = job_state::skipped;
      }
      json json_job;
      switch (s) {
        case job_state::queued:
        case job_state::skipped:
          json_job["number"] = job.number;
          json_job["state"] = str(s);
          json_job["gen_time"] = time_diff(job.generate_begin, job.generate_end);
          break;
        case job_state::rendering:
          json_job["number"] = job.number;
          json_job["state"] = str(s);
          json_job["gen_time"] = time_diff(job.generate_begin, job.generate_end);
          json_job["render_time"] = time_diff(render_begin, std::chrono::high_resolution_clock::now());
          break;
        case job_state::rendered:
          json_job["number"] = job.number;
          json_job["state"] = str(s);
          json_job["gen_time"] = time_diff(job.generate_begin, job.generate_end);
          json_job["render_time"] = time_diff(render_begin, render_end);
          hack_last_render_time = time_diff(render_begin, render_end);
          break;
      }
      if (job.chunks.size() > 1) {
        json json_chunks{{"global", json{}}, {"frame", json{}}};
        if (mode == modes::global_mode) {
          for (const auto &chunk : job.chunks) {
            json_chunks["global"].push_back(str(chunk.state));
          }
        } else if (mode == modes::frame_mode) {
          for (const auto &chunk : job.chunks) {
            json json_chunk;
            json_chunk["chunk"] = chunk.chunk;
            json_chunk["num_chunks"] = chunk.num_chunks;
            json_chunk["state"] = str(chunk.state);
            switch (chunk.state) {
              case job_state::queued:
              case job_state::skipped:
                break;
              case job_state::rendering:
                json_chunk["rendering_time"] = time_diff(chunk.render_begin, std::chrono::high_resolution_clock::now());
                break;
              case job_state::rendered:
                json_chunk["rendering_time"] = time_diff(chunk.render_begin, chunk.render_end);
                break;
            }
            json_chunks["frame"].push_back(json_chunk);
          }
        }
        json_job["chunks"] = json_chunks;
      }
      json_jobs.push_back(json_job);
    }
  }
  result["jobs"] = json_jobs;

  return json{{"type", "metrics"}, {"data", result}}.dump();
}

metrics::~metrics() {
  notify();
  curses.join();
  for (const auto &line : _output) {
    if (line.first <= 32) {
      std::cout << line.second;
    }
  }
}

void metrics::register_thread(int number, std::string desc) {
  std::unique_lock<std::mutex> lock(mut);
  threads_[number] = metrics::thread_data{number,
                                          std::move(desc),
                                          metrics::thread_state::idle,
                                          std::chrono::high_resolution_clock::now(),
                                          std::chrono::high_resolution_clock::now()};
}

void metrics::register_job(int number, int frame, int chunk, int num_chunks) {
  std::unique_lock<std::mutex> lock(mut);
  jobs_[number] = metrics::job{number,
                               frame,
                               chunk,
                               num_chunks,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::high_resolution_clock::now(),
                               false,
                               -1,
                               {}};
  jobs_[number].chunks.push_back(metrics::chunk{1,
                                                1,
                                                std::chrono::high_resolution_clock::now(),
                                                std::chrono::high_resolution_clock::now(),
                                                metrics::job_state::queued});
}
void metrics::complete_job(int number) {
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    jobs_[number].generate_end = std::chrono::high_resolution_clock::now();
  }
}

void metrics::skip_job(int number) {
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    jobs_[number].skipped = true;
  }
}

void metrics::render_job(int thread_number, int job_number, int chunk) {
  std::unique_lock<std::mutex> lock(mut);
  if (threads_.find(thread_number) != threads_.end()) {
    threads_[thread_number].idle_end = std::chrono::high_resolution_clock::now();
    threads_[thread_number].state = metrics::thread_state::busy;
    threads_[thread_number].job_number = job_number;
    threads_[thread_number].chunk = chunk;
  }
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) {
      chunk--;
    }
    if (chunk >= jobs_[job_number].chunks.size()) return;
    jobs_[job_number].chunks[chunk].render_begin = std::chrono::high_resolution_clock::now();
    jobs_[job_number].chunks[chunk].state = metrics::job_state::rendering;
  }
}

void metrics::resize_job(int number, int num_chunks) {
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    for (int i = 0; i < num_chunks; i++) {
      if (i == 0) {
        jobs_[number].chunks[0].num_chunks = num_chunks;
        continue;
      }
      jobs_[number].chunks.push_back(metrics::chunk{i + 1,
                                                    num_chunks,
                                                    std::chrono::high_resolution_clock::now(),
                                                    std::chrono::high_resolution_clock::now(),
                                                    metrics::job_state::queued});
    }
  }
}

void metrics::resize_job_objects(int number, int job_number, int chunk, int num_objects) {
  if (!record_objects) return;
  std::unique_lock<std::mutex> lock(mut);
  // TODO: Possibly do not need thread number ????
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) chunk--;
    for (int i = 0; i < num_objects; i++) {
      if (jobs_[job_number].chunks.size() <= chunk) continue;
      jobs_[job_number].chunks[chunk].objects.push_back(metrics::object{i,
                                                                        std::chrono::high_resolution_clock::now(),
                                                                        std::chrono::high_resolution_clock::now(),
                                                                        metrics::job_state::queued});
    }
  }
}

void metrics::set_render_job_object_state(
    int thread_num, int job_number, int chunk, int index, metrics::job_state state) {
  if (!record_objects) return;
  std::unique_lock<std::mutex> lock(mut);
  // TODO: Possibly do not need thread number ????
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) chunk--;
    if (jobs_.find(job_number) == jobs_.end()) return;
    if (jobs_[job_number].chunks.size() <= chunk) return;
    if (jobs_[job_number].chunks[chunk].objects.size() <= index) return;
    jobs_[job_number].chunks[chunk].objects[index].state = state;
    if (state == metrics::job_state::rendering) {
      jobs_[job_number].chunks[chunk].objects[index].render_begin = std::chrono::high_resolution_clock::now();
    } else {
      jobs_[job_number].chunks[chunk].objects[index].render_end = std::chrono::high_resolution_clock::now();
    }
  }
}

void metrics::complete_render_job(int thread_number, int job_number, int chunk) {
  std::unique_lock<std::mutex> lock(mut);
  if (threads_.find(thread_number) != threads_.end()) {
    threads_[thread_number].idle_begin = std::chrono::high_resolution_clock::now();
    threads_[thread_number].state = metrics::thread_state::idle;
  }
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) {
      chunk--;
    }
    if (chunk >= jobs_[job_number].chunks.size()) return;
    jobs_[job_number].chunks[chunk].render_end = std::chrono::high_resolution_clock::now();
    jobs_[job_number].chunks[chunk].state = metrics::job_state::rendered;
    for (const auto &chunk : jobs_[job_number].chunks) {
      if (chunk.state != metrics::job_state::rendered) return;
    }
    completed_frames++;
  }
}

void metrics::display(std::function<void(const std::string &)> f1,
                      std::function<void(const std::string &)> f2,
                      std::function<void(int, const std::string &)> f3) {
  std::unique_lock<std::mutex> lock(mut);

  // TODO: make this stuff use the JSON data perhaps?
  for (const auto &thread : threads_) {
    std::stringstream ss;
    ss << "Thread: " << thread.first << " " << thread.second.name << " " << str(thread.second.state);
    if (thread.second.state == thread_state::idle) {
      ss << ": Seconds Idle: " << time_diff(thread.second.idle_begin, std::chrono::high_resolution_clock::now());
    } else if (thread.second.state == thread_state::busy) {
      ss << ": Job: " << thread.second.job_number << " Chunk: " << thread.second.chunk << " Seconds Busy: ";
      ss << time_diff(thread.second.idle_end, std::chrono::high_resolution_clock::now());
    }
    f1(ss.str());
  }

  // TODO: this code will temporarily be duplicated in the JSON version
  if (jobs_.empty()) return;
  auto first_key = jobs_.begin()->first;
  auto last_key = jobs_.rbegin()->first;

  if (jobs_.size() > max_keep_jobs) {
    auto delete_until_key = last_key - max_keep_jobs;
    if (first_key < delete_until_key) {
      for (auto i = first_key; i < delete_until_key; i++) {
        jobs_.erase(i);
      }
    }
  }

  first_key = jobs_.begin()->first;

  for (size_t i = first_key; i <= last_key; i++) {
    const auto &job = jobs_[i];
    int queued = 0;
    int rendering = 0;
    int rendered = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin = job.chunks[0].render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end = job.chunks[0].render_begin;
    for (const auto &chunk : job.chunks) {
      if (chunk.state == job_state::queued) queued++;
      if (chunk.state == job_state::rendering) rendering++;
      if (chunk.state == job_state::rendered) rendered++;
      render_begin = std::min(render_begin, chunk.render_begin);
      render_end = std::max(render_end, chunk.render_end);
    }
    auto s = job_state::rendering;
    if (rendered > 0) {
      s = job_state::rendered;
    }
    if (queued == job.chunks.size()) {
      s = job_state::queued;
    }
    if (job.skipped) {
      s = job_state::skipped;
    }
    std::stringstream ss;
    switch (s) {
      case job_state::queued:
      case job_state::skipped:
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Gen.Time: " << time_diff(job.generate_begin, job.generate_end) << " ";
        break;
      case job_state::rendering:
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Gen.Time: " << time_diff(job.generate_begin, job.generate_end) << " "
           << "Render Time: " << time_diff(render_begin, std::chrono::high_resolution_clock::now()) << " ";
        break;
      case job_state::rendered:
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Gen.Time: " << time_diff(job.generate_begin, job.generate_end) << " "
           << "Render Time: " << time_diff(render_begin, render_end) << " ";
        hack_last_render_time = time_diff(render_begin, render_end);
        break;
    }
    if (job.chunks.size() > 1) {
      if (mode == modes::global_mode) {
        ss << "Chunks: ";
        for (const auto &chunk : job.chunks) {
          ss << "" << chunk.state;
        }
        f2(ss.str());
        ss.clear();
        ss.str("");
      } else if (mode == modes::frame_mode) {
        f2(ss.str());
        ss.clear();
        ss.str("");
        for (const auto &chunk : job.chunks) {
          ss << " Chunk: " << chunk.chunk << " of " << chunk.num_chunks << " " << str(chunk.state) << " ";
          switch (chunk.state) {
            case job_state::queued:
            case job_state::skipped:
              break;
            case job_state::rendering:
              ss << " Rendering T: " << time_diff(chunk.render_begin, std::chrono::high_resolution_clock::now()) << " ";
              break;
            case job_state::rendered:
              ss << " Rendered T: " << time_diff(chunk.render_begin, chunk.render_end) << " ";
              break;
          }
          for (const auto &obj : chunk.objects) {
            ss << obj.state;
          }
          f2(ss.str());
          ss.clear();
          ss.str("");
        }
        f2(ss.str());
        ss.clear();
        ss.str("");
      }
    } else {
      f2(ss.str());
      ss.clear();
      ss.str("");
    }
  }
  for (auto [level, str] : _output) {
    f3(level, str);
  }
  _output.clear();
}

void metrics::clear() {
  std::unique_lock<std::mutex> lock(mut);
  jobs_.clear();
}

void metrics::set_frame_mode() {
  mode = metrics::modes::frame_mode;
  record_objects = true;
}

void metrics::set_total_frames(size_t frames) {
  completed_frames = 0;
  max_frames = frames;
}

void metrics::log_callback(int level, const std::string &line) {
  std::unique_lock<std::mutex> lock(mut);
  _output.emplace_back(std::make_pair(level, line));
}

double metrics::time_diff(std::chrono::time_point<std::chrono::high_resolution_clock> begin,
                          std::chrono::time_point<std::chrono::high_resolution_clock> end) {
  std::chrono::duration<double, std::milli> diff = end - begin;
  return diff.count() / 1000.;
}