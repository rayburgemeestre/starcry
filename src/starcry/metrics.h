/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class TStarcry;

class metrics : std::enable_shared_from_this<metrics> {
public:
  enum thread_state {
    idle,
    busy,
  };
  std::string str(thread_state in) {
    switch (in) {
      case thread_state::idle:
        return "IDLE";
      case thread_state::busy:
        return "BUSY";
    }
    return "ERROR";
  }
  struct thread_data {
    int number;
    std::string name;
    metrics::thread_state state;
    std::chrono::time_point<std::chrono::high_resolution_clock> idle_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> idle_end;
    int job_number;
    int chunk;
  };

  enum job_state {
    queued,
    rendering,
    rendered,
    skipped,
    timeout,
  };
  std::string str(job_state in) {
    switch (in) {
      case job_state::queued:
        return "QUEUED";
      case job_state::rendering:
        return "RENDERING";
      case job_state::rendered:
        return "RENDERED";
      case job_state::skipped:
        return "SKIPPED";
      case job_state::timeout:
        return "TIMEOUT";
    }
    return "";
  }

  struct object {
    int number;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end;
    job_state state;
  };

  struct chunk {
    int chunk;
    int num_chunks;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end;
    metrics::job_state state;
    std::vector<metrics::object> objects;
  };

  struct generate_step_stats {
    enum class status_type {
      started = 0,
      ended = 1,
    } status;
    int max_steps = 1;
    int current_step = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> attempt_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> attempt_last;
    std::chrono::time_point<std::chrono::high_resolution_clock> attempt_end;
  };

  struct generate_steps_stats {
    std::unordered_map<int, generate_step_stats> stats_per_attempt;
  };

  struct job {
    int number;
    int frame;
    int chunk;
    int num_chunks;
    std::chrono::time_point<std::chrono::high_resolution_clock> generate_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> generate_end;
    bool skipped;
    int thread;
    std::vector<metrics::chunk> chunks;
    generate_steps_stats generate_steps;
  };
  enum class modes {
    global_mode,
    frame_mode,
  };
  metrics::modes mode = modes::global_mode;
  std::atomic<bool> record_objects{false};

private:
  std::map<int, metrics::thread_data> threads_;
  std::map<int, metrics::job> jobs_;
  std::recursive_mutex mut;

  int max_keep_jobs = 100;

  bool notty = false;

  size_t completed_frames = 0;
  size_t max_frames;
  double hack_last_render_time = 0.;

  std::string script_ = "input/test.js";
  std::function<void()> toggle_preview_callback_;
  std::string memory_usage_summary_;

public:
  explicit metrics(bool notty, std::function<void()> toggle_preview_callback);
  ~metrics();

  void set_script(const std::string& script);
  void init();
  void register_thread(int number, std::string desc);
  void register_job(int number, int frame, int chunk, int num_chunks);
  void render_job(int thread_number, int job_number, int chunk);
  void resize_job(int job_number, int num_chunks);
  void resize_job_objects(int number, int job_number, int chunk, int num_objects);
  void set_render_job_object_state(int thread_num, int job_num, int chunk_num, int index, metrics::job_state state);
  void set_steps(int job_number, int attempt, int max_steps);
  void update_steps(int job_number, int attempt, int max_steps);
  void complete_render_job(int thread_number, int job_number, int chunk, job_state state = job_state::rendered);
  void complete_job(int number);
  void skip_job(int number);
  void display(std::function<void(const std::string&)> meta_view_print,
               std::function<void(const std::string&)> stdout_view_print,
               std::function<void(int, const std::string&)> ffmpeg_view_print,
               std::function<void(const std::string&)> memory_view_print);
  void display(std::function<void(int, const std::string&)> web_view_print);
  void clear();

  void set_frame_mode();
  void set_total_frames(size_t frames);

  void log_callback(int level, const std::string& line);
  void notify();
  std::string to_json();
  void set_memory_usage_summary(const std::string& memory_usage_summary);

private:
  double time_diff(std::chrono::time_point<std::chrono::high_resolution_clock> begin,
                   std::chrono::time_point<std::chrono::high_resolution_clock> end);

  std::thread curses;
  std::vector<std::pair<int, std::string>> _output;

  std::shared_ptr<TStarcry> program;

  std::mutex cv_mut;
  std::condition_variable cv;
  bool ready = false;
  bool initialized = false;
};
