/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// class MeasureInterval;

#include <string>

#include <cstddef>
#include <cstdint>

#include "data/job.hpp"

class generator {
private:
  uint32_t canvas_w;
  uint32_t canvas_h;
  size_t current_job = 0;
  size_t num_chunks = 0;
  size_t lines_received = 0;
  int64_t jobs_queued_for_renderer = 0;
  int64_t max_jobs_queued_for_renderer = 1;
  bool has_max_jobs_queued_for_renderer = false;
  //  std::optional<actor> renderer_ptr;
  //  std::shared_ptr<MeasureInterval> fps_counter;
  size_t bitrate = 0;
  bool use_stdin = false;
  size_t use_fps = 25;
  bool realtime = false;
  std::function<bool(const data::job &)> on_new_job;
  std::function<void(size_t, int, int, int)> on_initialized;
  std::optional<size_t> custom_max_frames;

public:
  generator(std::function<void(size_t, int, int, int)> on_initialized,
            std::function<bool(const data::job &)> on_new_job,
            std::optional<size_t> custom_max_frames = std::nullopt);

  ~generator();

  void init(const std::string &filename);
  bool generate_frame();

  // kind of private handlers..
  void on_output_line(const std::string &s);
  void on_write_frame(data::job &the_job);
};

void call_print_exception(const std::string &fn);
template <typename T>
void call_print_exception(const std::string &fn, T arg);