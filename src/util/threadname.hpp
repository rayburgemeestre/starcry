/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fmt/core.h>
#include <sys/prctl.h>
#include <utility>

template <typename... Args>
void set_thread_name(Args&&... args) {
  auto formatted_string = fmt::format(std::forward<Args>(args)...);
  prctl(PR_SET_NAME, formatted_string.c_str(), NULL, NULL, NULL);
}