/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

void fast_forwarder(bool fast_ff,
                    int frame_of_interest,
                    int& min_intermediates,
                    int& max_intermediates,
                    auto callback_generate_frame,
                    auto callback_goto_frame,
                    bool caching,
                    std::set<int>& checkpoints) {
  double current = 0;
  if (caching && checkpoints.size()) {
    int goto_frame = 0;
    for (const auto f : checkpoints) {
      if (f > frame_of_interest) break;
      goto_frame = f;
    }
    if (goto_frame) {
      callback_goto_frame(goto_frame);
      current = goto_frame;
    }
  }
  double remaining = frame_of_interest - current;
  if (fast_ff && remaining > 1) {
    int backup_min_intermediates = min_intermediates;
    int backup_max_intermediates = max_intermediates;
    min_intermediates = 1;
    max_intermediates = 1;
    for (int i = 2; i <= remaining; i++) {
      callback_generate_frame();
    }
    min_intermediates = backup_min_intermediates;
    min_intermediates = backup_max_intermediates;
    // generate frame before with same stepsize
    callback_generate_frame();
  } else if (remaining > 0) {
    for (int i = 1; i <= remaining; i++) {
      callback_generate_frame();
    }
  }
}
