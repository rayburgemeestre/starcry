/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <map>
#include <string>

#include "util/v8_interact.hpp"

namespace data_staging {

class attrs {
private:
  std::map<std::string, std::string> strings_;
  std::map<std::string, double> numbers_;

public:
  attrs() = default;

  void set(const std::string& field, const std::string& str) {
    strings_[field] = str;
  }

  void set(const std::string& field, const double& val) {
    numbers_[field] = val;
  }

  [[nodiscard]] v8::Local<v8::Value> get(const std::string& field) const {
    if (strings_.contains(field)) {
      return v8_str(v8::Isolate::GetCurrent()->GetCurrentContext(), strings_.at(field));
    } else if (numbers_.contains(field)) {
      return v8::Number::New(v8::Isolate::GetCurrent(), numbers_.at(field));
    }
    return v8::Undefined(v8::Isolate::GetCurrent());
  }
};

}  // namespace data_staging
