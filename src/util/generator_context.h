#pragma once

#include <memory>

#include "util/v8_interact.hpp"

namespace v8 {
class Isolate;
}

class generator_context {
private:
  std::unique_ptr<v8_interact> v8_interact_instance;

public:
  v8::Persistent<v8::Object> script_obj;
  v8::Persistent<v8::Object> video_obj;
  v8::Persistent<v8::Array> scenes;
  v8::Persistent<v8::Object> gradients;
  v8::Persistent<v8::Object> objects;
  v8::Persistent<v8::Value> current_scene_val;
  v8::Persistent<v8::Object> current_scene_obj;

  // might be deprecated
  v8::Persistent<v8::Array> scene_objects;

  // deprecated
  v8::Local<v8::Array> instances;
  v8::Local<v8::Array> instances_next;
  v8::Local<v8::Array> instances_intermediate;

  generator_context();
  generator_context(const generator_context& other) = delete;
  generator_context(v8::Local<v8::Value> script_value, size_t current_scene_idx);

  void set_scene(size_t current_scene_idx);

  v8_interact& i() const;
};
