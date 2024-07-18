/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "texture_factory.h"

#include "util/v8_interact.hpp"

#include "data/texture.hpp"
#include "data/zernike_type.hpp"

namespace interpreter {

data::texture texture_factory::create_from_object(v8_interact& i, v8::Local<v8::Object>& texture_settings) {
  auto type = i.str(texture_settings, "type");
  auto zernike_type = i.integer_number(texture_settings, "zernike_type");
  auto texture_effect = i.integer_number(texture_settings, "effect");
  // perlin
  data::texture new_texture;
  new_texture.size = i.double_number(texture_settings, "size");
  new_texture.octaves = i.integer_number(texture_settings, "octaves");
  new_texture.persistence = i.double_number(texture_settings, "persistence");
  new_texture.percentage = i.double_number(texture_settings, "percentage");
  new_texture.scale = i.double_number(texture_settings, "scale");
  auto range = i.v8_array(texture_settings, "range");
  new_texture.strength = i.double_number(texture_settings, "strength");
  new_texture.speed = i.double_number(texture_settings, "speed");
  // zernikes
  new_texture.m = i.double_number(texture_settings, "m");
  new_texture.n = i.double_number(texture_settings, "n");
  new_texture.rho = i.double_number(texture_settings, "rho");
  new_texture.theta = i.double_number(texture_settings, "theta");

  data::texture::noise_type use_type = data::texture::noise_type::perlin;
  if (type == "fractal") {
    use_type = data::texture::noise_type::fractal;
  } else if (type == "turbulence") {
    use_type = data::texture::noise_type::turbulence;
  } else if (type == "zernike") {
    if (zernike_type == data::zernike_type::version1) {
      use_type = data::texture::noise_type::zernike_1;
    } else if (zernike_type == data::zernike_type::version2) {
      use_type = data::texture::noise_type::zernike_2;
    }
  }
  new_texture.type = use_type;
  new_texture.effect = data::texture::texture_effect(texture_effect);
  if (range->Length() == 4) {
    new_texture.fromX = i.double_number(range, 0);
    new_texture.begin = i.double_number(range, 1);
    new_texture.end = i.double_number(range, 2);
    new_texture.toX = i.double_number(range, 3);
  }
  return new_texture;
}

}  // namespace interpreter
