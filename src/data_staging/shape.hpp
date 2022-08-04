/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <any>
#include <variant>

#include "data_staging/circle.hpp"
#include "data_staging/line.hpp"
#include "data_staging/script.hpp"

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace data_staging {
using shape_t = std::variant<std::monostate, data_staging::circle, data_staging::line, data_staging::script>;
}