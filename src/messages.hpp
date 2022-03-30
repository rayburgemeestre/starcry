/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

#include "message_type.hpp"

#include "data/color.hpp"
#include "data/viewpoint.hpp"

enum class instruction_type {
  get_image,
  get_shapes,
  get_objects,
  get_bitmap,
  get_video,
  get_raw_image,
  get_raw_video,
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
  int num_chunks;
  bool raw = false;
  bool preview = false;
  bool last = false;
  // TODO: can this be "merged" with "frame" ? so we can avoid introducing this offset_frame.
  size_t offset_frames = 0;

  data::viewpoint viewpoint;

  // constructor for stills
  instruction(seasocks::WebSocket *client,
              instruction_type type,
              std::string script,
              size_t frame,
              int num_chunks,
              bool raw,
              bool preview,
              bool last_frame,
              const std::string &output_filename)
      : client(client),
        type(type),
        frame(frame),
        script(script),
        output_file(output_filename),
        num_chunks(num_chunks),
        raw(raw),
        preview(preview),
        last(last_frame) {}

  // constructor for video
  instruction(seasocks::WebSocket *client,
              instruction_type type,
              std::string script,
              std::string output_file,
              int num_chunks,
              bool raw,
              bool preview,
              size_t offset_frames)
      : client(client),
        type(type),
        frame(0),
        script(std::move(script)),
        output_file(std::move(output_file)),
        num_chunks(num_chunks),
        raw(raw),
        preview(preview),
        offset_frames(offset_frames) {}
};

class job_message : public message_type {
public:
  seasocks::WebSocket *client;
  instruction_type type;
  std::shared_ptr<data::job> job;
  bool raw = false;

  job_message(seasocks::WebSocket *client, instruction_type type, std::shared_ptr<data::job> job, bool raw)
      : client(client), type(type), job(job), raw(raw) {}
};

class render_msg : public message_type {
public:
  size_t job_number;
  size_t frame_number;
  size_t chunk;
  size_t num_chunks;
  size_t offset_x;  // for debug
  size_t offset_y;  // for debug
  bool last_frame;
  bool labels;
  seasocks::WebSocket *client;
  instruction_type type;
  std::string buffer;
  std::vector<uint32_t> pixels;
  std::vector<data::color> pixels_raw;
  uint32_t width;
  uint32_t height;
  std::string ID;
  std::string output_file;

  render_msg(seasocks::WebSocket *client,
             instruction_type type,
             size_t job_number,
             size_t frame_number,
             size_t chunk,
             size_t num_chunks,
             size_t offset_x,
             size_t offset_y,
             bool last_frame,
             bool labels,
             uint32_t width,
             uint32_t height,
             std::string buf)
      : job_number(job_number),
        frame_number(frame_number),
        chunk(chunk),
        num_chunks(num_chunks),
        offset_x(offset_x),
        offset_y(offset_y),
        last_frame(last_frame),
        labels(labels),
        client(client),
        type(type),
        buffer(std::move(buf)),
        width(width),
        height(height) {}

  // non-raw pixels only
  render_msg(seasocks::WebSocket *client,
             instruction_type type,
             size_t job_number,
             size_t frame_number,
             size_t chunk,
             size_t num_chunks,
             size_t offset_x,
             size_t offset_y,
             bool last_frame,
             bool labels,
             uint32_t width,
             uint32_t height,
             std::vector<uint32_t> &pixels,
             const std::string &output_file)
      : job_number(job_number),
        frame_number(frame_number),
        chunk(chunk),
        num_chunks(num_chunks),
        offset_x(offset_x),
        offset_y(offset_y),
        last_frame(last_frame),
        labels(labels),
        client(client),
        type(type),
        pixels(std::move(pixels)),
        width(width),
        height(height),
        output_file(output_file) {}

  // raw pixels only
  render_msg(seasocks::WebSocket *client,
             instruction_type type,
             size_t job_number,
             size_t frame_number,
             size_t chunk,
             size_t num_chunks,
             size_t offset_x,
             size_t offset_y,
             bool last_frame,
             bool labels,
             uint32_t width,
             uint32_t height,
             std::vector<data::color> pixels_raw,
             const std::string &output_file)
      : job_number(job_number),
        frame_number(frame_number),
        chunk(chunk),
        num_chunks(num_chunks),
        offset_x(offset_x),
        offset_y(offset_y),
        last_frame(last_frame),
        labels(labels),
        client(client),
        type(type),
        pixels_raw(std::move(pixels_raw)),
        width(width),
        height(height),
        output_file(output_file) {}

  void set_raw(std::vector<data::color> &pixels_raw) {
    std::swap(this->pixels_raw, pixels_raw);
  }
};
