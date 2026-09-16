/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// Compile-time member iteration via C++26 reflection (P2996).
//
// Reflection is gcc-only and needs gcc >= 16 with -freflection. clang implements it at no
// version, so clang can no longer build starcry -- see docs/move_to_reflection.md for what
// to reinstate if that changes. Failing here with a clear message beats several hundred
// lines of template errors from <meta>.
//
// See docs/move_to_reflection.md.

#if !defined(__cpp_impl_reflection) || __cpp_impl_reflection < 202506
#error "starcry requires C++26 reflection (P2996): build with g++-16 -std=c++26 -freflection. \
clang implements it at no version; see docs/move_to_reflection.md."
#endif

#define STARCRY_HAS_REFLECTION 1

#include <array>
#include <cstddef>
#include <meta>
#include <string_view>
#include <type_traits>

namespace data::reflect {

// Number of non-static data members of T, usable in constant expressions.
//
// Unlike the structured-binding approach zpp_bits uses, this has no upper bound. That is
// the point: data::shape sits at exactly 52 members against zpp::bits::members<52>, so it
// cannot currently grow.
template <typename T>
consteval std::size_t field_count() {
  return std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()).size();
}

// Member names in declaration order.
template <typename T>
consteval auto field_names() {
  std::array<std::string_view, field_count<T>()> out{};
  std::size_t i = 0;
  template for (constexpr auto m : std::define_static_array(
                    std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
    out[i++] = std::meta::identifier_of(m);
  }
  return out;
}

// Invoke f(name, member_ref) for each non-static data member, in declaration order.
//
// Takes a forwarding reference so the same overload serves reads (const T&) and writes
// (T&); the reference handed to f keeps the constness of obj, so a mutable call can
// assign through it.
template <typename T, typename F>
constexpr void for_each_field(T&& obj, F&& f) {
  using U = std::remove_cvref_t<T>;
  template for (constexpr auto m : std::define_static_array(
                    std::meta::nonstatic_data_members_of(^^U, std::meta::access_context::current()))) {
    f(std::meta::identifier_of(m), obj.[:m:]);
  }
}

}  // namespace data::reflect

