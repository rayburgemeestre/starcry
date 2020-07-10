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

#include "starcry.h"  // for render mode

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
class render_server;

namespace data {
class job;
}

class instruction : public message_type {
public:
  seasocks::WebSocket *client;
  instruction_type type;
  size_t frame;
  std::string script;
  std::string output_file;
  instruction(seasocks::WebSocket *client, instruction_type type, std::string script, size_t frame)
      : client(client), type(type), frame(frame), script(script), output_file("") {}
  instruction(seasocks::WebSocket *client, instruction_type type, std::string script, std::string output_file)
      : client(client), type(type), frame(0), script(std::move(script)), output_file(std::move(output_file)) {}
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
  std::vector<uint32_t> pixels;
  uint32_t width;
  uint32_t height;
  render_msg(seasocks::WebSocket *client, uint32_t width, uint32_t height, std::string buf)
      : client(client), buffer(std::move(buf)), width(width), height(height) {}
  render_msg(seasocks::WebSocket *client, uint32_t width, uint32_t height, std::vector<uint32_t> pixels)
      : client(client), pixels(std::move(pixels)), width(width), height(height) {}
};

class starcry_pipeline {
private:
  std::shared_ptr<seasocks::Server> server;
  std::shared_ptr<Handler> chat_handler;
  std::map<int, std::shared_ptr<bitmap_wrapper>> bitmaps;
  std::shared_ptr<generator> gen;
  std::map<int, std::shared_ptr<rendering_engine_wrapper>> engines;
  std::shared_ptr<pipeline_system> system;
  std::shared_ptr<queue> cmds;
  std::shared_ptr<queue> jobs;
  std::shared_ptr<queue> frames;
  std::shared_ptr<render_server> render_server;
  starcry::render_video_mode mode;

public:
  starcry_pipeline(
      size_t num_local_engines,
      bool enable_remote_workers,
      bool visualization_enabled,
      bool is_interactive,
      starcry::render_video_mode mode,
      std::function<void(starcry_pipeline &sc)> on_pipeline_initialized = [](auto &) {});

  void add_command(seasocks::WebSocket *client, const std::string &script, int frame_num);
  void add_command(seasocks::WebSocket *client, const std::string &script, const std::string &output_file);

private:
  void render_job(rendering_engine_wrapper &engine, const data::job &job, ALLEGRO_BITMAP *bmp);
  void copy_to_png(const std::vector<uint32_t> &source,
                   uint32_t width,
                   uint32_t height,
                   png::image<png::rgb_pixel> &dest);
};
