/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "logger.h"

std::ofstream logger::fi{"sc.log", std::ios_base::out | std::ios_base::app};

metrics* _metrics = nullptr;

void set_metrics(metrics* ptr) {
  _metrics = ptr;
}