/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "job_mapper.h"
#include "data/job.hpp"
#include "data/viewpoint.hpp"

namespace interpreter {

job_mapper::job_mapper(std::shared_ptr<data::job> job) : job_(job) {}

void job_mapper::map_background_color(v8_interact &i, v8::Local<v8::Object> &bg) {
  job_->background_color.r = i.double_number(bg, "r");
  job_->background_color.g = i.double_number(bg, "g");
  job_->background_color.b = i.double_number(bg, "b");
  job_->background_color.a = i.double_number(bg, "a");
}
void job_mapper::map_scale(double scale) {
  job_->scale = scale;
}
void job_mapper::map_canvas(uint32_t canvas_w, uint32_t canvas_h) {
  job_->width = canvas_w;
  job_->height = canvas_h;
  job_->canvas_w = canvas_w;
  job_->canvas_h = canvas_h;
}
void job_mapper::map_viewpoint(const data::viewpoint &viewpoint) {
  if (viewpoint.canvas_w) {
    job_->canvas_w = viewpoint.canvas_w;
    job_->width = viewpoint.canvas_w;
  }
  if (viewpoint.canvas_h) {
    job_->canvas_h = viewpoint.canvas_h;
    job_->height = viewpoint.canvas_h;
  }
  job_->scale *= viewpoint.scale;
  job_->view_x = viewpoint.offset_x;
  job_->view_y = viewpoint.offset_y;
}
void job_mapper::map_output_file(const std::string &output_file) {
  job_->output_file = output_file;
}
void job_mapper::map_last_frame(bool last_frame) {
  job_->last_frame = last_frame;
}
void job_mapper::map_rectangle(const util::rectangle<uint32_t> &rect, uint32_t chunk, uint32_t num_chunks) {
  job_->width = rect.width();
  job_->height = rect.height();
  job_->offset_x = rect.x();
  job_->offset_y = rect.y();
  job_->chunk = chunk;
  job_->num_chunks = num_chunks;
  job_->job_number = std::numeric_limits<uint32_t>::max();
}

uint32_t job_mapper::get_frame_number() const {
  return job_->frame_number;
}

}  // namespace interpreter
