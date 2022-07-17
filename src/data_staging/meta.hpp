/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>

namespace data_staging {

class meta {
private:
  std::string namespace_;
  std::string id_;
  int64_t unique_id_;
  int64_t parent_uid_id_ = -1;
  int64_t level_;

  // used for rendering purposes
  double dist_ = 0;
  int64_t steps_ = 0;

  // cumulative
  vector2d parent_location_ = vector2d{0, 0};

public:
  meta() = default;

  meta(std::string id, int64_t unique_id) : id_(std::move(id)), unique_id_(unique_id), parent_location_(0, 0) {}

  const std::string& namespace_name() const {
    return namespace_;
  }

  const std::string& id() const {
    return id_;
  }

  int64_t unique_id() const {
    return unique_id_;
  }

  int64_t parent_uid() const {
    return parent_uid_id_;
  }

  int64_t level() const {
    return level_;
  }

  void set_level(int64_t level) {
    level_ = level;
  }

  void set_parent_uid(int64_t parent_uid) {
    parent_uid_id_ = parent_uid;
  }

  const vector2d& parent_location_cref() const {
    return parent_location_;
  }

  void set_parent_location(const vector2d& parent_location) {
    parent_location_ = parent_location;
  }

  double distance() const {
    return dist_;
  }

  void set_distance(double dist) {
    dist_ = dist;
  }

  int64_t steps() const {
    return steps_;
  }
  void set_steps(int64_t steps) {
    steps_ = steps;
  }
};

}  // namespace data_staging
