/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <v8.h>
#include <v8pp/module.hpp>
#include "libplatform/libplatform.h"
#include "v8pp/class.hpp"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

namespace shapes::pos {
void add_to_context(v8pp::context &ctx);
}

namespace shapes::shape {
void add_to_context(v8pp::context &ctx);
}

namespace shapes::color {
void add_to_context(v8pp::context &ctx);
}

namespace shapes::gradient {
void add_to_context(v8pp::context &ctx);
}

namespace shapes::circle {
void add_to_context(v8pp::context &ctx);
}

namespace shapes::rectangle {
void add_to_context(v8pp::context &ctx);
}

namespace shapes::line {
void add_to_context(v8pp::context &ctx);
}
