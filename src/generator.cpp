/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator.h"

#include <fmt/core.h>
#include <cmath>
#include <memory>
#include <mutex>

#include "v8pp/module.hpp"

#include "scripting.h"
#include "util/generator.h"
#include "util/math.h"
#include "util/progress_visualizer.h"
#include "util/step_calculator.hpp"
#include "util/vector_logic.hpp"

#include "data/texture.hpp"
#include "shapes/circle.h"
#include "shapes/position.h"
#include "shapes/rectangle.h"

std::shared_ptr<v8_wrapper> context;

generator::generator() : visualizer(std::make_shared<progress_visualizer>("Job")) {
  static std::once_flag once;
  std::call_once(once, []() {
    context = nullptr;
  });
}

void generator::init(const std::string& filename, std::optional<double> rand_seed) {
  filename_ = filename;
  init_context();
  init_user_script();
  init_job();
  init_video_meta_info(rand_seed);
  init_gradients();
  init_textures();
  init_object_instances();
}

void generator::init_context() {
  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename());
  }
  context->reset();
  context->add_fun("output", &output_fun);
  context->add_fun("rand", &rand_fun_v2);
  context->add_fun("random_velocity", &random_velocity);
  context->add_fun("expf", &expf_fun);
  context->add_fun("logn", &logn_fun);
  context->add_include_fun();

  // TODO: wrap this also in context (wrapper)
  v8::HandleScope scope(context->context->isolate());
  v8pp::module consts(context->isolate);
  consts.set_const("normal", data::blending_type::normal)
      .set_const("lighten", data::blending_type::lighten)
      .set_const("darken", data::blending_type::darken)
      .set_const("multiply", data::blending_type::multiply)
      .set_const("average", data::blending_type::average)
      .set_const("add", data::blending_type::add)
      .set_const("subtract", data::blending_type::subtract)
      .set_const("difference", data::blending_type::difference)
      .set_const("negation", data::blending_type::negation_)
      .set_const("screen", data::blending_type::screen)
      .set_const("exclusion", data::blending_type::exclusion)
      .set_const("overlay", data::blending_type::overlay)
      .set_const("softlight", data::blending_type::softlight)
      .set_const("hardlight", data::blending_type::hardlight)
      .set_const("colordodge", data::blending_type::colordodge)
      .set_const("colorburn", data::blending_type::colorburn)
      .set_const("lineardodge", data::blending_type::lineardodge)
      .set_const("linearburn", data::blending_type::linearburn)
      .set_const("linearlight", data::blending_type::linearlight)
      .set_const("vividlight", data::blending_type::vividlight)
      .set_const("pinlight", data::blending_type::pinlight)
      .set_const("hardmix", data::blending_type::hardmix)
      .set_const("reflect", data::blending_type::reflect)
      .set_const("glow", data::blending_type::glow)
      .set_const("phoenix", data::blending_type::phoenix)
      .set_const("hue", data::blending_type::hue)
      .set_const("saturation", data::blending_type::saturation)
      .set_const("color", data::blending_type::color)
      .set_const("luminosity", data::blending_type::luminosity);
  context->context->set("blending_type", consts);
}

void generator::init_user_script() {
  std::ifstream stream(filename().c_str());
  if (!stream && filename() != "-") {
    throw std::runtime_error("could not locate file " + filename());
  }
  std::istreambuf_iterator<char> begin(filename() == "-" ? std::cin : stream), end;
  const auto source = std::string("script = ") + std::string(begin, end);
  context->run(source);
}

void generator::init_job() {
  job = std::make_shared<data::job>();
  job->background_color.r = 0;
  job->background_color.g = 0;
  job->background_color.b = 0;
  job->background_color.a = 0;
  job->width = 1920;   // canvas_w;
  job->height = 1080;  // canvas_h;
  job->job_number = 0;
  job->frame_number = frame_number;
  job->rendered = false;
  job->last_frame = false;
  job->num_chunks = 1;  // num_chunks;
  job->offset_x = 0;
  job->offset_y = 0;
  job->canvas_w = 1920;   // canvas_w;
  job->canvas_h = 1080;   // canvas_h;
  job->scale = 1.0;       // scale;
  job->compress = false;  // TODO: hardcoded
  job->save_image = false;
}

void generator::init_video_meta_info(std::optional<double> rand_seed) {
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto video = v8_index_object(context, val, "video").As<v8::Object>();
    auto duration = i.double_number(video, "duration");
    use_fps = i.double_number(video, "fps");
    canvas_w = i.double_number(video, "width");
    canvas_h = i.double_number(video, "height");
    seed = rand_seed ? *rand_seed : i.double_number(video, "rand_seed");
    tolerated_granularity = i.double_number(video, "granularity");
    if (i.has_field(video, "perlin_noise")) settings_.perlin_noise = i.boolean(video, "perlin_noise");
    if (i.has_field(video, "motion_blur")) settings_.motion_blur = i.boolean(video, "motion_blur");
    if (i.has_field(video, "grain_for_opacity")) settings_.grain_for_opacity = i.boolean(video, "grain_for_opacity");
    if (i.has_field(video, "extra_grain")) settings_.extra_grain = i.double_number(video, "extra_grain");
    if (i.has_field(video, "update_positions")) settings_.update_positions = i.boolean(video, "update_positions");
    if (i.has_field(video, "dithering")) settings_.dithering = i.boolean(video, "dithering");
    if (i.has_field(video, "max_intermediates")) max_intermediates = i.integer_number(video, "max_intermediates");
    if (i.has_field(video, "sample")) {
      auto sample = i.get(video, "sample").As<v8::Object>();
      sample_include = i.double_number(sample, "include");  // seconds
      sample_exclude = i.double_number(sample, "exclude");  // seconds
      sample_include_current = sample_include * use_fps;
      sample_exclude_current = sample_exclude * use_fps;
    }
    set_rand_seed(seed);

    max_frames = duration * use_fps;
    visualizer->set_max_frames(duration * use_fps);
    job->width = canvas_w;
    job->height = canvas_h;
    job->canvas_w = canvas_w;
    job->canvas_h = canvas_h;
    job->scale = i.double_number(video, "scale");

    video_scale = i.double_number(video, "scale");
    video_scale_next = i.double_number(video, "scale");
    video_scale_intermediate = i.double_number(video, "scale");
  });
}

void generator::init_gradients() {
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto gradient_objects = i.v8_obj(obj, "gradients");
    auto gradient_fields = gradient_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < gradient_fields->Length(); k++) {
      auto gradient_id = i.get_index(gradient_fields, k);
      auto positions = i.get(gradient_objects, gradient_id).As<v8::Array>();
      auto id = v8_str(isolate, gradient_id.As<v8::String>());
      for (size_t l = 0; l < positions->Length(); l++) {
        auto position = i.get_index(positions, l).As<v8::Object>();
        auto pos = i.double_number(position, "position");
        auto r = i.double_number(position, "r");
        auto g = i.double_number(position, "g");
        auto b = i.double_number(position, "b");
        auto a = i.double_number(position, "a");
        gradients[id].colors.emplace_back(std::make_tuple(pos, data::color{r, g, b, a}));
      }
    }
  });
}

void generator::init_textures() {
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    if (!i.has_field(obj, "textures")) return;
    auto texture_objects = i.v8_obj(obj, "textures");
    auto texture_fields = texture_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < texture_fields->Length(); k++) {
      auto texture_id = i.get_index(texture_fields, k);
      auto texture_settings = i.get(texture_objects, texture_id).As<v8::Object>();
      auto id = v8_str(isolate, texture_id.As<v8::String>());
      auto type = i.str(texture_settings, "type");
      textures[id].size = i.double_number(texture_settings, "size");
      textures[id].octaves = i.integer_number(texture_settings, "octaves");
      textures[id].persistence = i.double_number(texture_settings, "persistence");
      textures[id].percentage = i.double_number(texture_settings, "percentage");
      textures[id].scale = i.double_number(texture_settings, "scale");
      auto range = i.v8_array(texture_settings, "range");
      textures[id].strength = i.double_number(texture_settings, "strength");
      textures[id].speed = i.double_number(texture_settings, "speed");
      if (range->Length() == 4) {
        data::texture::noise_type use_type = data::texture::noise_type::perlin;
        if (type == "fractal") {
          use_type = data::texture::noise_type::fractal;
        } else if (type == "turbulence") {
          use_type = data::texture::noise_type::turbulence;
        }
        textures[id].type = use_type;
        textures[id].fromX = i.double_number(range, 0);
        textures[id].begin = i.double_number(range, 1);
        textures[id].end = i.double_number(range, 2);
        textures[id].toX = i.double_number(range, 3);
      }
    }
  });
}

void generator::init_object_instances() {
  context->run_array("script", [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
    auto objects = i.v8_array(obj, "objects");
    for (size_t I = 0; I < scenes->Length(); I++) {
      auto current_scene = i.get_index(scenes, I);
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      auto scene_objects = i.v8_array(sceneobj, "objects");
      auto instances = i.v8_array(sceneobj, "instances", v8::Array::New(isolate));
      auto instances_next = i.v8_array(sceneobj, "instances_next", v8::Array::New(isolate));
      auto instances_temp = i.v8_array(sceneobj, "instances_intermediate", v8::Array::New(isolate));
      size_t scene_instances_idx = instances->Length();
      for (size_t j = 0; j < scene_objects->Length(); j++) {
        auto scene_obj = i.get_index(scene_objects, j).As<v8::Object>();
        auto created_instance = util::generator::instantiate_objects(
            i, objects, instances, instances_next, instances_temp, scene_instances_idx, scene_obj);
        i.set_field(scene_objects, j, created_instance);
      }
    }
  });
}

bool generator::generate_frame() {
  // no sampling
  if (sample_include == 0 || sample_exclude == 0) {
    return _generate_frame();
  }
  while (true) {
    // sampling frames to include
    if (sample_include_current-- > 0) {
      return _generate_frame();
    }
    // frames to be skipped
    while (sample_exclude_current-- > 0) {
      total_skipped_frames++;
      bool ret = _generate_frame();
      job->job_number--;
      if (!ret) {
        // bail out if we find a last frame
        return false;
      }
    }
    // reset
    sample_include_current = sample_include * use_fps;
    sample_exclude_current = sample_exclude * use_fps;
  }
}

bool generator::_generate_frame() {
  try {
    job->shapes.clear();
    visualizer->set_start_timing();

    context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
      v8_interact i(isolate);
      auto obj = val.As<v8::Object>();
      auto scenes = i.v8_array(obj, "scenes");
      auto video = i.v8_obj(obj, "video");
      auto objects = i.v8_array(obj, "objects");
      for (size_t I = 0; I < scenes->Length(); I++) {
        auto current_scene = i.get_index(scenes, I);
        if (!current_scene->IsObject()) continue;
        auto sceneobj = current_scene.As<v8::Object>();
        auto instances = i.v8_array(sceneobj, "instances");
        auto intermediates = i.v8_array(sceneobj, "instances_intermediate");
        auto next_instances = i.v8_array(sceneobj, "instances_next");
        if (!next_instances->IsArray()) continue;

        parents.clear();
        prev_parents.clear();
        stepper.reset();
        indexes.clear();
        attempt = 0;
        max_dist_found = std::numeric_limits<double>::max();
        video_scales.clear();
        video_scales.push_back(video_scale);

        util::generator::garbage_collect_erased_objects(i, instances, next_instances, intermediates);

        while (max_dist_found > tolerated_granularity) {
          ++attempt;
          if (attempt >= 5) {
            throw std::runtime_error(fmt::format("Possible endless loop detected, max_step = {} (while {} > {})",
                                                 stepper.max_step,
                                                 max_dist_found,
                                                 tolerated_granularity));
            break;
          }
          max_dist_found = 0;
          if (attempt > 1) {
            if (!settings_.motion_blur) break;
            if (stepper.max_step < max_intermediates) {
              stepper.multiply(1.5);
              if (stepper.max_step > max_intermediates) stepper.max_step = max_intermediates;
            }
            revert_all_changes(i, instances, next_instances, intermediates);
          }
          step_calculator sc(stepper.max_step);
          job->resize_for_num_steps(stepper.max_step);

          stepper.rewind();
          while (stepper.has_next_step()) {
            qts.clear();
            create_next_instance_mapping(i, next_instances);
            update_object_positions(i, next_instances, stepper.max_step, video);
            update_object_interactions(i, next_instances, intermediates, video);
            util::generator::find_new_objects(i, objects, instances, next_instances, intermediates);
            convert_objects_to_render_job(i, next_instances, sc, video);
            util::generator::copy_instances(i, intermediates, next_instances);
            video_scale_intermediate = video_scale_next;
            video_scales.push_back(video_scale_intermediate);
            if (job->shapes.size() != stepper.max_step) {
              break;
            }
          }
          if (job->shapes.size() == stepper.max_step)   // didn't bail out with break above
            if (stepper.max_step == max_intermediates)  // config doesn't allow finer granularity any way, break.
              break;
        }

        if (!settings_.update_positions) {
          revert_position_updates(i, instances, next_instances, intermediates);
        }

        util::generator::copy_instances(i, instances, next_instances);
        video_scale = video_scale_next;
        video_scales.push_back(video_scale_next);
      }
    });
    job->job_number++;
    job->frame_number++;
    visualizer->display(job->job_number);
  } catch (std::exception& ex) {
    std::cout << ex.what() << std::endl;
  }
  job->last_frame = job->frame_number == max_frames;
  return job->frame_number != max_frames;
}

void generator::revert_all_changes(v8_interact& i,
                                   v8::Local<v8::Array>& instances,
                                   v8::Local<v8::Array>& next_instances,
                                   v8::Local<v8::Array>& intermediates) {
  job->shapes.clear();
  indexes.clear();

  // reset next and intermediate instances
  util::generator::copy_instances(i, next_instances, instances);
  util::generator::copy_instances(i, intermediates, instances);
  video_scale_next = video_scale;
  video_scale_intermediate = video_scale;
  video_scales.clear();
}

void generator::revert_position_updates(v8_interact& i,
                                        v8::Local<v8::Array>& instances,
                                        v8::Local<v8::Array>& next_instances,
                                        v8::Local<v8::Array>& intermediates) {
  for (size_t j = 0; j < next_instances->Length(); j++) {
    auto src = i.get_index(instances, j).As<v8::Object>();
    auto dst = i.get_index(next_instances, j).As<v8::Object>();
    auto dst2 = i.get_index(intermediates, j).As<v8::Object>();
    i.copy_field(dst, "x", src, "x");
    i.copy_field(dst, "y", src, "y");
    i.copy_field(dst, "x2", src, "x2");
    i.copy_field(dst, "y2", src, "y2");
    i.copy_field(dst2, "x", src, "x");
    i.copy_field(dst2, "y", src, "y");
    i.copy_field(dst2, "x2", src, "x2");
    i.copy_field(dst2, "y2", src, "y2");
  }
}

void generator::create_next_instance_mapping(v8_interact& i, v8::Local<v8::Array>& next_instances) {
  next_instance_mapping.clear();
  for (size_t j = 0; j < next_instances->Length(); j++) {
    auto next = i.get_index(next_instances, j).As<v8::Object>();
    auto unique_id = i.integer_number(next, "unique_id");
    next_instance_mapping[unique_id] = j;
  }
}

void generator::update_object_positions(v8_interact& i,
                                        v8::Local<v8::Array>& next_instances,
                                        int max_step,
                                        v8::Local<v8::Object>& video) {
  auto isolate = i.get_isolate();
  for (size_t j = 0; j < next_instances->Length(); j++) {
    auto instance = i.get_index(next_instances, j).As<v8::Object>();
    if (!instance->IsObject()) continue;

    update_time(i, instance);
    video_scale_next = i.double_number(video, "scale");

    std::string type = i.str(instance, "type");
    bool is_line = type == "line";
    std::string collision_group = i.str(instance, "collision_group");
    auto angle = i.double_number(instance, "angle");
    if (std::isnan(angle)) {
      angle = 0.;
    }
    auto x = i.double_number(instance, "x");
    auto y = i.double_number(instance, "y");
    auto x2 = i.double_number(instance, "x2");
    auto y2 = i.double_number(instance, "y2");
    auto velocity = i.double_number(instance, "velocity");
    auto vel_x = i.double_number(instance, "vel_x");
    auto vel_y = i.double_number(instance, "vel_y");
    auto vel_x2 = is_line ? i.double_number(instance, "vel_x2") : 0.0;
    auto vel_y2 = is_line ? i.double_number(instance, "vel_y2") : 0.0;

    if (!std::isnan(velocity)) {
      if (!std::isnan(vel_x)) {
        x += (vel_x * velocity) / static_cast<double>(max_step);
      }
      if (!std::isnan(vel_y)) {
        y += (vel_y * velocity) / static_cast<double>(max_step);
      }
      if (is_line) {
        if (!std::isnan(vel_x2)) {
          x2 += (vel_x2 * velocity) / static_cast<double>(max_step);
        }
        if (!std::isnan(vel_y2)) {
          y2 += (vel_y2 * velocity) / static_cast<double>(max_step);
        }
      }
    }

    // For now we only care about circles
    if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
      if (qts.find(collision_group) == qts.end()) {
        qts.insert(std::make_pair(collision_group,
                                  quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32)));
      }
      if (collision_group != "undefined") {
        auto unique_id = i.integer_number(instance, "unique_id");
        qts[collision_group].insert(point_type(position(x, y), unique_id));
      }
    }
    i.set_field(instance, "x", v8::Number::New(isolate, x));
    i.set_field(instance, "y", v8::Number::New(isolate, y));
    if (is_line) {
      i.set_field(instance, "x2", v8::Number::New(isolate, x2));
      i.set_field(instance, "y2", v8::Number::New(isolate, y2));
    }
    if (attempt == 1) {
      i.set_field(instance, "steps", v8::Number::New(isolate, 1));
    }
  }
}

void generator::update_object_interactions(v8_interact& i,
                                           v8::Local<v8::Array>& next_instances,
                                           v8::Local<v8::Array>& intermediates,
                                           v8::Local<v8::Object>& video) {
  stepper.reset_current();
  const auto isolate = i.get_isolate();
  for (size_t index = 0; index < next_instances->Length(); index++) {
    auto next_instance = i.get_index(next_instances, index).As<v8::Object>();
    auto previous_instance = i.get_index(intermediates, index).As<v8::Object>();
    if (!next_instance->IsObject() || !previous_instance->IsObject()) continue;
    auto motion_blur = !i.has_field(next_instance, "motion_blur") || i.boolean(next_instance, "motion_blur");
    if (!motion_blur) {
      i.set_field(next_instance, "steps", v8::Number::New(isolate, 1));
    } else {
      auto dist = get_max_travel_of_object(i, previous_instance, next_instance);
      auto steps = update_steps(dist);
      if (attempt > 1) {
        steps = std::max(steps, (int)i.integer_number(next_instance, "steps"));
      }
      i.set_field(next_instance, "steps", v8::Number::New(isolate, steps));
    }
    handle_collisions(i, next_instance, index, next_instances);
  }
}

void generator::handle_collisions(v8_interact& i,
                                  v8::Local<v8::Object> instance,
                                  size_t index,
                                  v8::Local<v8::Array> next_instances) {
  // Now do the collision detection part
  // NOTE: we multiple radius/size * 2.0 since we're not looking up a point, and querying the quadtree does
  // not check for overlap, but only whether the x,y is inside the specified range. If we don't want to miss
  // points on the edge of our circle, we need to widen the matching range.
  // TODO: for different sizes of circle collision detection, we need to somehow adjust the interface to this
  // lookup somehow.
  std::vector<point_type> found;
  auto type = i.str(instance, "type");
  auto collision_group = i.str(instance, "collision_group");
  if (collision_group == "undefined") {
    return;
  }
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto unique_id = i.integer_number(instance, "unique_id");
  qts[collision_group].query(unique_id, circle(position(x, y), radius * 2.0, radiussize * 2.0), found);
  if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
    for (const auto& collide : found) {
      const auto unique_id2 = collide.userdata;
      const auto index2 = next_instance_mapping.at(unique_id2);
      // TODO: can we uncomment this one again, if not, why-not?
      //  if (index2 <= index) continue;
      auto instance2 = i.get_index(next_instances, index2).As<v8::Object>();
      handle_collision(i, instance, instance2);
    }
  }
};

void generator::handle_collision(v8_interact& i, v8::Local<v8::Object> instance, v8::Local<v8::Object> instance2) {
  const auto isolate = i.get_isolate();
  auto unique_id = i.integer_number(instance, "unique_id");
  auto unique_id2 = i.integer_number(instance2, "unique_id");
  auto last_collide = i.double_number(instance, "last_collide");

  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  auto x2 = i.double_number(instance2, "x");
  auto y2 = i.double_number(instance2, "y");
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto radius2 = i.double_number(instance, "radius");
  auto radiussize2 = i.double_number(instance, "radiussize");

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius, radiussize);
  circle b(position(x2, y2), radius2, radiussize2);
  if (!a.overlaps(b)) return;

  if (!instance2->IsObject()) return;

  // they already collided, no need to let them collide again
  if (last_collide == unique_id2) return;

  // handle collision
  auto vel_x = i.double_number(instance, "vel_x");
  auto vel_y = i.double_number(instance, "vel_y");
  auto vel_x2 = i.double_number(instance2, "vel_x");
  auto vel_y2 = i.double_number(instance2, "vel_y");

  const auto normal = unit_vector(subtract_vector(vector2d(x, y), vector2d(x2, y2)));
  const auto ta = dot_product(vector2d(vel_x, vel_y), normal);
  const auto tb = dot_product(vector2d(vel_x2, vel_y2), normal);
  const auto optimized_p = (2.0 * (ta - tb)) / 2.0;

  // save velocities
  auto updated_vel1 = subtract_vector(vector2d(vel_x, vel_y), multiply_vector(normal, optimized_p));
  i.set_field(instance, "vel_x", v8::Number::New(isolate, updated_vel1.x));
  i.set_field(instance, "vel_y", v8::Number::New(isolate, updated_vel1.y));
  auto updated_vel2 = add_vector(vector2d(vel_x2, vel_y2), multiply_vector(normal, optimized_p));
  i.set_field(instance2, "vel_x", v8::Number::New(isolate, updated_vel2.x));
  i.set_field(instance2, "vel_y", v8::Number::New(isolate, updated_vel2.y));

  // save collision
  i.set_field(instance, "last_collide", v8::Number::New(isolate, unique_id2));
  i.set_field(instance2, "last_collide", v8::Number::New(isolate, unique_id));

  // call 'on collision' event
  auto on1 = i.get(instance, "on").As<v8::Object>();
  auto on2 = i.get(instance2, "on").As<v8::Object>();

  // TODO: proof-of-concept
  auto emit_event = [&](v8::Local<v8::Object> on1, v8::Local<v8::Object> instance, v8::Local<v8::Object> instance2) {
    size_t subobj_len_before = 0;
    size_t subobj_len_after = 0;
    if (i.has_field(instance, "subobj")) {
      auto subobj = i.get(instance, "subobj").As<v8::Array>();
      subobj_len_before = subobj->Length();
    }
    i.call_fun(on1, instance, "collide", instance2);
    if (i.has_field(instance, "subobj")) {
      auto subobj = i.get(instance, "subobj").As<v8::Array>();
      subobj_len_after = subobj->Length();
    }
    i.set_field(instance, "new_objects", v8::Boolean::New(i.get_isolate(), subobj_len_after > subobj_len_before));
  };
  emit_event(on1, instance, instance2);
  emit_event(on2, instance2, instance);
}

void generator::update_time(v8_interact& i, v8::Local<v8::Object>& instance) {
  auto extra = (static_cast<double>(stepper.next_step) / static_cast<double>(stepper.max_step));
  auto fn = static_cast<double>(job->frame_number - (1.0 - extra));
  const auto t = fn / max_frames;
  const auto e = static_cast<double>(1.0) / static_cast<double>(use_fps) / static_cast<double>(stepper.max_step);

  size_t subobj_len_before = 0;
  size_t subobj_len_after = 0;
  if (i.has_field(instance, "subobj")) {
    auto subobj = i.get(instance, "subobj").As<v8::Array>();
    subobj_len_before = subobj->Length();
  }
  i.call_fun(instance, "time", t, e);
  if (i.has_field(instance, "subobj")) {
    auto subobj = i.get(instance, "subobj").As<v8::Array>();
    subobj_len_after = subobj->Length();
  }
  i.set_field(instance, "new_objects", v8::Boolean::New(i.get_isolate(), subobj_len_after > subobj_len_before));

  i.set_field(instance, "__time__", v8::Number::New(i.get_isolate(), t));
  i.set_field(instance, "__elapsed__", v8::Number::New(i.get_isolate(), e));
}

int generator::update_steps(double dist) {
  max_dist_found = std::max(max_dist_found, fabs(dist));
  auto steps = (int)std::max(1.0, fabs(dist) / tolerated_granularity);
  if (steps > max_intermediates) {
    steps = max_intermediates;
  }
  stepper.update(steps);
  return steps;
}

double generator::get_max_travel_of_object(v8_interact& i,
                                           v8::Local<v8::Object>& previous_instance,
                                           v8::Local<v8::Object>& instance) {
  // Update level for all objects
  // TODO: move to better place
  auto level = i.integer_number(instance, "level");
  auto type = i.str(instance, "type");
  auto is_line = type == "line";
  auto label = i.str(instance, "label");
  auto shape_scale = i.has_field(instance, "scale") ? i.double_number(instance, "scale") : 1.0;
  auto rotate = [&](double src_x, double src_y, double* x, double* y, double angle) {
    if (angle == 0.) {
      return;
    }
    auto angle1 = angle + get_angle(src_x, src_y, *x, *y);
    while (angle1 > 360.) angle1 -= 360.;
    auto rads = angle1 * M_PI / 180.0;
    auto ratio = 1.0;
    auto dist = get_distance(src_x, src_y, *x, *y);
    auto move = dist * ratio * -1;
    *x = (cos(rads) * move);
    *y = (sin(rads) * move);
  };

  parents[level] = instance;
  prev_parents[level] = previous_instance;

  auto offset_x = static_cast<double>(0);
  auto offset_y = static_cast<double>(0);
  auto offset_x2 = static_cast<double>(0);
  auto offset_y2 = static_cast<double>(0);
  double angle = i.double_number(previous_instance, "angle");
  double pivot_x = i.double_number(instance, "x");
  double pivot_y = i.double_number(instance, "y");
  while (level > 0) {
    level--;
    offset_x += i.double_number(parents[level], "x");
    offset_y += i.double_number(parents[level], "y");
    if (level == 1) {
      pivot_x = i.double_number(parents[level], "x");
      pivot_y = i.double_number(parents[level], "y");
    }
    if (is_line) {
      // note, do not assume all parents are lines, use x & y relative
      offset_x2 += i.double_number(parents[level], "x");
      offset_y2 += i.double_number(parents[level], "y");
    }
    auto a = i.double_number(parents[level], "angle");
    if (!std::isnan(a)) angle += a;
  }

  level = i.double_number(previous_instance, "level");
  if (std::isnan(angle)) angle = 0.;
  auto prev_offset_x = static_cast<double>(0);
  auto prev_offset_y = static_cast<double>(0);
  auto prev_offset_x2 = static_cast<double>(0);
  auto prev_offset_y2 = static_cast<double>(0);
  while (level > 0) {
    level--;
    prev_offset_x += i.double_number(prev_parents[level], "x");
    prev_offset_y += i.double_number(prev_parents[level], "y");
    if (is_line) {
      prev_offset_x2 += i.double_number(prev_parents[level], "x2");
      prev_offset_y2 += i.double_number(prev_parents[level], "y2");
    }
    auto a = i.double_number(parents[level], "angle");
    if (!std::isnan(a)) angle += a;
  }

  auto x = offset_x + i.double_number(instance, "x");
  auto y = offset_y + i.double_number(instance, "y");
  auto x2 = is_line ? offset_x2 + i.double_number(instance, "x2") : 0.0;
  auto y2 = is_line ? offset_y2 + i.double_number(instance, "y2") : 0.0;

  // auto level_ = i.double_number(previous_instance, "level");
  if (angle != 0.) {
    auto angle1 = angle + get_angle(pivot_x, pivot_y, x, y);
    while (angle1 > 360.) angle1 -= 360.;
    auto rads = angle1 * M_PI / 180.0;
    auto ratio = 1.0;
    auto dist = get_distance(pivot_x, pivot_y, x, y);
    auto move = dist * ratio * -1;
    x = pivot_x + (cos(rads) * move);
    y = pivot_y + (sin(rads) * move);

    /*
    auto angle2 = angle + get_angle(0, 0, x2, y2);
    while (angle2 > 360.) angle2 -= 360.;
    rads = angle2 * M_PI / 180.0;
    dist = get_distance(offset_x2, offset_y2, x2, y2);
    move = dist * ratio * -1;
    x2 = offset_x + (cos(rads) * move);
    y2 = offset_y + (sin(rads) * move);
    */
  }

  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");

  i.set_field(instance, "transitive_x", v8::Number::New(i.get_isolate(), x));
  i.set_field(instance, "transitive_y", v8::Number::New(i.get_isolate(), y));
  if (is_line) {
    i.set_field(instance, "transitive_x2", v8::Number::New(i.get_isolate(), x2));
    i.set_field(instance, "transitive_y2", v8::Number::New(i.get_isolate(), y2));
  }

  // Calculate how many pixels are maximally covered by this instance, this is currently very simplified
  auto prev_x = prev_offset_x + i.double_number(previous_instance, "x");
  auto prev_y = prev_offset_y + i.double_number(previous_instance, "y");
  auto prev_x2 = is_line ? prev_offset_x2 + i.double_number(previous_instance, "x2") : 0.0;
  auto prev_y2 = is_line ? prev_offset_y2 + i.double_number(previous_instance, "y2") : 0.0;
  auto prev_radius = i.double_number(previous_instance, "radius");
  auto prev_radiussize = i.double_number(previous_instance, "radiussize");
  auto prev_rad = prev_radius + prev_radiussize;
  auto rad = radius + radiussize;
  auto prev_shape_scale = i.has_field(previous_instance, "scale") ? i.double_number(previous_instance, "scale") : 1.0;

  x *= video_scale_next * shape_scale;
  prev_x *= video_scale_intermediate * prev_shape_scale;
  y *= video_scale_next * shape_scale;
  prev_y *= video_scale_intermediate * prev_shape_scale;

  //  // TODO: make smarter for circles and lines, this is just temporary code
  //  rectangle canvas(position(-canvas_w / 2., -canvas_h/2.), position(canvas_w / 2., canvas_h/2.));
  //  if (is_line) {
  //    rectangle shape_rect(position(x, y), position(x2, y2));
  //    rectangle prev_shape_rect(position(prev_x - rad, prev_y - rad), position(prev_x + rad, prev_y + rad));
  //    if (!canvas.overlaps(shape_rect) && !canvas.overlaps(prev_shape_rect)) {
  //      return 0.;
  //    }
  //  } else {
  //    rectangle shape_rect(position(x - rad, y - rad), position(x + rad, y + rad));
  //    rectangle prev_shape_rect(position(prev_x - rad, prev_y - rad), position(prev_x + rad, prev_y + rad));
  //    if (!canvas.overlaps(shape_rect) && !canvas.overlaps(prev_shape_rect)) {
  //      return 0.;
  //    }
  //  }

  auto dist = sqrt(pow(x - prev_x, 2) + pow(y - prev_y, 2));
  // x2, y2
  if (is_line) {
    x2 *= video_scale_next * shape_scale;
    prev_x2 *= video_scale_intermediate * prev_shape_scale;
    y2 *= video_scale_next * shape_scale;
    prev_y2 *= video_scale_intermediate * prev_shape_scale;
    dist = std::max(dist, sqrt(pow(x2 - prev_x2, 2) + pow(y2 - prev_y2, 2)));
  }
  // TODO: stupid warp hack
  while (dist >= 1920 * 0.9) dist -= 1920;
  while (dist >= 1080 * 0.9) dist -= 1080;
  // radius
  rad *= video_scale_next * shape_scale;
  prev_rad *= video_scale_intermediate * prev_shape_scale;
  job->scale = video_scale_next;
  auto new_dist = fabs(prev_rad - rad);
  dist = std::max(dist, new_dist);
  return dist;
}

void generator::convert_objects_to_render_job(v8_interact& i,
                                              v8::Local<v8::Array> next_instances,
                                              step_calculator& sc,
                                              v8::Local<v8::Object> video) {
  // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
  // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
  for (size_t index = 0; index < next_instances->Length(); index++) {
    auto instance = i.get_index(next_instances, index).As<v8::Object>();
    if (!instance->IsObject()) continue;
    convert_object_to_render_job(i, instance, index, sc, video);
  }
}

void generator::convert_object_to_render_job(
    v8_interact& i, v8::Local<v8::Object> instance, size_t index, step_calculator& sc, v8::Local<v8::Object> video) {
  // Update level for all objects
  auto level = i.integer_number(instance, "level");
  auto exists = !i.has_field(instance, "exists") || i.boolean(instance, "exists");
  auto type = !exists ? "" : i.str(instance, "type");
  auto is_line = type == "line";
  parents[level] = instance;

  // See if we require this step for this object
  auto steps = i.integer_number(instance, "steps");
  if (!sc.do_step(steps, stepper.next_step)) {
    return;
  }
  auto id = i.str(instance, "id");
  auto label = i.str(instance, "label");
  auto time = i.double_number(instance, "__time__");
  auto transitive_x = i.double_number(instance, "transitive_x");
  auto transitive_y = i.double_number(instance, "transitive_y");
  auto transitive_x2 = is_line ? i.double_number(instance, "transitive_x2") : 0.0;
  auto transitive_y2 = is_line ? i.double_number(instance, "transitive_y2") : 0.0;
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto seed = i.double_number(instance, "seed");
  auto blending_type =
      i.has_field(instance, "blending_type") ? i.integer_number(instance, "blending_type") : data::blending_type::add;
  auto scale = i.has_field(instance, "scale") ? i.double_number(instance, "scale") : 1.0;
  auto unique_id = i.integer_number(instance, "unique_id");
  auto video_scale = i.double_number(video, "scale");
  auto shape_opacity = i.double_number(instance, "opacity");
  auto motion_blur = i.boolean(instance, "motion_blur");

  data::shape new_shape;
  new_shape.time = time;
  new_shape.x = transitive_x;
  new_shape.y = transitive_y;

  new_shape.gradients_.clear();
  new_shape.textures.clear();
  util::generator::copy_gradient_from_object_to_shape(i, instance, new_shape, gradients);
  util::generator::copy_texture_from_object_to_shape(i, instance, new_shape, textures);
  while (level > 0) {
    level--;
    util::generator::copy_gradient_from_object_to_shape(i, parents[level], new_shape, gradients);
    util::generator::copy_texture_from_object_to_shape(i, parents[level], new_shape, textures);
  }
  if (new_shape.gradients_.empty()) {
    new_shape.gradients_.emplace_back(1.0, data::gradient{});
    new_shape.gradients_[0].second.colors.emplace_back(std::make_tuple(0.0, data::color{1.0, 1, 1, 1}));
    new_shape.gradients_[0].second.colors.emplace_back(std::make_tuple(0.0, data::color{1.0, 1, 1, 1}));
    new_shape.gradients_[0].second.colors.emplace_back(std::make_tuple(1.0, data::color{0.0, 0, 0, 1}));
  }
  new_shape.z = 0;
  new_shape.radius = radius;
  new_shape.radius_size = radiussize;
  new_shape.blending_ = blending_type;
  new_shape.scale = scale;
  new_shape.opacity = std::isnan(shape_opacity) ? 1.0 : shape_opacity;
  new_shape.unique_id = unique_id;
  new_shape.seed = seed;
  new_shape.id = id;
  new_shape.label = label;
  new_shape.level = level;
  new_shape.motion_blur = motion_blur;

  if (type == "circle") {
    new_shape.type = data::shape_type::circle;
  } else if (type == "line") {
    new_shape.type = data::shape_type::line;
    new_shape.x2 = transitive_x2;
    new_shape.y2 = transitive_y2;
  } else {
    new_shape.type = data::shape_type::none;
  }
  // wrap this in a proper add method
  if (stepper.next_step != stepper.max_step) {
    indexes[unique_id][stepper.current_step] = job->shapes[stepper.current_step].size();
  } else {
    new_shape.indexes = indexes[unique_id];
  }
  job->shapes[stepper.current_step].push_back(new_shape);
  job->scale = video_scale;
  job->scales = video_scales;
};

std::shared_ptr<data::job> generator::get_job() const {
  return job;
}
