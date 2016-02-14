#pragma once
#include "common.h"

constexpr static int streamer_ffmpeg = 0;
constexpr static int streamer_allegro5 = 1;

behavior streamer(event_based_actor* self, const caf::actor &job_storage, int render_window_at, uint32_t settings);

