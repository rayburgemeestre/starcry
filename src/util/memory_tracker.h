/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

class memory_tracker {
private:
  std::string object_type_;
  bool moved_from_ = false;

public:
  memory_tracker() = delete;
  explicit memory_tracker(std::string object_type);
  memory_tracker(const memory_tracker& other);
  memory_tracker& operator=(const memory_tracker& other);
  memory_tracker(memory_tracker&& other);
  ~memory_tracker();
};

extern void memory_tracker_print();