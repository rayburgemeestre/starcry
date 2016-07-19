/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "common.h"
#include "data/pixels.hpp"
#include "streamer_output/allegro5_window.h"
#include "rendering_engine.hpp"
#include "caf/io/all.hpp"

allegro5_window::allegro5_window(actor_system &system)
    : system_(system), client_(nullptr)
{
}

void allegro5_window::initialize(uint32_t canvas_w, uint32_t canvas_h, event_based_actor *self, int port) {
    self_ = self;
    auto tmp = system_.middleman().remote_actor("127.0.0.1", port);
    if (!tmp) {
        cerr << "Error spawning remote actor: " << system_.render(tmp.error()) << endl;
    }
    client_ = std::move(make_unique<caf::actor>(*tmp));
}

void allegro5_window::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels)
{
    struct data::pixel_data2 dat;

    dat.pixels = pixels;
    rendering_engine re;
    // TODO: make non-blocking w/frame skipping so streamer won't slow things down
    //self_->sync_send(client_, canvas_w, canvas_h, dat).then([](){
    //    // no-op
    //});

    self_->send(*client_, canvas_w, canvas_h, dat);
}

void allegro5_window::finalize()
{
}
