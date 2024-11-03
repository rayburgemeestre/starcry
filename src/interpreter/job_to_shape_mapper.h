/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data/shape.hpp"
#include "data_staging/shape.hpp"
#include "job_holder.h"
#include "util/step_calculator.hpp"
#include "util/v8_interact.hpp"

class frame_stepper;
class scale_settings;
struct generator_state;

namespace interpreter {
class gradient_manager;
class texture_manager;
class job_holder;
class scenes;

class job_to_shape_mapper {
public:
  explicit job_to_shape_mapper(gradient_manager& gm,
                               texture_manager& tm,
                               job_holder& holder,
                               frame_stepper& stepper,
                               scenes& scenes,
                               scale_settings& scalesettings,
                               generator_state& state);

  void reset();

  void convert_objects_to_render_job(step_calculator& sc, v8::Local<v8::Object> video);

  void convert_object_to_render_job(data_staging::shape_t& shape, step_calculator& sc, v8::Local<v8::Object> video);

private:
  template <typename T>
  void copy_gradient_from_object_to_shape(T& source_object,
                                          data::shape& destination_shape,
                                          const std::unordered_map<std::string, data::gradient>& known_gradients_map);
  template <typename T>
  void copy_texture_from_object_to_shape(T& source_object,
                                         data::shape& destination_shape,
                                         const std::unordered_map<std::string, data::texture>& known_textures_map);

  gradient_manager& gradient_manager_;
  texture_manager& texture_manager_;
  job_holder& job_holder_;
  frame_stepper& frame_stepper_;
  scenes& scenes_;
  scale_settings& scalesettings_;
  generator_state& state_;

  std::unordered_map<size_t, std::map<int, size_t>> indexes;
};
}  // namespace interpreter
