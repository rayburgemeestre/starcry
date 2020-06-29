/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include "util/socketbuffer.h"

void socketbuffer::append(const char *buffer, size_t buflen) {
  buffer_.append(buffer, buflen);
}

const std::string &socketbuffer::get() {
  return buffer_;
}

size_t socketbuffer::length() {
  return buffer_.length();
}

void socketbuffer::erase_front(size_t len) {
  buffer_ = buffer_.substr(len);
}
