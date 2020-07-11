/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "allegro5/allegro5.h"

class bitmap_wrapper {
private:
  ALLEGRO_BITMAP *bitmap = nullptr;
  int bitmap_w = 0;
  int bitmap_h = 0;

public:
  bitmap_wrapper() = default;
  ~bitmap_wrapper() {
    if (bitmap != nullptr) {
      al_destroy_bitmap(bitmap);
    }
  }
  ALLEGRO_BITMAP *get(int width, int height) {
    if (bitmap == nullptr) {
      bitmap = al_create_bitmap(width, height);
      bitmap_w = width;
      bitmap_h = height;
    } else {
      if (bitmap_w != width || bitmap_h != height) {
        // TODO: we're dealing with a different size, print out warning as it maybe inefficient.
        // or use a map for the various sizes..
        if (bitmap != nullptr) al_destroy_bitmap(bitmap);

        // for now recreate
        bitmap = al_create_bitmap(width, height);
        bitmap_w = width;
        bitmap_h = height;
      }
    }
    return bitmap;
  }
};
