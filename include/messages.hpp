/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

#include "message_type.hpp"

enum class instruction_type {
  get_image,
  get_video,
};

namespace seasocks {
class WebSocket;
}

namespace data {
struct job;
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

