/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <util/compress_vector.h>
#include "common.h"
#include "streamer_output/allegro5_window.h"
#include "caf/io/all.hpp"

allegro5_window::allegro5_window(actor_system &system, event_based_actor *self, int port)
    : self_(self), system_(system), client_(nullptr), port_(port)
{
    std::cout << "connecting to port: " << port_ << endl;
    auto tmp = system_.middleman().remote_actor("127.0.0.1", port_);
    if (!tmp) {
        std::cerr << "Error spawning remote actor: " << system_.render(tmp.error()) << endl;
    }
    client_ = std::make_unique<caf::actor>(*tmp);
}

volatile bool frame_in_transit = false;

void allegro5_window::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels)
{
    // TODO: is a volatile bool enough in this case?
    if (!frame_in_transit) {
        frame_in_transit = true;
        compress_vector<uint32_t> cv;
        double compression_rate = 0;
        //auto pixels_copy = pixels;
        cv.compress(&pixels, &compression_rate);
        self_->request(*client_, infinite, canvas_w, canvas_h, pixels).then(
            [](){
                frame_in_transit = false;
            },
            [](error &e) {
                frame_in_transit = false;
            }
        );
    }
}

void allegro5_window::finalize()
{
}
