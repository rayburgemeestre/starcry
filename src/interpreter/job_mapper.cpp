/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "job_mapper.h"
#include "generator.h"

namespace interpreter {
    job_mapper::job_mapper(generator &gen) : gen_(gen) {

    }

    void job_mapper::convert_objects_to_render_job(step_calculator& sc, v8::Local<v8::Object> video) {
        //  // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
        //  // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
        //  for (size_t index = 0; index < next_instances->Length(); index++) {
        //    auto instance = i.get_index(next_instances, index).As<v8::Object>();
        //    if (!instance->IsObject()) continue;
        //    convert_object_to_render_job(i, instance, index, sc, video);
        //  }
        for (auto& shape :gen_. scenes_.next_shapes_current_scene()) {
            bool skip = false;
            meta_callback(shape, [&]<typename T>(const T& shape) {
                if (shape.meta_cref().is_destroyed()) {
                    skip = true;
                }
            });
            if (!skip) convert_object_to_render_job(shape, sc, video);
        }
    }

    void job_mapper::convert_object_to_render_job(data_staging::shape_t& shape,
                                                 step_calculator& sc,
                                                 v8::Local<v8::Object> video) {
        data::shape new_shape;

        const auto initialize = [&]<typename T>(T& shape) {
            auto level = 0;  // shape.level;
            // See if we require this step for this object
            // auto steps = i.integer_number(instance, "steps");
            // if (minimize_steps_per_object && !sc.do_step(steps, stepper.next_step)) {
            // TODO: make this a property also for objects, if they are vibrating they need this
            //  return;
            //}
            // auto id = i.str(instance, "id");
            // auto label = i.str(instance, "label");
            // auto time = i.double_number(instance, "__time__");

            // auto radius = shape.radius();           // i.double_number(instance, "radius");
            // auto radiussize = shape.radius_size();  // i.double_number(instance, "radiussize");
            auto seed = gen_.seed;
            if constexpr (std::is_same_v<T, data_staging::circle> || std::is_same_v<T, data_staging::line>) {
                seed = shape.styling_cref().seed();
            }
            auto scale = shape.generic_cref().scale();

            auto shape_opacity = shape.generic_cref().opacity();
            auto warp_width = shape.toroidal_ref().warp_width();
            auto warp_height = shape.toroidal_ref().warp_height();

            // auto text_font = i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "";

            // TODO: might not need this param after all
// auto dist = i.double_number(instance, "__dist__");
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
// auto random_hash = i.str(instance, "__random_hash__");
#endif

            // temp
            new_shape.level = level;
            new_shape.time = shape.meta_cref().get_time();
            // new_shape.dist = dist;

            new_shape.gradients_.clear();
            new_shape.textures.clear();
            std::string gradient_id_str;

            copy_gradient_from_object_to_shape(shape, new_shape, gen_.gradients);
            copy_texture_from_object_to_shape(shape, new_shape, gen_.textures);

            // temp hack
            std::string namespace_;
            std::string gradient_id;
            if constexpr (!std::is_same_v<T, data_staging::script>) {
                gradient_id = shape.styling_cref().gradient();
                new_shape.hue = shape.styling_cref().hue();
            }

            if (!gradient_id.empty()) {
                if (new_shape.gradients_.empty()) {
                    auto& known_gradients_map = gen_.gradients;
                    if (known_gradients_map.contains(gradient_id)) {
                        new_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
                    }
                }
            }

            if (new_shape.gradients_.empty()) {
                new_shape.gradients_.emplace_back(1.0, data::gradient{});
                new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
                new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
                new_shape.gradients_[0].second.colors.emplace_back(1.0, data::color{0.0, 0, 0, 1});
            }
            new_shape.z = 0;
            // new_shape.vel_x = vel_x;
            // new_shape.vel_y = vel_y;
            if constexpr (!std::is_same_v<T, data_staging::script>) {
                new_shape.blending_ = shape.styling_cref().blending_type();
            }
            new_shape.scale = scale;
            new_shape.opacity = std::isnan(shape_opacity) ? 1.0 : shape_opacity;
            // new_shape.unique_id = unique_id;
#ifdef DEBUG_NUM_SHAPES
            // new_shape.random_hash = random_hash;
#endif
            new_shape.seed = seed;
            new_shape.id = shape.meta_cref().id();
            // new_shape.label = label;
            // new_shape.motion_blur = motion_blur;
            new_shape.warp_width = warp_width;
            new_shape.warp_height = warp_height;

            // }
            // wrap this in a proper add method
            if (gen_.stepper.next_step != gen_.stepper.max_step) {
                gen_.indexes[shape.meta_cref().unique_id()][gen_.stepper.current_step] = gen_.job->shapes[gen_.stepper.current_step].size();
            } else {
                new_shape.indexes = gen_.indexes[shape.meta_cref().unique_id()];
            }
            // logger(INFO) << "sizeof shape: " << sizeof(new_shape) << " circle was size: " << sizeof(shape) <<
            // std::endl;
            //                   if (job->shapes[stepper.current_step].capacity() < 10000) {
            //                     logger(INFO) << "resizing to fix capacity" << std::endl;
            //                     job->shapes[stepper.current_step].reserve(10000);
            //                   }
            // why is this shit super slow
            gen_.job->shapes[gen_.stepper.current_step].emplace_back(std::move(new_shape));
            // and this reasonably fast
            // job->shapes_prototype_test[stepper.current_step].emplace_back(shape);
            // job->shapes[stepper.current_step].emplace_back(data::shape{});
            //                   if (job->shapes[stepper.current_step].size() > 9999)
            //                     logger(INFO) << "current_step = " << stepper.current_step << ", shapes: " <<
            //                     job->shapes[stepper.current_step].size() << std::endl;
            gen_.job->scale = gen_.scalesettings.video_scale;
            gen_.job->scales = gen_.scalesettings.video_scales;
        };

        // Update level for all objects
        meta_visit(
                shape,
                [&](data_staging::circle& shape) {
                    new_shape.type = data::shape_type::circle;
                    new_shape.radius = shape.radius();
                    new_shape.radius_size = shape.radius_size();
                    new_shape.x = shape.transitive_location_ref().position_cref().x;
                    new_shape.y = shape.transitive_location_ref().position_cref().y;
                    initialize(shape);
                },
                [&](data_staging::line& shape) {
                    new_shape.type = data::shape_type::line;
                    new_shape.radius = 0;
                    new_shape.radius_size = shape.line_width();
                    new_shape.x = shape.transitive_line_start_ref().position_cref().x;
                    new_shape.y = shape.transitive_line_start_ref().position_cref().y;
                    new_shape.x2 = shape.transitive_line_end_ref().position_cref().x;
                    new_shape.y2 = shape.transitive_line_end_ref().position_cref().y;
                    initialize(shape);
                },
                [&](data_staging::text& shape) {
                    new_shape.type = data::shape_type::text;
                    new_shape.text_font = shape.font_name();
                    new_shape.text = shape.text_cref();
                    new_shape.text_size = shape.text_size();
                    new_shape.align = shape.text_align();
                    new_shape.text_fixed = shape.text_fixed();
                    // TODO: new_shape.text_font = shape.text_font();
                    new_shape.x = shape.transitive_location_ref().position_cref().x;
                    new_shape.y = shape.transitive_location_ref().position_cref().y;
                    initialize(shape);
                },
                [&](data_staging::script& shape) {
                    new_shape.type = data::shape_type::script;
                    new_shape.x = shape.transitive_location_ref().position_cref().x;
                    new_shape.y = shape.transitive_location_ref().position_cref().y;
                    initialize(shape);
                });
    }

    template <typename T>
    void job_mapper::copy_gradient_from_object_to_shape(
            T& source_object,
            data::shape& destination_shape,
            std::unordered_map<std::string, data::gradient>& known_gradients_map) {
        if constexpr (!std::is_same_v<T, data_staging::script>) {
            std::string namespace_ = source_object.meta_cref().namespace_name();
            std::string gradient_id = namespace_ + source_object.styling_ref().gradient();

            if (!gradient_id.empty()) {
                if (destination_shape.gradients_.empty()) {
                    if (known_gradients_map.find(gradient_id) != known_gradients_map.end()) {
                        destination_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
                    }
                }
            }
            for (const auto& [opacity, gradient_id] : source_object.styling_ref().get_gradients_cref()) {
                destination_shape.gradients_.emplace_back(opacity, known_gradients_map[gradient_id]);
            }
        }
    }

    template <typename T>
    void job_mapper::copy_texture_from_object_to_shape(T& source_object,
                                                      data::shape& destination_shape,
                                                      std::unordered_map<std::string, data::texture>& known_textures_map) {
        if constexpr (!std::is_same_v<T, data_staging::script>) {
            std::string namespace_ = source_object.meta_cref().namespace_name();
            std::string texture_id = namespace_ + source_object.styling_ref().texture();

            if (!texture_id.empty()) {
                if (destination_shape.textures.empty()) {
                    if (known_textures_map.find(texture_id) != known_textures_map.end()) {
                        destination_shape.textures.emplace_back(1.0, known_textures_map[texture_id]);
                    }
                }
            }

            for (const auto& [opacity, texture_id] : source_object.styling_ref().get_textures_cref()) {
                destination_shape.textures.emplace_back(opacity, known_textures_map[texture_id]);
            }
        }
    }


}