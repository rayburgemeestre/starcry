#include "util/native_generator_context.h"
#include "util/logger.h"

native_generator_context::native_generator_context() : v8_interact_instance() {}

native_generator_context::native_generator_context(v8::Local<v8::Value> script_value, size_t current_scene_idx)
    : v8_interact_instance(std::make_unique<v8_interact>()) {
  auto& i = this->i();
  script_obj.Reset(i.get_isolate(), script_value.As<v8::Object>());
  video_obj.Reset(i.get_isolate(), i.v8_array(script_obj, "video"));
  scenes.Reset(i.get_isolate(), i.v8_array(script_obj, "scenes"));
  current_scene_val.Reset(i.get_isolate(), i.get_index(scenes, 0));
  objects.Reset(i.get_isolate(), i.v8_obj(script_obj, "objects"));
  gradients.Reset(i.get_isolate(), i.v8_obj(script_obj, "gradients"));
  set_scene(current_scene_idx);
}

void native_generator_context::set_scene(size_t current_scene_idx) {
  auto& i = this->i();
  // auto isolate = i.get_isolate();

  current_scene_val.Reset(i.get_isolate(), i.get_index(scenes, current_scene_idx));
  current_scene_obj.Reset(i.get_isolate(), current_scene_val.As<v8::Object>());

  scene_objects.Reset(i.get_isolate(), i.v8_array(current_scene_obj, "objects"));
  // instances = i.v8_array(current_scene_obj, "instances", v8::Array::New(isolate));
  // instances_next = i.v8_array(current_scene_obj, "instances_next", v8::Array::New(isolate));
  // instances_intermediate = i.v8_array(current_scene_obj, "instances_intermediate", v8::Array::New(isolate));
}

v8_interact& native_generator_context::i() const {
  return *v8_interact_instance;
}
