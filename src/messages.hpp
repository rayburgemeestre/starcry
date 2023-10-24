/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

#include "message_type.hpp"

#include "data/color.hpp"
#include "data/job.hpp"
#include "data/viewpoint.hpp"

enum class instruction_type2 { image, video };

namespace seasocks {
class WebSocket;
}

namespace data {
struct job;
class frame_request;
class video_request;
}  // namespace data

class instruction : public message_type {
public:
  instruction() {}
};

class frame_instruction : public instruction {
public:
  std::shared_ptr<data::frame_request> frame_;

  frame_instruction(std::shared_ptr<data::frame_request> req) : frame_(req) {}

  const data::frame_request& frame() {
    return *frame_;
  }

  const std::shared_ptr<data::frame_request> frame_ptr() {
    return frame_;
  }
};

class video_instruction : public instruction {
public:
  std::shared_ptr<data::video_request> video_;

  video_instruction(std::shared_ptr<data::video_request> req) : video_(req) {}

  const data::video_request& video() {
    return *video_;
  }

  const std::shared_ptr<data::video_request> video_ptr() {
    return video_;
  }

  data::video_request& video_ref() {
    return *video_;
  }
};

class job_message : public message_type {
public:
  std::shared_ptr<instruction> original_instruction;
  std::shared_ptr<data::job> job;

  job_message(std::shared_ptr<instruction> original_instruction, std::shared_ptr<data::job> job)
      : original_instruction(original_instruction), job(job) {}
};

class render_msg : public message_type {
public:
  std::shared_ptr<job_message> original_job_message;

  std::string buffer;
  std::vector<uint32_t> pixels;
  std::vector<data::color> pixels_raw;
  uint32_t width = 0;
  uint32_t height = 0;
  std::string ID;  // ???

  render_msg(std::shared_ptr<job_message> original_job_message, std::string buf)
      : original_job_message(original_job_message), buffer(std::move(buf)) {
    _init();
  }

  // new
  render_msg(std::shared_ptr<job_message> original_job_message) : original_job_message(original_job_message) {
    _init();
  }

  // non-raw pixels only
  render_msg(std::shared_ptr<job_message> original_job_message, std::vector<uint32_t>& pixels)
      : original_job_message(original_job_message), pixels(std::move(pixels)) {
    _init();
  }

  // raw pixels only
  render_msg(std::shared_ptr<job_message> original_job_message, std::vector<data::color> pixels_raw)
      : original_job_message(original_job_message), pixels_raw(std::move(pixels_raw)) {
    _init();
  }

  void set_buffer(std::string& buf) {
    std::swap(this->buffer, buf);
  }

  void set_pixels(std::vector<uint32_t>& pixels) {
    std::swap(this->pixels, pixels);
  }

  void set_raw(std::vector<data::color>& pixels_raw) {
    std::swap(this->pixels_raw, pixels_raw);
  }

  void set_width(uint32_t w) {
    width = w;
  }
  void set_height(uint32_t h) {
    height = h;
  }

private:
  void _init() {
    width = original_job_message->job->width;
    height = original_job_message->job->height;
  }
};
