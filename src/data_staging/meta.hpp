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
  double time_;

  // used for rendering purposes
  double dist_ = 0;
  int64_t steps_ = 0;
  bool is_pivot_ = false;
  bool destroyed_ = false;

public:
  meta() = default;

  meta(std::string id, int64_t unique_id) : id_(std::move(id)), unique_id_(unique_id) {}

  double get_time() const {
    return time_;
  }

  void set_time(double time) {
    time_ = time;
  }

  const std::string& namespace_name() const {
    return namespace_;
  }

  const std::string& id() const {
    return id_;
  }

  void set_id(const std::string& id) {
    id_ = id;
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

  void set_unique_id(int64_t unique_id) {
    unique_id_ = unique_id;
  }

  void set_namespace(std::string namespace_name) {
    this->namespace_ = namespace_name;
  }

  bool is_pivot() const {
    return is_pivot_;
  }

  void set_pivot(bool is_pivot) {
    is_pivot_ = is_pivot;
  }

  void set_destroyed() {
    destroyed_ = true;
  }

  bool is_destroyed() const {
    return destroyed_;
  }
};

}  // namespace data_staging
