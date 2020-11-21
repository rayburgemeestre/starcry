/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator_v2.h"

#include <cmath>
#include <memory>
#include <mutex>

#include "primitives.h"
#include "primitives_v8.h"
#include "scripting_v2.h"
#include "util/generator.h"
#include "util/quadtree.h"
#include "util/step_calculator.hpp"
#include "util/vector_logic.hpp"

#include "shapes/circle_v2.h"
#include "shapes/position.h"
#include "shapes/rectangle_v2.h"

extern std::shared_ptr<v8_wrapper> context;

generator_v2::generator_v2() {
  static std::once_flag once;
  std::call_once(once, []() {
    context = nullptr;
  });
}

void generator_v2::init(const std::string& filename) {
  init_context(filename);
  init_user_script(filename);
  init_job();
  init_video_meta_info();
  init_gradients();
  init_object_instances();
}

void generator_v2::init_context(const std::string& filename) {
  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename);
  }
  context->reset();
  context->add_fun("output", &output_fun);
  context->add_fun("rand", &rand_fun_v2);
  context->add_include_fun();
}

void generator_v2::init_user_script(const std::string& filename) {
  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }
  std::istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
  const auto source = std::string("script = ") + std::string(begin, end);
  context->run(source);
}

void generator_v2::init_job() {
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
  job->num_chunks = 1;    // num_chunks;
  job->canvas_w = 1920;   // canvas_w;
  job->canvas_h = 1080;   // canvas_h;
  job->scale = 1.0;       // scale;
  job->compress = false;  // TODO: hardcoded
  job->save_image = false;
}

void generator_v2::init_video_meta_info() {
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto video = v8_index_object(context, val, "video").As<v8::Object>();
    auto duration = i.double_number(video, "duration");
    use_fps = i.double_number(video, "fps");
    canvas_w = i.double_number(video, "width");
    canvas_h = i.double_number(video, "height");
    auto seed = i.double_number(video, "rand_seed");
    tolerated_granularity = i.double_number(video, "granularity");
    set_rand_seed(seed);

    max_frames = duration * use_fps;
    job->width = canvas_w;
    job->height = canvas_h;
    job->canvas_w = canvas_w;
    job->canvas_h = canvas_h;
    job->scale = std::max(i.double_number(video, "scale"), static_cast<double>(1));
  });
}

void generator_v2::init_gradients() {
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

void generator_v2::init_object_instances() {
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

bool generator_v2::generate_frame() {
  job->shapes.clear();

  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
    auto video = i.v8_obj(obj, "video");
    for (size_t I = 0; I < scenes->Length(); I++) {
      auto current_scene = i.get_index(scenes, I);
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      auto instances = i.v8_array(sceneobj, "instances");
      auto intermediates = i.v8_array(sceneobj, "instances_intermediate");
      auto next_instances = i.v8_array(sceneobj, "instances_next");
      if (!next_instances->IsArray()) continue;

      parents.clear();
      max_step = 1;
      attempt = 0;
      current_step_max = std::numeric_limits<int>::max();

      while (current_step_max > tolerated_granularity) {
        if (++attempt != 1) {
          cleanup_previous_attempt(i, instances, next_instances, intermediates);
        }
        job->resize_for_num_steps(max_step);
        step_calculator sc(max_step);

        // For each steps (i.e., this frame could be divided into 10 steps)
        for (current_step = 0; current_step < max_step; current_step++) {
          quadtree qt(rectangle_v2(position(-width() / 2, -height() / 2), width(), height()), 32);

          update_object_positions(i, next_instances, max_step, qt);

          update_object_interactions(i, next_instances, intermediates, qt);

          add_objects_to_shapes(i, next_instances, sc, video);

          // Copy our "next" instances to intermediate instances, which will serve as a "previous" instance for the next
          // step.
          util::generator::copy_instances(i, intermediates, next_instances);

          if (job->shapes.size() != max_step) {
            break;
          }
        }
      }

      // At this point we are sure that we've handled the frame with sufficiently fine-grained granularity, and we can
      // commit to saving the "next" instances over the current instances.
      util::generator::copy_instances(i, instances, next_instances, true);
    }
  });
  job->job_number++;
  job->frame_number++;
  return job->frame_number != max_frames;
}

void generator_v2::cleanup_previous_attempt(v8_interact& i,
                                            v8::Local<v8::Array>& instances,
                                            v8::Local<v8::Array>& next_instances,
                                            v8::Local<v8::Array>& intermediates) {
  job->shapes.clear();
  indexes.clear();
  // reset next and intermediate instances
  util::generator::copy_instances(i, next_instances, instances);
  util::generator::copy_instances(i, intermediates, instances);
}

void generator_v2::update_object_positions(v8_interact& i,
                                           v8::Local<v8::Array>& next_instances,
                                           int max_step,
                                           quadtree& qt) {
  auto isolate = i.get_isolate();
  for (size_t j = 0; j < next_instances->Length(); j++) {
    auto instance = i.get_index(next_instances, j).As<v8::Object>();
    if (!instance->IsObject()) continue;

    update_time(i, instance);

    std::string type = i.str(instance, "type");
    auto x = i.double_number(instance, "x");
    auto y = i.double_number(instance, "y");
    auto vel_x = i.double_number(instance, "vel_x");
    auto vel_y = i.double_number(instance, "vel_y");
    if (!std::isnan(vel_x)) {
      x += (vel_x / static_cast<double>(max_step));
    }
    if (!std::isnan(vel_y)) {
      y += vel_y / static_cast<double>(max_step);
    }

    // For now we only care about circles
    if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
      qt.insert(point_type(position(x, y), j));
    }
    i.set_field(instance, "x", v8::Number::New(isolate, x));
    i.set_field(instance, "y", v8::Number::New(isolate, y));
    i.set_field(instance, "vel_x", v8::Number::New(isolate, vel_x));
    i.set_field(instance, "vel_y", v8::Number::New(isolate, vel_y));
  }
}

void generator_v2::update_object_interactions(v8_interact& i,
                                              v8::Local<v8::Array>& next_instances,
                                              v8::Local<v8::Array>& intermediates,
                                              quadtree& qt) {
  // reset current step max
  current_step_max = 1;

  const auto isolate = i.get_isolate();
  // For each instance determine how far the object has travelled and if it's within the allowed granularity,
  //  also do the collision detection efficiently using the quadtree we just created
  for (size_t index = 0; index < next_instances->Length(); index++) {
    auto instance = i.get_index(next_instances, index).As<v8::Object>();
    auto previous_instance = i.get_index(intermediates, index).As<v8::Object>();
    if (!instance->IsObject() || !previous_instance->IsObject()) continue;
    update_time(i, instance);
    auto dist = get_max_travel_of_object(i, previous_instance, instance);
    auto steps = update_steps(dist);
    handle_collisions(i, instance, index, next_instances, qt);
    if (attempt > 1) {
      steps = std::max(steps, (int)i.integer_number(instance, "steps"));
    }
    i.set_field(instance, "steps", v8::Number::New(isolate, steps));
  }
}

void generator_v2::handle_collisions(
    v8_interact& i, v8::Local<v8::Object> instance, size_t index, v8::Local<v8::Array> next_instances, quadtree& qt) {
  // Now do the collision detection part
  // NOTE: we multiple radius/size * 2.0 since we're not looking up a point, and querying the quadtree does
  // not check for overlap, but only whether the x,y is inside the specified range. If we don't want to miss
  // points on the edge of our circle, we need to widen the matching range.
  // TODO: for different sizes of circle collision detection, we need to somehow adjust the interface to this
  // lookup somehow.
  std::vector<point_type> found;
  auto type = i.str(instance, "type");
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  qt.query(index, circle_v2(position(x, y), radius * 2.0, radiussize * 2.0), found);
  if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
    for (const auto& collide : found) {
      const auto index2 = collide.userdata;
      // TODO: can we uncomment this one again, if not, why-not?
      //  if (index2 <= index) continue;
      auto instance2 = i.get_index(next_instances, index2).As<v8::Object>();
      handle_collision(i, index, index2, instance, instance2);
    }
  }
};

void generator_v2::handle_collision(
    v8_interact& i, size_t index, size_t index2, v8::Local<v8::Object> instance, v8::Local<v8::Object> instance2) {
  const auto isolate = i.get_isolate();
  auto last_collide = i.double_number(instance, "last_collide");

  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");

  auto x2 = i.double_number(instance2, "x");
  auto y2 = i.double_number(instance2, "y");
  if (!instance2->IsObject()) return;

  // they already collided, no need to let them collide again
  if (last_collide == index2) return;

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
  i.set_field(instance, "last_collide", v8::Number::New(isolate, index2));
  i.set_field(instance2, "last_collide", v8::Number::New(isolate, index));
}

void generator_v2::update_time(v8_interact& i, v8::Local<v8::Object>& instance) {
  // Update time here again
  const auto t = static_cast<double>(job->frame_number) / max_frames;
  const auto e = static_cast<double>(1.0) / static_cast<double>(use_fps) / static_cast<double>(max_step);
  i.call_fun(instance, "time", t, e);
}

int generator_v2::update_steps(double dist) {
  auto steps = (int)std::max(1.0, fabs(dist) / tolerated_granularity);
  max_step = std::max(max_step, steps);
  current_step_max = std::max(current_step_max, steps);
  return steps;
}

double generator_v2::get_max_travel_of_object(v8_interact& i,
                                              v8::Local<v8::Object>& previous_instance,
                                              v8::Local<v8::Object>& instance) {
  auto type = i.str(instance, "type");
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");

  // Calculate how many pixels are maximally covered by this instance, this is currently very simplified
  auto prev_x = i.double_number(previous_instance, "x");
  auto prev_y = i.double_number(previous_instance, "y");
  auto prev_radius = i.double_number(previous_instance, "radius");
  auto prev_radiussize = i.double_number(previous_instance, "radiussize");
  auto prev_rad = prev_radius + prev_radiussize;
  auto rad = radius + radiussize;
  auto dist = sqrt(pow(x - prev_x, 2) + pow(y - prev_y, 2));
  // TODO: stupid warp hack
  while (dist >= 1920 * 0.9) dist -= 1920;
  while (dist >= 1080 * 0.9) dist -= 1080;
  dist = std::max(dist, fabs(prev_rad - rad));
  return dist;
}

void generator_v2::add_objects_to_shapes(v8_interact& i,
                                         v8::Local<v8::Array> next_instances,
                                         step_calculator& sc,
                                         v8::Local<v8::Object> video) {
  // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
  // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
  for (size_t index = 0; index < next_instances->Length(); index++) {
    auto instance = i.get_index(next_instances, index).As<v8::Object>();
    if (!instance->IsObject()) continue;
    add_object_to_shapes(i, instance, index, sc, video);
  }
}

void generator_v2::add_object_to_shapes(
    v8_interact& i, v8::Local<v8::Object> instance, size_t index, step_calculator& sc, v8::Local<v8::Object> video) {
  // Update level for all objects
  auto level = i.double_number(instance, "level");
  parents[level] = instance;

  // See if we require this step for this object
  auto steps = i.integer_number(instance, "steps");
  if (!sc.do_step(steps, current_step + 1)) {
    return;
  }
  auto id = i.str(instance, "id");
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto type = i.str(instance, "type");
  auto scale = std::max(i.double_number(video, "scale"), static_cast<double>(1));

  data::shape new_shape;
  new_shape.x = x;
  new_shape.y = y;

  new_shape.gradients_.clear();
  util::generator::copy_gradient_from_object_to_shape(i, instance, new_shape, gradients);
  while (level > 0) {
    level--;
    new_shape.x += i.double_number(parents[level], "x");
    new_shape.y += i.double_number(parents[level], "y");
    util::generator::copy_gradient_from_object_to_shape(i, parents[level], new_shape, gradients);
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
  new_shape.blending_ = data::blending_type::add;

  if (type == "circle") {
    new_shape.type = data::shape_type::circle;
    // wrap this in a proper add method
    if (current_step + 1 != max_step) {
      indexes[index][current_step] = job->shapes[current_step].size();
    } else {
      new_shape.indexes = indexes[index];
    }
    job->shapes[current_step].push_back(new_shape);
  } else {
    new_shape.type = data::shape_type::none;
  }
  job->scale = scale;
};

std::shared_ptr<data::job> generator_v2::get_job() const {
  return job;
}
