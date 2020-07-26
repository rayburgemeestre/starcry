/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>

#include "cereal/types/vector.hpp"

namespace data {

struct pixel_data2 {
  size_t job_number;
  std::vector<uint32_t> pixels;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(job_number, pixels);
  }
};

}  // namespace data
