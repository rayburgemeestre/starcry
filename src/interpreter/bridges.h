/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "interpreter/object_bridge.h"

namespace interpreter {

class generator;

class bridges {
private:
  generator& gen_;

  std::shared_ptr<object_bridge<data_staging::circle>> object_bridge_circle = nullptr;
  std::shared_ptr<object_bridge<data_staging::ellipse>> object_bridge_ellipse = nullptr;
  std::shared_ptr<object_bridge<data_staging::line>> object_bridge_line = nullptr;
  std::shared_ptr<object_bridge<data_staging::script>> object_bridge_script = nullptr;
  std::shared_ptr<object_bridge<data_staging::text>> object_bridge_text = nullptr;

public:
  bridges(generator& gen);
  void init();

  template <typename T>
  auto& get() {
    if constexpr (std::is_same_v<T, data_staging::circle>)
      return circle();
    else if constexpr (std::is_same_v<T, data_staging::ellipse>)
      return ellipse();
    else if constexpr (std::is_same_v<T, data_staging::line>)
      return line();
    else if constexpr (std::is_same_v<T, data_staging::script>)
      return script();
    else if constexpr (std::is_same_v<T, data_staging::text>)
      return text();
    else
      static_assert(!sizeof(T));
  }

  std::shared_ptr<object_bridge<data_staging::circle>>& circle();
  std::shared_ptr<object_bridge<data_staging::ellipse>>& ellipse();
  std::shared_ptr<object_bridge<data_staging::line>>& line();
  std::shared_ptr<object_bridge<data_staging::script>>& script();
  std::shared_ptr<object_bridge<data_staging::text>>& text();
};

}  // namespace interpreter