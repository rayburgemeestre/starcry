/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>

namespace data {
struct job;
}

namespace interpreter {

class job_holder {
  std::shared_ptr<data::job> job;
public:

  void init();

  std::shared_ptr<data::job> get() const;
  std::shared_ptr<data::job>& get_ref();

};

}
