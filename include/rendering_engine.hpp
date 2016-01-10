#pragma once

// #define DEBUGMODE

#include "allegro5/allegro.h"
//#include "allegro5/debug.h"
#include <allegro5/allegro_image.h>

#include <mutex>
using namespace std;

static std::mutex m;

inline void print_bitmap(ALLEGRO_BITMAP *bmp) {
    int flags = al_get_bitmap_flags(bmp);
    std::cout << "bitmap contains: " << endl;
    if (flags & ALLEGRO_MEMORY_BITMAP) std::cout << "ALLEGRO_MEMORY_BITMAP, ";
    if (flags & ALLEGRO_MEMORY_BITMAP) std::cout << "ALLEGRO_MEMORY_BITMAP, ";
    if (flags & _ALLEGRO_KEEP_BITMAP_FORMAT) std::cout << "_ALLEGRO_KEEP_BITMAP_FORMAT, ";
    if (flags & ALLEGRO_FORCE_LOCKING) std::cout << "ALLEGRO_FORCE_LOCKING, ";
    if (flags & ALLEGRO_NO_PRESERVE_TEXTURE) std::cout << "ALLEGRO_NO_PRESERVE_TEXTURE, ";
    if (flags & _ALLEGRO_ALPHA_TEST) std::cout << "_ALLEGRO_ALPHA_TEST, ";
    if (flags & _ALLEGRO_INTERNAL_OPENGL) std::cout << "_ALLEGRO_INTERNAL_OPENGL, ";
    if (flags & ALLEGRO_MIN_LINEAR) std::cout << "ALLEGRO_MIN_LINEAR, ";
    if (flags & ALLEGRO_MAG_LINEAR) std::cout << "ALLEGRO_MAG_LINEAR, ";
    if (flags & ALLEGRO_MIPMAP) std::cout << "ALLEGRO_MIPMAP, ";
    if (flags & _ALLEGRO_NO_PREMULTIPLIED_ALPHA) std::cout << "_ALLEGRO_NO_PREMULTIPLIED_ALPHA, ";
    if (flags & ALLEGRO_VIDEO_BITMAP) std::cout << "ALLEGRO_VIDEO_BITMAP, ";
    if (flags & ALLEGRO_CONVERT_BITMAP) std::cout << "ALLEGRO_CONVERT_BITMAP, ";
    std::cout << ";" << endl;
}

class rendering_engine
{
public:
    void initialize() {
        if (!al_init()) {
            fprintf(stderr, "Failed to initialize allegro!\n");
            return;
        }
        if (!al_init_image_addon()) {
            fprintf(stderr, "Failed to initialize allegro!\n");
            return;
        }
    }

    template <typename image, typename shapes_t>
    void render(image bmp, shapes_t & shapes) {
        std::unique_lock<std::mutex> lock(m);
        al_set_target_bitmap(bmp);
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
        for (auto &shape : shapes) {
            al_clear_to_color(al_map_rgba((int)(shape.radius * 50), (int)(shape.radius * 50), 0, 255));
        }
    }

    template <typename image>
    void write_image(image bmp, std::string prefix) {
        std::unique_lock<std::mutex> lock(m);
        stringstream ss;
        ss << "/tmp/" << prefix << ".bmp";
        bool ret = al_save_bitmap(ss.str().c_str(), bmp);
        if (!ret)
            throw std::runtime_error("rendering_engine::write_image al_save_bitmap() returned false");
    }

    template <typename image>
    vector<ALLEGRO_COLOR> serialize_bitmap(image bitmap, uint32_t width, uint32_t height) {
        vector<ALLEGRO_COLOR> pixels;
        pixels.reserve(width * height);
        al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
        for (uint32_t y=0; y<height; y++) {
            for (uint32_t x=0; x<width; x++) {
                pixels.emplace_back(al_get_pixel(bitmap, x, y));
            }
        }
        al_unlock_bitmap(bitmap);
        return pixels; // RVO
    }

    ALLEGRO_BITMAP * unserialize_bitmap(vector<ALLEGRO_COLOR> &pixels, uint32_t width, uint32_t height) {
        ALLEGRO_BITMAP *bitmap = al_create_bitmap(width, height);
        al_set_target_bitmap(bitmap);
        size_t index = 0;
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                al_put_pixel(x, y, pixels[index++]);
            }
        }
        return bitmap;
    }

    ALLEGRO_DISPLAY *display = NULL;
};
