/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <vector>

namespace seasocks {
class WebSocket;
}

namespace data {
class frame_request {
private:
  std::string script_;
  std::string output_filename_;

  int frame_num_ = 0;
  int num_chunks_ = 0;

  seasocks::WebSocket* client_ = nullptr;

  bool compressed_image_ = false;   // compressed PNG
  bool raw_bitmap_ = false;         // bitmap (no headers)
  bool raw_image_ = false;          // EXR
  bool metadata_objects_ = false;   // metadata about the objects for interactive UI features
  bool renderable_shapes_ = false;  // renderer input, can be used to render client-side with WASM

  bool preview_ = false;
  bool last_frame_ = false;
  std::vector<int64_t> selected_ids_;

public:
  frame_request(std::string script, int frame_num, int num_chunks)
      : script_(std::move(script)), frame_num_(frame_num), num_chunks_(num_chunks) {}

  void set_websocket(seasocks::WebSocket* client) {
    client_ = client;
  }
  void set_output(const std::string& output_filename) {
    output_filename_ = output_filename;
  }
  void enable_compressed_image() {
    compressed_image_ = true;
  }
  void enable_raw_bitmap() {
    raw_bitmap_ = true;
  }
  void enable_raw_image() {
    raw_image_ = true;
  }
  void enable_metadata_objects() {
    metadata_objects_ = true;
  }
  void enable_renderable_shapes() {
    renderable_shapes_ = true;
  }
  void set_preview_mode() {
    preview_ = true;
  }
  void set_last_frame() {
    last_frame_ = true;
  }
  void set_selected_ids(std::vector<int64_t> selected_ids) {
    selected_ids_ = std::move(selected_ids);
  }
  //////
  const std::string& script() const {
    return script_;
  }
  const std::string& output_file() const {
    return output_filename_;
  }
  int frame_num() const {
    return frame_num_;
  }
  int num_chunks() const {
    return num_chunks_;
  }
  seasocks::WebSocket* client() const {
    return client_;
  }
  bool compressed_image() const {
    return compressed_image_;
  }
  bool raw_bitmap() const {
    return raw_bitmap_;
  }
  bool raw_image() const {
    return raw_image_;
  }
  bool metadata_objects() const {
    return metadata_objects_;
  }
  bool renderable_shapes() const {
    return renderable_shapes_;
  }
  bool preview() const {
    return preview_;
  }
  bool last_frame() const {
    return last_frame_;
  }
  [[nodiscard]] const std::vector<int64_t>& selected_ids() const {
    return selected_ids_;
  }
};
}  // namespace data