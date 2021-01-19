/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

class socketbuffer {
public:
  socketbuffer() = default;
  ~socketbuffer() = default;

  void append(const char *, size_t);

  const std::string &get();
  size_t length();

  void erase_front(size_t);

protected:
  std::string buffer_;
};
