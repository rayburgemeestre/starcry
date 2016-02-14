#include "render_window.h"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "caf/io/all.hpp"
#include "rendering_engine.hpp"

using render                = atom_constant<atom("render    ")>;
ALLEGRO_DISPLAY *display    = NULL;
struct data::pixel_data data_;
uint32_t width_             = 0;
uint32_t height_            = 0;

behavior render_loop(event_based_actor* self) {
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize allegro!\n");
        return {};
    }
    if (!al_init_image_addon()) {
        fprintf(stderr, "Failed to initialize allegro!\n");
        return {};
    }
    display = al_create_display(1280, 720);
    al_clear_to_color(al_map_rgb_f(1, 0, 0));
    al_flip_display();
    if (!display) {
        fprintf(stderr, "failed to create display!\n");
        return {};
    }
    return {
        [=](render) {
            al_clear_to_color(al_map_rgb_f(1, 1, 0));
            rendering_engine re;
            al_lock_bitmap(al_get_target_bitmap(), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
            if (data_.pixels.size())
                re.unserialize_bitmap_(data_.pixels, width_, height_);
            al_unlock_bitmap(al_get_target_bitmap());
            al_flip_display();
            //using namespace std::literals;
            //std::this_thread::sleep_for(0.1s);
            self->send(self, render::value);
        },
        [=](uint32_t width, uint32_t height, struct data::pixel_data data) {
            std::swap(width_, width);
            std::swap(height_, height);
            std::swap(data_.pixels, data.pixels);
        }
    };
}

behavior render_window(event_based_actor* self, int port) {
    io::publish(self, port);

    auto renderloop = spawn(render_loop);
    data::pixel_data data;
    self->send(renderloop, render::value);

    return {
        [=](uint32_t width, uint32_t height, struct data::pixel_data data) -> message {
            self->send(renderloop, width, height, data);
            return make_message();
        }
    };
}


