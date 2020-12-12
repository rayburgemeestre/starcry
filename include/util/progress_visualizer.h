/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <chrono>

class progress_visualizer {
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> begin;
  double max_frames{};
  double max_frame_rendered{};
  double counter;

public:
  progress_visualizer();

  void initialize();

  void set_max_frames(double max_frames);
  void set_frame_rendered(double frame);

  void display(double frame);
};
