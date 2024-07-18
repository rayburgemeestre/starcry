/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <unordered_map>
#include <string>

#include "data/gradient.hpp"

namespace interpreter {
class generator;


class gradient_manager {
public:
  void add_gradient(const std::string& id, data::gradient& gradient);
  void clear();

private:
  std::unordered_map<std::string, data::gradient> gradients_;
};

}  // namespace interpreter
