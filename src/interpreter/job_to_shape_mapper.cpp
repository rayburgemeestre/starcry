/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "job_to_shape_mapper.h"
#include "abort_exception.hpp"
#include "generator.h"
#include "gradient_manager.h"

namespace interpreter {
job_to_shape_mapper::job_to_shape_mapper(generator& gen, gradient_manager& gm, texture_manager& tm)
    : gen_(gen), gradient_manager_(gm), texture_manager_(tm) {}

void job_to_shape_mapper::convert_objects_to_render_job(step_calculator& sc, v8::Local<v8::Object> video) {
  //  // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
  //  // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
  //  for (size_t index = 0; index < next_instances->Length(); index++) {
  //    auto instance = i.get_index(next_instances, index).As<v8::Object>();
  //    if (!instance->IsObject()) continue;
  //    convert_object_to_render_job(i, instance, index, sc, video);
  //  }
  for (auto& shape : gen_.scenes_.next_shapes_current_scene()) {
    bool skip = false;
    meta_callback(shape, [&]<typename T>(const T& shape) {
      if (shape.meta_cref().is_destroyed()) {
        skip = true;
      }
    });
    if (!skip) convert_object_to_render_job(shape, sc, video);
  }
}

void job_to_shape_mapper::convert_object_to_render_job(data_staging::shape_t& shape,
                                                       step_calculator& sc,
                                                       v8::Local<v8::Object> video) {
  data::shape new_shape;

  // @add_field@
  const auto initialize = [&]<typename T>(T& shape) {
    auto level = shape.meta_cref().level();
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
    auto recursive_scale = shape.generic_cref().recursive_scale();

    auto shape_opacity = shape.generic_cref().opacity();
    auto warp_width = shape.toroidal_ref().warp_width();
    auto warp_height = shape.toroidal_ref().warp_height();
    auto warp_x = shape.toroidal_ref().warp_x();
    auto warp_y = shape.toroidal_ref().warp_x();

    // auto text_font = i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "";

    // TODO: might not need this param after all
    // auto dist = i.double_number(instance, "__dist__");
    new_shape.random_hash = shape.meta_cref().random_hash();

    new_shape.unique_id = shape.meta_cref().unique_id();
    // temp
    new_shape.level = level;
    new_shape.time = shape.meta_cref().get_time();
    new_shape.dist = shape.meta_cref().distance();
    new_shape.steps = shape.meta_cref().steps();

    new_shape.gradients_.clear();
    new_shape.textures.clear();

    copy_gradient_from_object_to_shape(shape, new_shape, gradient_manager_.gradients_map());
    copy_texture_from_object_to_shape(shape, new_shape, texture_manager_.textures_map());

    new_shape.texture_3d_ = shape.styling_cref().texture_3d();
    new_shape.texture_offset_x = shape.styling_cref().texture_offset_x();
    new_shape.texture_offset_y = shape.styling_cref().texture_offset_y();

    new_shape.zernike_type_ = shape.styling_cref().zernike_type();
    new_shape.texture_effect_ = shape.styling_cref().texture_effect();

    // temp hack
    std::string namespace_;
    std::string gradient_id;
    std::string texture_id;
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      gradient_id = shape.styling_cref().gradient();
      texture_id = shape.styling_cref().texture();
      new_shape.hue = shape.styling_cref().hue();
    }

    new_shape.gradient_id_str = "";
    if (!gradient_id.empty()) {
      new_shape.gradient_id_str = gradient_id;
      if (new_shape.gradients_.empty()) {  // instead of if-statement we can also clear() the array
        const auto& known_gradients_map = gradient_manager_.gradients_map();
        if (known_gradients_map.contains(gradient_id)) {
          new_shape.gradients_.emplace_back(1.0, known_gradients_map.at(gradient_id));
        } else {
          new_shape.gradients_.emplace_back(1.0, data::gradient{});
          new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
          new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
          new_shape.gradients_[0].second.colors.emplace_back(1.0, data::color{0.0, 0, 0, 1});
          new_shape.gradient_id_str += "[white:2]";
        }
      }
    } else if (new_shape.gradients_.empty()) {
      new_shape.gradients_.emplace_back(1.0, data::gradient{});
      new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
      new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
      new_shape.gradients_[0].second.colors.emplace_back(1.0, data::color{0.0, 0, 0, 1});
      new_shape.gradient_id_str += "[white:1]";
    } else {
      new_shape.gradient_id_str += "[mixed]";
    }

    new_shape.texture_id_str = texture_id;

    new_shape.z = 0;
    // TODO: more types of shapes could use velocity properties
    if constexpr (std::is_same_v<T, data_staging::circle>) {
      auto vel = shape.movement_cref().velocity();
      new_shape.velocity = shape.movement_cref().velocity_speed();
      new_shape.vel_x = vel.x;
      new_shape.vel_y = vel.y;
    }
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      new_shape.blending_ = shape.styling_cref().blending_type();
    }
    new_shape.scale = scale;
    new_shape.recursive_scale = recursive_scale;
    new_shape.opacity = std::isnan(shape_opacity) ? 1.0 : shape_opacity;
    new_shape.seed = seed;
    new_shape.id = shape.meta_cref().id();
    // new_shape.label = label;
    // new_shape.motion_blur = motion_blur;
    new_shape.warp_width = warp_width;
    new_shape.warp_height = warp_height;
    new_shape.warp_x = warp_x;
    new_shape.warp_y = warp_y;

    // wrap this in a proper add method
    if (gen_.stepper.next_step != gen_.stepper.max_step) {
      gen_.indexes[shape.meta_cref().unique_id()][gen_.stepper.current_step] =
          gen_.job->shapes[gen_.stepper.current_step].size();
    } else {
      new_shape.indexes = gen_.indexes[shape.meta_cref().unique_id()];
    }
    if (!(gen_.stepper.current_step >= int(gen_.job->shapes.size()))) {
      gen_.job->shapes[gen_.stepper.current_step].emplace_back(std::move(new_shape));
    } else {
      throw abort_exception("current step exceeds shapes size");
    }
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
      [&](data_staging::ellipse& shape) {
        new_shape.type = data::shape_type::ellipse;
        new_shape.longest_diameter = shape.longest_diameter();
        new_shape.shortest_diameter = shape.shortest_diameter();
        new_shape.rotate = shape.generic_cref().rotate();
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
void job_to_shape_mapper::copy_gradient_from_object_to_shape(
    T& source_object,
    data::shape& destination_shape,
    const std::unordered_map<std::string, data::gradient>& known_gradients_map) {
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    std::string namespace_ = source_object.meta_cref().namespace_name();
    std::string gradient_id = namespace_ + source_object.styling_ref().gradient();

    if (!gradient_id.empty()) {
      if (destination_shape.gradients_.empty()) {
        if (known_gradients_map.find(gradient_id) != known_gradients_map.end()) {
          destination_shape.gradients_.emplace_back(1.0, known_gradients_map.at(gradient_id));
        }
      }
    }
    for (const auto& [opacity, gradient_id] : source_object.styling_ref().get_gradients_cref()) {
      destination_shape.gradients_.emplace_back(opacity, known_gradients_map.at(gradient_id));
    }
  }
}

template <typename T>
void job_to_shape_mapper::copy_texture_from_object_to_shape(
    T& source_object,
    data::shape& destination_shape,
    const std::unordered_map<std::string, data::texture>& known_textures_map) {
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    std::string namespace_ = source_object.meta_cref().namespace_name();
    std::string texture_id = namespace_ + source_object.styling_ref().texture();

    if (!texture_id.empty()) {
      if (destination_shape.textures.empty()) {
        if (known_textures_map.find(texture_id) != known_textures_map.end()) {
          destination_shape.textures.emplace_back(1.0, known_textures_map.at(texture_id));
        }
      }
    }

    for (const auto& [opacity, texture_id] : source_object.styling_ref().get_textures_cref()) {
      destination_shape.textures.emplace_back(opacity, known_textures_map.at(texture_id));
    }
  }
}

}  // namespace interpreter
