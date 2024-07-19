/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "job_mapper.h"
#include "data/job.hpp"

namespace interpreter {

job_mapper::job_mapper(std::shared_ptr<data::job> job) : job_(job) {}

void job_mapper::map_background_color(v8_interact &i, v8::Local<v8::Object> &bg) {
  job_->background_color.r = i.double_number(bg, "r");
  job_->background_color.g = i.double_number(bg, "g");
  job_->background_color.b = i.double_number(bg, "b");
  job_->background_color.a = i.double_number(bg, "a");
}
void job_mapper::set_scale(double scale) {
  job_->scale = scale;
}
void job_mapper::set_canvas(uint32_t canvas_w, uint32_t canvas_h) {
  job_->width = canvas_w;
  job_->height = canvas_h;
  job_->canvas_w = canvas_w;
  job_->canvas_h = canvas_h;
}
uint32_t job_mapper::get_frame_number() const {
  return job_->frame_number;
}

}  // namespace interpreter
