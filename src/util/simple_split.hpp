/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <array>
#include <string>
#include <tuple>

namespace detail {
// Helper to generate tuple type with N strings
template <std::size_t N>
struct tuple_n_strings {
  using type = decltype(std::tuple_cat(std::tuple<std::string>(), typename tuple_n_strings<N - 1>::type()));
};

template <>
struct tuple_n_strings<1> {
  using type = std::tuple<std::string>;
};

// Helper to fill remaining tuple elements with empty strings
template <std::size_t I = 0, typename Tuple>
constexpr void fill_empty(Tuple& t) {
  if constexpr (I < std::tuple_size_v<Tuple>) {
    std::get<I>(t) = std::string();
    fill_empty<I + 1>(t);
  }
}

// Helper to split string and fill tuple
template <std::size_t I = 0, typename Tuple>
constexpr void fill_splits(std::string_view msg, std::size_t prev_pos, Tuple& t, char delim) {
  if constexpr (I < std::tuple_size_v<Tuple>) {
    auto pos = msg.find(delim, prev_pos);

    if (pos == std::string_view::npos) {
      if (prev_pos < msg.length()) {
        std::get<I>(t) = std::string(msg.substr(prev_pos));
      }
      fill_empty<I + 1>(t);
      return;
    }

    std::get<I>(t) = std::string(msg.substr(prev_pos, pos - prev_pos));
    fill_splits<I + 1>(msg, pos + 1, t, delim);
  }
}
}  // namespace detail

// Main split function
template <std::size_t N>
typename detail::tuple_n_strings<N>::type split(std::string_view msg, char delim = '_') {
  typename detail::tuple_n_strings<N>::type result;
  detail::fill_splits(msg, 0, result, delim);
  return result;
}
