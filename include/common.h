/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "caf/all.hpp"

using std::endl;
using namespace caf;

auto nothing = [](){};

inline void print_on_exit(const actor& hdl, const std::string& name) {
    hdl->attach_functor([=](abstract_actor *ptr, uint32_t reason) {
        aout(ptr) << name << " exited with reason " << reason << " (" << caf::exit_reason::as_string(reason) << ")" << endl;
    });
}

#include <experimental/optional>

namespace std {
    using std::experimental::optional;
    using std::experimental::make_optional;
}
