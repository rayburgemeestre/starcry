/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry/command_handler.h"
#include "starcry/metrics.h"

#include "generator.h"
#include "messages.hpp"
#include "starcry.h"
#include "util/image_splitter.hpp"

command_handler::command_handler(starcry &sc) : sc(sc) {}

void command_handler::to_job(std::shared_ptr<instruction> &cmd_def) {
  sc.gen->fast_forward(cmd_def->frame);
  sc.gen->generate_frame();

  auto the_job = sc.gen->get_job();

  if (cmd_def->viewpoint.canvas_w) {
    the_job->canvas_w = cmd_def->viewpoint.canvas_w;
    the_job->width = cmd_def->viewpoint.canvas_w;
  }
  if (cmd_def->viewpoint.canvas_h) {
    the_job->canvas_h = cmd_def->viewpoint.canvas_h;
    the_job->height = cmd_def->viewpoint.canvas_h;
  }

  util::ImageSplitter<uint32_t> is{the_job->canvas_w, the_job->canvas_h};

  the_job->scale *= cmd_def->viewpoint.scale;
  the_job->view_x = cmd_def->viewpoint.offset_x;
  the_job->view_y = cmd_def->viewpoint.offset_y;
  the_job->output_file = cmd_def->output_file;

  if (cmd_def->num_chunks == 1) {
    the_job->last_frame = cmd_def->last;
    sc.jobs->push(std::make_shared<job_message>(cmd_def->client, cmd_def->type, the_job, cmd_def->raw));
  } else {
    sc.metrics_->resize_job(the_job->job_number, cmd_def->num_chunks);
    const auto rectangles = is.split(cmd_def->num_chunks, util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
    for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
      the_job->width = rectangles[i].width();
      the_job->height = rectangles[i].height();
      the_job->offset_x = rectangles[i].x();
      the_job->offset_y = rectangles[i].y();
      the_job->chunk = counter;
      the_job->num_chunks = cmd_def->num_chunks;
      counter++;

      the_job->job_number = std::numeric_limits<uint32_t>::max();
      the_job->last_frame = cmd_def->last;
      sc.jobs->push(std::make_shared<job_message>(
          cmd_def->client, cmd_def->type, std::make_shared<data::job>(*the_job), cmd_def->raw));
    }
  }
}

std::shared_ptr<render_msg> command_handler::to_render_msg(std::shared_ptr<job_message> &cmd_def, image &bmp) {
  return nullptr;
}

void command_handler::handle_frame(std::shared_ptr<render_msg> &job_msg) {}
