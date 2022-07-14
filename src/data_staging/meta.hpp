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
  int64_t parent_uid_id_;
  int64_t level_;

public:
  meta(std::string id, int64_t unique_id) : id_(std::move(id)), unique_id_(unique_id) {}

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
};

}  // namespace data_staging
