/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "bridges.h"

namespace interpreter {
bridges::bridges(generator& gen) : gen_(gen) {}

void bridges::init() {
  object_bridge_circle = std::make_shared<object_bridge<data_staging::circle>>(&gen_);
  object_bridge_ellipse = std::make_shared<object_bridge<data_staging::ellipse>>(&gen_);
  object_bridge_line = std::make_shared<object_bridge<data_staging::line>>(&gen_);
  object_bridge_text = std::make_shared<object_bridge<data_staging::text>>(&gen_);
  object_bridge_script = std::make_shared<object_bridge<data_staging::script>>(&gen_);
}

std::shared_ptr<object_bridge<data_staging::circle>>& bridges::circle() {
  return object_bridge_circle;
}

std::shared_ptr<object_bridge<data_staging::ellipse>>& bridges::ellipse() {
  return object_bridge_ellipse;
}

std::shared_ptr<object_bridge<data_staging::line>>& bridges::line() {
  return object_bridge_line;
}

std::shared_ptr<object_bridge<data_staging::script>>& bridges::script() {
  return object_bridge_script;
}

std::shared_ptr<object_bridge<data_staging::text>>& bridges::text() {
  return object_bridge_text;
}
}  // namespace interpreter