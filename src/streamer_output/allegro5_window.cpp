#include "common.h"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "streamer_output/allegro5_window.h"
#include <allegro5/allegro.h>
#include <cstdio>
#include "rendering_engine.hpp"
#include "caf/io/all.hpp"

void allegro5_window::initialize(event_based_actor *self, int port) {
    self_ = self;
    client_ = io::remote_actor("127.0.0.1", port);
}

void allegro5_window::add_frame(std::vector<ALLEGRO_COLOR> &pixels)
{
    uint32_t w = 1280;
    uint32_t h = 720;
    struct data::pixel_data dat;
    dat.pixels = pixels;
    rendering_engine re;
    // TODO: make non-blocking w/frame skipping so streamer won't slow things down
    self_->sync_send(client_, w, h, dat).then([](){
        // no-op
    });
}

void allegro5_window::finalize()
{
}
