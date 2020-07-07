/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <allegro5/color.h>
#include <vector>

namespace caf {
class event_based_actor;
class actor;
class actor_system;
}  // namespace caf

class allegro5_window {
public:
  allegro5_window(actor_system &system, caf::event_based_actor *self, int port);

  void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels);
  void finalize();

private:
  caf::event_based_actor *self_;
  caf::actor_system &system_;
  std::unique_ptr<caf::actor> client_;
  uint32_t port_;
};