#include "util/generator_context.h"
#include "util/logger.h"

generator_context::generator_context() : v8_interact_instance(nullptr) {}

generator_context::generator_context(v8::Isolate* isolate, v8::Local<v8::Value> script_value, size_t current_scene_idx)
    : v8_interact_instance(std::make_unique<v8_interact>(isolate)) {
  auto& i = this->i();
  script_obj = script_value.As<v8::Object>();
  scenes = i.v8_array(script_obj, "scenes");
  objects = i.v8_array(script_obj, "objects");
  set_scene(current_scene_idx);
}

void generator_context::set_scene(size_t current_scene_idx) {
  auto& i = this->i();
  auto isolate = i.get_isolate();

  current_scene_val = i.get_index(scenes, current_scene_idx);
  current_scene_obj = current_scene_val.As<v8::Object>();

  scene_objects = i.v8_array(current_scene_obj, "objects");
  instances = i.v8_array(current_scene_obj, "instances", v8::Array::New(isolate));
  instances_next = i.v8_array(current_scene_obj, "instances_next", v8::Array::New(isolate));
  instances_intermediate = i.v8_array(current_scene_obj, "instances_intermediate", v8::Array::New(isolate));
}

v8_interact& generator_context::i() const {
  return *v8_interact_instance;
}
