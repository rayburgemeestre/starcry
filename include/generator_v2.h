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

class generator_v2 {
private:
  size_t max_frames = 0;
  uint32_t canvas_w = 0;
  uint32_t canvas_h = 0;
  size_t current_job = 0;
  size_t num_chunks = 0;
  size_t bitrate = 0;
  size_t use_fps = 25;
  std::unordered_map<std::string, data::gradient> gradients;

public:
  generator_v2();
  ~generator_v2() = default;

  void init(const std::string &filename);
  bool generate_frame();
  std::shared_ptr<data::job> get_job() const;
};

void call_print_exception(const std::string &fn);
template <typename T>
void call_print_exception(const std::string &fn, T arg);