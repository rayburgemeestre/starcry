/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include "libplatform/libplatform.h"
#include "v8.h"
#include "v8pp/class.hpp"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

namespace fs = std::filesystem;

/**
 * Header only wrapper around v8pp which provides a simple interface for calling functions,
 *  running javascript code, printing exceptions, etc.
 */
class v8_wrapper {
public:
  v8_wrapper(std::string filename);

  ~v8_wrapper();

  void recreate_isolate_in_this_thread();

  void reset();

  template <typename T = void>
  T run(std::string const& source);

  inline void run_array(std::string const& source, std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback);

  inline void enter_object(std::string const& source, std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback);
  inline void loop_object(std::string const& source, std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback);

  inline void call_array(std::string const& function_name,
                         std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback);

  v8::Local<v8::Value> call_raw(std::string const& function_name);

  v8::Handle<v8::Value> run_raw(std::string const& source);

  template <typename T = void>
  T call(std::string const& function_name,
         v8::Handle<v8::Value>* args,
         size_t args_len,
         std::function<void(v8::Isolate*)> initialize_args);

  template <typename T>
  inline void add_fun(const std::string& name, T func);

  template <typename T>
  inline void add_class(T func);

  inline void add_include_fun();

  void rethrow_as_runtime_error(v8::Isolate* isolate, v8::TryCatch& try_catch);

  inline void set_filename(const std::string& filename);

  v8::Isolate* isolate = nullptr;
  v8pp::context* context;
  v8::Isolate::CreateParams create_params;
  std::unique_ptr<v8::Platform> platform;
  std::string filename_;  // for error messages
  std::mutex mut;
};

inline v8_wrapper::v8_wrapper(std::string filename) : context(nullptr), platform(nullptr), filename_(filename) {
  v8::V8::InitializeExternalStartupData("starcry");
#if V8_MAJOR_VERSION >= 6
  platform = v8::platform::NewDefaultPlatform();
#else
  platform = v8::platform::CreateDefaultPlatform();
#endif
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  recreate_isolate_in_this_thread();
}

inline void v8_wrapper::recreate_isolate_in_this_thread() {
  if (!create_params.array_buffer_allocator) {
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  }

  delete (context);

  if (isolate != nullptr) {
    isolate->Exit();
    isolate->Dispose();
    isolate = nullptr;
  }

  isolate = v8::Isolate::New(create_params);
  isolate->Enter();

  context = new v8pp::context(isolate);
}

inline v8_wrapper::~v8_wrapper() {
  isolate->Exit();
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;
}

inline void v8_wrapper::reset() {
  delete (context);
  context = new v8pp::context(isolate);
}

template <typename T>
inline T v8_wrapper::run(std::string const& source) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);
  v8::Handle<v8::Value> result = context->run_script(source, filename_);
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return v8pp::from_v8<T>(isolate, result);
}

inline v8::Handle<v8::Value> v8_wrapper::run_raw(std::string const& source) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);
  v8::Handle<v8::Value> result = context->run_script(source, filename_);
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return result;
}

template <>
inline void v8_wrapper::run<void>(std::string const& source) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);
  context->run_script(source, filename_);
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
}

void v8_wrapper::call_array(std::string const& function_name,
                            std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback) {
  if (run<bool>("typeof " + function_name + " == 'undefined'")) {
    return;
  }
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  v8::Local<v8::Function> func = global->Get(isolate->GetCurrentContext(), v8pp::to_v8(isolate, function_name.c_str()))
                                     .ToLocalChecked()
                                     .As<v8::Function>();

  v8::Local<v8::Array> result =
      func->Call(isolate->GetCurrentContext(), global, 0, {}).ToLocalChecked().As<v8::Array>();
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  for (size_t i = 0; i < result->Length(); i++) {
    callback(isolate, result->Get(isolate->GetCurrentContext(), i).ToLocalChecked());
  }
}

inline void v8_wrapper::run_array(std::string const& source,
                                  std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  try_catch.SetCaptureMessage(true);
  // v8::Handle<v8::Array> result = context->run_script(source, filename_).As<v8::Array>();
  v8::Handle<v8::Value> result = context->run_script(source, filename_);
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  callback(isolate, result);
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
}
inline void v8_wrapper::enter_object(std::string const& source,
                                     std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  try_catch.SetCaptureMessage(true);
  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  auto result = global->Get(isolate->GetCurrentContext(), v8pp::to_v8(isolate, source)).ToLocalChecked();
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  callback(isolate, result);
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
}

inline void v8_wrapper::loop_object(std::string const& source,
                                    std::function<void(v8::Isolate*, v8::Local<v8::Value>)> callback) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  try_catch.SetCaptureMessage(true);
  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  auto result = global->Get(isolate->GetCurrentContext(), v8pp::to_v8(isolate, source)).ToLocalChecked();
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  if (result->IsObject()) {
    auto obj = result.As<v8::Object>();
    auto fields = obj->GetOwnPropertyNames(isolate->GetCurrentContext()).ToLocalChecked();
    for (size_t k = 0; k < fields->Length(); k++) {
      auto field = fields->Get(isolate->GetCurrentContext(), k).ToLocalChecked();
      auto element = obj->Get(isolate->GetCurrentContext(), field).ToLocalChecked();
      auto str = v8pp::from_v8<std::string>(isolate, field);
      callback(isolate, element);
    }
  } else if (result->IsArray()) {
    auto array = result.As<v8::Array>();
    for (auto i = uint32_t(0); i < array->Length(); i++) {
      auto element = array->Get(isolate->GetCurrentContext(), i).ToLocalChecked();
      callback(isolate, element);
    }
  }
  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
}

// TODO: replace call(fn) and call(fn, T) with a template function
inline v8::Local<v8::Value> v8_wrapper::call_raw(std::string const& function_name) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  v8::Local<v8::Function> func = global->Get(isolate->GetCurrentContext(), v8pp::to_v8(isolate, function_name.c_str()))
                                     .ToLocalChecked()
                                     .As<v8::Function>();

  auto result = func->Call(isolate->GetCurrentContext(), global, 0, {}).ToLocalChecked();

  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  // return v8pp::from_v8<T>(isolate, result);
  return result;
}

template <typename T>
T v8_wrapper::call(std::string const& function_name,
                   v8::Handle<v8::Value>* args,
                   size_t args_len,
                   std::function<void(v8::Isolate*)> initialize_args) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();

#if V8_MAJOR_VERSION >= 7
  v8::Local<v8::Function> func =
      global
          ->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, function_name.c_str()).ToLocalChecked())
          .ToLocalChecked()
          .As<v8::Function>();
#elif V8_MAJOR_VERSION >= 6
  v8::Local<v8::Function> func =
      global->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, function_name.c_str()))
          .ToLocalChecked()
          .As<v8::Function>();
#endif

  initialize_args(isolate);
  auto result = func->Call(isolate->GetCurrentContext(), global, args_len, args).ToLocalChecked();

  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return v8pp::from_v8<T>(isolate, result);
}
/* backup
// TODO: replace call(fn) and call(fn, T) with a template function
inline void v8_wrapper::call(std::string const& function_name) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  v8::Local<v8::Function> func = global->Get(isolate->GetCurrentContext(), v8pp::to_v8(isolate, function_name.c_str()))
                                     .ToLocalChecked()
                                     .As<v8::Function>();

  (void)func->Call(isolate->GetCurrentContext(), global, 0, {});

  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return;  // v8pp::from_v8<T>(isolate, result);
}


template <typename T>
inline void v8_wrapper::call(std::string const& function_name, T param) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  v8::Local<v8::Function> func =
      global
          ->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, function_name.c_str()).ToLocalChecked())
          .ToLocalChecked()
          .As<v8::Function>();

  v8::Handle<v8::Value> args[1];
  args[0] = v8pp::to_v8(isolate, param);
  (void)func->Call(isolate->GetCurrentContext(), global, 1, args);

  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return;  // v8pp::from_v8<T>(isolate, result);
}
*/

template <typename T>
inline void v8_wrapper::add_fun(const std::string& name, T func) {
  v8::HandleScope scope(context->isolate());
  context->function(name.c_str(), func);
}
template <typename T>
inline void v8_wrapper::add_class(T func) {
  v8::HandleScope scope(context->isolate());
  // caller is required to add the class to the context
  func(*context);
}

inline void v8_wrapper::add_include_fun() {
  v8::HandleScope scope(context->isolate());
  context->function("include", [=, this](const v8::FunctionCallbackInfo<v8::Value>& args) -> v8::Handle<v8::Value> {
    for (int i = 0; i < args.Length(); i++) {
      std::string const str = v8pp::from_v8<std::string>(
          context->isolate(), args[0]->ToString(context->isolate()->GetCurrentContext()).ToLocalChecked());

      // load_file loads the file with this name into a string,
      // I imagine you can write a function to do this :)
      std::string file(str.c_str());
      fs::path p = filename_;
      p.remove_filename();
      p.append(file);

      std::ifstream stream(p);
      if (!stream) {
        throw std::runtime_error("could not locate file " + std::string(p.c_str()));
      }
      std::istreambuf_iterator<char> begin(stream), end;
      std::string js_file(begin, end);

      if (js_file.length() > 0) {
        v8::Handle<v8::String> source = v8::String::NewFromUtf8(context->isolate(), js_file.c_str()).ToLocalChecked();
        auto script_origin = v8::String::NewFromUtf8(context->isolate(), p.c_str()).ToLocalChecked();
#if V8_MAJOR_VERSION > 9 || (V8_MAJOR_VERSION == 9 && V8_MINOR_VERSION >= 7)
        v8::ScriptOrigin so(context->isolate(), script_origin);
#else
                              v8::ScriptOrigin so(to_v8(context->isolate(), script_origin));
#endif
        v8::Handle<v8::Script> script =
            v8::Script::Compile(context->isolate()->GetCurrentContext(), source, &so).ToLocalChecked();
        return script->Run(context->isolate()->GetCurrentContext()).ToLocalChecked();
      }
    }
    return v8::Undefined(context->isolate());
  });
}

inline const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

inline void v8_wrapper::rethrow_as_runtime_error(v8::Isolate* isolate, v8::TryCatch& try_catch) {
  auto ex = try_catch.Exception();
  v8::Local<v8::Context> context(isolate->GetCurrentContext());

  std::string const msg = v8pp::from_v8<std::string>(isolate, ex->ToString(context).ToLocalChecked());
  std::stringstream ss;
  v8::Handle<v8::Message> const& message(try_catch.Message());
  if (!message.IsEmpty()) {
    int linenum = message->GetLineNumber(context).FromJust();
    auto script_resource = [=, this]() -> std::string {
      const auto val = message->GetScriptResourceName()->ToString(isolate->GetCurrentContext());
      v8::String::Utf8Value resource(isolate, message->GetScriptResourceName());
      return !val.IsEmpty() ? std::string(ToCString(resource)) : filename_;
    }();
    v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
    const char* sourceline_string = ToCString(sourceline);
    ss << "" << msg << '\n'
       << "[file: " << script_resource << ", line: " << std::dec << linenum << ".]\n"
       << sourceline_string << '\n';

    int start = message->GetStartColumn();
    int end = message->GetEndColumn();
    for (int i = 0; i < start; i++) {
      ss << '-';
    }
    for (int i = start; i < end; i++) {
      ss << '^';
    }
    ss << '\n';
  }
  throw std::runtime_error(ss.str());
}

inline void v8_wrapper::set_filename(const std::string& filename) {
  filename_ = filename;
}