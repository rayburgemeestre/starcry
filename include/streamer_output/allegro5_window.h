/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include <allegro5/color.h>

namespace caf {
    class event_based_actor;
    class actor;
    class actor_system;
}

class allegro5_window
{
public:
    allegro5_window(actor_system &system);

    void initialize(uint32_t canvas_w, uint32_t canvas_h, caf::event_based_actor* self, int port);
    void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<ALLEGRO_COLOR> &pixels);
    void finalize();

private:
    caf::event_based_actor *self_;
    caf::actor_system &system_;
    std::unique_ptr<caf::actor> client_;
};