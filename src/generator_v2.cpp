/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator_v2.h"

#include <memory>
#include <mutex>

#include "primitives.h"
#include "primitives_v8.h"
#include "scripting.h"
#include "v8_wrapper.hpp"

extern std::shared_ptr<v8_wrapper> context;

void output_fun(const std::string& s) {
  std::cout << s << std::endl;
}

v8::Local<v8::String> v8_str(std::shared_ptr<v8_wrapper> context, const std::string& str) {
  return v8::String::NewFromUtf8(context->isolate, str.c_str()).ToLocalChecked();
};

v8::Local<v8::Value> v8_index_object(std::shared_ptr<v8_wrapper> context,
                                     v8::Local<v8::Value> val,
                                     const std::string& str) {
  return val.As<v8::Object>()->Get(context->isolate->GetCurrentContext(), v8_str(context, str)).ToLocalChecked();
}

generator_v2::generator_v2() {
  static std::once_flag once;
  std::call_once(once, []() {
    context = nullptr;
  });
}

void generator_v2::init(const std::string& filename) {
  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename);
  }
  context->reset();
  context->add_fun("output", &output_fun);
  context->add_include_fun();
  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }
  std::istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
  const auto source = std::string(begin, end);
  context->run_array(source, [](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    auto objects = v8_index_object(context, val, "objects");
    auto object1 = v8_index_object(context, objects, "obj1");
    // get time function
    v8::Local<v8::Function> fun = object1.As<v8::Object>()
                                      ->Get(isolate->GetCurrentContext(), v8_str(context, "time"))
                                      .ToLocalChecked()
                                      .As<v8::Function>();
    // call function with correct 'this'
    v8::Handle<v8::Value> args[1];
    args[0] = v8pp::to_v8(isolate, 0.5);
    fun->Call(isolate->GetCurrentContext(), object1, 1, args).ToLocalChecked();
  });
}
