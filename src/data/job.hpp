/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"

#include "color.hpp"
#include "data_staging/shape.hpp"
#include "shape.hpp"

namespace data {

struct job {
  uint32_t width = 1920;
  uint32_t height = 1080;
  double view_x = 0;
  double view_y = 0;
  uint32_t offset_x = 0;
  uint32_t offset_y = 0;
  uint32_t canvas_w = 1920;
  uint32_t canvas_h = 1080;
  size_t job_number = 0;
  uint32_t frame_number = 0;
  bool rendered = false;
  bool last_frame = false;
  uint32_t chunk = 0;
  uint32_t num_chunks = 1;
  data::color background_color;
  // for each rendering pass the list of shapes
  std::vector<std::vector<data::shape>> shapes;
  double scale = 1.0;  // deprecated
  std::vector<double> scales;
  uint32_t bitrate = 1000;
  bool compress = false;
  bool save_image = false;
  bool is_raw = false;
  std::string output_file;

  inline bool operator<(const job &other) const {
    return job_number < other.job_number;  // there can be no ties
  }

  template <class Archive>
  void serialize(Archive &ar) {
    ar(width,
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
