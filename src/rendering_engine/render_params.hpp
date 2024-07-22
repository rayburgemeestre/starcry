/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class metrics;
namespace data {
struct color;
struct shape;
struct settings;
}  // namespace data

struct render_params {
  size_t thread_num, job_num, chunk_num, num_chunks;
  std::shared_ptr<metrics> metrics;
  const data::color &bg_color;
  const std::vector<std::vector<data::shape>> &shapes;
  double view_x, view_y;
  uint32_t canvas_w, canvas_h;
  double top_scale;
  std::vector<double> scales;
  bool verbose;
  const data::settings &settings;
  bool debug;
  const std::vector<int64_t> &selected_ids;
};
