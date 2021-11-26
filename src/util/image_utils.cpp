/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "util/image_utils.h"

// TODO: we need some better project-wide generic solution for pseudo random numbers
extern double rand_fun_vx();

void copy_to_png(const std::vector<data::color> &source,
                 uint32_t width,
                 uint32_t height,
                 png::image<png::rgb_pixel> &dest,
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
      uint8_t r = r1 * m;
      uint8_t g = g1 * m;
      uint8_t b = b1 * m;
      if (dithering) {
        double r_dbl = r1 * m;
        double g_dbl = g1 * m;
        double b_dbl = b1 * m;
        double r_offset = r_dbl - static_cast<double>(r);
        double g_offset = g_dbl - static_cast<double>(g);
        double b_offset = b_dbl - static_cast<double>(b);
        if (rand_fun_vx() >= r_offset && r > 0) {
          r -= 1;
        }
        if (rand_fun_vx() >= g_offset && g > 0) {
          g -= 1;
        }
        if (rand_fun_vx() >= b_offset && b > 0) {
          b -= 1;
        }
      }
      dest[y][x] = png::rgb_pixel(r, g, b);
      index++;
    }
  }
}