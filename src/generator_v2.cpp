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

  v8::Isolate* get_isolate() {
    return isolate;
  }

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

  //  v8::Maybe<double> maybe_double_number(v8::Local<v8::Object>& obj, const std::string& field) {
  //    return v8_number(obj, field)->NumberValue(isolate->GetCurrentContext());
  //  }

  int64_t integer_number(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_number(obj, field)->IntegerValue(isolate->GetCurrentContext()).ToChecked();
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
  copy_object(isolate, obj_def, new_instance);
  new_instance->SetPrototype(isolate->GetCurrentContext(), scene_obj);
  new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "x"), v8_x);
  new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "y"), v8_y);
  new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "vel_x"), v8_vel_x);
  new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "vel_y"), v8_vel_y);
  new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "subobj"), v8::Array::New(isolate));
  new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "level"), v8::Number::New(isolate, level));
  if (v8_gradients->IsArray() && v8_gradients->Length() > 0) {
    new_instance->Set(isolate->GetCurrentContext(), v8_str(context, "gradients"), v8_gradients);
  }

  // Copy over scene properties to instance properties
  auto props = i.v8_obj(new_instance, "props");
  auto obj_fields = scene_props->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
  for (size_t k = 0; k < obj_fields->Length(); k++) {
    auto field_name = obj_fields->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
    auto field_value = scene_props->Get(isolate->GetCurrentContext(), field_name).ToLocalChecked();
    props->Set(isolate->GetCurrentContext(), field_name, field_value);
  }

  // Call init function on new instance
  v8::Local<v8::Function> fun = new_instance.As<v8::Object>()
                                    ->Get(isolate->GetCurrentContext(), v8_str(context, "init"))
                                    .ToLocalChecked()
                                    .As<v8::Function>();
  v8::Handle<v8::Value> args[1];
  args[0] = v8pp::to_v8(isolate, 0.5);
  fun->Call(isolate->GetCurrentContext(), new_instance, 1, args).ToLocalChecked();

  // Add the instance to the scene for later rendering.
  scene_instances->Set(isolate->GetCurrentContext(), scene_instances_idx++, new_instance);

  // Recurse for the sub objects the init function populated.
  scene_obj->Set(isolate->GetCurrentContext(), v8_str(context, "level"), v8::Number::New(isolate, level));
  auto subobjs = i.v8_array(new_instance, "subobj");
  for (size_t k = 0; k < subobjs->Length(); k++) {
    auto subobj = subobjs->Get(isolate->GetCurrentContext(), k).ToLocalChecked().As<v8::Object>();
    auto instance = process_object(i, isolate, objects, scene_instances, scene_instances_idx, subobj, &scene_obj);
    subobjs->Set(isolate->GetCurrentContext(), k, instance);
  }
  return new_instance;
}

// TODO: expose this via a function instead
extern std::mt19937 mt;

void generator_v2::init(const std::string& filename) {
  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename);
  }
  context->reset();
  context->add_fun("output", &output_fun);
  context->add_fun("rand", &rand_fun);
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

  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }

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
    mt.seed(seed);

    max_frames = duration * use_fps;
    job.width = canvas_w;
    job.height = canvas_h;
    job.canvas_w = canvas_w;
    job.canvas_h = canvas_h;
    job.scale = std::max(i.double_number(video, "scale"), static_cast<double>(1));
  });

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
        auto instance = process_object(i, isolate, objects, scene_instances, scene_instances_idx, scene_obj);
        scene_objects->Set(isolate->GetCurrentContext(), j, instance);
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
      auto scene_instances = i.v8_array(sceneobj, "instances");
      if (!scene_instances->IsArray()) continue;
      std::unordered_map<int64_t, v8::Local<v8::Object>> parents;
      for (size_t j = 0; j < scene_instances->Length(); j++) {
        auto instance = scene_instances->Get(isolate->GetCurrentContext(), j).ToLocalChecked().As<v8::Object>();
        if (!instance->IsObject()) {
          continue;
        }
        int64_t level = i.integer_number(instance, "level");
        parents[level] = instance;
        v8::Local<v8::Function> fun2 = instance.As<v8::Object>()
                                           ->Get(isolate->GetCurrentContext(), v8_str(context, "time"))
                                           .ToLocalChecked()
                                           .As<v8::Function>();
        v8::Handle<v8::Value> args[2];
        args[0] = v8pp::to_v8(isolate, static_cast<double>(assistant->the_job->frame_number) / max_frames);
        args[1] = v8pp::to_v8(isolate, static_cast<double>(1.0) / static_cast<double>(use_fps));
        fun2->Call(isolate->GetCurrentContext(), instance, 2, args).ToLocalChecked();
        data::shape new_shape;
        std::string type = i.str(instance, "type");

        auto x = i.double_number(instance, "x");
        auto y = i.double_number(instance, "y");
        auto vel_x = i.double_number(instance, "vel_x");
        auto vel_y = i.double_number(instance, "vel_y");

        if (!std::isnan(vel_x)) {
          x += vel_x;
        }
        if (!std::isnan(vel_y)) {
          y += vel_y;
        }
        instance->Set(isolate->GetCurrentContext(), v8_str(context, "x"), v8::Number::New(isolate, x));
        instance->Set(isolate->GetCurrentContext(), v8_str(context, "y"), v8::Number::New(isolate, y));

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
        new_shape.type = data::shape_type::circle;
        new_shape.radius = i.double_number(instance, "radius");
        new_shape.radius_size = i.double_number(instance, "radiussize");
        new_shape.blending_ = data::blending_type::add;
        if (type == "circle") {
          assistant->the_job->shapes.push_back(new_shape);
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
