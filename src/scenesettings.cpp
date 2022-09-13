/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "scenesettings.h"

void scene_settings::reset() {}

bool scene_settings::update(double t) {
  if (scene_durations[current_scene_next] < (t - offset_next)) {
    if (scene_durations.size() - 1 > current_scene_next) {
      offset_next += scene_durations[current_scene_next];
      return true;
    }
  }
  return false;
}

void scene_settings::update() {
  current_scene_intermediate = current_scene_next;
  offset_intermediate = offset_next;
}

void scene_settings::revert() {
  current_scene_next = current_scene;
  current_scene_intermediate = current_scene;
  offset_next = offset;
  offset_intermediate = offset;
  scene_initialized = scene_initialized_previous;
}

void scene_settings::commit() {
  current_scene = current_scene_next;
  offset = offset_next;
}
