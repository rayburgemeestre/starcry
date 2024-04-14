/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-builtins"
#endif
#include <boost/uuid/uuid.hpp>             // for uuid
#include <boost/uuid/uuid_generators.hpp>  // for uuid_generators
#include <boost/uuid/uuid_io.hpp>          // for to_string
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include <fmt/core.h>

#include "cereal/archives/binary.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"

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
class reload_request;
class frame_request;
class video_request;
}  // namespace data

class instruction : public message_type {
public:
  instruction() {}
};

class reload_instruction : public instruction {
public:
  std::shared_ptr<data::reload_request> req_;

  reload_instruction(std::shared_ptr<data::reload_request> req) : req_(req) {}

  const data::reload_request& req() {
    return *req_;
  }

  const std::shared_ptr<data::reload_request> req_ptr() {
    return req_;
  }
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

  std::string suspend_id;

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

  void suspend() {
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid1 = generator();
    suspend_id = to_string(uuid1);

    std::ostringstream os;
    cereal::BinaryOutputArchive archive_out(os);
    archive_out(this->buffer);
    archive_out(this->pixels);
    archive_out(this->pixels_raw);

    this->buffer.clear();
    this->pixels.clear();
    this->pixels_raw.clear();
    this->buffer.shrink_to_fit();
    this->pixels.shrink_to_fit();
    this->pixels_raw.shrink_to_fit();

    std::string filename = fmt::format("/tmp/render_msg_buffer-{}", suspend_id);

    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
      throw std::runtime_error("failed to open suspend serialization file");
    }
    outfile << os.str();
    outfile.close();
  }

  void unsuspend() {
    std::string filename = fmt::format("/tmp/render_msg_buffer-{}", suspend_id);

    std::ifstream infile(filename, std::ios::binary);
    if (!infile.is_open()) {
      // not suspended, no need to restore
      return;
    }

    std::ostringstream os;
    os << infile.rdbuf();

    infile.close();

    std::istringstream is(os.str());
    cereal::BinaryInputArchive archive_in(is);

    try {
      archive_in(this->buffer);
      archive_in(this->pixels);
      archive_in(this->pixels_raw);
    } catch (const cereal::Exception& e) {
      logger(ERROR) << "Deserialization failed: " << e.what() << std::endl;
      throw;
    }
    unlink(filename.c_str());
  }

private:
  void _init() {
    width = original_job_message->job->width;
    height = original_job_message->job->height;
  }
};
