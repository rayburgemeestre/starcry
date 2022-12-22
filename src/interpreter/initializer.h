/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <optional>

namespace interpreter {
class generator;

class initializer {
public:
  explicit initializer(generator &gen);

  void init_context();
  void init_user_script();
  void init_video_meta_info(std::optional<double> rand_seed, bool preview);
  void init_gradients();
  void init_textures();
  void init_toroidals();
  void init_object_definitions();

private:
  void reset_context();

  generator &gen_;
};
}  // namespace interpreter