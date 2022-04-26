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
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
    std::unordered_map<int64_t, int64_t> nums;
    for (size_t i = 0; i < shapes.size(); i++) {
      for (const auto &shape : shapes[i]) {
        nums[shape.unique_id]++;
      }
    }
#endif
    for (const auto &shape : shapes[shapes.size() - 1]) {
      if (shape.type == data::shape_type::circle) {
        json circle = {
            {"index", index},
            {"id", shape.id},
            {"label", shape.label.empty() ? shape.id : shape.label},
            {"level", shape.level},
            {"gradient", shape.gradient_id_str},
            {"type", "circle"},
            {"x", shape.x},
            {"y", shape.y},
#ifdef DEBUG_NUM_SHAPES
            {"#", nums[shape.unique_id]},
#else
            {"#", -1},
#endif
        };
        shapes_json.push_back(circle);
      }
      if (shape.type == data::shape_type::line) {
        json line = {
            {"index", index},
            {"id", shape.id},
            {"label", shape.label.empty() ? shape.id : shape.label},
            {"level", shape.level},
            {"gradient", shape.gradient_id_str},
            {"type", "line"},
            {"x", shape.x},
            {"y", shape.y},
            {"x2", shape.x2},
            {"y2", shape.y2},
#ifdef DEBUG_NUM_SHAPES
            {"#", nums[shape.unique_id]},
#else
            {"#", -1},
#endif
        };
        shapes_json.push_back(line);
      }
      index++;
    }
  }
  return std::make_shared<render_msg>(job_msg->client,
                                      job_msg->type,
                                      job.job_number,
                                      job.frame_number,
                                      job.chunk,
                                      job.num_chunks,
                                      job.offset_x,
                                      job.offset_y,
                                      job.last_frame,
                                      false,
                                      job.width,
                                      job.height,
                                      shapes_json.dump());
}

void command_get_objects::handle_frame(std::shared_ptr<render_msg> &job_msg) {
  job_msg->ID = sc.webserv->get_client_id(job_msg->client);
  auto fun = [&](std::shared_ptr<ObjectsHandler> objects_handler, std::shared_ptr<render_msg> job_msg) {
    if (objects_handler->_links.find(job_msg->ID) != objects_handler->_links.end()) {
      auto con = objects_handler->_links[job_msg->ID];  // find con that matches ID this msg is from
      objects_handler->callback(con, job_msg->buffer);
    }
  };
  if (sc.webserv) sc.webserv->execute_objects(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
}
