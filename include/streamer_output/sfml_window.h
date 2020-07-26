/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "data/color.hpp"

class sfml_window {
public:
  sfml_window();

  void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels);
  void finalize();

private:
  float ball_radius = 16.f;
  int bpp = 32;
  bool vsync = false;
  sf::RenderWindow window;
  // sf::Font font;
  // sf::Text text;
  sf::CircleShape ball;
};
