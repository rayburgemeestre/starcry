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
  virtual ~socketbuffer() = default;

  virtual void append(const char *);
  virtual void append(const char *, size_t);

  [[nodiscard]] const std::string &get() const;
  [[nodiscard]] size_t length() const;

  void erase_front(size_t);

  [[nodiscard]] bool has_line() const;
  std::string get_line();
  std::string get_raw();

  void gc();

protected:
  std::string buffer_;
  size_t index_ = 0;

  size_t gc_line_num_;
  size_t gc_every_;
};
