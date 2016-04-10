#include "actors/render_window.h"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "caf/io/all.hpp"
#include "rendering_engine.hpp"
#include "util/settings.hpp"

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
    auto queue = al_create_event_queue();
    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_GENERATE_EXPOSE_EVENTS);
    display = al_create_display(1920, 1080); // TODO: read this from -d CLI parameter
    al_clear_to_color(al_map_rgb_f(1, 0, 0));
    al_register_event_source(queue, al_get_display_event_source(display));
    al_flip_display();
    if (!display) {
        fprintf(stderr, "failed to create display!\n");
        return {};
    }
    return {
        [=](render) {
            al_clear_to_color(al_map_rgb_f(0, 0, 0));
            rendering_engine re;
            if (data_.pixels.size()) {
                auto bmp = re.unserialize_bitmap(data_.pixels, width_, height_);
                al_draw_scaled_bitmap(bmp,
                                      0, 0, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
                                      0, 0, al_get_display_width(display), al_get_display_height(display),
                                      0);
                al_destroy_bitmap(bmp);
            }
            ALLEGRO_EVENT event;
            al_wait_for_event_timed(queue, &event, 0.001);
            if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
               al_acknowledge_resize(event.display.source);
            }
            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                al_unregister_event_source(queue, al_get_display_event_source(display));
                al_destroy_event_queue(queue);
                al_destroy_display(display);
                self->quit(exit_reason::user_shutdown);
                return;
            }
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

uint16_t bind_render_window(event_based_actor* self, uint16_t port) {
    uint16_t bound_port = 0;
    try {
        // first try to bind to specified port
        bound_port = io::publish(self, port);
        cout << "succesfully bound to previous port " << port << endl;
    }
    catch (bind_failure &err) {
        cout << "could not publish ourselves on the previously known port." << endl;
        // try to bind to an available (unprivileged) port
        try {
            bound_port = io::publish(self, 0);
        }
        catch (bind_failure &err) {
            cout << "could not publish ourselves on a new port either." << endl;
        }
    }
    return bound_port;

}

behavior render_window(event_based_actor* self, uint16_t port) {
    uint16_t bound_port = bind_render_window(self, port);
    if (bound_port == 0) {
        self->quit(exit_reason::user_shutdown);
    }
    cout << "publishing GUI on port: " << bound_port << endl;

    settings conf;
    conf.user.gui_port = bound_port;
    conf.save();

    auto renderloop = spawn(render_loop);
    self->link_to(renderloop);
    data::pixel_data data;
    self->send(renderloop, render::value);

    return {
        [=](uint32_t width, uint32_t height, struct data::pixel_data data) -> message {
            self->send(renderloop, width, height, data);
            return make_message();
        },
        on("ping") >> [](string) -> message {
            return make_message("pong");
        }
    };
}
