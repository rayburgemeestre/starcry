/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator_v2.h"

#include <memory>
#include <mutex>

#include "primitives.h"
#include "primitives_v8.h"
#include "scripting.h"
#include "util/assistant.h"
#include "v8_wrapper.hpp"

extern std::unique_ptr<assistant_> assistant;
extern std::shared_ptr<v8_wrapper> context;

void output_fun(const std::string& s) {
  std::cout << s << std::endl;
}

v8::Local<v8::String> v8_str(std::shared_ptr<v8_wrapper> context, const std::string& str) {
  return v8::String::NewFromUtf8(context->isolate, str.c_str()).ToLocalChecked();
};

std::string v8_str(v8::Isolate* isolate, const v8::Local<v8::String>& str) {
  v8::String::Utf8Value out(isolate, str);
  return std::string(*out);
}

v8::Local<v8::Value> v8_index_object(std::shared_ptr<v8_wrapper> context,
                                     v8::Local<v8::Value> val,
                                     const std::string& str) {
  return val.As<v8::Object>()->Get(context->isolate->GetCurrentContext(), v8_str(context, str)).ToLocalChecked();
}

generator_v2::generator_v2() {
  static std::once_flag once;
  std::call_once(once, []() {
    context = nullptr;
  });
}

void generator_v2::init(const std::string& filename) {
  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename);
  }
  context->reset();
  context->add_fun("output", &output_fun);
  context->add_include_fun();

  // prepare job object
  assistant = std::make_unique<assistant_>();
  // assistant->generator = this;
  assistant->the_job = std::make_unique<data::job>();
  auto& job = *assistant->the_job;
  job.background_color.r = 0;
  job.background_color.g = 0;
  job.background_color.b = 0;
  job.background_color.a = 0;
  job.width = 1920;   // canvas_w;
  job.height = 1080;  // canvas_h;
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

  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }

  std::istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
  const auto source = std::string("script = ") + std::string(begin, end);
  context->run_array(source, [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    auto video = v8_index_object(context, val, "video");
    auto frames = v8_index_object(context, video, "frames")
                      .As<v8::Number>()
                      ->NumberValue(isolate->GetCurrentContext())
                      .ToChecked();
    max_frames = frames;
    auto objects = v8_index_object(context, val, "objects");
    v8::Local<v8::Array> object =
        objects.As<v8::Object>()->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    for (size_t i = 0; i < object->Length(); i++) {
      std::string obj_name(
          v8_str(isolate, object->Get(isolate->GetCurrentContext(), i).ToLocalChecked().As<v8::String>()));
      auto the_object = v8_index_object(context, objects, obj_name);
      if (!object->IsObject()) continue;
      if (!the_object->IsObject()) continue;
      // call init function
      v8::Local<v8::Function> fun = the_object.As<v8::Object>()
                                        ->Get(isolate->GetCurrentContext(), v8_str(context, "init"))
                                        .ToLocalChecked()
                                        .As<v8::Function>();
      v8::Handle<v8::Value> args[1];
      args[0] = v8pp::to_v8(isolate, 0.5);
      fun->Call(isolate->GetCurrentContext(), the_object, 1, args).ToLocalChecked();
    }
  });
}

void handle_object(v8::Isolate* isolate,
                   v8::Local<v8::Value> objects,
                   v8::Local<v8::Object>& obj_def,
                   v8::Local<v8::Object>& obj_def2) {
  const auto type = v8_str(isolate, v8_index_object(context, obj_def, "type").As<v8::String>());
  if (type == "circle") {
    auto radius = v8_index_object(context, obj_def, "radius")
                      .As<v8::Number>()
                      ->NumberValue(isolate->GetCurrentContext())
                      .ToChecked();
    data::shape new_shape;
    auto x =
        v8_index_object(context, obj_def2, "x").As<v8::Number>()->NumberValue(isolate->GetCurrentContext()).ToChecked();
    auto y =
        v8_index_object(context, obj_def2, "y").As<v8::Number>()->NumberValue(isolate->GetCurrentContext()).ToChecked();
    auto props1obj = v8_index_object(context, obj_def, "props").As<v8::Object>();
    auto props1 = props1obj.As<v8::Object>();
    auto props = v8_index_object(context, obj_def2, "props").As<v8::Object>();
    if (props->IsObject()) {
      v8::Local<v8::Array> propz = props->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
      for (size_t i = 0; i < propz->Length(); i++) {
        auto prop_obj = props->Get(isolate->GetCurrentContext(), i).ToLocalChecked();
        std::string prop_name(
            v8_str(isolate, propz->Get(isolate->GetCurrentContext(), i).ToLocalChecked().As<v8::String>()));
        auto the_value = v8_index_object(context, props, prop_name);
        if (prop_name == "maxradius") {
          if (props1->IsObject()) {
            props1obj->Set(isolate->GetCurrentContext(), v8_str(context, "maxradius"), the_value.As<v8::Number>());
          }
        }
      }
    }

    new_shape.x = x;
    new_shape.y = y;
    new_shape.z = 0;  // circ.get_z();
    new_shape.type = data::shape_type::circle;
    new_shape.radius = radius;    // 10.0;//circ.get_radius();
    new_shape.radius_size = 5.0;  // circ.get_radiussize();
    // new_shape.gradient_ = circ.get_gradient().to_data_gradient();
    new_shape.gradient_.colors.emplace_back(std::make_tuple(0.0, data::color{1.0, 1, 1, 1}));
    new_shape.gradient_.colors.emplace_back(std::make_tuple(1.0, data::color{0.0, 0, 0, 1}));
    new_shape.blending_ = data::blending_type::add;  // circ.blending_type_;
    assistant->the_job->shapes.push_back(new_shape);
  }
  // iterate over the sub object and recurse into them
  auto sub_objects = v8_index_object(context, obj_def, "subobj").As<v8::Array>();
  for (size_t k = 0; k < sub_objects->Length(); k++) {
    auto sub_obj = sub_objects->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
    auto obj_id = v8_index_object(context, sub_obj, "object").As<v8::String>();
    auto sub_obj_def = v8_index_object(context, objects, v8_str(isolate, obj_id)).As<v8::Object>();
    if (!sub_obj_def->IsObject()) continue;
    auto tmp = sub_obj.As<v8::Object>();
    handle_object(isolate, objects, sub_obj_def, tmp);
  }
};

bool generator_v2::generate_frame() {
  assistant->the_previous_job = assistant->the_job;

  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    auto objects = v8_index_object(context, val, "objects");
    v8::Local<v8::Array> object =
        objects.As<v8::Object>()->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    for (size_t i = 0; i < object->Length(); i++) {
      std::string obj_name(
          v8_str(isolate, object->Get(isolate->GetCurrentContext(), i).ToLocalChecked().As<v8::String>()));
      auto the_object = v8_index_object(context, objects, obj_name);
      if (!object->IsObject()) continue;
      if (!the_object->IsObject()) continue;
      // call time function
      v8::Local<v8::Function> fun = the_object.As<v8::Object>()
                                        ->Get(isolate->GetCurrentContext(), v8_str(context, "time"))
                                        .ToLocalChecked()
                                        .As<v8::Function>();
      v8::Handle<v8::Value> args[1];
      args[0] = v8pp::to_v8(isolate, 0.5);
      fun->Call(isolate->GetCurrentContext(), the_object, 1, args).ToLocalChecked();
    }
  });

  assistant->the_job->shapes.clear();

  context->run_array("script", [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    auto scenes = v8_index_object(context, val, "scenes").As<v8::Array>();
    auto objects = v8_index_object(context, val, "objects").As<v8::Array>();
    for (size_t i = 0; i < scenes->Length(); i++) {
      auto current_scene = scenes->Get(isolate->GetCurrentContext(), i).ToLocalChecked();
      if (!current_scene->IsObject()) continue;
      auto name = v8_index_object(context, current_scene, "name").As<v8::String>();
      auto scene_objects = v8_index_object(context, current_scene, "objects").As<v8::Array>();
      for (size_t j = 0; j < scene_objects->Length(); j++) {
        auto current_obj = scene_objects->Get(isolate->GetCurrentContext(), j).ToLocalChecked();
        // recurse this stuff into function handling the object
        if (!current_obj->IsObject()) continue;
        auto id = v8_index_object(context, current_obj, "id").As<v8::String>();
        auto obj_def = v8_index_object(context, objects, v8_str(isolate, id)).As<v8::Object>();
        if (!obj_def->IsObject()) continue;
        auto tmp = current_obj.As<v8::Object>();
        handle_object(isolate, objects, obj_def, tmp);
      }
    }
  });

  // assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
  // assistant->the_job.shapes[0].reset( assistant->the_previous_job.shapes[0] );

  return true;  // false;
  // return !write_frame_fun();
}

std::shared_ptr<data::job> generator_v2::get_job() const {
  return assistant->the_job;
}
