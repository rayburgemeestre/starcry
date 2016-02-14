#pragma once

#include <vector>
#include <allegro5/color.h>

namespace caf {
    class event_based_actor;
    class actor;
}

class allegro5_window
{
public:
    caf::event_based_actor *self_;
    caf::actor client_;

    void initialize(uint32_t canvas_w, uint32_t canvas_h, caf::event_based_actor* self, int port);
    void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<ALLEGRO_COLOR> &pixels);
    void finalize();
};