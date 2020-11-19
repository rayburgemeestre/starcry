/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>

#include <cstddef>
#include <cstdint>

#include "data/job.hpp"

#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

class generator_v2 {
private:
  std::shared_ptr<data::job> job;
  uint32_t frame_number;

  size_t max_frames = 0;
  int32_t canvas_w = 0;
  int32_t canvas_h = 0;
  double granularity = 1;
  size_t use_fps = 25;
  std::unordered_map<std::string, data::gradient> gradients;
  std::unordered_map<size_t, std::map<int, size_t>> indexes;
  int current_step_max = std::numeric_limits<int>::max();

public:
  generator_v2();
  ~generator_v2() = default;

  void init(const std::string &filename);
  void init_context(const std::string &filename);
  void init_user_script(const std::string &filename);
  void init_job();
  void init_video_meta_info();
  void init_gradients();
  void init_object_instances();

  bool generate_frame();
  void cleanup_previous_attempt(v8_interact &i,
                                v8::Local<v8::Array> &instances,
                                v8::Local<v8::Array> &next_instances,
                                v8::Local<v8::Array> &intermediates);

  std::shared_ptr<data::job> get_job() const;
  double fps() const {
    return use_fps;
  }
  int32_t width() const {
    return canvas_w;
  }
  int32_t height() const {
    return canvas_h;
  }
};

void call_print_exception(const std::string &fn);
template <typename T>
void call_print_exception(const std::string &fn, T arg);
