/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <atomic>

class actor_info
{
public:
    std::atomic<bool> flag_;
    actor_info(const caf::actor &actor) : flag_(true) {
        actor->attach_functor([=](abstract_actor* ptr, uint32_t reason) {
            aout(ptr) << "renderer" << " exited with reason " << reason << endl;
            flag_.store(false);
        });
    }
    bool running () { return flag_; }
};
