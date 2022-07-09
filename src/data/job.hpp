/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// #include "caf/meta/type_name.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"

#include "color.hpp"
#include "data_staging/shape.hpp"
#include "shape.hpp"

namespace data {

struct job {
  // if you modify these fields, you need to update announce or the added fields won't be transferred..
  bool same_host;
  uint32_t width;
  uint32_t height;
  double view_x;
  double view_y;
  uint32_t offset_x;
  uint32_t offset_y;
  uint32_t canvas_w;
  uint32_t canvas_h;
  size_t job_number;
  uint32_t frame_number;
  bool rendered;
  bool last_frame;
  uint32_t chunk;
  uint32_t num_chunks;
  data::color background_color;
  // for each rendering pass the list of shapes
  std::vector<std::vector<data::shape>> shapes;
  double scale;  // deprecated
  std::vector<double> scales;
  uint32_t bitrate;
  bool compress;
  bool save_image;
  bool is_raw;
  std::string output_file;

  inline bool operator<(const job &other) const {
    return job_number < other.job_number;  // there can be no ties
  }

  template <class Archive>
  void serialize(Archive &ar) {
    ar(same_host,
       width,
       height,
       view_x,
       view_y,
       offset_x,
       offset_y,
       canvas_w,
       canvas_h,
       job_number,
       frame_number,
       rendered,
       last_frame,
       chunk,
       num_chunks,
       background_color,
       shapes,
       scale,
       scales,
       bitrate,
       compress,
       save_image,
       is_raw,
       output_file);
  }

  inline void resize_for_num_steps(int steps) {
    for (int current_step = 0; current_step < steps; current_step++) {
      // make sure we have shapes vectors for each step we plan to create
      shapes.emplace_back();
    }
  }
};

inline bool operator==(const job &lhs, const job &rhs) {
  return lhs.job_number == rhs.job_number;
}

}  // namespace data
