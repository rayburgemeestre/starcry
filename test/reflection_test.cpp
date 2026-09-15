/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <catch2/catch_test_macros.hpp>

#include "data/shape_reflect.hpp"

#if !STARCRY_HAS_REFLECTION

// Built with clang, or with gcc below 16, or without -freflection. Nothing to test; the
// case exists so the absence is visible in the report rather than silently missing.
TEST_CASE("Reflection is not available in this build") {
  SUCCEED("compiled without C++26 reflection, see docs/move_to_reflection.md");
}

#else

#include <map>
#include <string>
#include <string_view>

#include "data/shape.hpp"

namespace {

struct point {
  double x = 0;
  double y = 0;
  std::string label;
};

// Deliberately past zpp::bits::members<52>, which is the ceiling data::shape is currently
// stuck against. This is the case that demonstrates reflection removes it.
struct wide {
  double f00 = 0, f01 = 0, f02 = 0, f03 = 0, f04 = 0, f05 = 0, f06 = 0, f07 = 0, f08 = 0, f09 = 0;
  double f10 = 0, f11 = 0, f12 = 0, f13 = 0, f14 = 0, f15 = 0, f16 = 0, f17 = 0, f18 = 0, f19 = 0;
  double f20 = 0, f21 = 0, f22 = 0, f23 = 0, f24 = 0, f25 = 0, f26 = 0, f27 = 0, f28 = 0, f29 = 0;
  double f30 = 0, f31 = 0, f32 = 0, f33 = 0, f34 = 0, f35 = 0, f36 = 0, f37 = 0, f38 = 0, f39 = 0;
  double f40 = 0, f41 = 0, f42 = 0, f43 = 0, f44 = 0, f45 = 0, f46 = 0, f47 = 0, f48 = 0, f49 = 0;
  double f50 = 0, f51 = 0, f52 = 0, f53 = 0, f54 = 0, f55 = 0, f56 = 0, f57 = 0, f58 = 0, f59 = 0;
};

}  // namespace

TEST_CASE("Reflection reports the member count") {
  STATIC_REQUIRE(data::reflect::field_count<point>() == 3);
}

TEST_CASE("Reflection reports member names in declaration order") {
  constexpr auto names = data::reflect::field_names<point>();
  STATIC_REQUIRE(names.size() == 3);
  STATIC_REQUIRE(names[0] == "x");
  STATIC_REQUIRE(names[1] == "y");
  STATIC_REQUIRE(names[2] == "label");
}

TEST_CASE("for_each_field visits every member exactly once") {
  point p;
  std::map<std::string, int> seen;
  data::reflect::for_each_field(p, [&](std::string_view name, auto&) {
    seen[std::string(name)]++;
  });
  REQUIRE(seen.size() == 3);
  REQUIRE(seen["x"] == 1);
  REQUIRE(seen["y"] == 1);
  REQUIRE(seen["label"] == 1);
}

TEST_CASE("for_each_field can write through the reference") {
  point p{1.5, -2.5, "unchanged"};
  data::reflect::for_each_field(p, [](std::string_view, auto& ref) {
    if constexpr (std::is_same_v<std::remove_reference_t<decltype(ref)>, double>) ref *= 2;
  });
  REQUIRE(p.x == 3.0);
  REQUIRE(p.y == -5.0);
  REQUIRE(p.label == "unchanged");  // non-double members left alone
}

TEST_CASE("for_each_field accepts a const object") {
  const point p{7, 8, "ro"};
  int visits = 0;
  data::reflect::for_each_field(p, [&](std::string_view, const auto&) { visits++; });
  REQUIRE(visits == 3);
}

TEST_CASE("Reflection handles more members than zpp::bits::members<52> allows") {
  // The whole reason for the migration: data::shape has 52 members against a cap of 52,
  // so a 53rd field does not compile today.
  STATIC_REQUIRE(data::reflect::field_count<wide>() == 60);

  wide w;
  int visits = 0;
  data::reflect::for_each_field(w, [&](std::string_view, auto&) { visits++; });
  REQUIRE(visits == 60);
}

TEST_CASE("Reflection walks the real data::shape") {
  STATIC_REQUIRE(data::reflect::field_count<data::shape>() > 0);

  data::shape s;
  s.x = 12.5;
  s.y = -3.25;
  s.radius = 7.0;

  std::map<std::string, double> doubles;
  data::reflect::for_each_field(s, [&](std::string_view name, const auto& ref) {
    if constexpr (std::is_same_v<std::remove_cvref_t<decltype(ref)>, double>) {
      doubles[std::string(name)] = ref;
    }
  });

  // values read back through reflection match the ones written directly
  REQUIRE(doubles.at("x") == 12.5);
  REQUIRE(doubles.at("y") == -3.25);
  REQUIRE(doubles.at("radius") == 7.0);

  // and the walk covers the members shapes_to_json writes out by hand
  const auto names = data::reflect::field_names<data::shape>();
  const auto has = [&](std::string_view n) {
    return std::find(names.begin(), names.end(), n) != names.end();
  };
  REQUIRE(has("x"));
  REQUIRE(has("y"));
  REQUIRE(has("z"));
  REQUIRE(has("radius"));
  REQUIRE(has("opacity"));
}

#endif  // STARCRY_HAS_REFLECTION
