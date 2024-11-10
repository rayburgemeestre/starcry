#include "util/generator_context.h"
#include "util/logger.h"

generator_context::generator_context() : v8_interact_instance() {}

generator_context::generator_context(v8::Local<v8::Value> script_value, size_t current_scene_idx)
    : v8_interact_instance(std::make_unique<v8_interact>()) {
  auto& i = this->i();
  script_obj.Reset(i.get_isolate(), script_value.As<v8::Object>());
  const auto initialize_array = [&](auto& target_object, const std::string& key_field) {
    if (i.has_field(script_obj, key_field)) {
      target_object.Reset(i.get_isolate(), i.v8_array(script_obj, key_field));
    } else {
      target_object.Reset(i.get_isolate(), v8::Array::New(i.get_isolate()));
    }
  };
  const auto initialize_object = [&](auto& target_object, const std::string& key_field) {
    if (i.has_field(script_obj, key_field)) {
      target_object.Reset(i.get_isolate(), i.v8_obj(script_obj, key_field));
    } else {
      target_object.Reset(i.get_isolate(), v8::Object::New(i.get_isolate()));
    }
  };
  initialize_array(video_obj, "video");
  initialize_array(scenes, "scenes");
  auto scenes_array = scenes.Get(i.get_isolate());
  if (scenes_array->Length() == 0) {
    throw std::runtime_error("No scenes defined in script");
  }
  current_scene_val.Reset(i.get_isolate(), i.get_index(scenes, 0));
  initialize_object(objects, "objects");
  initialize_object(gradients, "gradients");
  initialize_object(textures, "textures");
  set_scene(current_scene_idx);
}

void generator_context::set_scene(size_t current_scene_idx) {
  auto& i = this->i();

  current_scene_val.Reset(i.get_isolate(), i.get_index(scenes, current_scene_idx));
  v8::Local<v8::Value> v = current_scene_val.Get(i.get_isolate());
  v8::Local<v8::Object> vo = v->ToObject(i.get_isolate()->GetCurrentContext()).ToLocalChecked();
  current_scene_obj.Reset(i.get_isolate(), vo);

  scene_objects.Reset(i.get_isolate(), i.v8_array(current_scene_obj, "objects"));
}

v8_interact& generator_context::i() const {
  return *v8_interact_instance;
}
