/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include "util/socketbuffer.h"

void socketbuffer::append(const char *buffer)
{
  buffer_.append(buffer);
}

void socketbuffer::append(const char *buffer, size_t buflen)
{
  buffer_.append(buffer, buflen);
}

const std::string &socketbuffer::get()
{
  return buffer_;
}

size_t socketbuffer::length()
{
  return buffer_.length();
}

bool socketbuffer::has_line()
{
  return buffer_.find("\n") != std::string::npos;
}

std::string socketbuffer::get_line()
{
  size_t pos = buffer_.find("\n");
  std::string line = buffer_.substr(0, pos + 1);
  buffer_ = buffer_.substr(pos + 1);
  return line;
}

void socketbuffer::erase_front(size_t len)
{
  buffer_ = buffer_.substr(len);
}

std::string socketbuffer::get_raw()
{
  std::string retbuffer;
  std::swap(retbuffer, buffer_);
  return retbuffer;
}
