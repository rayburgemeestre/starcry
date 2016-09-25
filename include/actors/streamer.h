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

struct rendered_job
{
    size_t frame_number;
    size_t chunk;
    size_t num_chunks;
    bool last_frame;
    std::vector<uint32_t> pixels;

    rendered_job(size_t frame_number,
                 size_t chunk,
                 size_t num_chunks,
                 bool last_frame,
                 std::vector<uint32_t> &pixels)
        : frame_number(frame_number),
          chunk(chunk),
          num_chunks(num_chunks),
          last_frame(last_frame),
          pixels(pixels)
    {}

    bool operator <(const rendered_job &other) const {
        if (frame_number == other.frame_number)
            return chunk < other.chunk;
        return frame_number < other.frame_number;
    }
};

class ffmpeg_h264_encode;
class allegro5_window;

struct streamer_data
{
    int render_window_at;
    std::string output_file;
    uint32_t settings;
    size_t bitrate;
    size_t fps;
    std::shared_ptr<MeasureInterval> fps_counter;
    size_t num_pixels = 0;
    size_t min_items_in_streamer_queue = 10;
    size_t current_frame = 0;
    std::optional<size_t> last_frame_streamed;
    std::set<rendered_job> rendered_jobs_set;
    std::shared_ptr<ffmpeg_h264_encode> ffmpeg;
    std::shared_ptr<allegro5_window> allegro5;
};

behavior streamer(stateful_actor<streamer_data> * self, std::optional<size_t> port);

