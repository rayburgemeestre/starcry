/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fmt/core.h>

#include "generator.h"
#include "starcry.h"
#include "starcry/command_get_bitmap.h"
#include "starcry/command_get_objects.h"

#include "image.hpp"
#include "util/logger.h"
#include "webserver.h"  // BitmapHandler

command_get_bitmap::command_get_bitmap(starcry &sc) : command_handler(sc) {}

void command_get_bitmap::to_job(std::shared_ptr<instruction> &cmd_def) {
  sc.metrics_->clear();
  command_handler::to_job(cmd_def);
}

std::shared_ptr<render_msg> command_get_bitmap::to_render_msg(std::shared_ptr<job_message> &job_msg, image &bmp) {
  auto &job = *job_msg->job;
  job.job_number = std::numeric_limits<uint32_t>::max();
  auto transfer_pixels = sc.pixels_vec_to_pixel_data(bmp.pixels(), sc.gen->settings());
  auto msg =
      std::make_shared<render_msg>(job_msg->client,
                                   job_msg->type,
                                   job.job_number,
                                   job.frame_number,
                                   job.chunk,
                                   job.num_chunks,
                                   job.offset_x,
                                   job.offset_y,
                                   job.last_frame,
                                   this->sc.get_viewpoint().labels,
                                   this->sc.get_viewpoint().canvas_w ? this->sc.get_viewpoint().canvas_w : job.width,
                                   this->sc.get_viewpoint().canvas_h ? this->sc.get_viewpoint().canvas_h : job.height,
                                   transfer_pixels);

  if (this->sc.get_viewpoint().labels) {
    // TODO: refactor the logic out of the commands
    auto json_msg = command_get_objects(sc).to_render_msg(job_msg, bmp);
    msg->buffer.assign(json_msg->buffer);
  }

  sc.features().caching = this->sc.get_viewpoint().caching;

  if (job_msg->raw || this->sc.get_viewpoint().raw || this->sc.get_viewpoint().save) {
    msg->set_raw(bmp.pixels());
  }
  return msg;
}

void command_get_bitmap::handle_frame(std::shared_ptr<render_msg> &job_msg) {
  auto fun = [&](std::shared_ptr<BitmapHandler> bmp_handler, std::shared_ptr<render_msg> job_msg) {
    std::string buffer;
    for (const auto &i : job_msg->pixels) {
      buffer.append((char *)&i, sizeof(i));
    }
    bmp_handler->callback(job_msg->client, buffer, job_msg->width, job_msg->height);
  };

  if (sc.webserv) sc.webserv->execute_bitmap(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);

  if (job_msg->labels) {
    job_msg->ID = sc.webserv->get_client_id(job_msg->client);
    command_get_objects(sc).handle_frame(job_msg);
  }
}
