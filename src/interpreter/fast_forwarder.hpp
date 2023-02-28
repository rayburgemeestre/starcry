/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

void fast_forwarder(
    bool fast_ff, int frame_of_interest, int& min_intermediates, int& max_intermediates, auto callback) {
  if (fast_ff && frame_of_interest > 2) {
    int backup_min_intermediates = min_intermediates;
    int backup_max_intermediates = max_intermediates;
    min_intermediates = 1;
    max_intermediates = 1;
    for (int i = 2; i < frame_of_interest; i++) {
      callback();
    }
    min_intermediates = backup_min_intermediates;
    min_intermediates = backup_max_intermediates;
    // generate frame before with same stepsize
    callback();
  } else if (frame_of_interest > 1) {
    for (int i = 1; i < frame_of_interest; i++) {
      callback();
    }
  }
}
