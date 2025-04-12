/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "specification_field.hpp"
#include "util/v8_interact.hpp"

inline specification_fields create_video_spec(v8_interact& i) {
  specification_fields spec;
  auto isolate = i.get_isolate();
  auto context = isolate->GetCurrentContext();

  // Basic video properties
  spec["fps"] = {"int", v8::Number::New(isolate, 25), "Frames per second"};
  spec["width"] = {"int", v8::Number::New(isolate, 1920), "Video width in pixels"};
  spec["height"] = {"int", v8::Number::New(isolate, 1080), "Video height in pixels"};

  // Scaling and transformation
  spec["scale"] = {"float", v8::Number::New(isolate, 1.0), "Global scaling factor"};
  spec["init_scale"] = {"float", v8::Number::New(isolate, 0.5), "Initial scaling factor"};
  spec["scale_ratio"] = {"bool", v8::Boolean::New(isolate, false), "Apply ratio scaling"};

  // Randomization and seeding
  spec["rand_seed"] = {"int", v8::Number::New(isolate, 1), "Random seed for reproducibility"};

  // Granularity controls
  spec["granularity"] = {"int", v8::Number::New(isolate, 100), "Level of detail/steps"};
  spec["min_granularity"] = {"int", v8::Number::New(isolate, 2), "Minimum granularity level"};
  spec["max_granularity"] = {"int", v8::Number::New(isolate, 2), "Maximum granularity level"};

  // Background and color
  auto bg_color = v8::Object::New(isolate);
  bg_color->Set(context, v8_str(isolate, "r"), v8::Number::New(isolate, 0.0)).Check();
  bg_color->Set(context, v8_str(isolate, "g"), v8::Number::New(isolate, 0.0)).Check();
  bg_color->Set(context, v8_str(isolate, "b"), v8::Number::New(isolate, 0.0)).Check();
  bg_color->Set(context, v8_str(isolate, "a"), v8::Number::New(isolate, 1.0)).Check();
  spec["bg_color"] = {"object", bg_color, "Background color in RGBA format"};
  spec["gamma"] = {"float", v8::Number::New(isolate, 1.0), "Gamma correction factor"};

  // Motion and animation
  spec["minimize_steps_per_object"] = {"bool", v8::Boolean::New(isolate, false), "Optimize object motion steps"};
  spec["update_positions"] = {"bool", v8::Boolean::New(isolate, true), "Update object positions each frame"};
  spec["min_intermediates"] = {"int", v8::Number::New(isolate, 1), "Minimum intermediate frames"};
  spec["max_intermediates"] = {"int", v8::Number::New(isolate, 1), "Maximum intermediate frames"};
  spec["motion_blur"] = {"bool", v8::Boolean::New(isolate, true), "Apply motion blur effect"};
  spec["fast_ff"] = {"bool", v8::Boolean::New(isolate, true), "Enable fast-forward optimization"};

  // Visual effects
  spec["grain_for_opacity"] = {"bool", v8::Boolean::New(isolate, false), "Apply grain based on opacity"};
  spec["dithering"] = {"bool", v8::Boolean::New(isolate, true), "Apply dithering effect"};
  spec["extra_grain"] = {"float", v8::Number::New(isolate, 0.2), "Additional grain intensity"};
  spec["perlin_noise"] = {"bool", v8::Boolean::New(isolate, true), "Apply Perlin noise"};

  // Physics simulation
  spec["gravity_range"] = {"float", v8::Number::New(isolate, 10000.0), "Maximum range for gravity effects"};
  spec["gravity_G"] = {"float", v8::Number::New(isolate, 1.0), "Gravitational constant"};
  spec["gravity_constrain_dist_min"] = {"float", v8::Number::New(isolate, 5.0), "Minimum gravity constraint distance"};
  spec["gravity_constrain_dist_max"] = {"float", v8::Number::New(isolate, 25.0), "Maximum gravity constraint distance"};

  // Sampling
  auto sample = v8::Object::New(isolate);
  sample->Set(context, v8_str(isolate, "include"), v8::Number::New(isolate, 1.0)).Check();
  sample->Set(context, v8_str(isolate, "exclude"), v8::Number::New(isolate, 5.0)).Check();
  spec["sample"] = {"object", sample, "Frame sampling configuration"};

  return spec;
}

inline specification_fields create_object_spec(v8_interact& i) {
  specification_fields spec;
  auto isolate = i.get_isolate();

  // TODO
  spec["placeholder"] = {"int", v8::Number::New(isolate, 1000), "Place"};
  spec["placeholder2"] = {"int", v8::Number::New(isolate, 1002), "Holder"};

  return spec;
}

inline std::string specification_to_json(v8_interact& i,
                                         std::function<specification_fields(v8_interact& i)> create_spec_fun) {
  auto isolate = i.get_isolate();
  auto spec = create_spec_fun(i);
  auto context = isolate->GetCurrentContext();

  auto v8_spec = v8::Object::New(isolate);
  for (const auto& [key, field] : spec) {
    auto field_obj = v8::Object::New(isolate);
    field_obj->Set(context, v8_str(isolate, "type"), v8_str(isolate, field.type)).Check();
    field_obj->Set(context, v8_str(isolate, "default"), field.default_value).Check();
    field_obj->Set(context, v8_str(isolate, "description"), v8_str(isolate, field.description)).Check();

    v8_spec->Set(context, v8_str(isolate, key), field_obj).Check();
  }

  v8::Local<v8::String> script =
      v8::String::NewFromUtf8(isolate, "(function(obj) { return JSON.stringify(obj, null, 2); })").ToLocalChecked();

  v8::Local<v8::Script> compiled_script = v8::Script::Compile(context, script).ToLocalChecked();

  v8::Local<v8::Function> json_function = v8::Local<v8::Function>::Cast(compiled_script->Run(context).ToLocalChecked());

  v8::Local<v8::Value> args[] = {v8_spec};
  v8::Local<v8::Value> result = json_function->Call(context, context->Global(), 1, args).ToLocalChecked();

  v8::String::Utf8Value json(isolate, result);
  return *json;
}