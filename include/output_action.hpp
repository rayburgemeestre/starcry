/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "allegro5/allegro.h"

namespace caf {
    class actor_system;
}

template <class output>
class output_action
{
private:
    bool enabled_;
    output output_;

public:
    output_action() : enabled_(false) {}

    output_action(caf::actor_system &x) : enabled_(false), output_(x) {}

    void enable(bool enable) {
        enabled_ = enable;
    }
    void initialize(uint32_t canvas_w, uint32_t canvas_h, event_based_actor* self, int port) {
        if (!enabled_) return;
        output_.initialize(canvas_w, canvas_h, self, port);
    }
    void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels) {
        if (!enabled_) return;
        output_.add_frame(canvas_w, canvas_h, pixels);
    }
    void finalize() {
        if (!enabled_) return;
        output_.finalize();
    }
    // TODO: this is no longer really an abstract output, consider refactoring
    void set_filename(std::string filename) {
        if (!enabled_) return;
        output_.set_filename(filename);
    }
    void set_bitrate(size_t bitrate) {
        if (!enabled_) return;
        output_.set_bitrate(bitrate);
    }
};

// TODO: eventually this should move outside..
#include "streamer_output/ffmpeg_encode.h"
#include "streamer_output/allegro5_window.h"

class output_aggregate {
private:
    // TODO: see if we can put this in one container with boost::hana
    output_action<ffmpeg_h264_encode> &ffmpeg_;
    output_action<allegro5_window> &allegro5_;
    bool initialized_ = false;

public:
    output_aggregate(decltype(ffmpeg_) ffmpeg, decltype(allegro5_) allegro5)
        : ffmpeg_(ffmpeg), allegro5_(allegro5) {}

    void initialize(uint32_t canvas_w, uint32_t canvas_h, event_based_actor* self, int port) {
        if (initialized_) return;
        initialized_ = true;
        ffmpeg_.initialize(canvas_w, canvas_h, self, port);
        allegro5_.initialize(canvas_w, canvas_h, self, port);
    }
    void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels) {
        ffmpeg_.add_frame(canvas_w, canvas_h, pixels);
        allegro5_.add_frame(canvas_w, canvas_h, pixels);
    }
    void finalize() {
        ffmpeg_.finalize();
        allegro5_.finalize();
    }
    void set_filename(std::string filename) {
        ffmpeg_.set_filename(filename);
    }
    void set_bitrate(size_t bitrate) {
        ffmpeg_.set_bitrate(bitrate);
    }
};
