/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <memory>

#include "util/logger.h"
#include "util/v8_interact.hpp"
#include "util/vector_logic.hpp"

#include "v8.h"

enum class cascade_type {
  disabled,
  enabled,
  center,
  start,
  end,
};

namespace data_staging {
class cascade {
private:
  cascade_type type_ = cascade_type::disabled;
  int64_t unique_id_ = 0;

public:
  cascade() = default;
  cascade(cascade_type type, int64_t unique_id) : type_(type), unique_id_(unique_id) {}

  cascade_type type() const {
    return type_;
  }
  int64_t unique_id() const {
    return unique_id_;
  }
};
}  // namespace data_staging