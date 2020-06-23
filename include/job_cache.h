/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "data/job.hpp"
#include "data/pixels.hpp"
#include "data/shape.hpp"

class job_cache {
private:
  std::unordered_map<size_t, std::vector<data::shape>> cache_shapes;
  std::unordered_map<size_t, std::vector<uint32_t>> cache_pixels;
  std::mutex mut;

public:
  job_cache();
  bool take(data::job &job);
  bool take(data::pixel_data2 &dat);
  std::vector<data::shape> retrieve(data::job &job);
  std::vector<uint32_t> retrieve(data::pixel_data2 &dat);
};
