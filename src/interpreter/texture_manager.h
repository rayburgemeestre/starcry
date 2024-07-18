/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <unordered_map>

#include "data/texture.hpp"

namespace interpreter {
class generator;

class texture_manager {
public:
  void add_texture(const std::string& id, data::texture& texture);
  void clear();
  const std::unordered_map<std::string, data::texture>& textures_map();

private:
  std::unordered_map<std::string, data::texture> textures_;
};

}  // namespace interpreter
