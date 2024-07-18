/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gradient_manager.h"

namespace interpreter {

void gradient_manager::add_gradient(const std::string& id, data::gradient& gradient) {
  gradients_[id] = gradient;
}

void gradient_manager::clear() {
  gradients_.clear();
}

const std::unordered_map<std::string, data::gradient>& gradient_manager::gradients_map() {
  return gradients_;
}

}  // namespace interpreter