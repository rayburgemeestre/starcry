/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fmt/core.h>

#include "cereal/archives/json.hpp"
#include "starcry.h"
#include "starcry/command_get_shapes.h"
#include "webserver.h"  // ShapesHandler

#include <sstream>

command_get_shapes::command_get_shapes(starcry &sc) : command_handler(sc) {}

std::shared_ptr<render_msg> command_get_shapes::to_render_msg(std::shared_ptr<job_message> &job_msg, image &bmp) {
  auto &job = *job_msg->job;
  std::ostringstream os;
  {
    cereal::JSONOutputArchive archive(os);
    archive(job);
  }
  return std::make_shared<render_msg>(job_msg->client, job_msg->type, job.job_number, job.width, job.height, os.str());
}

void command_get_shapes::handle_frame(std::shared_ptr<render_msg> &job_msg) {
  auto fun = [&](std::shared_ptr<ShapesHandler> shapes_handler, std::shared_ptr<render_msg> job_msg) {
    shapes_handler->callback(job_msg->client, job_msg->buffer);
  };
  if (sc.webserv) sc.webserv->execute_shapes(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
}
