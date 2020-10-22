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
  const auto source = std::string(begin, end);
  context->run_array(source, [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    auto objects = v8_index_object(context, val, "objects");
    v8::Local<v8::Array> object =
        objects.As<v8::Object>()->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    for (size_t i = 0; i < object->Length(); i++) {
      std::string obj_name(
          v8_str(isolate, object->Get(isolate->GetCurrentContext(), i).ToLocalChecked().As<v8::String>()));
      auto the_object = v8_index_object(context, objects, obj_name);
      if (!object->IsObject()) continue;
      if (!the_object->IsObject()) continue;
      std::cout << "initializing object: " << obj_name << std::endl;

      // see if state is saved, (before)
      auto test = v8_index_object(context, the_object, "test")
                      .As<v8::Integer>()
                      ->IntegerValue(isolate->GetCurrentContext())
                      .ToChecked();
      std::cout << "test = " << test << std::endl;

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

bool generator_v2::generate_frame() {
  assistant->the_previous_job = assistant->the_job;

  // TODO iterate over objects invoke time etc.
  context->run_array("x", [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    auto scenes = v8_index_object(context, val, "scenes").As<v8::Array>();
    auto objects = v8_index_object(context, val, "objects").As<v8::Array>();
    for (size_t i = 0; i < scenes->Length(); i++) {
      auto current_scene = scenes->Get(isolate->GetCurrentContext(), i).ToLocalChecked();
      if (!current_scene->IsObject()) continue;
      auto name = v8_index_object(context, current_scene, "name").As<v8::String>();
      auto scene_objects = v8_index_object(context, current_scene, "objects").As<v8::Array>();
      std::cout << "at scene: " << v8_str(isolate, name) << std::endl;
      for (size_t j = 0; j < scene_objects->Length(); j++) {
        auto current_obj = scene_objects->Get(isolate->GetCurrentContext(), j).ToLocalChecked();
        // recurse this stuff into function handling the object
        if (!current_obj->IsObject()) continue;
        auto id = v8_index_object(context, current_obj, "id").As<v8::String>();
        std::cout << "at scene obj: " << v8_str(isolate, id) << std::endl;
        auto obj_def = v8_index_object(context, objects, v8_str(isolate, id)).As<v8::Object>();
        if (!obj_def->IsObject()) continue;
        auto test = v8_index_object(context, obj_def, "test")
                        .As<v8::Integer>()
                        ->IntegerValue(isolate->GetCurrentContext())
                        .ToChecked();
        auto type = v8_index_object(context, obj_def, "type").As<v8::String>();
        auto sub_objects = v8_index_object(context, obj_def, "subobj").As<v8::Array>();
        std::cout << "found def to make instance of, with test value " << test << " of type: " << v8_str(isolate, type)
                  << " with subobjects.size of: " << sub_objects->Length() << std::endl;
        // iterate over the sub object and recurse into them
      }
    }
  });

  // assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
  // assistant->the_job.shapes[0].reset( assistant->the_previous_job.shapes[0] );

  return false;
  // return !write_frame_fun();
}

std::shared_ptr<data::job> generator_v2::get_job() const {
  return assistant->the_job;
}
