/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

class socketbuffer {
public:
  socketbuffer();
  ~socketbuffer();

  virtual void append(const char *);
  virtual void append(const char *, size_t);

  const std::string &get();
  size_t length();

  void erase_front(size_t);

  bool has_line();
  std::string get_line();
  std::string get_raw();

  void gc();

protected:
  std::string buffer_;
  size_t index_;

  size_t gc_line_num_;
  size_t gc_every_;
};
