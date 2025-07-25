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
  spec["brightness"] = {"float", v8::Number::New(isolate, 1.0), "Brightness correction factor"};
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

// Object type property mapping structure for documentation
// This shows which properties are available for each object type
namespace object_property_mapping {
// Common properties available to all object types:
// - unique_id, random_hash, level
// - x, y, z, angle, rotate, scale, recursive_scale
// - opacity, hue, seed, blending_type
// - gradient, gradients, texture, texture_3d, texture_offset_x/y
// - texture_effect, zernike_type
// - props (custom properties container)

// Circle-specific:
// - radius, radiussize
// - mass, vel_x, vel_y, velocity (physics)
// - collision_group, gravity_group, unique_group

// Ellipse-specific:
// - longest_diameter, shortest_diameter (or aliases: a, b)
// - radiussize
// - mass, vel_x, vel_y, velocity (physics)
// - collision_group, gravity_group, unique_group

// Line-specific:
// - x2, y2, z2 (end coordinates)
// - radiussize (represents line width)
// - unique_group
// Note: Lines do NOT have mass, velocity, collision_group, or gravity_group

// Text-specific:
// - text, text_size, text_align, text_fixed
// - mass, vel_x, vel_y, velocity (physics)
// Note: Text does NOT have collision_group, gravity_group, or unique_group

// Script-specific:
// - Has most common properties but NO gradients array
// - No physics properties
}  // namespace object_property_mapping

inline specification_fields create_object_spec(v8_interact& i) {
  specification_fields spec;
  auto isolate = i.get_isolate();

  // Common properties for all objects
  spec["unique_id"] = {"int", v8::Number::New(isolate, 0), "Unique identifier for the object"};
  spec["random_hash"] = {
      "string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Random hash for object identification"};
  spec["level"] = {"int", v8::Number::New(isolate, 0), "Object hierarchy level"};

  // Transform properties (common to all)
  spec["x"] = {"float", v8::Number::New(isolate, 0.0), "X position coordinate"};
  spec["y"] = {"float", v8::Number::New(isolate, 0.0), "Y position coordinate"};
  spec["z"] = {"float", v8::Number::New(isolate, 0.0), "Z depth coordinate"};
  spec["angle"] = {"float", v8::Number::New(isolate, 0.0), "Object angle in degrees"};
  spec["rotate"] = {"float", v8::Number::New(isolate, 0.0), "Rotation in degrees"};
  spec["scale"] = {"float", v8::Number::New(isolate, 1.0), "Object scale factor"};
  spec["recursive_scale"] = {"float", v8::Number::New(isolate, 1.0), "Recursive scaling for child objects"};

  // Physics properties (common to circle, ellipse, text)
  spec["mass"] = {"float", v8::Number::New(isolate, 1.0), "Object mass for physics calculations"};
  spec["vel_x"] = {"float", v8::Number::New(isolate, 0.0), "Velocity X component"};
  spec["vel_y"] = {"float", v8::Number::New(isolate, 0.0), "Velocity Y component"};
  spec["velocity"] = {"float", v8::Number::New(isolate, 0.0), "Overall velocity magnitude"};

  // Visual properties (common to all)
  spec["opacity"] = {"float", v8::Number::New(isolate, 1.0), "Object opacity (0-1)"};
  spec["hue"] = {"float", v8::Number::New(isolate, 0.0), "Hue adjustment (0-360)"};
  spec["seed"] = {"int", v8::Number::New(isolate, 0), "Random seed for procedural effects"};
  // TODO: fix blending_type
  spec["blending_type"] = {"int", v8::Number::New(isolate, 0), "Blending mode (0=normal, 1=lighten, 2=darken, etc.)"};

  // Gradient and texture properties (common to all)
  spec["gradient"] = {"string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Gradient identifier"};
  spec["texture"] = {"string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Texture identifier"};
  spec["texture_3d"] = {"int", v8::Number::New(isolate, 0), "3D texture type"};
  spec["texture_offset_x"] = {"float", v8::Number::New(isolate, 0.0), "Texture X offset"};
  spec["texture_offset_y"] = {"float", v8::Number::New(isolate, 0.0), "Texture Y offset"};
  // TODO: fix texture_effect and zernike_type
  spec["texture_effect"] = {"int", v8::Number::New(isolate, 0), "Texture effect (0=opacity, 1=color)"};
  spec["zernike_type"] = {"int", v8::Number::New(isolate, 0), "Zernike polynomial type (0=version1, 1=version2)"};

  // Group properties (common to circle, ellipse)
  spec["collision_group"] = {
      "string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Collision detection group"};
  spec["gravity_group"] = {
      "string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Gravity interaction group"};
  spec["unique_group"] = {
      "string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Unique grouping identifier"};

  // Circle-specific properties
  spec["radius"] = {"float", v8::Number::New(isolate, 50.0), "Circle radius"};
  spec["radiussize"] = {"float", v8::Number::New(isolate, 1.0), "Radius size"};

  // Ellipse-specific properties
  spec["longest_diameter"] = {"float", v8::Number::New(isolate, 100.0), "Ellipse longest diameter (a)"};
  spec["shortest_diameter"] = {"float", v8::Number::New(isolate, 50.0), "Ellipse shortest diameter (b)"};
  spec["a"] = {"float", v8::Number::New(isolate, 100.0), "Ellipse semi-major axis (alias for longest_diameter)"};
  spec["b"] = {"float", v8::Number::New(isolate, 50.0), "Ellipse semi-minor axis (alias for shortest_diameter)"};

  // Line-specific properties
  spec["x2"] = {"float", v8::Number::New(isolate, 100.0), "Line end X coordinate"};
  spec["y2"] = {"float", v8::Number::New(isolate, 100.0), "Line end Y coordinate"};
  spec["z2"] = {"float", v8::Number::New(isolate, 0.0), "Line end Z coordinate"};
  // Note: for lines, radiussize represents line width

  // Text-specific properties
  spec["text"] = {"string", v8::String::NewFromUtf8(isolate, "").ToLocalChecked(), "Text content to display"};
  spec["text_size"] = {"float", v8::Number::New(isolate, 16.0), "Font size in pixels"};
  spec["text_align"] = {
      "string", v8::String::NewFromUtf8(isolate, "left").ToLocalChecked(), "Text alignment (left, center, right)"};
  spec["text_fixed"] = {"bool", v8::Boolean::New(isolate, false), "Whether text has fixed position"};

  // Dynamic properties container
  spec["props"] = {"object", v8::Object::New(isolate), "Custom properties container"};

  // Gradients array
  spec["gradients"] = {"array", v8::Array::New(isolate, 0), "Array of gradient stops"};

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