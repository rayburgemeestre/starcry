/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <X11/Xlib.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

enum class ScaleMode {
  NONE,          // Original size, no scaling
  STRETCH,       // Stretch to fill window completely
  STRETCH_RATIO  // Stretch maintaining aspect ratio, black bars
};

class gui_window {
public:
  gui_window();
  ~gui_window();

  void add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels);
  void finalize();
  void toggle_window();

private:
  void update_window();
  void stop();

  bool running = true;

  int bpp = 32;
  bool vsync = true;

  std::thread runner;
  std::atomic<bool> runner_flag = false;
  std::mutex mut;

  uint32_t cached_canvas_w = 0;
  uint32_t cached_canvas_h = 0;
  std::vector<uint32_t> cached_pixels;

  int window_width_ = 0;
  int window_height_ = 0;

  int current_width = 0;
  int current_height = 0;

  ScaleMode scale_mode = ScaleMode::STRETCH_RATIO;

  // X11 related
  Display *display = nullptr;
  int screen = -1;
  Window root;
  Window window;
  GC gc;
  XImage *image = nullptr;
  Atom wmDeleteWindow;
};