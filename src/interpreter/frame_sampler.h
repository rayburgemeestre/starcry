/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>

namespace interpreter {

class frame_sampler {
public:
  explicit frame_sampler();

  bool sample(size_t use_fps, std::function<bool(bool)> callback);

  void set_sample_include(double sample_include, double use_fps) {
    this->sample_include = sample_include;
    this->sample_include_current = sample_include * use_fps;
  }
  void set_sample_exclude(double sample_exclude, double use_fps) {
    this->sample_exclude = sample_exclude;
    this->sample_exclude_current = sample_exclude * use_fps;
  }

private:
  double sample_include = 0.;
  double sample_exclude = 0.;
  double sample_include_current = 0.;
  double sample_exclude_current = 0.;
  double total_skipped_frames = 0.;
};
}  // namespace interpreter