/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>  // for std::clamp
#include <random>

#include "color_blender.hpp"
#include "data/gradient.hpp"
#include "data/settings.hpp"
#include "data/shape.hpp"
#include "data/texture.hpp"
#include "util/math.h"

static std::mt19937 mt_v3;
void set_rand_seed_v3(double seed) {
  mt_v3.seed(seed);
}
double rand_fun_v3() {
  return (mt_v3() / (double)mt_v3.max());
}

namespace draw_logic {

class coord {
public:
  double x;
  double y;

  coord(double x, double y) : x(x), y(y) {}
};

class line {
public:
  double x;
  double y;
  double x2;
  double y2;
  double size;

public:
  line(double x, double y, double x2, double y2, double size = 1) : x(x), y(y), x2(x2), y2(y2), size(size) {}

  line(coord a, coord b, double size = 1) : x(a.x), y(a.y), x2(b.x), y2(b.y), size(size) {}

  coord from() {
    return coord(x, y);
  }
  coord to() {
    return coord(x2, y2);
  }
  bool vertical() const {
    return x == x2;
  }
  bool horizontal() const {
    return y == y2;
  }
  coord center() const {
    return coord(((x - x2) / 2) + x2, ((y - y2) / 2) + y2);
  }

  double slope() const {
    return (y - y2) / (x - x2);
  }
  double intersect() const {
    // y = mx + b
    double mx = slope() * this->x;
    double y = this->y;
    // see: http://www.purplemath.com/modules/strtlneq.htm
    double b = y - mx;
    return b;
  }
  double angle() {
    double dx = x - x2;
    double dy = y - y2;

    if (dx == 0 && dy == 0) return 0;

    if (dx == 0) {
      if (dy < 0)
        return 270;
      else
        return 90;
    }

    double slope = dy / dx;
    double angle = atan(slope);
    if (dx < 0) angle += pi;

    angle = 180.0 * angle / pi;

    while (angle < 0.0) angle += 360.0;

    return angle;
  }
};

// inline auto constexpr squared_dist(auto num, auto num2) { return (num - num2) * (num - num2); }
template <typename T, typename T2>
inline auto constexpr squared_dist(T &&num, T2 &&num2) {
  return (num - num2) * (num - num2);
}

// new replacements
inline coord move_plus(coord c, double angle, double rotate, double move) {
  double tmpAngle = angle + rotate;  // go left...
  if (tmpAngle > 360.0) tmpAngle -= 360.0;
  double rads = tmpAngle * pi / 180;
  return coord(c.x + move * cos(rads), c.y + move * sin(rads));
}
inline coord move_minus(coord c, double angle, double rotate, double move) {
  double tmpAngle = angle + rotate;  // go left...
  if (tmpAngle < 0.0) tmpAngle += 360.0;
  double rads = tmpAngle * pi / 180;
  return coord(c.x + move * cos(rads), c.y + move * sin(rads));
}
inline coord move(coord c, double angle, double rotate, double move) {
  if (rotate >= 0) {
    return move_plus(c, angle, rotate, move);
  } else {
    return move_minus(c, angle, rotate, move);
  }
}

class draw_logic {
public:
  /**
   * Constructor
   */
  draw_logic() {
#if 1 == 0
    font_.reserve(1024);
    for (size_t i = 0; i < 1024; i++) {
      font_.push_back(nullptr);
    }
#endif
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
   *     In case the floating point values contain integers the optimization will be used (reuse_sqrt=true).
   *
   * - We will also focus on processing only the pixels visible around it's edges. If the radius is 100 and the
   *   radius size is only 5 pixels, we we only check the pixels between circle radii 97 (inner-) and 103 (outer
   *   circle). For this we calculate the half of the "x chord length" for both the outer- and inner circle, and
   *   substract them.
   */
  inline void render_circle(image &bmp, const data::shape &shape, double opacity, const data::settings &settings) {
    double circle_x = ((shape.x * scale_) + center_x_) - offset_x_;
    double circle_y = ((shape.y * scale_) + center_y_) - offset_y_;
    auto radius = shape.radius * scale_ * shape.scale;
    auto radius_size = shape.radius_size * scale_ * shape.scale;

    bool reuse_sqrt = floor(circle_x) == circle_x && floor(circle_y) == circle_y;

    // There is a {-1, +1} for compensating rounding that occurs with floating point.
    int radius_outer_circle = round_to_int(radius + radius_size + 1);
    int radius_inner_circle = round_to_int(radius - radius_size - 1);

    for (int rel_y = 0; rel_y < radius_outer_circle; rel_y++) {
      // Below a -1 has been added, due to an off-by-one error that happens only in case objects are
      // moving at fast(er?) velocity, and becomes noticeable as scanlines if you are rendering with > 1 chunks.
      // I fixed this here, as well as added - 1 to bottom left and right as "abs_y_bottom - 1".
      // In the end this was the easier fixed, for some reason extending the radius_outer_circle with more pixels
      // doesn't have an effect.
      int abs_y_top = static_cast<int>(circle_y - rel_y) - 1;
      int abs_y_bottom = static_cast<int>(circle_y + rel_y);

      if ((abs_y_top < 0) && (abs_y_bottom > static_cast<int>(height_))) break;

      int hxcl_outer = half_chord_length<decltype(radius_outer_circle), double>(radius_outer_circle, rel_y);
      int hxcl_inner = 0;

      if (radius_inner_circle >= rel_y)
        hxcl_inner = half_chord_length<decltype(radius_inner_circle), double>(radius_inner_circle, rel_y);

      for (int rel_x = hxcl_inner; rel_x < hxcl_outer; rel_x++) {
        int abs_x_left = static_cast<int>(circle_x - rel_x);
        int abs_x_right = static_cast<int>(circle_x + rel_x);

        if (abs_x_left < 0 && abs_x_right > static_cast<int>(width_)) continue;

        double diff_from_center = reuse_sqrt ? distance<double>(abs_x_left, circle_x, abs_y_top, circle_y) : -1;

        render_circle_pixel(bmp,
                            shape,
                            radius,
                            radius_size,
                            circle_x,
                            circle_y,
                            abs_x_left,
                            abs_y_top,
                            diff_from_center,
                            opacity,
                            settings);

        // bottom left
        if (rel_y != 0)
          render_circle_pixel(bmp,
                              shape,
                              radius,
                              radius_size,
                              circle_x,
                              circle_y,
                              abs_x_left,
                              abs_y_bottom - 1,  // workaround for off-by-one error
                              diff_from_center,
                              opacity,
                              settings);

        // top right
        if (rel_x != 0)
          render_circle_pixel(bmp,
                              shape,
                              radius,
                              radius_size,
                              circle_x,
                              circle_y,
                              abs_x_right,
                              abs_y_top,
                              diff_from_center,
                              opacity,
                              settings);
        // bottom right
        if (rel_x != 0 && rel_y != 0)
          render_circle_pixel(bmp,
                              shape,
                              radius,
                              radius_size,
                              circle_x,
                              circle_y,
                              abs_x_right,
                              abs_y_bottom - 1,  // workaround for off-by-one error
                              diff_from_center,
                              opacity,
                              settings);
      }
    }
  }

  inline void render_text(double textX, double textY, double textsize, string text, string align) {
    textX = ((textX * scale_) + center_x_) - offset_x_;
    textY = ((textY * scale_) + center_y_) - offset_y_;
#if 1 == 0
    size_t index = textsize * scale_;
    auto alignment =
        align == "center" ? ALLEGRO_ALIGN_CENTER : (align == "left" ? ALLEGRO_ALIGN_LEFT : ALLEGRO_ALIGN_RIGHT);

    if (index >= font_.size()) {
      std::cout << "Cannot read out of font_ bounds with index " << index << "  due to : " << font_.size() << '\n';
      std::exit(1);
    }
    if (font_[index] == nullptr) {
      font_[index] = std::make_unique<memory_font>(memory_font::fonts::monaco, index, 0);
      if (!font_[index]) {
        fprintf(stderr, "Could not load monaco ttf font.\n");
        return;
      }
    }
    al_draw_text(font_[index]->font(),
                 al_map_rgb(255, 255, 255),
                 textX,
                 textY - (index /*font height*/ / 2),
                 alignment,
                 text.c_str());
#endif
  }

  void render_circle_pixel(image &bmp,
                           const data::shape &shape,
                           double radius,
                           double radiussize,
                           double posX,
                           double posY,
                           int absX,
                           int absY,
                           double diffFromCenter,
                           double opacity,
                           const data::settings &settings) {
    if (absX < 0 || absY < 0 || absX >= static_cast<int>(width_) || absY >= static_cast<int>(height_)) return;

    if (diffFromCenter == -1) diffFromCenter = distance<double>(absX, posX, absY, posY);

    double bottom = radius - radiussize;
    double colorIdxNormalized = (diffFromCenter - bottom) / (radiussize * 2);
    if (colorIdxNormalized > 1.0 || colorIdxNormalized < 0.0) return;

    double test = fabs(colorIdxNormalized - 0.5);
    double Opacity = test * 2.0;

    // http://liballeg.org/a5docs/trunk/graphics.html#al_load_bitmap_flags
    // by default Allegro pre-multiplies the alpha channel of an image
    // TODO, check al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
    // TODO, check ALLEGRO_NO_PREMULTIPLIED_ALPHA, because I prefer separate alpha channel
    //       unless it's not recommended for other reasons I don't understand..
    // auto & bg_opacity = Opacity;
    // auto fg_opacity = 1.0 - Opacity;

    if (shape.gradients_.empty()) {
      throw std::runtime_error("gradients cannot not be empty");
    }

    render_pixel(bmp, shape, posX, posY, absX, absY, Opacity, opacity, settings);
  }

  template <typename blending_type_>
  void blend_pixel(image &bmp, const int &absX, const int &absY, const data::color &clr) {
    // auto clr2 = blender<blending_type_>(color(bg.r * (1.0 - clr.a), bg.g * (1.0 - clr.a), bg.b * (1.0 - clr.a), 0.0),
    // color(clr.r * clr.a, clr.g * clr.a, clr.b * clr.a, 1.0));
    auto &bg = bmp.get(absX, absY);
    auto clr2 = blender<blending_type_>(color(bg.r, bg.g, bg.b, bg.a), color(clr.r, clr.g, clr.b, clr.a));
    double r = (bg.r * (1.0 - clr2.get_a())) + (clr2.get_r() * clr2.get_a());
    double g = (bg.g * (1.0 - clr2.get_a())) + (clr2.get_g() * clr2.get_a());
    double b = (bg.b * (1.0 - clr2.get_a())) + (clr2.get_b() * clr2.get_a());
    double a = 0;
    bmp.set(absX, absY, r, g, b, a);
  }

  void render_line(image &bmp, const data::shape &shape, double opacity, const data::settings &settings) {
    auto x1 = ((shape.x * scale_) + center_x_) - offset_x_;
    auto y1 = ((shape.y * scale_) + center_y_) - offset_y_;
    auto x2 = ((shape.x2 * scale_) + center_x_) - offset_x_;
    auto y2 = ((shape.y2 * scale_) + center_y_) - offset_y_;
    auto size = shape.radius_size * scale_ * shape.scale;

    // test
    // al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), size);
    // if (red > 0.2 || green > 0.2 || blue > 0.2)
    //    al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), 2.0);
    // return; // end

    line aline(x1, y1, x2, y2, size);

    /**
     * Create rectangle for aline.
     *
     * With corners:
     *   a                     b
     * x  +-------------------+  x2
     * y  +-------------------+  y2
     *   d                     c
     *
     * As lines:
     *              A
     *  D +-------------------+ B
     *    +-------------------+
     *              C
     */
    // three extra surrounding pixels so we make up for rounding errors and in a diagonal direction
    double radiussizeplusextra = aline.size + 3;
    coord a = move(aline.from(), aline.angle(), 90, radiussizeplusextra);
    coord d = move(aline.from(), aline.angle(), -90, radiussizeplusextra);
    coord b = move(aline.to(), aline.angle(), 90, radiussizeplusextra);
    coord c = move(aline.to(), aline.angle(), -90, radiussizeplusextra);
    a = move(a, line(a, aline.from(), 1).angle(), -90, radiussizeplusextra);
    d = move(d, line(d, aline.from(), 1).angle(), 90, radiussizeplusextra);
    b = move(b, line(b, aline.to(), 1).angle(), 90, radiussizeplusextra);
    c = move(c, line(c, aline.to(), 1).angle(), 90, radiussizeplusextra);
    line A(a, b), B(b, c), C(c, d), D(d, a);

    double top_y = std::min({a.y, b.y, c.y, d.y});
    double bottom_y = std::max({a.y, b.y, c.y, d.y});

    // Make sure we do not iterate unnecessary pixels, the ones outside the canvas.
    if (top_y < 0) top_y = 0;
    if (bottom_y > height_) bottom_y = height_;

    for (int current_y = top_y; current_y <= bottom_y; current_y += 1) {
      // http://www.mathopenref.com/coordintersection.html "When one line is vertical"
      double current_x = 0;
      double intersection_x1 = 0;
      double intersection_x2 = 0;
      bool has_intersection_x1 = false;
      bool has_intersection_x2 = false;

      auto process_intersection = [&](const auto &aline) {
        if (!aline.horizontal() && min(aline.y, aline.y2) <= current_y && max(aline.y, aline.y2) >= current_y) {
          if (aline.vertical()) {  // Todo: change to IsVertical()?
            // Horizontal line, intersection with an infinite line within
            //  y-range, cannot have another X then aline.x or aline.x2
            current_x = aline.x;
          } else {
            current_x = (current_y - aline.intersect()) / aline.slope();
          }
          if (!has_intersection_x1) {
            has_intersection_x1 = true;
            intersection_x1 = current_x;
          } else {
            has_intersection_x2 = true;
            intersection_x2 = current_x;
          }
        }
      };
      process_intersection(A);
      process_intersection(B);
      process_intersection(C);
      process_intersection(D);

      if (has_intersection_x1 && has_intersection_x2) {
        // The following two "TODO" items fix some glitch that has been in the original code for some time as well.
        // This requires some digging as to why the patching was necessary. As long as I don't see any visual artifacts
        // I'm kind of okay with these hacks to fix it
        // Without the fix you will see the final column of pixels in the first row, fixing only the first TODO item
        // will move this column to the last column on the screen, aand the change from <= to < will get rid of this
        // last column
        int x_left = min(intersection_x1, intersection_x2) - 1;  // TODO: figure out why this - 1 is needed!
        int x_right = max(intersection_x1, intersection_x2);
        // Do not loop through unnecessary pixels
        if (x_left < 0) x_left = 0;
        if (x_right > static_cast<int>(width_)) x_right = static_cast<int>(width_);

        for (int x = x_left; x < /* <= */ x_right; x++) {  // TODO#2: figure out why this <= was wrong!
          double dist_pixel = sqrt(squared_dist(x, aline.center().x) + squared_dist(aline.center().y, current_y));
          double dist_max = sqrt(squared_dist(aline.center().x, aline.x2) + squared_dist(aline.center().y, aline.y2));

          double intersect_x = 0;
          double intersect_y = 0;
          // These if-statements probably need some documentation
          // EDIT: not sure if this is correct, as in, doesn't it 'flip' the line (horizontally or vertically??)
          if (aline.angle() == 180 || aline.angle() == 0 || aline.angle() == 360) {
            intersect_x = x;
            intersect_y = aline.y;
          } else if (aline.angle() == 270 || aline.angle() == 90) {
            intersect_x = aline.center().x;
            intersect_y = current_y;
          } else {
            coord tmp1 = move(coord(x, current_y), aline.angle(), 90, aline.size);
            coord tmp2 = move(coord(x, current_y), aline.angle(), 90 + 180, aline.size);

            // It doesn't matter this if this line doesn't cross the center line from all places,
            //  because we handle it like an infinite line (no begin or ending). we just use slope + intersect.
            // Edit: nonetheless the line is now extended in such a way it always does..
            line tmp(tmp1, tmp2);

            // I solved the equation by example from http://www.mathopenref.com/coordintersection.html

            // Cannot calculate intersection if x == x2. No need for it either.
            // The angle probably was 0.0000001 or something, hence the if-statement
            // a few lines above wasn't triggered (if angle == 0.0)...
            // But now after rounding it appears so that x == x2. The small angle
            // didn't influence a difference between x and x2, that's why this 'extra' check
            // is required.
            double nom = (-1 * tmp.intersect()) + aline.intersect();
            double denom = tmp.slope() - aline.slope();
            if (aline.vertical() || tmp.vertical()) {
              intersect_x = tmp.x;
            } else {
              intersect_x = (nom) / (denom);
            }
            // idem..
            if (tmp.vertical()) {
              intersect_y = aline.y;
            } else {
              intersect_y = (intersect_x * tmp.slope()) + tmp.intersect();
            }
          }

          if ((intersect_x >= min(aline.x, aline.x2) && intersect_x <= max(aline.x, aline.x2)) ||
              (intersect_y >= min(aline.y, aline.y2) && intersect_y <= max(aline.y, aline.y2))) {
            double dist_from_center_line = sqrt(squared_dist(x, intersect_x) + squared_dist(current_y, intersect_y));
            double normalized_dist_from_center = (dist_pixel / dist_max);
            double normalized_dist_from_line = (dist_from_center_line / aline.size);
            render_line_pixel(bmp,
                              shape,
                              aline.center().x,
                              aline.center().y,
                              x,
                              current_y,
                              normalized_dist_from_center,
                              normalized_dist_from_line,
                              opacity,
                              settings);
          }
        }
      }
    }

    // al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), size);
    // if (red > 0.2 || green > 0.2 || blue > 0.2)
    //    al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), 2.0);
  }

  void render_line_pixel(image &bmp,
                         const data::shape &shape,
                         double posX,
                         double posY,
                         int absX,
                         int absY,
                         double normalized_dist_from_center,
                         double normalized_dist_from_line,
                         double opacity,
                         const data::settings &settings) {
    if (normalized_dist_from_center > 1.0 || normalized_dist_from_center < 0.0) {
      return;
    }
    if (normalized_dist_from_line > 1.0 || normalized_dist_from_line < 0.0) {
      return;
    }

    // CompiledGradient *ptrGradient = m_object->GetGradient();
    // LColor color = ptrGradient->getColor( (1.0 - normalized_dist_from_line));
    // TODO: make the line modes configurable
    // line mode 1
    // double num = 1.0 - (1.0 - normalized_dist_from_center) * (1.0 - normalized_dist_from_line);
    auto expf = [](double v, double factor) {
      auto max = factor;
      auto maxexp = log(max + 1.0) / log(2.0);
      auto linear = v;
      auto expf = ((pow(2.0, (linear)*maxexp)) - 1.0) / max;
      return expf;
    };
    //    auto logn = [](double v, double factor) {
    //      auto max = factor;
    //      auto maxexp = log(max + 1.0) / log(2.0);
    //      auto linear = v;
    //      auto maxpow = pow(2.0, maxexp);
    //      auto logn = (maxpow - (pow(2.0, (1.0 - linear) * maxexp))) / max;
    //      return logn;
    //    };
    double num = 1.0 - (1.0 - expf(normalized_dist_from_center, 1000)) * (1.0 - normalized_dist_from_line);
    // line mode 2
    // double num = 1.0 - (1.0 - normalized_dist_from_line);
    // num = 1.0 - num; // just to get it to look like it supposed to a bit right now
    // LColor color = ptrGradient->getColor( num );

    // al_put_pixel(absX, absY, al_map_rgba_f(0, 0, (1.0 /*- test*/) * num, 0));
    // auto bg = al_get_pixel(al_get_target_bitmap(), absX, absY);
    // auto &bg = bmp.get(absX, absY);
    // bg.b = (bg.b * num) + 1.0 * (1.0 - num); // we blend ourselves..
    // bg.r = min(1., bg.r + gradient_.get(num).r);
    // bg.g = min(1., bg.g + gradient_.get(num).g);
    // bg.b = min(1., bg.b + gradient_.get(num).b);

    render_pixel(bmp, shape, posX, posY, absX, absY, num, opacity, settings);
  }

  void render_pixel(image &bmp,
                    const data::shape &shape,
                    double posX,
                    double posY,
                    int absX,
                    int absY,
                    double num,  // Opacity in circle pixel fun
                    double opacity,
                    const data::settings &settings) {
    data::color clr{0, 0, 0, 0};
    for (const auto &grad : shape.gradients_) {
      double gradient_opacity = grad.first;
      const auto &the_gradient = grad.second;
      const auto tmp = the_gradient.get(num);
      clr.r += gradient_opacity * tmp.r;
      clr.g += gradient_opacity * tmp.g;
      clr.b += gradient_opacity * tmp.b;
      clr.a += gradient_opacity * tmp.a;
    }
    // TODO: another knob for this? "ignore opacity"
    // sometimes gives a fun effect
    // clr.a = 1.0;
    //    bg.r = clr.r;  // gradient_.get(num).r;
    //    bg.g = clr.g;  // gradient_.get(num).g;
    //    bg.b = clr.b;  // gradient_.get(num).b;

    //---
    // auto max = 10.0;  // TODO: tweak this a bit, or make configurable at least.
    // auto maxexp = log(max + 1.0) / log(2.0);

    auto linear = clr.a * opacity;
    // auto expf = ((pow(2.0, (linear)*maxexp)) - 1.0) / max;

    // auto maxpow = pow(2.0, maxexp);
    // auto logn = (maxpow - (pow(2.0, (1.0 - linear) * maxexp))) / max;
    //---

    // --- perlin noise ---
    double noise = 1.0;
    if (settings.perlin_noise) {
      if (!shape.textures.empty()) {
        noise = 0;
        for (const auto &texture : shape.textures) {
          noise +=
              ::clamp(texture.second.get(
                          num, absX - posX, absY - posY, shape.time, scale_, std::isnan(shape.seed) ? 1. : shape.seed),
                      0.0,
                      1.0);
        }
        noise /= static_cast<double>(shape.textures.size());
      }
    }
    // --- end ---

    // shape opacity
    linear *= shape.opacity;

    // ------------motion blur------------

    // clr.a *= opacity;
    // clr.a *= expf;
    // clr.a *= logn;

    auto rand1 = (rand_fun_v3() * 2.0) - 1.0;  // -1 .. +1
    auto amount_of_blur = 1.0 - opacity;       // i.e. 0.5 blur
    amount_of_blur += settings.extra_grain;    // extra default grain amount
    // clr.a = logn * noise * (1.0 - amount_of_blur * rand1);
    // use noise to dial down opacity
    if (settings.grain_for_opacity) {
      clr.a = (linear * noise) * (1.0 - amount_of_blur * rand1);
      clr.a = ::clamp(clr.a, 0.0, 1.0);
    } else {
      clr.a = linear * noise;
      clr.a = ::clamp(clr.a, 0.0, 1.0);
    }
    // ------------motion blur------------

    // was:
    //  bmp.set(absX, absY, bg.r, bg.g, bg.b, bg.a);

    switch (shape.blending_.type()) {
      case data::blending_type::lighten:
        blend_pixel<lighten>(bmp, absX, absY, clr);
        break;
      case data::blending_type::darken:
        blend_pixel<darken>(bmp, absX, absY, clr);
        break;
      case data::blending_type::multiply:
        blend_pixel<multiply>(bmp, absX, absY, clr);
        break;
      case data::blending_type::average:
        blend_pixel<average>(bmp, absX, absY, clr);
        break;
      case data::blending_type::add:
        blend_pixel<add>(bmp, absX, absY, clr);
        break;
      case data::blending_type::subtract:
        blend_pixel<subtract>(bmp, absX, absY, clr);
        break;
      case data::blending_type::difference:
        blend_pixel<difference>(bmp, absX, absY, clr);
        break;
      case data::blending_type::negation_:
        blend_pixel<negation_>(bmp, absX, absY, clr);
        break;
      case data::blending_type::screen:
        blend_pixel<screen>(bmp, absX, absY, clr);
        break;
      case data::blending_type::exclusion:
        blend_pixel<exclusion>(bmp, absX, absY, clr);
        break;
      case data::blending_type::overlay:
        blend_pixel<overlay>(bmp, absX, absY, clr);
        break;
      case data::blending_type::softlight:
        blend_pixel<softlight>(bmp, absX, absY, clr);
        break;
      case data::blending_type::hardlight:
        blend_pixel<hardlight>(bmp, absX, absY, clr);
        break;
      case data::blending_type::colordodge:
        blend_pixel<colordodge>(bmp, absX, absY, clr);
        break;
      case data::blending_type::colorburn:
        blend_pixel<colorburn>(bmp, absX, absY, clr);
        break;
      case data::blending_type::lineardodge:
        blend_pixel<lineardodge>(bmp, absX, absY, clr);
        break;
      case data::blending_type::linearburn:
        blend_pixel<linearburn>(bmp, absX, absY, clr);
        break;
      case data::blending_type::linearlight:
        blend_pixel<linearlight>(bmp, absX, absY, clr);
        break;
      case data::blending_type::vividlight:
        blend_pixel<vividlight>(bmp, absX, absY, clr);
        break;
      case data::blending_type::pinlight:
        blend_pixel<pinlight>(bmp, absX, absY, clr);
        break;
      case data::blending_type::hardmix:
        blend_pixel<hardmix>(bmp, absX, absY, clr);
        break;
      case data::blending_type::reflect:
        blend_pixel<reflect>(bmp, absX, absY, clr);
        break;
      case data::blending_type::glow:
        blend_pixel<glow>(bmp, absX, absY, clr);
        break;
      case data::blending_type::phoenix:
        blend_pixel<phoenix>(bmp, absX, absY, clr);
        break;
      case data::blending_type::hue:
        blend_pixel<hue>(bmp, absX, absY, clr);
        break;
      case data::blending_type::saturation:
        blend_pixel<saturation>(bmp, absX, absY, clr);
        break;
      case data::blending_type::color:
        blend_pixel<color_blend>(bmp, absX, absY, clr);
        break;
      case data::blending_type::luminosity:
        blend_pixel<luminosity>(bmp, absX, absY, clr);
        break;
      case data::blending_type::normal:
      default:
        blend_pixel<normal>(bmp, absX, absY, clr);
        break;
    }
  };

  void scale(double scale) {
    scale_ = scale;
  }
  void width(uint32_t width) {
    width_ = width;
  }
  void height(uint32_t height) {
    height_ = height;
  }
  void center(double x, double y) {
    center_x_ = x;
    center_y_ = y;
  }
  void offset(double x, double y) {
    offset_x_ = x;
    offset_y_ = y;
  }

private:
  double scale_;
  double center_x_;
  double center_y_;
  double offset_x_;
  double offset_y_;
  uint32_t width_;
  uint32_t height_;
  // std::vector<std::unique_ptr<memory_font>> font_;
};

}  // namespace draw_logic
