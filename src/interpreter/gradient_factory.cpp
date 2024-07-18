/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gradient_factory.h"

#include "data/gradient.hpp"
#include "util/logger.h"

namespace interpreter {

  data::gradient gradient_factory::create_from_array(const v8::Local<v8::Array>& positions, v8_interact& i) {
    data::gradient new_gradient;
    for (size_t l = 0; l < positions->Length(); l++) {
      auto position = i.get_index(positions, l).As<v8::Object>();
      auto pos = i.double_number(position, "position");
      auto r = i.double_number(position, "r");
      auto g = i.double_number(position, "g");
      auto b = i.double_number(position, "b");
      auto a = i.double_number(position, "a");
      new_gradient.colors.emplace_back(pos, data::color{r, g, b, a});
    }
    return new_gradient;
  }

  data::gradient gradient_factory::create_from_string(const std::string& color_string) {
    data::gradient new_gradient;
    auto r = std::stoi(color_string.substr(1, 2), nullptr, 16) / 255.;
    auto g = std::stoi(color_string.substr(3, 2), nullptr, 16) / 255.;
    auto b = std::stoi(color_string.substr(5, 2), nullptr, 16) / 255.;
    double index = 0.9;

    if (color_string.length() >= 8 && color_string[7] == '@') {
      try {
        auto remainder = std::stod(color_string.substr(8));
        index = std::clamp(remainder, 0.0, 1.0);
      } catch (const std::exception& e) {
        logger(DEBUG) << "Error parsing color string: " << e.what() << std::endl;
      }
    }

    new_gradient.colors.emplace_back(0.0, data::color{r, g, b, 1.});
    new_gradient.colors.emplace_back(index, data::color{r, g, b, 1.});
    new_gradient.colors.emplace_back(1.0, data::color{r, g, b, 0.});

    return new_gradient;
  }

}
