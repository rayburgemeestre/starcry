/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>

struct generator_options {
  bool debug = false;
  int custom_width = 0;
  int custom_height = 0;
  double custom_scale = 0;
  std::optional<bool> custom_grain;
  int custom_granularity = 0;
};
