#include "allegro5/allegro.h"

template <class output>
class output_action
{
private:
    bool enabled_;
    output output_;

public:
    output_action() : enabled_(false) {}

    void enable(bool enable) {
        enabled_ = enable;
    }
    void initialize(event_based_actor* self, int port) {
        if (!enabled_) return;
        output_.initialize(self, port);
    }
    void add_frame(std::vector<ALLEGRO_COLOR> &pixels) {
        if (!enabled_) return;
        output_.add_frame(pixels);
    }
    void finalize() {
        if (!enabled_) return;
        output_.finalize();
    }
};

// TODO: eventually this should move outside..
#include "ffmpeg/h264_encode.h"
#include "util/allegro5_window.h"

class output_aggregate {
private:
    // TODO: see if we can put this in one container with boost::hana
    output_action<ffmpeg_h264_encode> &ffmpeg_;
    output_action<allegro5_window> &allegro5_;

public:
    output_aggregate(decltype(ffmpeg_) ffmpeg, decltype(allegro5_) allegro5)
        : ffmpeg_(ffmpeg), allegro5_(allegro5) {}

    void initialize(event_based_actor* self, int port) {
        ffmpeg_.initialize(self, port);
        allegro5_.initialize(self, port);
    }
    void add_frame(std::vector<ALLEGRO_COLOR> &pixels) {
        ffmpeg_.add_frame(pixels);
        allegro5_.add_frame(pixels);
    }
    void finalize() {
        ffmpeg_.finalize();
        allegro5_.finalize();
    }
};
