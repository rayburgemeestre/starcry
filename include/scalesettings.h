/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>

#include "util/transaction.h"

class scale_settings : public transaction {
public:
  scale_settings() = default;
  virtual ~scale_settings() = default;

  void reset() override;
  void update();
  void revert() override;
  void commit() override;

  double video_scale = 1.;
  double video_scale_next = 1.;
  double video_scale_intermediate = 1.;
  std::vector<double> video_scales;
};
