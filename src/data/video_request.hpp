/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>

namespace data {
class video_request {
private:
  std::string script_;
  std::string output_filename_;

  int num_chunks_ = 0;
  size_t offset_frames_ = 0;

  bool compressed_video_ = false;  // compressed video
  bool raw_video_ = false;         // raw video (individual EXR frames)

  bool preview_ = false;

public:
  video_request(std::string script, std::string output_file, int num_chunks, size_t offset_frames)
      : script_(std::move(script)),
        output_filename_(output_file),
        num_chunks_(num_chunks),
        offset_frames_(offset_frames) {}

  void set_output_file(const std::string& new_output_file) {
    output_filename_ = new_output_file;
  }

  void enable_compressed_video() {
    compressed_video_ = true;
  }
  void enable_raw_video() {
    raw_video_ = true;
  }
  void set_preview_mode() {
    preview_ = true;
  }
  ////
  const std::string& script() const {
    return script_;
  }
  const std::string& output_file() const {
    return output_filename_;
  }
  int num_chunks() const {
    return num_chunks_;
  }
  size_t offset_frames() const {
    return offset_frames_;
  }
  bool compressed_video() const {
    return compressed_video_;
  }
  bool raw_video() const {
    return raw_video_;
  }
  bool preview() const {
    return preview_;
  }
};
}  // namespace data