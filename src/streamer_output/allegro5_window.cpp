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

allegro5_window::allegro5_window(actor_system &system, event_based_actor *self, int port)
    : system_(system), client_(nullptr), self_(self), port_(port)
{
    std::cout << "connecting to port: " << port_ << endl;
    auto tmp = system_.middleman().remote_actor("127.0.0.1", port_);
    if (!tmp) {
        cerr << "Error spawning remote actor: " << system_.render(tmp.error()) << endl;
    }
    client_ = std::move(make_unique<caf::actor>(*tmp));
}

//allegro5_window::allegro5_window(allegro5_window &&other)
//    : system_(other.system_),
//      client_(std::move(other.client_)),
//      self_(std::move(other.self_)),
//      port_(std::move(other.port_))
//{
//}

volatile bool frame_in_transit = false;
void allegro5_window::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels)
{
    // TODO: is a volatile bool enough in this case?
    if (!frame_in_transit) {
        frame_in_transit = true;
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
