/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <data/job.hpp>
#include <data/pixels.hpp>
#include <memory>
#include <mutex>

#include "piper.h"

#include "seasocks/PrintfLogger.h"
#include "seasocks/Response.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/PathHandler.h"
#include "seasocks/util/RootPageHandler.h"

#include "png.hpp"

class Handler;

enum class instruction_type {
  get_image,
  get_video,
};

class bitmap_wrapper;
class generator;
class rendering_engine_wrapper;
class ALLEGRO_BITMAP;

namespace data {
class job;
}

class instruction : public message_type {
public:
  seasocks::WebSocket *client;
  instruction_type type;
  size_t frame;
  instruction(seasocks::WebSocket *client, instruction_type type, size_t frame)
      : client(client), type(type), frame(frame) {}
};

class job_message : public message_type {
public:
  seasocks::WebSocket *client;
  std::shared_ptr<data::job> job;
  job_message(seasocks::WebSocket *client, std::shared_ptr<data::job> job) : client(client), job(job) {}
};

class render_msg : public message_type {
public:
  seasocks::WebSocket *client;
  std::string buffer;
  render_msg(seasocks::WebSocket *client, std::string buf) : client(client), buffer(std::move(buf)) {}
};

class starcry_pipeline {
private:
  std::shared_ptr<seasocks::Server> server;
  std::shared_ptr<Handler> chat_handler;
  std::shared_ptr<bitmap_wrapper> bitmap;
  std::shared_ptr<generator> gen;
  std::shared_ptr<rendering_engine_wrapper> engine;

  std::shared_ptr<pipeline_system> system;
  std::shared_ptr<queue> cmds;
  std::shared_ptr<queue> jobs;
  std::shared_ptr<queue> frames;

  std::shared_ptr<data::job> the_job;

public:
  starcry_pipeline();

  void add_command(seasocks::WebSocket *client, int frame_num);

private:
  void render_job(rendering_engine_wrapper &engine, const data::job &job, ALLEGRO_BITMAP *bmp);
  void copy_to_png(const std::vector<uint32_t> &source,
                   uint32_t width,
                   uint32_t height,
                   png::image<png::rgb_pixel> &dest);
};
