/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <SFML/Graphics.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "data/color.hpp"

class sfml_window {
public:
  sfml_window();
  ~sfml_window();

  void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels);
  void finalize();

private:
  void update_window();

  int bpp = 32;
  bool vsync = false;
  sf::RenderWindow window;
  std::thread runner;
  std::atomic<bool> runner_flag = false;
  std::mutex mut;

  uint32_t cached_canvas_w = 0;
  uint32_t cached_canvas_h = 0;
  std::vector<uint32_t> cached_pixels;
};