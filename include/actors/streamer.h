/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "common.h"

constexpr static int streamer_ffmpeg = 0;
constexpr static int streamer_allegro5 = 1;

class MeasureInterval;

struct streamer_data
{
    int render_window_at;
    std::string output_file;
    uint32_t settings;
    std::shared_ptr<MeasureInterval> fps_counter;
    size_t num_pixels = 0;
    size_t min_items_in_streamer_queue = 10;
    size_t current_frame = 0;
};

behavior streamer(stateful_actor<streamer_data> * self, std::optional<size_t> port);

