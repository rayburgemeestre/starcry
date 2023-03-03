/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "frame_sampler.h"

namespace interpreter {

frame_sampler::frame_sampler(generator &) {}

bool frame_sampler::sample(size_t use_fps, std::function<bool(bool)> callback) {
  // no sampling
  if (sample_include == 0 || sample_exclude == 0) {
    return callback(false);
  }
  while (true) {
    // sampling frames to include
    if (sample_include_current-- > 0) {
      return callback(false);
    }
    // frames to be skipped
    while (sample_exclude_current-- > 0) {
      total_skipped_frames++;
      bool ret = callback(true);
      if (!ret) {
        // bail out if we find a last frame
        return false;
      }
    }
    // reset
    sample_include_current = sample_include * static_cast<double>(use_fps);
    sample_exclude_current = sample_exclude * static_cast<double>(use_fps);
  }
  return true;
}
}  // namespace interpreter