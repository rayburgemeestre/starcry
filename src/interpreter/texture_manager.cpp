/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "texture_manager.h"

namespace interpreter {

void texture_manager::add_texture(const std::string& id, data::texture& texture) {
  textures_[id] = texture;
}

void texture_manager::clear() {
  textures_.clear();
}

const std::unordered_map<std::string, data::texture>& texture_manager::textures_map() {
  return textures_;
}

}  // namespace interpreter
