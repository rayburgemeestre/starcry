/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <caf/meta/type_name.hpp>

#include "color.hpp"
#include "shape.hpp"

namespace data {

struct job {
  // if you modify these fields, you need to update announce or the added fields won't be transferred..
  uint32_t width;
  uint32_t height;
  uint32_t offset_x;
  uint32_t offset_y;
  uint32_t canvas_w;
  uint32_t canvas_h;
  size_t job_number;
  size_t frame_number;
  bool rendered;
  bool last_frame;
  size_t chunk;
  size_t num_chunks;
  data::color background_color;
  std::vector<shape> shapes;
  double scale;
  size_t bitrate;
  bool compress;
  bool save_image;

  inline bool operator<(const job &other) const {
    return job_number < other.job_number;  // there can be no ties
  }
};

inline bool operator==(const job &lhs, const job &rhs) {
  return lhs.job_number == rhs.job_number;
}

template <class Processor>
void serialize(Processor &proc, data::job &x, const unsigned int) {
  proc &x.width;
  proc &x.height;
  proc &x.offset_x;
  proc &x.offset_y;
  proc &x.canvas_w;
  proc &x.canvas_h;
  proc &x.job_number;
  proc &x.frame_number;
  proc &x.rendered;
  proc &x.last_frame;
  proc &x.chunk;
  proc &x.num_chunks;
  proc &x.background_color;
  proc &x.shapes;
  proc &x.scale;
  proc &x.bitrate;
  proc &x.compress;
  proc &x.save_image;
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector &f, data::job &x) {
  return f(caf::meta::type_name("data::job"),
           x.width,
           x.height,
           x.offset_x,
           x.offset_y,
           x.canvas_w,
           x.canvas_h,
           x.job_number,
           x.frame_number,
           x.rendered,
           x.last_frame,
           x.chunk,
           x.num_chunks,
           x.background_color,
           x.shapes,
           x.scale,
           x.bitrate,
           x.compress,
           x.save_image);
}
}  // namespace data
