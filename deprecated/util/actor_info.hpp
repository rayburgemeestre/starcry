/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <atomic>

#include "caf/error.hpp"

class actor_info {
public:
  std::atomic<bool> flag_;
  actor_info(const std::string &actor_name, const caf::actor &actor) : flag_(true) {
    actor->attach_functor([=](const caf::error &reason) {
      std::cout << "actor " << actor_name << " exited: " << actor.home_system().render(reason) << std::endl;
      flag_.store(false);
    });
  }
  bool running() {
    return flag_;
  }
};
