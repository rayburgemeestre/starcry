/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <experimental/filesystem>
#include <memory>
#include <mutex>
#include <sstream>
#include "libplatform/libplatform.h"
#include "v8.h"
#include "v8pp/class.hpp"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

namespace fs = std::experimental::filesystem;

/**
 * Header only wrapper around v8pp which provides a simple interface for calling functions,
 *  running javascript code, printing exceptions, etc.
 */
class v8_wrapper {
public:
  v8_wrapper(std::string filename);

  ~v8_wrapper();

  template <typename T = void>
  T run(std::string const& source);

  void call(std::string const& function_name);

  template <typename T>
  void call(std::string const& function_name, T param);

  template <typename T>
  inline void add_fun(const std::string& name, T func);

  template <typename T>
  inline void add_class(T func);

  inline void add_include_fun();

  void rethrow_as_runtime_error(v8::Isolate* isolate, v8::TryCatch& try_catch);

  v8pp::context* context;
  std::unique_ptr<v8::Platform> platform;
  std::string filename_;  // for error messages
  std::mutex mut;
};

v8_wrapper::v8_wrapper(std::string filename) : context(nullptr), platform(nullptr), filename_(filename) {
  v8::V8::InitializeExternalStartupData("starcry");
#if V8_MAJOR_VERSION >= 7
  platform = v8::platform::NewDefaultPlatform();
#else
  platform = v8::platform::CreateDefaultPlatform();
#endif
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  context = new v8pp::context();  // Making it unique_ptr breaks it!
}

template <typename T>
T v8_wrapper::run(std::string const& source) {
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

template <>
void v8_wrapper::run<void>(std::string const& source) {
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

// TODO: replace call(fn) and call(fn, T) with a template function
void v8_wrapper::call(std::string const& function_name) {
  v8::Isolate* isolate = context->isolate();
  v8::HandleScope scope(isolate);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(false);
  try_catch.SetCaptureMessage(true);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  v8::Local<v8::Function> func = global->Get(isolate->GetCurrentContext(), v8pp::to_v8(isolate, function_name.c_str()))
                                     .ToLocalChecked()
                                     .As<v8::Function>();

  func->Call(isolate->GetCurrentContext(), global, 0, {});

  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return;  // v8pp::from_v8<T>(isolate, result);
}

template <typename T>
void v8_wrapper::call(std::string const& function_name, T param) {
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
  func->Call(isolate->GetCurrentContext(), global, 1, args);

  if (try_catch.HasCaught()) {
    rethrow_as_runtime_error(isolate, try_catch);
  }
  return;  // v8pp::from_v8<T>(isolate, result);
}

template <typename T>
inline void v8_wrapper::add_fun(const std::string& name, T func) {
  v8::HandleScope scope(context->isolate());
  context->set(name.c_str(), v8pp::wrap_function(context->isolate(), name.c_str(), func));
}
template <typename T>
inline void v8_wrapper::add_class(T func) {
  v8::HandleScope scope(context->isolate());
  func(*context);
}

inline void v8_wrapper::add_include_fun() {
  v8::HandleScope scope(context->isolate());
  context->set(
      "include",
      v8pp::wrap_function(
          context->isolate(), "include", [=](const v8::FunctionCallbackInfo<v8::Value>& args) -> v8::Handle<v8::Value> {
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
                v8::Handle<v8::String> source =
                    v8::String::NewFromUtf8(context->isolate(), js_file.c_str()).ToLocalChecked();
                auto script_origin = v8::String::NewFromUtf8(context->isolate(), p.c_str()).ToLocalChecked();
                v8::ScriptOrigin so(script_origin);
                v8::Handle<v8::Script> script =
                    v8::Script::Compile(context->isolate()->GetCurrentContext(), source, &so).ToLocalChecked();
                return script->Run(context->isolate()->GetCurrentContext()).ToLocalChecked();
              }
            }
            return v8::Undefined(context->isolate());
          }));
}

v8_wrapper::~v8_wrapper() {
  // context' ownership is probably taken over by someone else.
  // delete(context);
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
}

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}
void v8_wrapper::rethrow_as_runtime_error(v8::Isolate* isolate, v8::TryCatch& try_catch) {
  auto ex = try_catch.Exception();
  v8::Local<v8::Context> context(isolate->GetCurrentContext());

  std::string const msg =
      v8pp::from_v8<std::string>(isolate, ex->ToString(isolate->GetCurrentContext()).ToLocalChecked());
  std::stringstream ss;
  v8::Handle<v8::Message> const& message(try_catch.Message());
  if (!message.IsEmpty()) {
    int linenum = message->GetLineNumber(context).FromJust();
    auto script_resource = [=]() -> std::string {
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
