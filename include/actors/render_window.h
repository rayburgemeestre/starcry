/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "common.h"

namespace caf {
    class event_based_actor;
    class actor_system;
}

behavior render_window(caf::event_based_actor* self, uint16_t port);
