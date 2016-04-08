#pragma once

// #define DEBUGMODE

#include "allegro5/allegro.h"
//#include "allegro5/debug.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <iostream>
#include <sstream>
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

template <typename T, typename double_type>
constexpr int half_chord_length(T radiusOuterCircle, T relativeY) {
    return static_cast<int>(sqrt(static_cast<double_type>(squared(radiusOuterCircle) - squared(relativeY))) + 0.5);
}
template<typename double_type>
inline int round_to_int(double_type in)
{
    return static_cast<int>(0.5 + in);
}

#include "draw_logic.hpp"
#include "data/shape.hpp"

class rendering_engine
{
public:
    void initialize() {
        if (!al_init()) {
            fprintf(stderr, "Failed to initialize allegro!\n");
            return;
        }
        if (!al_init_image_addon()) {
            fprintf(stderr, "Failed to initialize allegro image addon!\n");
            return;
        }
        if (!al_init_font_addon()) {
            fprintf(stderr, "Failed to initialize allegro font addon!\n");
            return;
        }
        if (!al_init_ttf_addon()) {
            fprintf(stderr, "Failed to initialize allegro ttf addon!\n");
            return;
        }
        if (!al_init_primitives_addon()) {
            fprintf(stderr, "Failed to initialize allegro primitives addon!\n");
            return;
        }
    }

    template <typename image, typename shapes_t>
    void render(image bmp, shapes_t & shapes, uint32_t offset_x, uint32_t offset_y, uint32_t canvas_w, uint32_t canvas_h, std::string label = "") {
        std::unique_lock<std::mutex> lock(m);
        auto old_bmp = al_get_target_bitmap();
        al_set_target_bitmap(bmp);
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
        uint32_t width = al_get_bitmap_width(bmp);
        uint32_t height = al_get_bitmap_height(bmp);
        al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);

        if (!font) {
            font = al_load_ttf_font("Monaco_Linux-Powerline.ttf", 14, 0);
            if (!font){
                fprintf(stderr, "Could not load monaco ttf font.\n");
                // TODO: do this with RAII
                al_unlock_bitmap(bmp);
                al_set_target_bitmap(old_bmp);
                return;
            }
        }

        double scale = 3.0;
        draw_logic_.scale(scale);
        draw_logic_.width(canvas_w);
        draw_logic_.height(canvas_h);
        draw_logic_.center(canvas_w / 2, canvas_h / 2);
        draw_logic_.offset(offset_x, offset_y);
        draw_logic_.font(font);
        for (auto shape : shapes) {
            if (shape.type == data::shape_type::circle)
                draw_logic_.render_circle<double>(shape.x, shape.y, shape.radius, shape.radius_size);
            else if (shape.type == data::shape_type::line)
                draw_logic_.render_line<double>(shape.x, shape.y, shape.x2, shape.y2, shape.radius_size, shape.r, shape.g, shape.b);
            else if (shape.type == data::shape_type::text)
                draw_logic_.render_text<double>(shape.x, shape.y, shape.text, shape.align);
        }
        if (!label.empty()) {
            for (uint32_t y = 0; y < height; y++) al_put_pixel(0, y, al_map_rgba(0, 255, 0, 255));
            for (uint32_t x = 0; x < width; x++)  al_put_pixel(x, 0, al_map_rgba(255, 0, 0, 255));
            al_draw_text(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, label.c_str());
        }
        // TODO: do this with RAII
        al_unlock_bitmap(bmp);
        al_set_target_bitmap(old_bmp);
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
        auto old_bmp = al_get_target_bitmap();
        al_set_target_bitmap(bitmap);
        al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
        unserialize_bitmap_(pixels, width, height);
        al_unlock_bitmap(bitmap);
        // TODO: do this with RAII
        al_set_target_bitmap(old_bmp);
        return bitmap;
    }

    inline void unserialize_bitmap_(vector<ALLEGRO_COLOR> &pixels, uint32_t width, uint32_t height) {
        size_t index = 0;
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                al_put_pixel(x, y, pixels[index++]);
            }
        }
        return;
    }

    void run_display_function(std::function<void(ALLEGRO_DISPLAY *)> func) {
        std::unique_lock<std::mutex> lock(m);
        func(display);
    }

    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_FONT *font = NULL;
    draw_logic draw_logic_;
};