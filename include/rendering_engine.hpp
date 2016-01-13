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

template <typename T>
constexpr T squared(T num) {
    return (num * num);
}
template <typename T>
constexpr T squared_dist(T num, T num2) {
    return (num - num2) * (num - num2);
}

template <typename T>
constexpr T distance(T x1, T x2, T y1, T y2) {
    return sqrt(squared_dist<T>(x1, x2) + squared_dist<T>(y1, y2));
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
    void render(image bmp, shapes_t & shapes, uint32_t offset_x, uint32_t offset_y) {
        std::unique_lock<std::mutex> lock(m);
        al_set_target_bitmap(bmp);
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
        uint32_t width = al_get_bitmap_width(bmp);
        uint32_t height = al_get_bitmap_height(bmp);
        // size_t index = 0;
        auto shape = shapes[0]; // just for some test effect
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                if (y <= 1)
                    al_put_pixel(x, y, al_map_rgba(0, 255, 0, 255));
                else if (x <= 1)
                    al_put_pixel(x, y, al_map_rgba(255, 0, 0, 255));
                else {
                    double diffFromCenter = distance<double>(offset_x, x, offset_y, y) * shape.radius;
                    al_put_pixel(x, y, al_map_rgba(diffFromCenter, diffFromCenter, diffFromCenter, 255));
                }
            }
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
