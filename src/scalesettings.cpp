/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "scalesettings.h"

void scale_settings::reset() {
  video_scales.clear();
  video_scales.push_back(video_scale);
}

void scale_settings::update() {
  video_scale_intermediate = video_scale_next;
  video_scales.push_back(video_scale_intermediate);
}

void scale_settings::revert() {
  video_scale_next = video_scale;
  video_scale_intermediate = video_scale;
  video_scales.clear();
}

void scale_settings::commit() {
  video_scale = video_scale_next;
  video_scales.push_back(video_scale_next);
}
