/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "toroidal_manager.h"

namespace interpreter {

void toroidal_manager::add_toroidal(const std::string& id, data::toroidal& toroidal) {
  toroidals_[id] = toroidal;
}

void toroidal_manager::clear() {
  toroidals_.clear();
}

const std::unordered_map<std::string, data::toroidal>& toroidal_manager::toroidals_map() {
  return toroidals_;
}

}  // namespace interpreter