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
#include <sstream>
#include <fstream>
#include <utility>

#include "cereal/archives/json.hpp"
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

// The by-hand serializers were deleted when clang support was dropped, so these goldens
// are what remains of the equivalence check: they were captured from the hand-written
// implementations while both were still present and verified identical. They are the
// regression guard on the reflected output.
namespace {
constexpr const char* golden_json_default = R"GOLDEN({"align":"","b":0.0,"blending_":"normal","dist":0.0,"g":0.0,"gradient_id_str":"","gradients_":[],"hue":0.0,"id":"","indexes":[],"label":"","level":0,"longest_diameter":0.0,"motion_blur":true,"opacity":1.0,"r":0.0,"radius":0.0,"radius_size":0.0,"random_hash":"","recursive_scale":1.0,"rotate":0.0,"scale":1.0,"seed":0.0,"shortest_diameter":0.0,"steps":0,"text":"","text_fixed":false,"text_font":"","text_size":10.0,"texture_3d_":"raw","texture_effect_":"opacity","texture_id_str":"","texture_offset_x":0.0,"texture_offset_y":0.0,"textures":[],"time":0.0,"type":"none","unique_id":0,"vel_x":0.0,"vel_y":0.0,"velocity":0.0,"warp_height":0,"warp_width":0,"warp_x":0,"warp_y":0,"x":0.0,"x2":0.0,"y":0.0,"y2":0.0,"z":0.0,"z2":0.0,"zernike_type_":"version1"})GOLDEN";
constexpr const char* golden_json_sample = R"GOLDEN({"align":"left","b":0.0,"blending_":"normal","dist":1.5,"g":0.0,"gradient_id_str":"","gradients_":[],"hue":0.5,"id":"ident","indexes":[],"label":"lbl","level":2,"longest_diameter":0.0,"motion_blur":false,"opacity":0.25,"r":0.0,"radius":11.0,"radius_size":2.0,"random_hash":"","recursive_scale":1.0,"rotate":0.0,"scale":1.0,"seed":0.0,"shortest_diameter":0.0,"steps":4,"text":"cereal","text_fixed":false,"text_font":"","text_size":13.0,"texture_3d_":"raw","texture_effect_":"opacity","texture_id_str":"","texture_offset_x":0.0,"texture_offset_y":0.0,"textures":[],"time":0.0,"type":"ellipse","unique_id":77,"vel_x":0.0,"vel_y":0.0,"velocity":0.0,"warp_height":0,"warp_width":0,"warp_x":0,"warp_y":0,"x":3.5,"x2":0.0,"y":-1.25,"y2":0.0,"z":0.75,"z2":0.0,"zernike_type_":"version1"})GOLDEN";
constexpr const char* golden_cereal_default = R"GOLDEN({
    "value0": 0.0,
    "value1": 0.0,
    "value2": 0.0,
    "value3": 0.0,
    "value4": 0.0,
    "value5": 0.0,
    "value6": 0.0,
    "value7": 0.0,
    "value8": 0.0,
    "value9": 0.0,
    "value10": 0,
    "value11": 0.0,
    "value12": 0.0,
    "value13": 0.0,
    "value14": 0.0,
    "value15": 0.0,
    "value16": 0.0,
    "value17": 0.0,
    "value18": 10.0,
    "value19": 0.0,
    "value20": "",
    "value21": "",
    "value22": false,
    "value23": "",
    "value24": [],
    "value25": [],
    "value26": {
        "value0": 0
    },
    "value27": {
        "value0": 0
    },
    "value28": {
        "value0": 0
    },
    "value29": 0.0,
    "value30": 0.0,
    "value31": "",
    "value32": "",
    "value33": {
        "value0": 0
    },
    "value34": [],
    "value35": 0.0,
    "value36": 1.0,
    "value37": 1.0,
    "value38": 1.0,
    "value39": 0.0,
    "value40": 0,
    "value41": "",
    "value42": "",
    "value43": 0,
    "value44": "",
    "value45": true,
    "value46": 0,
    "value47": 0,
    "value48": 0,
    "value49": 0,
    "value50": 0.0,
    "value51": 0
})GOLDEN";
constexpr const char* golden_cereal_sample = R"GOLDEN({
    "value0": 0.0,
    "value1": 3.5,
    "value2": -1.25,
    "value3": 0.75,
    "value4": 0.0,
    "value5": 0.0,
    "value6": 0.0,
    "value7": 0.0,
    "value8": 0.0,
    "value9": 0.0,
    "value10": 3,
    "value11": 0.0,
    "value12": 0.0,
    "value13": 0.0,
    "value14": 11.0,
    "value15": 0.0,
    "value16": 0.0,
    "value17": 0.0,
    "value18": 13.0,
    "value19": 2.0,
    "value20": "cereal",
    "value21": "left",
    "value22": false,
    "value23": "",
    "value24": [],
    "value25": [],
    "value26": {
        "value0": 0
    },
    "value27": {
        "value0": 0
    },
    "value28": {
        "value0": 0
    },
    "value29": 0.0,
    "value30": 0.0,
    "value31": "",
    "value32": "",
    "value33": {
        "value0": 0
    },
    "value34": [],
    "value35": 0.0,
    "value36": 1.0,
    "value37": 1.0,
    "value38": 0.25,
    "value39": 0.5,
    "value40": 77,
    "value41": "ident",
    "value42": "lbl",
    "value43": 2,
    "value44": "",
    "value45": false,
    "value46": 0,
    "value47": 0,
    "value48": 0,
    "value49": 0,
    "value50": 1.5,
    "value51": 4
})GOLDEN";

data::shape sample_shape() {
  data::shape s;
  s.x = 3.5; s.y = -1.25; s.z = 0.75;
  s.radius = 11; s.radius_size = 2; s.text_size = 13;
  s.text = "cereal"; s.align = "left"; s.label = "lbl"; s.id = "ident";
  s.level = 2; s.motion_blur = false; s.opacity = 0.25; s.hue = 0.5;
  s.unique_id = 77; s.steps = 4; s.dist = 1.5;
  s.type = data::shape_type::ellipse;
  return s;
}
}  // namespace

TEST_CASE("shape_to_json output matches the captured golden") {
  nlohmann::json a, b;
  data::shape_to_json(data::shape{}, a);
  data::shape_to_json(sample_shape(), b);
  REQUIRE(a.dump() == golden_json_default);
  REQUIRE(b.dump() == golden_json_sample);
}

TEST_CASE("cereal archive matches the captured golden") {
  std::ostringstream d_os, s_os;
  { cereal::JSONOutputArchive ar(d_os); data::shape d; d.serialize(ar); }
  { cereal::JSONOutputArchive ar(s_os); auto s = sample_shape(); s.serialize(ar); }
  REQUIRE(d_os.str() == golden_cereal_default);
  REQUIRE(s_os.str() == golden_cereal_sample);
}

TEST_CASE("every shape member reaches the json output") {
  nlohmann::json j;
  data::shape_to_json(data::shape{}, j);
  size_t keys = 0;
  for (auto it = j.begin(); it != j.end(); ++it) keys++;
  REQUIRE(keys == data::reflect::field_count<data::shape>());
}

#endif  // STARCRY_HAS_REFLECTION
