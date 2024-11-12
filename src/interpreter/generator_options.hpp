/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>

struct generator_options {
  bool debug = false;
  int custom_width = 0;
  int custom_height = 0;
  double custom_scale = 0;
  std::optional<bool> custom_grain;
  int custom_granularity = 0;
  generator_options() = default;
};

// TODO: separate out to different file

// intermediate state
struct generator_state {
  size_t max_frames = 0;
  int32_t canvas_w = 0;
  int32_t canvas_h = 0;
  double seed = 1;
  generator_state() = default;
};

// configuration
struct generator_config {
  std::string filename;
  double tolerated_granularity = 1;
  bool minimize_steps_per_object = true;
  size_t fps = 25;
  int min_intermediates = 1.;
  int max_intermediates = 30.;
  bool fast_ff = false;
  bool debug;
  bool caching = false;
  generator_config() = default;
};
