/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "bridges.h"

#include "interpreter/spawner.h"

namespace interpreter {

bridges::bridges(object_definitions& definitions) : definitions_(definitions) {}

void bridges::init(spawner& spawner_) {
  object_bridge_circle = std::make_shared<object_bridge<data_staging::circle>>(definitions_, spawner_);
  object_bridge_ellipse = std::make_shared<object_bridge<data_staging::ellipse>>(definitions_, spawner_);
  object_bridge_line = std::make_shared<object_bridge<data_staging::line>>(definitions_, spawner_);
  object_bridge_text = std::make_shared<object_bridge<data_staging::text>>(definitions_, spawner_);
  object_bridge_script = std::make_shared<object_bridge<data_staging::script>>(definitions_, spawner_);
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