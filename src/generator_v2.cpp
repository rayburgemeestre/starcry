/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator_v2.h"

#include <cmath>
#include <memory>
#include <mutex>
#include <random>

#include "primitives.h"
#include "primitives_v8.h"
#include "scripting.h"
#include "util/assistant.h"
#include "util/quadtree.h"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"
#include "util/vector_logic.hpp"

#include "shapes/circle_v2.h"
#include "shapes/position.h"
#include "shapes/rectangle_v2.h"

extern std::unique_ptr<assistant_> assistant;
extern std::shared_ptr<v8_wrapper> context;

void output_fun(const std::string& s) {
  std::cout << s << std::endl;
}

generator_v2::generator_v2() {
  static std::once_flag once;
  std::call_once(once, []() {
    context = nullptr;
  });
}

void handle_error(v8::Maybe<bool> res) {
  if (res.ToChecked() == false) {
    std::cout << "Failed to set value" << std::endl;
  }
}

void copy_object(v8::Isolate* isolate, v8::Local<v8::Object> obj_def, v8::Local<v8::Object> new_instance) {
  auto obj_fields = obj_def->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
  for (size_t k = 0; k < obj_fields->Length(); k++) {
    auto field_name = obj_fields->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
    auto field_value = obj_def->Get(isolate->GetCurrentContext(), field_name).ToLocalChecked();
    if (field_value->IsObject() && !field_value->IsFunction()) {
      v8::Local<v8::Object> new_sub_instance = v8::Object::New(isolate);
      copy_object(isolate, field_value.As<v8::Object>(), new_sub_instance);
      handle_error(new_instance->Set(isolate->GetCurrentContext(), field_name, new_sub_instance));
    } else {
      handle_error(new_instance->Set(isolate->GetCurrentContext(), field_name, field_value));
    }
  }
}

void transfer_gradient(v8_interact& i,
                       v8::Local<v8::Object>& instance,
                       data::shape& new_shape,
                       std::unordered_map<std::string, data::gradient>& gradients) {
  auto isolate = i.get_isolate();
  std::string gradient_id = i.str(instance, "gradient");
  if (!gradient_id.empty()) {
    if (new_shape.gradients_.empty()) {
      if (gradients.find(gradient_id) != gradients.end()) {
        new_shape.gradients_.emplace_back(1.0, gradients[gradient_id]);
      }
    }
  }
  auto gradient_array = i.v8_array(instance, "gradients");
  if (new_shape.gradients_.empty()) {
    for (size_t k = 0; k < gradient_array->Length(); k++) {
      auto gradient_data = gradient_array->Get(isolate->GetCurrentContext(), k).ToLocalChecked().As<v8::Array>();
      if (!gradient_data->IsArray()) {
        continue;
      }
      auto opacity = gradient_data->Get(isolate->GetCurrentContext(), 0)
                         .ToLocalChecked()
                         .As<v8::Number>()
                         ->NumberValue(isolate->GetCurrentContext())
                         .ToChecked();
      auto gradient_id =
          v8_str(isolate, gradient_data->Get(isolate->GetCurrentContext(), 1).ToLocalChecked().As<v8::String>());
      new_shape.gradients_.emplace_back(opacity, gradients[gradient_id]);
    }
  }
}

v8::Local<v8::Object> process_object(v8_interact& i,
                                     v8::Isolate* isolate,
                                     v8::Local<v8::Array>& objects,
                                     v8::Local<v8::Array>& scene_instances,
                                     v8::Local<v8::Array>& scene_instances_next,
                                     v8::Local<v8::Array>& scene_instances_intermediate,
                                     size_t& scene_instances_idx,
                                     v8::Local<v8::Object>& scene_obj,
                                     v8::Local<v8::Object>* parent_object = nullptr) {
  // Parent object
  int64_t level = 0;
  if (parent_object != nullptr) {
    level = i.integer_number(*parent_object, "level") + 1;
  }

  // The object from the scene
  auto id = i.str(scene_obj, "id");
  auto v8_id = i.v8_string(scene_obj, "id");
  auto v8_x = i.v8_number(scene_obj, "x");
  auto v8_y = i.v8_number(scene_obj, "y");
  auto v8_vel_x = i.v8_number(scene_obj, "vel_x");
  auto v8_vel_y = i.v8_number(scene_obj, "vel_y");
  auto scene_props = i.v8_obj(scene_obj, "props");

  // The object definition that will be instantiated
  auto obj_def = v8_index_object(context, objects, id).template As<v8::Object>();
  auto v8_gradients = v8_index_object(context, obj_def, "gradients").As<v8::Array>();
  if (!obj_def->IsObject()) {
    return {};
  }

  // The new object instantiation as specified by the scene object
  v8::Local<v8::Object> new_instance = v8::Object::New(isolate);
  v8::Local<v8::Object> new_instance_next = v8::Object::New(isolate);
  v8::Local<v8::Object> new_instance_intermediate = v8::Object::New(isolate);

  auto copy_object_and_initialize = [&](v8::Local<v8::Object> new_instance, const std::string& annotation) {
    copy_object(isolate, obj_def, new_instance);
    handle_error(new_instance->SetPrototype(isolate->GetCurrentContext(), scene_obj));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "id"), v8_id));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "x"), v8_x));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "y"), v8_y));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "vel_x"), v8_vel_x));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "vel_y"), v8_vel_y));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "subobj"), v8::Array::New(isolate)));
    handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "meta"), v8_str(context, annotation)));
    handle_error(
        new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "level"), v8::Number::New(isolate, level)));
    if (v8_gradients->IsArray() && v8_gradients->Length() > 0) {
      handle_error(new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "gradients"), v8_gradients));
    }

    // Copy over scene properties to instance properties
    auto props = i.v8_obj(new_instance, "props");
    auto obj_fields = scene_props->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = obj_fields->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
      auto field_value = scene_props->Get(isolate->GetCurrentContext(), field_name).ToLocalChecked();
      handle_error(props->Set(isolate->GetCurrentContext(), field_name, field_value));
    }

    // Call init function on new instance
    v8::Local<v8::Function> fun = new_instance.As<v8::Object>()
                                      ->Get(isolate->GetCurrentContext(), v8_str(context, "init"))
                                      .ToLocalChecked()
                                      .As<v8::Function>();
    v8::Handle<v8::Value> args[1];
    args[0] = v8pp::to_v8(isolate, 0.5);
    fun->Call(isolate->GetCurrentContext(), new_instance, 1, args).ToLocalChecked();
  };
  copy_object_and_initialize(new_instance, "instance");
  copy_object_and_initialize(new_instance_next, "next");
  copy_object_and_initialize(new_instance_intermediate, "intermediate");

  // Add the instance to the scene for later rendering.
  handle_error(scene_instances->Set(isolate->GetCurrentContext(), scene_instances_idx, new_instance));
  handle_error(scene_instances_next->Set(isolate->GetCurrentContext(), scene_instances_idx, new_instance_next));
  handle_error(
      scene_instances_intermediate->Set(isolate->GetCurrentContext(), scene_instances_idx, new_instance_intermediate));
  scene_instances_idx++;

  // Recurse for the sub objects the init function populated.
  handle_error(scene_obj->Set(isolate->GetCurrentContext(), v8_str(context, "level"), v8::Number::New(isolate, level)));
  auto subobjs = i.v8_array(new_instance, "subobj");
  auto subobjs2 = i.v8_array(new_instance_next, "subobj");
  auto subobjs3 = i.v8_array(new_instance_intermediate, "subobj");
  for (size_t k = 0; k < subobjs->Length(); k++) {
    auto subobj = subobjs->Get(isolate->GetCurrentContext(), k).ToLocalChecked().As<v8::Object>();
    auto instance = process_object(i,
                                   isolate,
                                   objects,
                                   scene_instances,
                                   scene_instances_next,
                                   scene_instances_intermediate,
                                   scene_instances_idx,
                                   subobj,
                                   &scene_obj);
    handle_error(subobjs->Set(isolate->GetCurrentContext(), k, instance));
    handle_error(subobjs2->Set(isolate->GetCurrentContext(), k, instance));
    handle_error(subobjs3->Set(isolate->GetCurrentContext(), k, instance));
  }

  // This is the object that will be also placed as a pointer in the subobject array for the object
  // return new_instance_next;
  return new_instance_next;
}

// TODO: expose this via a function instead
extern std::mt19937 mt;

void generator_v2::init(const std::string& filename) {
  // Initialize Javascript Context
  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename);
  }
  context->reset();
  context->add_fun("output", &output_fun);
  context->add_fun("rand", &rand_fun);
  context->add_include_fun();

  // Prepare job object
  assistant = std::make_unique<assistant_>();
  assistant->the_job = std::make_unique<data::job>();
  auto& job = *assistant->the_job;
  job.background_color.r = 0;
  job.background_color.g = 0;
  job.background_color.b = 0;
  job.background_color.a = 0;
  job.width = 1920;   // canvas_w;
  job.height = 1080;  // canvas_h;
  job.job_number = 0;
  job.frame_number = assistant->current_frame;
  job.rendered = false;
  job.last_frame = false;
  job.num_chunks = 1;    // num_chunks;
  job.canvas_w = 1920;   // canvas_w;
  job.canvas_h = 1080;   // canvas_h;
  job.scale = 1.0;       // scale;
  job.compress = false;  // TODO: hardcoded
  job.save_image = false;
  assistant->max_frames = 250;  // max_frames;
  // if (custom_max_frames) {
  //   assistant->max_frames = *custom_max_frames;
  // }
  // assistant->realtime = realtime;

  // Evaluate the user defined javascript project
  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }

  // Extract video meta information
  std::istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
  const auto source = std::string("script = ") + std::string(begin, end);
  context->run_array(source, [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto video = v8_index_object(context, val, "video").As<v8::Object>();
    auto duration = i.double_number(video, "duration");
    use_fps = i.double_number(video, "fps");
    canvas_w = i.double_number(video, "width");
    canvas_h = i.double_number(video, "height");
    auto seed = i.double_number(video, "rand_seed");
    granularity = i.double_number(video, "granularity");
    mt.seed(seed);

    max_frames = duration * use_fps;
    job.width = canvas_w;
    job.height = canvas_h;
    job.canvas_w = canvas_w;
    job.canvas_h = canvas_h;
    job.scale = std::max(i.double_number(video, "scale"), static_cast<double>(1));
  });

  // Extract all the gradients defined by the project
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto gradient_objects = i.v8_obj(obj, "gradients");
    auto gradient_fields = gradient_objects->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    for (size_t k = 0; k < gradient_fields->Length(); k++) {
      auto gradient_id = gradient_fields->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
      auto positions =
          gradient_objects->Get(isolate->GetCurrentContext(), gradient_id).ToLocalChecked().As<v8::Array>();
      auto id = v8_str(isolate, gradient_id.As<v8::String>());
      for (size_t l = 0; l < positions->Length(); l++) {
        auto position = positions->Get(isolate->GetCurrentContext(), l).ToLocalChecked().As<v8::Object>();
        auto pos = i.double_number(position, "position");
        auto r = i.double_number(position, "r");
        auto g = i.double_number(position, "g");
        auto b = i.double_number(position, "b");
        auto a = i.double_number(position, "a");
        gradients[id].colors.emplace_back(std::make_tuple(pos, data::color{r, g, b, a}));
      }
    }
  });

  // Instantiate all the objects defined in the scene recursively, three times(!)
  context->run_array("script", [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
    auto objects = i.v8_array(obj, "objects");
    for (size_t I = 0; I < scenes->Length(); I++) {
      auto current_scene = scenes->Get(isolate->GetCurrentContext(), I).ToLocalChecked();
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      auto scene_objects = i.v8_array(sceneobj, "objects");
      // current instances
      auto scene_instances = i.v8_array(sceneobj, "instances");
      // current'
      auto scene_instances_next = i.v8_array(sceneobj, "instances_next");
      // current' intermediate state per step
      auto scene_instances_intermediate = i.v8_array(sceneobj, "instances_intermediate");
      if (!scene_instances->IsArray()) {
        handle_error(
            sceneobj->Set(isolate->GetCurrentContext(), v8_str(context, "instances"), v8::Array::New(isolate)));
        handle_error(
            sceneobj->Set(isolate->GetCurrentContext(), v8_str(context, "instances_next"), v8::Array::New(isolate)));
        handle_error(sceneobj->Set(
            isolate->GetCurrentContext(), v8_str(context, "instances_intermediate"), v8::Array::New(isolate)));
      }
      scene_instances = i.v8_array(sceneobj, "instances");
      scene_instances_next = i.v8_array(sceneobj, "instances_next");
      scene_instances_intermediate = i.v8_array(sceneobj, "instances_intermediate");
      size_t scene_instances_idx = scene_instances->Length();
      for (size_t j = 0; j < scene_objects->Length(); j++) {
        auto scene_obj = scene_objects->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
        auto instance = process_object(i,
                                       isolate,
                                       objects,
                                       scene_instances,
                                       scene_instances_next,
                                       scene_instances_intermediate,
                                       scene_instances_idx,
                                       scene_obj);
        handle_error(scene_objects->Set(isolate->GetCurrentContext(), j, instance));
      }
    }
  });
}

bool generator_v2::generate_frame() {
  assistant->the_previous_job = assistant->the_job;
  assistant->the_job->shapes.clear();

  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
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
          assistant->the_job->shapes.clear();
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
          assistant->the_job->shapes.emplace_back();
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
            args[0] = v8pp::to_v8(isolate, static_cast<double>(assistant->the_job->frame_number) / max_frames);
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
            args[0] = v8pp::to_v8(isolate, static_cast<double>(assistant->the_job->frame_number) / max_frames);
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

            data::shape new_shape;
            new_shape.x = x;
            new_shape.y = y;

            new_shape.gradients_.clear();
            transfer_gradient(i, instance, new_shape, gradients);
            while (level > 0) {
              level--;
              new_shape.x += i.double_number(parents[level], "x");
              new_shape.y += i.double_number(parents[level], "y");
              transfer_gradient(i, parents[level], new_shape, gradients);
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
                indexes[j][current_step] = assistant->the_job->shapes[current_step].size();
              } else {
                new_shape.indexes = indexes[j];
              }
              assistant->the_job->shapes[current_step].push_back(new_shape);
            } else {
              new_shape.type = data::shape_type::none;
            }
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
          if (assistant->the_job->shapes.size() != max_step) {
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
  assistant->the_job->job_number++;
  assistant->the_job->frame_number++;
  return assistant->the_job->frame_number != max_frames;
  // assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
  // assistant->the_job.shapes[0].reset( assistant->the_previous_job.shapes[0] );
  // return !write_frame_fun();
}

std::shared_ptr<data::job> generator_v2::get_job() const {
  return assistant->the_job;
}
