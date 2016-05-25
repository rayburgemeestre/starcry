/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "common.h"

#include "rendering_engine.hpp"
//class ALLEGRO_BITMAP;
struct worker_data
{
    size_t worker_num                   = 0;
    ALLEGRO_BITMAP * bitmap             = nullptr;
    rendering_engine engine; // TODO: convert to unique_ptr ?
    uint32_t width                      = 0;
    uint32_t height                     = 0;
};

behavior worker(caf::stateful_actor<worker_data> * self, /*const caf::actor &renderer,*/ size_t worker_num, bool remote);

behavior renderer(event_based_actor* self, const caf::actor &job_storage, const caf::actor &streamer, int range_begin, int range_end);
