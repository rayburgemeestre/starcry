/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry/metrics.h"
#include "util/logger.h"
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#else
#pragma GCC diagnostic push
// doesn't work, see: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#include "tvision/tstarcry.h"
#ifdef __clang__
// no idea why this pop does not work
// #pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

#include "starcry.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

metrics::metrics(bool notty, std::function<void()> toggle_preview_callback)
    : notty(notty), toggle_preview_callback_(toggle_preview_callback) {
  // caller needs to call init, so we can use shared_from_this()
}

void metrics::set_script(const std::string &script) {
  script_ = script;
  if (program && !program->exited) {
    program->setScript(script_);
  }
}

void metrics::init() {
  curses = std::thread([&, this]() {
    if (notty) {
      initialized = true;
      std::unique_lock<std::mutex> lock(cv_mut);
      cv.wait(lock, [&] {
        return ready;
      });
    } else {
      program = std::make_shared<TStarcry>(this, toggle_preview_callback_);
      program->setScript(script_);
      initialized = true;
      program->run();
      program->shutDown();
      program->suspend();
      if (program->user_exited) {
        // Setup handler
        auto signal_handler = [](int signal) {
          if (signal == SIGABRT) {
            logger(INFO) << "User exited the UI" << std::endl;
            std::_Exit(EXIT_SUCCESS);
          } else {
            logger(WARNING) << "Unexpected signal " << signal << " received" << std::endl;
          }
          std::_Exit(EXIT_FAILURE);
        };
        auto previous_handler = std::signal(SIGABRT, signal_handler);
        if (previous_handler == SIG_ERR) {
          std::_Exit(EXIT_FAILURE);
        }
        std::abort();
      }
    }
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
  json result = {};
  if (!initialized) return result.dump();
  std::unique_lock<std::mutex> lock(mut);

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
    size_t first_key = jobs_.begin()->first;
    size_t last_key = jobs_.rbegin()->first;

    if (jobs_.size() > static_cast<size_t>(max_keep_jobs)) {
      auto delete_until_key = last_key - max_keep_jobs;
      if (first_key < delete_until_key) {
        for (auto i = first_key; i < delete_until_key; i++) {
          jobs_.erase(i);
        }
      }
    }

    first_key = jobs_.begin()->first;

    for (size_t i = first_key; i <= last_key; i++) {
      // TODO: this prevents a crash, but figure out why it crashed
      if (!jobs_.contains(i)) continue;
      const auto &job = jobs_[i];
      size_t queued = 0;
      // size_t rendering = 0;
      size_t rendered = 0;
      std::chrono::time_point<std::chrono::high_resolution_clock> render_begin = job.chunks[0].render_begin;
      std::chrono::time_point<std::chrono::high_resolution_clock> render_end = job.chunks[0].render_begin;
      for (const auto &chunk : job.chunks) {
        if (chunk.state == job_state::queued) queued++;
        // if (chunk.state == job_state::rendering) rendering++;
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
  // for (const auto &line : _output) {
  //   if (line.first <= 32) {
  //     std::cout << line.second;
  //   }
  // }
}

void metrics::register_thread(int number, std::string desc) {
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  threads_[number] = metrics::thread_data{number,
                                          std::move(desc),
                                          metrics::thread_state::idle,
                                          std::chrono::high_resolution_clock::now(),
                                          std::chrono::high_resolution_clock::now()};
}

void metrics::register_job(int number, int frame, int chunk, int num_chunks) {
  if (!initialized) return;
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
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    jobs_[number].generate_end = std::chrono::high_resolution_clock::now();
  }
}

void metrics::skip_job(int number) {
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    jobs_[number].skipped = true;
  }
}

void metrics::render_job(int thread_number, int job_number, int chunk) {
  if (!initialized) return;
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
    if (size_t(chunk) >= jobs_[job_number].chunks.size()) return;
    jobs_[job_number].chunks[chunk].render_begin = std::chrono::high_resolution_clock::now();
    jobs_[job_number].chunks[chunk].state = metrics::job_state::rendering;
  }
}

void metrics::resize_job(int number, int num_chunks) {
  if (!initialized) return;
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
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  // TODO: Possibly do not need thread number ????
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) chunk--;
    for (int i = 0; i < num_objects; i++) {
      if (jobs_[job_number].chunks.size() <= (size_t)chunk) continue;
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
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  // TODO: Possibly do not need thread number ????
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) chunk--;
    if (jobs_[job_number].chunks.size() <= size_t(chunk)) return;
    if (jobs_[job_number].chunks[chunk].objects.size() <= size_t(index)) return;
    jobs_[job_number].chunks[chunk].objects[index].state = state;
    if (state == metrics::job_state::rendering) {
      jobs_[job_number].chunks[chunk].objects[index].render_begin = std::chrono::high_resolution_clock::now();
    } else {
      jobs_[job_number].chunks[chunk].objects[index].render_end = std::chrono::high_resolution_clock::now();
    }
  }
}

void metrics::set_steps(int job_number, int attempt, int max_steps) {
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(job_number) != jobs_.end()) {
    generate_step_stats stats;
    stats.status = generate_step_stats::status_type::started;
    stats.attempt_start = std::chrono::high_resolution_clock::now();
    stats.attempt_last = std::chrono::high_resolution_clock::now();
    stats.max_steps = max_steps;
    jobs_[job_number].generate_steps.stats_per_attempt[attempt] = stats;
  }
}

void metrics::update_steps(int job_number, int attempt, int next_step) {
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(job_number) != jobs_.end()) {
    auto found_attempt = jobs_[job_number].generate_steps.stats_per_attempt.find(attempt);
    if (found_attempt == jobs_[job_number].generate_steps.stats_per_attempt.end()) {
      return;  // prevent crash
    }
    auto &stats = found_attempt->second;
    stats.status = generate_step_stats::status_type::ended;

    stats.attempt_last = stats.attempt_end;
    stats.attempt_end = std::chrono::high_resolution_clock::now();
    stats.current_step = next_step;
  }
}

void metrics::complete_render_job(int thread_number, int job_number, int chunk) {
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  if (threads_.find(thread_number) != threads_.end()) {
    threads_[thread_number].idle_begin = std::chrono::high_resolution_clock::now();
    threads_[thread_number].state = metrics::thread_state::idle;
  }
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) {
      chunk--;
    }
    if (size_t(chunk) >= jobs_[job_number].chunks.size()) return;
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
  const auto flush = [&](auto &fun, std::stringstream &ss) {
    std::string item;
    while (std::getline(ss, item, '\n')) {
      fun(item);
    }
    ss.clear();
    ss.str("");
  };

  if (!initialized) return;
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
  size_t first_key = jobs_.begin()->first;
  size_t last_key = jobs_.rbegin()->first;

  if (jobs_.size() > size_t(max_keep_jobs)) {
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
    size_t queued = 0;
    // size_t rendering = 0;
    size_t rendered = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin = job.chunks[0].render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end = job.chunks[0].render_begin;
    for (const auto &chunk : job.chunks) {
      if (chunk.state == job_state::queued) queued++;
      // if (chunk.state == job_state::rendering) rendering++;
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
      case job_state::queued: {
        const auto &attempts = job.generate_steps.stats_per_attempt;
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Generating attempt #" << attempts.size() << "\n";
        for (size_t attempt_num = 1; attempt_num <= attempts.size(); attempt_num++) {
          const auto &stats = attempts.at(attempt_num);
          ss << "#" << attempt_num << " Step: " << stats.current_step << "/" << stats.max_steps
             << " Busy.Time: " << time_diff(stats.attempt_start, stats.attempt_end) << " / "
             << time_diff(stats.attempt_last, stats.attempt_end) << std::endl;
        }
        break;
      }
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
        flush(f2, ss);
      } else if (mode == modes::frame_mode) {
        flush(f2, ss);
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
          flush(f2, ss);
        }
        flush(f2, ss);
      }
    } else {
      flush(f2, ss);
    }
  }
  if (program && !program->exited) {
    program->setProgress(completed_frames, max_frames);
  }

  for (const auto &[level, str] : _output) {
    f3(level, str);
  }
  _output.clear();
}

void metrics::clear() {
  if (!initialized) return;
  std::unique_lock lock{mut};
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
  if (!initialized) return;
  std::unique_lock<std::mutex> lock(mut);
  _output.emplace_back(std::make_pair(level, line));
}

double metrics::time_diff(std::chrono::time_point<std::chrono::high_resolution_clock> begin,
                          std::chrono::time_point<std::chrono::high_resolution_clock> end) {
  std::chrono::duration<double, std::milli> diff = end - begin;
  return diff.count() / 1000.;
}
