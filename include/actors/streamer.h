/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "common.h"

constexpr static int streamer_ffmpeg = 0;
constexpr static int streamer_allegro5 = 1;

behavior streamer(event_based_actor* self, const caf::actor &job_storage, int render_window_at, std::string output_file, uint32_t settings);

