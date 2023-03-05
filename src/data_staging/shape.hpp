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
#include "data_staging/text.hpp"

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace data_staging {
using shape_t =
    std::variant<std::monostate, data_staging::circle, data_staging::line, data_staging::text, data_staging::script>;
}

void meta_visit(auto& shape, auto&& handle_circle, auto&& handle_line, auto&& handle_text, auto&& handle_script) {
  if (shape.valueless_by_exception()) {
    return;
  }
  std::visit(overloaded{[](std::monostate) {}, handle_circle, handle_line, handle_text, handle_script}, shape);
}

template <typename T>
void meta_callback(T& shape, auto&& callback) {
  meta_visit(
      shape,
      [&callback](data_staging::circle& c) {
        callback(c);
      },
      [&callback](data_staging::line& l) {
        callback(l);
      },
      [&callback](data_staging::text& t) {
        callback(t);
      },
      [&callback](data_staging::script& s) {
        callback(s);
      });
}

template <typename T>
void meta_callback(const T& shape, auto&& callback) {
  meta_visit(
      shape,
      [&callback](const data_staging::circle& c) {
        callback(c);
      },
      [&callback](const data_staging::line& l) {
        callback(l);
      },
      [&callback](const data_staging::text& t) {
        callback(t);
      },
      [&callback](const data_staging::script& s) {
        callback(s);
      });
}