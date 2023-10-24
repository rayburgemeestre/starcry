/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>

namespace data {
class reload_request {
private:
  std::string script_;

public:
  reload_request(std::string script) : script_(std::move(script)) {}

  const std::string& script() const {
    return script_;
  }
};
}  // namespace data
