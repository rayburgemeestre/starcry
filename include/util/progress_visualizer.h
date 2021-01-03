/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <chrono>
#include <string>

class progress_visualizer {
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> begin;
  std::chrono::time_point<std::chrono::high_resolution_clock> begin_for_item;
  double max_frames{};
  double max_frame_rendered{};
  double counter;
  std::string elem;
  int offset;

public:
  progress_visualizer(std::string elem = "Frame", int offset = 0);

  void initialize();

  void set_max_frames(double max_frames);
  void set_frame_rendered(double frame);
  void set_start_timing();

  void display(double frame, int chunk = 1, int num_chunks = 1);
};
