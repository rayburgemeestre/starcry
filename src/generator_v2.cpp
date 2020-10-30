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
}

std::string v8_str(v8::Isolate* isolate, const v8::Local<v8::String>& str) {
  v8::String::Utf8Value out(isolate, str);
  return std::string(*out);
}

v8::Local<v8::Value> v8_index_object(std::shared_ptr<v8_wrapper> context,
                                     v8::Local<v8::Value> val,
                                     const std::string& str) {
  return val.As<v8::Object>()->Get(context->isolate->GetCurrentContext(), v8_str(context, str)).ToLocalChecked();
}

/*
 157 i::Handle<i::String> v8_str(i::Isolate* isolate, const char* str) {¬
 158   return isolate->factory()->NewStringFromAsciiChecked(str);¬
 159 }¬
 160 Local<String> v8_str(Isolate* isolate, const char* str) {¬
 161   return Utils::ToLocal(v8_str(reinterpret_cast<i::Isolate*>(isolate), str));¬
 162 }¬
 */
class v8_interact {
private:
  v8::Isolate* isolate;
  v8::Local<v8::Context> ctx;

public:
  v8_interact(v8::Isolate* isolate) : isolate(isolate), ctx(isolate->GetCurrentContext()) {}

  v8::Local<v8::Object> v8_obj(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(context, obj, field).As<v8::Object>();
  }

  v8::Local<v8::Array> v8_array(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(context, obj, field).As<v8::Array>();
  }

  v8::Local<v8::Number> v8_number(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(context, obj, field).As<v8::Number>();
  }

  v8::Local<v8::String> v8_string(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(context, obj, field).As<v8::String>();
  }

  double double_number(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_number(obj, field)->NumberValue(isolate->GetCurrentContext()).ToChecked();
  }

  std::string str(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_str(isolate, v8_string(obj, field));
  }
};

generator_v2::generator_v2() {
  static std::once_flag once;
  std::call_once(once, []() {
    context = nullptr;
  });
}

void copy_object(v8::Isolate* isolate, v8::Local<v8::Object> obj_def, v8::Local<v8::Object> new_instance) {
  auto obj_fields = obj_def->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
  for (size_t k = 0; k < obj_fields->Length(); k++) {
    auto field_name = obj_fields->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
    auto field_value = obj_def->Get(isolate->GetCurrentContext(), field_name).ToLocalChecked();
    if (field_value->IsObject() && !field_value->IsFunction()) {
      v8::Local<v8::Object> new_sub_instance = v8::Object::New(isolate);
      copy_object(isolate, field_value.As<v8::Object>(), new_sub_instance);
      new_instance->Set(isolate->GetCurrentContext(), field_name, new_sub_instance);
    } else {
      new_instance->Set(isolate->GetCurrentContext(), field_name, field_value);
    }
  }
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
      // // call init function
      // v8::Local<v8::Function> fun = the_object.As<v8::Object>()
      //                                   ->Get(isolate->GetCurrentContext(), v8_str(context, "init"))
      //                                   .ToLocalChecked()
      //                                   .As<v8::Function>();
      // v8::Handle<v8::Value> args[1];
      // args[0] = v8pp::to_v8(isolate, 0.5);
      // fun->Call(isolate->GetCurrentContext(), the_object, 1, args).ToLocalChecked();
    }
  });

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
      auto scene_instances = i.v8_array(sceneobj, "instances");
      if (!scene_instances->IsArray()) {
        sceneobj->Set(isolate->GetCurrentContext(), v8_str(context, "instances"), v8::Array::New(isolate));
      }
      scene_instances = i.v8_array(sceneobj, "instances");
      size_t scene_instances_idx = scene_instances->Length();
      for (size_t j = 0; j < scene_objects->Length(); j++) {
        auto scene_obj = scene_objects->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
        auto id = i.str(scene_obj, "id");
        auto x = i.str(scene_obj, "x");
        auto y = i.str(scene_obj, "y");
        auto v8_x = i.v8_number(scene_obj, "x");
        auto v8_y = i.v8_number(scene_obj, "y");
        auto scene_props = i.v8_obj(scene_obj, "props");
        auto maxradius_prop = i.v8_number(scene_props, "maxradius");
        auto obj_def = v8_index_object(context, objects, id).As<v8::Object>();
        if (!obj_def->IsObject()) continue;
        auto type = i.str(obj_def, "type");
        v8::Local<v8::Object> new_instance = v8::Object::New(isolate);
        copy_object(isolate, obj_def, new_instance);
        new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "x"), v8_x);
        new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "y"), v8_y);
        auto props = i.v8_obj(new_instance, "props");
        props->Set(isolate->GetCurrentContext(), v8_str(context, "maxradius"), maxradius_prop);
        scene_instances->Set(isolate->GetCurrentContext(), scene_instances_idx++, new_instance);
      }
    }
  });
}

bool generator_v2::generate_frame() {
  assistant->the_previous_job = assistant->the_job;
  assistant->the_job->shapes.clear();

  context->run_array("script", [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i(isolate);
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
    for (size_t I = 0; I < scenes->Length(); I++) {
      auto current_scene = scenes->Get(isolate->GetCurrentContext(), I).ToLocalChecked();
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      auto scene_instances = i.v8_array(sceneobj, "instances");
      if (!scene_instances->IsArray()) continue;
      for (size_t j = 0; j < scene_instances->Length(); j++) {
        auto instance = scene_instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
        if (!instance->IsObject()) {
          continue;
        }
        v8::Local<v8::Function> fun2 = instance.As<v8::Object>()
                                           ->Get(isolate->GetCurrentContext(), v8_str(context, "time"))
                                           .ToLocalChecked()
                                           .As<v8::Function>();
        v8::Handle<v8::Value> args[1];
        args[0] = v8pp::to_v8(isolate, 0.5);
        fun2->Call(isolate->GetCurrentContext(), instance, 1, args).ToLocalChecked();
        data::shape new_shape;
        new_shape.x = i.double_number(instance, "x");
        new_shape.y = i.double_number(instance, "y");
        new_shape.z = 0;
        new_shape.type = data::shape_type::circle;
        new_shape.radius = i.double_number(instance, "radius");
        new_shape.radius_size = 5.0;
        new_shape.gradient_.colors.emplace_back(std::make_tuple(0.0, data::color{1.0, 1, 1, 1}));
        new_shape.gradient_.colors.emplace_back(std::make_tuple(1.0, data::color{0.0, 0, 0, 1}));
        new_shape.blending_ = data::blending_type::add;
        assistant->the_job->shapes.push_back(new_shape);
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
