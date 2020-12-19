/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fmt/core.h>

#include "image.hpp"
#include "starcry.h"
#include "starcry/command_get_image.h"
#include "webserver.h"  // ImageHandler

#include <sstream>

command_get_image::command_get_image(starcry &sc) : command_handler(sc) {}

std::shared_ptr<render_msg> command_get_image::to_render_msg(std::shared_ptr<job_message> &job_msg, image &bmp) {
  auto &job = *job_msg->job;
  job.job_number = std::numeric_limits<uint32_t>::max();
  png::image<png::rgb_pixel> image(job.width, job.height);
  sc.copy_to_png(bmp.pixels(), job.width, job.height, image);
  std::ostringstream ss;
  image.write_stream(ss);
  return std::make_shared<render_msg>(job_msg->client, job_msg->type, job.job_number, job.width, job.height, ss.str());
}

void command_get_image::handle_frame(std::shared_ptr<render_msg> &job_msg) {
  auto fun = [&](std::shared_ptr<ImageHandler> chat_handler, std::shared_ptr<render_msg> job_msg) {
    chat_handler->callback(job_msg->client, job_msg->buffer);
  };
  if (sc.webserv) {
    sc.webserv->execute_image(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
  }
}
