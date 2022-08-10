/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/socketbuffer.h"

void socketbuffer::append(const char *buffer) {
  buffer_.append(buffer);
}

void socketbuffer::append(const char *buffer, size_t buflen) {
  buffer_.append(buffer, buflen);
}

const std::string &socketbuffer::get() const {
  return buffer_;
}

size_t socketbuffer::length() const {
  return buffer_.length();
}

void socketbuffer::erase_front(size_t len) {
  buffer_ = buffer_.substr(len);
}

bool socketbuffer::has_line() const {
  return buffer_.find('\n', index_) != std::string::npos;
}

std::string socketbuffer::get_line() {
  size_t pos = buffer_.find('\n', index_);

  std::string line = buffer_.substr(index_, pos + 1 - index_);

  // Do not update directly, it's too slow
  // buffer_ = buffer_.substr(pos + 1);

  // Instead update an index
  index_ = pos + 1;

  return line;
}

std::string socketbuffer::get_raw() {
  std::string retbuffer;
  std::swap(retbuffer, buffer_);

  return retbuffer;
}

void socketbuffer::gc() {
  buffer_ = buffer_.substr(index_);
  index_ = 0;
}
