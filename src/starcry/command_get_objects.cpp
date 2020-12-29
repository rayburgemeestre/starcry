/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <fmt/core.h>

#include "starcry.h"
#include "starcry/command_get_objects.h"
#include "webserver.h"  // ObjectsHandler

#include "nlohmann/json.hpp"
using json = nlohmann::json;

command_get_objects::command_get_objects(starcry &sc) : command_handler(sc) {}

std::shared_ptr<render_msg> command_get_objects::to_render_msg(std::shared_ptr<job_message> &job_msg, image &bmp) {
  auto &job = *job_msg->job;
  json shapes_json = {};
  auto &shapes = job_msg->job->shapes;
  size_t index = 0;
  if (!shapes.empty()) {
    for (const auto &shape : shapes[shapes.size() - 1]) {
      if (shape.type == data::shape_type::circle) {
        json circle = {
            {"index", index},
            {"id", shape.id},
            {"label", shape.label.empty() ? shape.id : shape.label},
            {"level", shape.level},
            {"type", "circle"},
            {"x", shape.x},
            {"y", shape.y},
        };
        shapes_json.push_back(circle);
      }
      if (shape.type == data::shape_type::line) {
        json circle = {
            {"index", index},
            {"id", shape.id},
            {"label", shape.label.empty() ? shape.id : shape.label},
            {"level", shape.level},
            {"type", "line"},
            {"x", shape.x},
            {"y", shape.y},
            {"x2", shape.x2},
            {"y2", shape.y2},
        };
        shapes_json.push_back(circle);
      }
      index++;
    }
  }
  return std::make_shared<render_msg>(
      job_msg->client, job_msg->type, job.job_number, job.last_frame, job.width, job.height, shapes_json.dump());
}

void command_get_objects::handle_frame(std::shared_ptr<render_msg> &job_msg) {
  auto fun = [&](std::shared_ptr<ObjectsHandler> objects_handler, std::shared_ptr<render_msg> job_msg) {
    objects_handler->callback(job_msg->client, job_msg->buffer);
  };
  if (sc.webserv) sc.webserv->execute_objects(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
}
