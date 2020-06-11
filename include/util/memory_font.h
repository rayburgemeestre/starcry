/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_memfile.h>
#include <memory>

class memory_font {
public:
  enum class fonts {
    monaco = 1,
  };

  memory_font(fonts font, int size, int flags = 0);

  const ALLEGRO_FONT* font() const;

private:
  void initialize(unsigned char* font_data, size_t font_data_len, const char* font_filename);

  const fonts font_;
  int size_;
  int flags_;
  std::unique_ptr<ALLEGRO_FILE, decltype(&al_fclose)> allegro_file_;
  std::unique_ptr<ALLEGRO_FONT, decltype(&al_destroy_font)> allegro_font_;
};
