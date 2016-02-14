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

        /*
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                if (y <= 1)
                    al_put_pixel(x, y, al_map_rgba(0, 255, 0, 255));
                else if (x <= 1)
                    al_put_pixel(x, y, al_map_rgba(255, 0, 0, 255));
                else {
                    double diffFromCenter = distance<double>(x, shape.x, y, shape.y) * shape.radius;
                    al_put_pixel(x, y, al_map_rgba(diffFromCenter, diffFromCenter, diffFromCenter, 255) );
                }
            }
        }
         */
        scale_ = 1.0;
        width_ = canvas_w;
        height_ = canvas_h;
        for (auto shape : shapes) {
            shape.x -= offset_x;
            shape.y -= offset_y;
            shape.x += canvas_w / 2;
            shape.y += canvas_h / 2;
            render_circle<double>(shape.x, shape.y, shape.radius, shape.radius_size);
        }
        if (!label.empty()) {
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
            al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, ALLEGRO_ALIGN_LEFT, label.c_str());
        }
        // TODO: do this with RAII
        al_unlock_bitmap(bmp);
        al_set_target_bitmap(old_bmp);
    }

    /**
     * Circle drawing function
     *
     * Optimizations used:
     *
     * - Each pixel for the circle eventually will get a color depending on the distance that pixel has from the
     *   circle center.
     *
     *   - Calculating which pixels to do the calculation for requires computing and to avoid some calculations we
     *     process only a quarter of the circle and derive the other three quarters from it.
     *
     *   - As Circles can be positioned on non-integer coordinates we cannot always re-use the distance calculation
     *     for the three other quarters, as there can be subtle differences.
     *     In case the floating point values contain integers the optimization will be used (reuseSqrt=true).
     *
     * - We will also focus on processing only the pixels visible around it's edges. If the radius is 100 and the
     *   radius size is only 5 pixels, we we only check the pixels between circle radii 97 (inner-) and 103 (outer
     *   circle). For this we calculate the half of the "x chord length" for both the outer- and inner circle, and
     *   substract them.
     */
    template <typename double_type>
    inline void render_circle(double_type circleX, double_type circleY, double_type radius, double_type radiusSize)
    {
        circleX             = (circleX * scale_);// + centerX_;
        circleY             = (circleY * scale_);// + centerY_;
        radius             *= scale_;
        radiusSize         *= scale_;

        bool reuseSqrt      = floor(circleX) == circleX && floor(circleY) == circleY;

        // There is a {-1, +1} for compensating rounding that occurs with floating point.
        int radiusOuterCircle = round_to_int<double_type>(radius + radiusSize + 1);
        int radiusInnerCircle = round_to_int<double_type>(radius - radiusSize - 1);

        for (int relY=0; relY<radiusOuterCircle; relY++) {
            int absYTop    = static_cast<int>(circleY - relY);
            int absYBottom = static_cast<int>(circleY + relY);

            if ((absYTop < 0) && (absYBottom > static_cast<int>(height_)))
                break;

            int hxcl_outer = half_chord_length<decltype(radiusOuterCircle), double_type>(radiusOuterCircle, relY);
            int hxcl_inner = 0;

            if (radiusInnerCircle >= relY)
                hxcl_inner = half_chord_length<decltype(radiusInnerCircle), double_type>(radiusInnerCircle, relY);

            for (int relX = hxcl_inner; relX < hxcl_outer; relX++) {
                int absXLeft  = static_cast<int>(circleX - relX);
                int absXRight = static_cast<int>(circleX + relX);

                if (absXLeft < 0 && absXRight > static_cast<int>(width_))
                    continue;

                double_type diffFromCenter = reuseSqrt ? distance<double_type>(absXLeft, circleX, absYTop, circleY) : -1;

                render_circle_pixel(radius, radiusSize, circleX, circleY, absXLeft, absYTop, diffFromCenter);

                if (relY != 0)
                    render_circle_pixel(radius, radiusSize, circleX, circleY, absXLeft, absYBottom, diffFromCenter);
                if (relX != 0)
                    render_circle_pixel(radius, radiusSize, circleX, circleY, absXRight, absYTop, diffFromCenter);
                if (relX != 0 && relY != 0)
                    render_circle_pixel(radius, radiusSize, circleX, circleY, absXRight, absYBottom, diffFromCenter);
            }
        }
    }

    template <typename double_type>
    void render_circle_pixel(double_type radius, double_type radiussize, double_type posX, double_type posY, int absX, int absY, double_type diffFromCenter)
    {
        if (absX < 0 || absY < 0 || absX >= static_cast<int>(width_) || absY >= static_cast<int>(height_))
            return;

        if (diffFromCenter == -1)
            diffFromCenter = distance<double_type>(absX, posX, absY, posY);

        double_type bottom = radius - radiussize;
        double_type colorIdxNormalized = (diffFromCenter - bottom) / (radiussize * 2);
        if (colorIdxNormalized > 1.0 || colorIdxNormalized < 0.0)
            return;

        double_type test = fabs(colorIdxNormalized - 0.5);
        double_type Opacity = test * 2.0;

        //engine_.put_pixel_alpha(absX, absY, 255.0 - (test * 255.0), 0, 0, Opacity);
        al_put_pixel(absX, absY, al_map_rgba(0, 255.0 - (test * 255.0), 0, Opacity));
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
    int thread_;
    // temporary engine stuff, will be separate class
    double scale_;
    uint32_t width_;
    uint32_t height_;
};
