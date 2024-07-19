/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <memory>

#include "util/v8_interact.hpp"

namespace data {
struct job;
}

namespace interpreter {

class job_mapper {
private:
  std::shared_ptr<data::job> job_;

public:
  explicit job_mapper(std::shared_ptr<data::job> job);

  void map_background_color(v8_interact& i, v8::Local<v8::Object>& bg);
  void set_scale(double scale);
  void set_canvas(uint32_t canvas_w, uint32_t canvas_h);
  uint32_t get_frame_number() const;
};
}  // namespace interpreter