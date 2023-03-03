/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "util/step_calculator.hpp"
#include "util/v8_interact.hpp"
#include "data/shape.hpp"
#include "data_staging/shape.hpp"

namespace interpreter {
    class generator;

    class job_mapper {
    private:

    public:
        explicit job_mapper(generator &gen);

        void convert_objects_to_render_job(step_calculator& sc, v8::Local<v8::Object> video);

        void convert_object_to_render_job(data_staging::shape_t& shape,
                                          step_calculator& sc,
                                          v8::Local<v8::Object> video);

    private:
        template <typename T>
        void copy_gradient_from_object_to_shape(T& source_object,
                                                data::shape& destination_shape,
                                                std::unordered_map<std::string, data::gradient>& known_gradients_map);
        template <typename T>
        void copy_texture_from_object_to_shape(T& source_object,
                                               data::shape& destination_shape,
                                               std::unordered_map<std::string, data::texture>& known_textures_map);

        generator& gen_;
    };
}
