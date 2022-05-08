/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <vector>

#include "png.hpp"

#include "data/color.hpp"

void copy_to_png(const std::vector<data::color> &source,
                 uint32_t width,
                 uint32_t height,
                 png::image<png::rgba_pixel> &dest,
                 bool dithering);

std::vector<uint32_t> pixels_vec_to_pixel_data(const std::vector<data::color> &pixels_in, const bool &dithering);

void pixels_vec_insert_checkers_background(std::vector<uint32_t> &pixels, int width, int height);