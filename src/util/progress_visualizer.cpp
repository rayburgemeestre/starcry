/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/progress_visualizer.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <sstream>

std::mutex progress_visualizer_mut;

progress_visualizer::progress_visualizer(std::string elem)
    : begin(std::chrono::high_resolution_clock::now()), counter(0), elem(std::move(elem)) {}

void progress_visualizer::initialize() {
  begin = std::chrono::high_resolution_clock::now();
  begin_for_item = begin;
  counter = 0;
}

void progress_visualizer::set_max_frames(double max_frames) {
  this->max_frames = max_frames;
}

void progress_visualizer::set_frame_rendered(double max_frame_rendered) {
  this->max_frame_rendered = std::max(this->max_frame_rendered, max_frame_rendered);
  this->counter++;
}

void progress_visualizer::set_start_timing() {
  begin_for_item = std::chrono::high_resolution_clock::now();
}

void progress_visualizer::display(double frame) {
  std::unique_lock<std::mutex> lock(progress_visualizer_mut);
  set_frame_rendered(frame);
  std::stringstream msg;
  auto current = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> idle = current - begin;
  double fps = 1000. / (idle.count() / counter);
  double remaining = (idle.count() / 1000. / counter) * (max_frames - counter);
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  auto cols = std::floor(size.ws_col);
  std::chrono::duration<double, std::milli> since_last = current - begin_for_item;
  msg << elem << ": " << max_frame_rendered << " FPS: " << fps << " ETA: " << remaining
      << " Since last: " << since_last.count();
  std::cout << "\r" << msg.str();
  for (decltype(cols) i = msg.str().size(); i < cols; i++) {
    std::cout << " ";
  }
  std::cout << "\n";
  for (decltype(cols) i = 0; i < cols; i++) {
    bool progress = (max_frame_rendered / max_frames * cols) < i;
    std::cout << (progress ? "░" : "█");
  }
  std::cout.flush();
  begin_for_item = current;
}
