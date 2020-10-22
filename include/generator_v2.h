/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>

#include <cstddef>
#include <cstdint>

#include "data/job.hpp"

class generator_v2 {
private:
public:
  generator_v2();
  ~generator_v2() = default;

  void init(const std::string &filename);
  bool generate_frame();
  std::shared_ptr<data::job> get_job() const;
};

void call_print_exception(const std::string &fn);
template <typename T>
void call_print_exception(const std::string &fn, T arg);
