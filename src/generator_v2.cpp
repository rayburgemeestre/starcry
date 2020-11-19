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
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"
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
  context->run("var video = script.video;");
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
    granularity = i.double_number(video, "granularity");
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
      auto scene_instances = i.v8_array(sceneobj, "instances");
      auto scene_instances_next = i.v8_array(sceneobj, "instances_next");
      auto scene_instances_intermediate = i.v8_array(sceneobj, "instances_intermediate");
      if (!scene_instances->IsArray()) {
        i.set_field(sceneobj, "instances", v8::Array::New(isolate));
        i.set_field(sceneobj, "instances_next", v8::Array::New(isolate));
        i.set_field(sceneobj, "instances_intermediate", v8::Array::New(isolate));
      }
      scene_instances = i.v8_array(sceneobj, "instances");
      scene_instances_next = i.v8_array(sceneobj, "instances_next");
      scene_instances_intermediate = i.v8_array(sceneobj, "instances_intermediate");
      size_t scene_instances_idx = scene_instances->Length();
      for (size_t j = 0; j < scene_objects->Length(); j++) {
        auto scene_obj = i.get_index(scene_objects, j).As<v8::Object>();
        auto created_instance = util::generator::instantiate_objects(i,
                                                                     objects,
                                                                     scene_instances,
                                                                     scene_instances_next,
                                                                     scene_instances_intermediate,
                                                                     scene_instances_idx,
                                                                     scene_obj);
        i.set_field(scene_objects, j, created_instance);
      }
    }
  });
}

bool generator_v2::generate_frame() {
  previous_job = job;
  job->shapes.clear();

  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
    auto video = i.v8_obj(obj, "video");
    for (size_t I = 0; I < scenes->Length(); I++) {
      auto current_scene = scenes->Get(isolate->GetCurrentContext(), I).ToLocalChecked();
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      auto intermediates = i.v8_array(sceneobj, "instances_intermediate");
      auto next_instances = i.v8_array(sceneobj, "instances_next");
      if (!next_instances->IsArray()) continue;
      std::unordered_map<int64_t, v8::Local<v8::Object>> parents;

      int max_step = 1;
      int current_step_max = 1;
      int attempt = 0;

      // TODO: consider also unordered map for the steps?
      std::unordered_map<size_t, std::map<int, size_t>> indexes;

      // Endless loop: Continue as long as we discover we need more granularity to render motion blur properly
      do {
        attempt++;

        // First iteration failed, step size was larger than specified granularity allowed, reset state first to start
        // over with a clean slate
        if (attempt > 1) {
          job->shapes.clear();
          indexes.clear();
          // Copy instance values to instance_next values (resetting all instance next values)
          {
            auto instances = i.v8_array(sceneobj, "instances");
            for (size_t j = 0; j < next_instances->Length(); j++) {
              auto instance = instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
              auto next = next_instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();

              handle_error(
                  next->Set(isolate->GetCurrentContext(),
                            v8_str(context, "x"),
                            instance->Get(isolate->GetCurrentContext(), v8_str(context, "x")).ToLocalChecked()));
              handle_error(
                  next->Set(isolate->GetCurrentContext(),
                            v8_str(context, "y"),
                            instance->Get(isolate->GetCurrentContext(), v8_str(context, "y")).ToLocalChecked()));
              // undo collisions
              handle_error(
                  next->Set(isolate->GetCurrentContext(),
                            v8_str(context, "vel_x"),
                            instance->Get(isolate->GetCurrentContext(), v8_str(context, "vel_x")).ToLocalChecked()));
              handle_error(
                  next->Set(isolate->GetCurrentContext(),
                            v8_str(context, "props"),
                            instance->Get(isolate->GetCurrentContext(), v8_str(context, "props")).ToLocalChecked()));

              handle_error(
                  next->Set(isolate->GetCurrentContext(),
                            v8_str(context, "vel_y"),
                            instance->Get(isolate->GetCurrentContext(), v8_str(context, "vel_y")).ToLocalChecked()));
              handle_error(next->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "last_collide"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "last_collide")).ToLocalChecked()));
              handle_error(
                  next->Set(isolate->GetCurrentContext(),
                            v8_str(context, "radius"),
                            instance->Get(isolate->GetCurrentContext(), v8_str(context, "radius")).ToLocalChecked()));
              handle_error(next->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "radiussize"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "radiussize")).ToLocalChecked()));
            }
          }
          // Copy instance values to instance_intermediate (resetting all intermediate values)
          {
            auto instances = i.v8_array(sceneobj, "instances");
            auto intermediateinstances = i.v8_array(sceneobj, "instances_intermediate");
            for (size_t j = 0; j < next_instances->Length(); j++) {
              auto instance = instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
              auto intermediate =
                  intermediateinstances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "x"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "x")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "y"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "y")).ToLocalChecked()));
              // undo collisions
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "vel_x"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "vel_x")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "vel_y"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "vel_y")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "last_collide"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "last_collide")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "radius"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "radius")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "radiussize"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "radiussize")).ToLocalChecked()));
            }
          }
        }

        // For the current granularity update the Javascript objects (at attempt zero, this is 1 step)
        for (int current_step = 0; current_step < max_step; current_step++) {
          // make sure we have shapes vectors for each step we plan to create
          job->shapes.emplace_back();
        }
        // pre-calculate step map for fast determination of whether an object should be part of the step
        // TODO: extract into helper object
        std::unordered_map<int, std::unordered_map<int, bool>> step_map;
        for (int i = 1; i <= max_step; i++) {
          int diff = max_step / i;
          int frame = max_step;
          step_map[i][frame] = true;
          for (int j = 1; j < i; j++) {
            frame = max_step - (diff * j);
            step_map[i][frame] = true;
          }
        }
        // helper function
        auto do_step = [&](int step_value, int current_step) {
          const auto& step_map_c = step_map;
          if (step_map_c.find(step_value) != step_map_c.end()) {
            const auto val = step_map_c.at(step_value).find(current_step);
            if (val != step_map_c.at(step_value).end()) {
              return val->second;
            }
          }
          return false;
        };

        // For each steps (i.e., this frame could be divided into 10 steps)
        for (int current_step = 0; current_step < max_step; current_step++) {
          // Initialize a quadtree
          quadtree qt(rectangle_v2(position(-width() / 2, -height() / 2), width(), height()), 32);

          // For each scene instance, let time progress and update it's x, y and velocity and also add to quadtree
          // We manipulate only the "next" instance, not the current instance, as we might choose to revert these
          // changes if the granularity is not sufficient.
          for (size_t j = 0; j < next_instances->Length(); j++) {
            auto instance = next_instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
            if (!instance->IsObject()) {
              continue;
            }
            v8::Local<v8::Function> fun2 = instance.As<v8::Object>()
                                               ->Get(isolate->GetCurrentContext(), v8_str(context, "time"))
                                               .ToLocalChecked()
                                               .As<v8::Function>();
            v8::Handle<v8::Value> args[2];
            args[0] = v8pp::to_v8(isolate, static_cast<double>(job->frame_number) / max_frames);
            args[1] = v8pp::to_v8(
                isolate, static_cast<double>(1.0) / static_cast<double>(use_fps) / static_cast<double>(max_step));
            fun2->Call(isolate->GetCurrentContext(), instance, 2, args).ToLocalChecked();
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
            if (type == "circle" &&
                i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
              qt.insert(point_type(position(x, y), j));
            }
            handle_error(
                instance->Set(isolate->GetCurrentContext(), v8_str(context, "x"), v8::Number::New(isolate, x)));
            handle_error(
                instance->Set(isolate->GetCurrentContext(), v8_str(context, "y"), v8::Number::New(isolate, y)));
            handle_error(
                instance->Set(isolate->GetCurrentContext(), v8_str(context, "vel_x"), v8::Number::New(isolate, vel_x)));
            handle_error(
                instance->Set(isolate->GetCurrentContext(), v8_str(context, "vel_y"), v8::Number::New(isolate, vel_y)));
          }

          // Now that we're going over all the instances a second time, at smaller steps potentially, we'll still keep
          // track of the required steps we find this time. It could be objects were speeding up exponentially and the
          // previously chosen stepsize we is still not sufficient to handle with this kind of movement. If it's still
          // too high, we can discard everything again and try once more with a bigger step size.
          current_step_max = 1;

          // For each instance determine how far the object has travelled and if it's within the allowed granularity,
          //  also do the collision detection efficiently using the quadtree we just created
          for (size_t j = 0; j < next_instances->Length(); j++) {
            auto instance = next_instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
            if (!instance->IsObject()) {
              continue;
            }
            auto previous_instance =
                intermediates->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
            if (!previous_instance->IsObject()) {
              continue;
            }

            // Update time here again
            v8::Local<v8::Function> fun2 = instance.As<v8::Object>()
                                               ->Get(isolate->GetCurrentContext(), v8_str(context, "time"))
                                               .ToLocalChecked()
                                               .As<v8::Function>();
            v8::Handle<v8::Value> args[2];
            args[0] = v8pp::to_v8(isolate, static_cast<double>(job->frame_number) / max_frames);
            args[1] = v8pp::to_v8(
                isolate, static_cast<double>(1.0) / static_cast<double>(use_fps) / static_cast<double>(max_step));
            fun2->Call(isolate->GetCurrentContext(), instance, 2, args).ToLocalChecked();

            auto type = i.str(instance, "type");
            auto x = i.double_number(instance, "x");
            auto y = i.double_number(instance, "y");
            auto radius = i.double_number(instance, "radius");
            auto radiussize = i.double_number(instance, "radiussize");
            auto last_collide = i.double_number(instance, "last_collide");

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
            auto steps = (int)std::max(1.0, fabs(dist) / granularity);
            max_step = std::max(max_step, steps);
            current_step_max = std::max(current_step_max, steps);

            // Now do the collision detection part
            std::vector<point_type> found;
            auto index = j;
            // NOTE: we multiple radius/size * 2.0 since we're not looking up a point, and querying the quadtree does
            // not check for overlap, but only whether the x,y is inside the specified range. If we don't want to miss
            // points on the edge of our circle, we need to widen the matching range.
            // TODO: for different sizes of circle collision detection, we need to somehow adjust the interface to this
            // lookup somehow.
            qt.query(index, circle_v2(position(x, y), radius * 2.0, radiussize * 2.0), found);
            if (type == "circle" &&
                i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
              for (const auto& collide : found) {
                const auto index2 = collide.userdata;
                if (index2 <= index) {
                  // TODO: can we uncomment this one again, if not, why-not?
                  // continue;
                }
                auto instance2 =
                    next_instances->Get(isolate->GetCurrentContext(), index2).ToLocalChecked().As<v8::Object>();
                auto x2 = i.double_number(instance2, "x");
                auto y2 = i.double_number(instance2, "y");
                if (!instance2->IsObject()) {
                  continue;
                }

                // they already collided, no need to let them collide again
                if (last_collide == index2) {
                  continue;
                }

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
                handle_error(instance->Set(
                    isolate->GetCurrentContext(), v8_str(context, "vel_x"), v8::Number::New(isolate, updated_vel1.x)));
                handle_error(instance->Set(
                    isolate->GetCurrentContext(), v8_str(context, "vel_y"), v8::Number::New(isolate, updated_vel1.y)));
                auto updated_vel2 = add_vector(vector2d(vel_x2, vel_y2), multiply_vector(normal, optimized_p));
                handle_error(instance2->Set(
                    isolate->GetCurrentContext(), v8_str(context, "vel_x"), v8::Number::New(isolate, updated_vel2.x)));
                handle_error(instance2->Set(
                    isolate->GetCurrentContext(), v8_str(context, "vel_y"), v8::Number::New(isolate, updated_vel2.y)));

                // save collision
                handle_error(instance->Set(
                    isolate->GetCurrentContext(), v8_str(context, "last_collide"), v8::Number::New(isolate, index2)));
                handle_error(instance2->Set(
                    isolate->GetCurrentContext(), v8_str(context, "last_collide"), v8::Number::New(isolate, index)));
              }
            }

            if (attempt > 1) {
              steps = std::max(steps, (int)i.integer_number(instance, "steps"));
            }
            handle_error(
                instance->Set(isolate->GetCurrentContext(), v8_str(context, "steps"), v8::Number::New(isolate, steps)));
          }

          // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
          // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
          for (size_t j = 0; j < next_instances->Length(); j++) {
            auto instance = next_instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
            if (!instance->IsObject()) {
              continue;
            }
            // Update level for all objects
            auto level = i.double_number(instance, "level");
            parents[level] = instance;

            // See if we require this step for this object
            auto steps = i.integer_number(instance, "steps");
            if (!do_step(steps, current_step + 1)) {
              continue;
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
                indexes[j][current_step] = job->shapes[current_step].size();
              } else {
                new_shape.indexes = indexes[j];
              }
              job->shapes[current_step].push_back(new_shape);
            } else {
              new_shape.type = data::shape_type::none;
            }
            job->scale = scale;
          }

          // Copy our "next" instances to intermediate instances, which will serve as a "previous" instance for the next
          // step.
          {
            auto instances = i.v8_array(sceneobj, "instances_next");
            auto intermediateinstances = i.v8_array(sceneobj, "instances_intermediate");
            for (size_t j = 0; j < next_instances->Length(); j++) {
              auto instance = instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
              auto intermediate =
                  intermediateinstances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "x"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "x")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "y"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "y")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "vel_x"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "vel_x")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "vel_y"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "vel_y")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "last_collide"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "last_collide")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "radius"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "radius")).ToLocalChecked()));
              handle_error(intermediate->Set(
                  isolate->GetCurrentContext(),
                  v8_str(context, "radiussize"),
                  instance->Get(isolate->GetCurrentContext(), v8_str(context, "radiussize")).ToLocalChecked()));
            }
          }
          if (job->shapes.size() != max_step) {
            break;
          }
        }
      } while (current_step_max > 2);

      // At this point we are sure that we've handled the frame with sufficiently fine-grained granularity, and we can
      // commit to saving the "next" instances over the current instances.
      {
        auto instances = i.v8_array(sceneobj, "instances");
        auto nextinstances = i.v8_array(sceneobj, "instances_next");
        for (size_t j = 0; j < next_instances->Length(); j++) {
          auto instance = instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
          auto next = nextinstances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
          handle_error(instance->Set(isolate->GetCurrentContext(),
                                     v8_str(context, "x"),
                                     next->Get(isolate->GetCurrentContext(), v8_str(context, "x")).ToLocalChecked()));
          handle_error(instance->Set(isolate->GetCurrentContext(),
                                     v8_str(context, "y"),
                                     next->Get(isolate->GetCurrentContext(), v8_str(context, "y")).ToLocalChecked()));
          handle_error(
              instance->Set(isolate->GetCurrentContext(),
                            v8_str(context, "vel_x"),
                            next->Get(isolate->GetCurrentContext(), v8_str(context, "vel_x")).ToLocalChecked()));
          handle_error(
              instance->Set(isolate->GetCurrentContext(),
                            v8_str(context, "vel_y"),
                            next->Get(isolate->GetCurrentContext(), v8_str(context, "vel_y")).ToLocalChecked()));
          handle_error(
              instance->Set(isolate->GetCurrentContext(),
                            v8_str(context, "last_collide"),
                            next->Get(isolate->GetCurrentContext(), v8_str(context, "last_collide")).ToLocalChecked()));
          handle_error(
              instance->Set(isolate->GetCurrentContext(),
                            v8_str(context, "radius"),
                            next->Get(isolate->GetCurrentContext(), v8_str(context, "radius")).ToLocalChecked()));
          handle_error(
              instance->Set(isolate->GetCurrentContext(),
                            v8_str(context, "radiussize"),
                            next->Get(isolate->GetCurrentContext(), v8_str(context, "radiussize")).ToLocalChecked()));
        }
      }
    }
  });
  job->job_number++;
  job->frame_number++;
  return job->frame_number != max_frames;
  // assistant->the_job->shapes[0] = assistant->the_previous_job->shapes[0];
  // assistant->the_job->shapes[0].reset( assistant->the_previous_job->shapes[0] );
  // return !write_frame_fun();
}

std::shared_ptr<data::job> generator_v2::get_job() const {
  return job;
}
