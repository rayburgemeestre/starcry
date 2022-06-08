/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "util/v8_wrapper.hpp"

#include <iostream>
#include <utility>

extern v8::Local<v8::Context> context;

inline void handle_error(v8::Maybe<bool> res) {
  if (res.ToChecked() == false) {
    std::cout << "Failed to set value" << std::endl;
  }
}

inline v8::Local<v8::String> v8_str(v8::Local<v8::Context> context, const std::string& str) {
  // TODO: try function caching here??
  return v8::String::NewFromUtf8(context->GetIsolate(), str.c_str()).ToLocalChecked();
}

inline std::string v8_str(v8::Isolate* isolate, const v8::Local<v8::String>& str) {
  v8::String::Utf8Value out(isolate, str);
  return std::string(*out);
}

inline v8::Local<v8::Value> v8_index_object(v8::Local<v8::Context> context,
                                            v8::Local<v8::Value> val,
                                            const std::string& str) {
  return val.As<v8::Object>()->Get(context, v8_str(context, str)).ToLocalChecked();
}

inline v8::Local<v8::Value> v8_index_object(v8::Local<v8::Context> context, v8::Local<v8::Value> val, size_t index) {
  return val.As<v8::Object>()->Get(context, index).ToLocalChecked();
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

  v8::Local<v8::Context> get_context() {
    return ctx;
  }

  v8::Local<v8::Object> v8_obj(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(ctx, obj, field).As<v8::Object>();
  }

  v8::Local<v8::Array> v8_array(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(ctx, obj, field).As<v8::Array>();
  }

  v8::Local<v8::Array> v8_array(v8::Local<v8::Object>& obj,
                                const std::string& field,
                                v8::Local<v8::Array> default_val) {
    auto tmp = v8_index_object(ctx, obj, field).As<v8::Array>();
    if (!tmp->IsArray()) {
      set_field(obj, field, default_val);
      tmp = v8_index_object(ctx, obj, field).As<v8::Array>();
    }
    return tmp;
  }

  v8::Local<v8::Number> v8_number(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(ctx, obj, field).As<v8::Number>();
  }

  v8::Local<v8::Number> v8_number(v8::Local<v8::Array>& obj, size_t index) {
    return v8_index_object(ctx, obj, index).As<v8::Number>();
  }

  v8::Local<v8::String> v8_string(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_index_object(ctx, obj, field).As<v8::String>();
  }

  v8::Local<v8::String> v8_string(v8::Local<v8::Array>& obj, size_t index) {
    return v8_index_object(ctx, obj, index).As<v8::String>();
  }

  double double_number(v8::Local<v8::Object>& obj, const std::string& field) {
    return v8_number(obj, field)->NumberValue(isolate->GetCurrentContext()).ToChecked();
  }

  double double_number(v8::Local<v8::Array>& obj, size_t index) {
    return v8_number(obj, index)->NumberValue(isolate->GetCurrentContext()).ToChecked();
  }

  double boolean(v8::Local<v8::Object>& obj, const std::string& field) {
    auto tmp = obj->Get(get_context(), v8_str(ctx, field)).ToLocalChecked();
    if (!tmp->IsBoolean()) return false;
    return tmp->BooleanValue(isolate);
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

  std::string str(v8::Local<v8::Array>& obj, size_t index) {
    return v8_str(isolate, v8_string(obj, index));
  }

  bool has_field(v8::Local<v8::Object> source, const std::string& source_field) {
    return source->Has(ctx, v8_str(ctx, source_field)).ToChecked();
  }

  void copy_field(v8::Local<v8::Object> dest,
                  const std::string& dest_field,
                  v8::Local<v8::Object> source,
                  const std::optional<std::string>& source_field = {}) {
    if (source_field) {
      handle_error(
          dest->Set(ctx, v8_str(ctx, dest_field), source->Get(ctx, v8_str(ctx, *source_field)).ToLocalChecked()));
    }
    // assume field is same as dest field if left unspecified
    handle_error(dest->Set(ctx, v8_str(ctx, dest_field), source->Get(ctx, v8_str(ctx, dest_field)).ToLocalChecked()));
  }

  void copy_field_if_exists(v8::Local<v8::Object> dest,
                            const std::string& dest_field,
                            v8::Local<v8::Object> source,
                            const std::optional<std::string>& source_field = {}) {
    if (source_field) {
      if (!has_field(source, *source_field)) return;
      handle_error(
          dest->Set(ctx, v8_str(ctx, dest_field), source->Get(ctx, v8_str(ctx, *source_field)).ToLocalChecked()));
    }
    // assume field is same as dest field if left unspecified
    if (!has_field(source, dest_field)) return;
    handle_error(dest->Set(ctx, v8_str(ctx, dest_field), source->Get(ctx, v8_str(ctx, dest_field)).ToLocalChecked()));
  }

  void set_field(v8::Local<v8::Object> object, v8::Local<v8::Value> field, v8::Local<v8::Value> value) {
    handle_error(object->Set(ctx, field, value));
  }
  void set_field(v8::Local<v8::Object> object, const std::string& field, v8::Local<v8::Value> value) {
    handle_error(object->Set(ctx, v8_str(ctx, field), value));
  }
  void set_field(v8::Local<v8::Object> object, size_t field_index, v8::Local<v8::Value> value) {
    handle_error(object->Set(ctx, field_index, value));
  }
  void remove_field(v8::Local<v8::Object> object, const std::string& field) {
    handle_error(object->Delete(ctx, v8_str(ctx, field)));
  }

  void set_fun(v8::Local<v8::Object> object, const std::string& field, v8::Local<v8::Function> fun) {
    handle_error(object->Set(ctx, v8_str(ctx, field), fun));
  }

  template <class... Args>
  void call_fun(v8::Local<v8::Object> object, const std::string& field, Args... args) {
    return call_fun(object, object, field, std::forward<Args>(args)...);
  }

  template <class... Args>
  void call_fun(v8::Local<v8::Object> object, v8::Local<v8::Object> self, const std::string& field, Args... args) {
    auto v8_field = v8_str(ctx, field);
    auto a = isolate->GetCurrentContext();
    auto b = object.As<v8::Object>();
    if (!b->IsObject()) {
      return;
    }
    auto c = b->Has(a, v8_field);
    auto has_field = c.ToChecked();
    if (!has_field) return;
    auto funref = object.As<v8::Object>()->Get(isolate->GetCurrentContext(), v8_field);
    if (funref.IsEmpty()) {
      // std::cout << "exit 1" << std::endl;
      return;
    }
    auto fundef = funref.ToLocalChecked();
    if (!fundef->IsFunction()) {
      // std::cout << "exit 2" << std::endl;
      return;
      // throw std::runtime_error("type not function");
    }
    auto fun = fundef.As<v8::Function>();
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#endif
    v8::Handle<v8::Value> argz[sizeof...(Args)];
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    size_t index = 0;
    (void(argz[index++] = v8pp::to_v8(isolate, args)), ...);
    fun->Call(isolate->GetCurrentContext(), self, sizeof...(Args), argz).ToLocalChecked();
  }

  v8::Local<v8::Function> get_fun(const std::string& field) {
    auto v8_field = v8_str(ctx, field);
    auto global = isolate->GetCurrentContext()->Global();
    auto funref = global->Get(isolate->GetCurrentContext(), v8_field);
    if (funref.IsEmpty()) {
      // std::cout << "exit 1" << std::endl;
      // std::exit(1);
    }
    auto fundef = funref.ToLocalChecked();
    if (!fundef->IsFunction()) {
      // std::cout << "exit 2" << std::endl;
      // std::exit(1);
    }
    auto fun = fundef.As<v8::Function>();
    return fun;
  }

  v8::Local<v8::Value> get_index(v8::Local<v8::Array> array, size_t index) {
    return array->Get(ctx, index).ToLocalChecked();
  }
  v8::Local<v8::Value> get_index(v8::Local<v8::Array> array, v8::Local<v8::Value> index) {
    return array->Get(ctx, index).ToLocalChecked();
  }
  v8::Local<v8::Value> get(v8::Local<v8::Object> array, v8::Local<v8::Value> index) {
    return array->Get(ctx, index).ToLocalChecked();
  }
  v8::Local<v8::Value> get(v8::Local<v8::Object> array, const std::string& index) {
    return array->Get(ctx, v8_str(ctx, index)).ToLocalChecked();
  }
  v8::Local<v8::Array> prop_names(v8::Local<v8::Object> obj) {
    v8::Local<v8::Array> out;
    if (!obj->IsObject() || !obj->GetOwnPropertyNames(ctx).ToLocal(&out)) {
      out = v8::Array::New(get_isolate());
    }
    return out;
  };
  void set_prototype(v8::Local<v8::Object> dest, v8::Local<v8::Object> source) {
    handle_error(dest->SetPrototype(isolate->GetCurrentContext(), source));
  }

  // utils
  void recursively_copy_object(v8::Local<v8::Object>& dest_object, v8::Local<v8::Object>& source_object) {
    auto isolate = get_isolate();
    auto obj_fields = prop_names(source_object);
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = get_index(obj_fields, k);
      auto field_name_str = str(obj_fields, k);
      if (field_name_str == "__random_hash__") continue;  // do not overwrite
      auto field_value = get(source_object, field_name);
      if (field_value->IsArray()) {
        v8::Local<v8::Array> new_sub_array = v8::Array::New(isolate);
        auto arr = field_value.As<v8::Array>();
        for (size_t l = 0; l < arr->Length(); l++) {
          auto a_src = get_index(arr, l);
          if (a_src->IsObject()) {
            auto a_dst = v8::Object::New(isolate);
            auto a_src_obj = a_src.As<v8::Object>();
            recursively_copy_object(a_dst, a_src_obj);
            call_fun(new_sub_array, "push", a_dst);
          } else {
            call_fun(new_sub_array, "push", a_src);
          }
        }
        set_field(dest_object, field_name, new_sub_array);
      } else if (field_value->IsObject() && !field_value->IsFunction()) {
        v8::Local<v8::Object> new_sub_instance = v8::Object::New(isolate);
        auto tmp = field_value.As<v8::Object>();
        recursively_copy_object(new_sub_instance, tmp);
        set_field(dest_object, field_name, new_sub_instance);
      } else {
        set_field(dest_object, field_name, field_value);
      }
    }
  }

  void empty_and_resize(v8::Local<v8::Array>& obj, size_t len) {
    while (obj->Length() > 0) {
      call_fun(obj, "pop");
    }
    for (size_t j = 0; j < len; j++) {
      call_fun(obj, "push", v8::Object::New(get_isolate()));
    }
  }
};
