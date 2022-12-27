/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/memory_tracker.h"

#include <mutex>

#include "logger.h"

std::mutex super_global_mutex;

std::map<std::string, int> object_counts;

#define SILENT
#define DISABLED

memory_tracker::memory_tracker(std::string object_type) : object_type_(std::move(object_type)) {
#ifndef DISABLED
  std::unique_lock<std::mutex> lock(super_global_mutex);
  object_counts[object_type_]++;
#ifndef SILENT
  logger(DEBUG) << "memory_tracker::memory_tracker(" << object_type_ << ") count = " << object_counts[object_type_]
                << std::endl;
#endif
#endif
}

memory_tracker::memory_tracker(const memory_tracker& other) {
#ifndef DISABLED
  std::unique_lock<std::mutex> lock(super_global_mutex);
  object_type_ = other.object_type_;
  object_counts[object_type_]++;
#ifndef SILENT
  logger(DEBUG) << "memory_tracker::memory_tracker[copy](" << object_type_
                << ") count = " << object_counts[object_type_] << std::endl;
#endif
#endif
}

memory_tracker& memory_tracker::operator=(const memory_tracker& other) {
#ifndef DISABLED
  std::unique_lock<std::mutex> lock(super_global_mutex);
  object_type_ = other.object_type_;
  object_counts[object_type_]++;

#ifndef SILENT
  logger(DEBUG) << "memory_tracker::memory_tracker[copy_assign](" << object_type_
                << ") count = " << object_counts[object_type_] << std::endl;
#endif
#endif
  return *this;
}

memory_tracker::memory_tracker(memory_tracker&& other) {
#ifndef DISABLED
  std::unique_lock<std::mutex> lock(super_global_mutex);
  object_type_ = other.object_type_;  // do not move
  other.moved_from_ = true;           // do not decrement in destructor!
  // object_counts[object_type_]++;
#ifndef SILENT
  logger(DEBUG) << "memory_tracker::memory_tracker[move](" << object_type_
                << ") count = " << object_counts[object_type_] << std::endl;
#endif
#endif
}

memory_tracker::~memory_tracker() {
#ifndef DISABLED
  if (moved_from_) {
    return;
  }
  std::unique_lock<std::mutex> lock(super_global_mutex);
  object_counts[object_type_]--;
#ifndef SILENT
  logger(DEBUG) << "memory_tracker::~memory_tracker(" << object_type_ << ") count = " << object_counts[object_type_]
                << ", sizeof = " << sizeof(*this) << std::endl;
#endif
#endif
}

void memory_tracker_print() {
#ifndef DISABLED
  std::unique_lock<std::mutex> lock(super_global_mutex);
  // iterateo over object_counts
  for (auto& it : object_counts) {
    logger(INFO) << "memory_tracker_print: " << it.first << " count = " << it.second << std::endl;
  }
#endif
}
