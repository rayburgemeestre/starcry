/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "util/image_utils.h"
#include "util/random.hpp"

void copy_to_png(util::random_generator &rand,
                 const std::vector<data::color> &source,
                 uint32_t width,
                 uint32_t height,
                 png::image<png::rgba_pixel> &dest,
                 bool dithering) {
  size_t index = 0;
  auto m = std::numeric_limits<uint8_t>::max();
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      // BGRA -> RGB
      // uint8_t *data = (uint8_t *)&(source[index]);
      // dest[y][x] = png::rgb_pixel(*(data + 2), *(data + 1), *(data + 0));

      // My attempt at "random" dithering, probably inefficient
      double r1 = source[index].r;
      double g1 = source[index].g;
      double b1 = source[index].b;
      double a1 = source[index].a;
      uint8_t r = r1 * m;
      uint8_t g = g1 * m;
      uint8_t b = b1 * m;
      uint8_t a = a1 * m;
      if (dithering) {
        double r_dbl = r1 * m;
        double g_dbl = g1 * m;
        double b_dbl = b1 * m;
        double a_dbl = a1 * m;
        double r_offset = r_dbl - static_cast<double>(r);
        double g_offset = g_dbl - static_cast<double>(g);
        double b_offset = b_dbl - static_cast<double>(b);
        double a_offset = a_dbl - static_cast<double>(a);
        if (rand.get() >= r_offset && r > 0) {
          r -= 1;
        }
        if (rand.get() >= g_offset && g > 0) {
          g -= 1;
        }
        if (rand.get() >= b_offset && b > 0) {
          b -= 1;
        }
        if (rand.get() >= a_offset && a > 0) {
          a -= 1;
        }
      }
      dest[y][x] = png::rgba_pixel(r, g, b, a);
      index++;
    }
  }
}

std::vector<uint32_t> pixels_vec_to_pixel_data(const std::vector<data::color> &pixels_in, const bool &dithering) {
  util::random_generator rand;  // TODO(performance): make this static, in some thread-safe way, just putting static
                                // gives all kinds of weird errors inside the random class..
  std::vector<uint32_t> pixels_out;
  pixels_out.reserve(pixels_in.size());

  if (dithering) {
    for (const auto &pix : pixels_in) {
      uint32_t color;
      char *cptr = (char *)&color;
      auto r = (char)(pix.r * 255);
      auto g = (char)(pix.g * 255);
      auto b = (char)(pix.b * 255);
      auto a = (char)(pix.a * 255);
      double r_dbl = pix.r * 255.;
      double g_dbl = pix.g * 255.;
      double b_dbl = pix.b * 255.;
      double a_dbl = pix.a * 255.;
      double r_offset = r_dbl - static_cast<double>(r);
      double g_offset = g_dbl - static_cast<double>(g);
      double b_offset = b_dbl - static_cast<double>(b);
      double a_offset = a_dbl - static_cast<double>(a);
      if (rand.get() >= r_offset && r > 0) {
        r -= 1;
      }
      if (rand.get() >= g_offset && g > 0) {
        g -= 1;
      }
      if (rand.get() >= b_offset && b > 0) {
        b -= 1;
      }
      if (rand.get() >= a_offset && a > 0) {
        a -= 1;
      }
      *cptr = r;
      cptr++;
      *cptr = g;
      cptr++;
      *cptr = b;
      cptr++;
      *cptr = a;
      pixels_out.push_back(color);
    }
  } else {
    for (const auto &pix : pixels_in) {
      uint32_t color;
      char *cptr = (char *)&color;
      *cptr = (char)(pix.r * 255);
      cptr++;
      *cptr = (char)(pix.g * 255);
      cptr++;
      *cptr = (char)(pix.b * 255);
      cptr++;
      *cptr = (char)(pix.a * 255);
      pixels_out.push_back(color);
    }
  }
  return pixels_out;
}

void pixels_vec_insert_checkers_background(std::vector<uint32_t> &pixels, int width, int height) {
  uint8_t *p = (uint8_t *)&pixels[0];
  int x = 0, y = 0;
  bool flag = true;
  for (size_t i = 0; i < size_t(width * height); i++) {
    uint8_t &r = *(p++);
    uint8_t &g = *(p++);
    uint8_t &b = *(p++);
    uint8_t &a = *(p++);

    data::color checker;
    checker.r = flag ? 0.75 : 1.;
    checker.g = flag ? 0.75 : 1.;
    checker.b = flag ? 0.75 : 1.;
    checker.a = 1.;

    data::color current;
    current.r = double(r) / 255.;
    current.g = double(g) / 255.;
    current.b = double(b) / 255.;
    current.a = double(a) / 255.;

    data::color new_col = blend(checker, current);

    r = new_col.r * 255;
    g = new_col.g * 255;
    b = new_col.b * 255;
    a = new_col.a * 255;

    x++;
    if (x == width) {
      x = 0;
      y++;
      if (y % 20 == 0) {
        flag = !flag;
      }
    }
    if (x % 20 == 0) {
      flag = !flag;
    }
  }
}