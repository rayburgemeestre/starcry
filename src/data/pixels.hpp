/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>

#include "cereal/types/vector.hpp"
#include "data/color.hpp"

namespace data {

struct pixel_data {
  size_t job_number;
  std::vector<uint32_t> pixels;
  std::vector<data::color> pixels_raw;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(job_number, pixels, pixels_raw);
  }
};

}  // namespace data
