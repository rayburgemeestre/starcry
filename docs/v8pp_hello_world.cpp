#include "v8/libplatform/libplatform.h"
#include "v8/v8.h"
#include <v8pp/context.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  v8pp::context context;
  //context.set_lib_path("path/to/plugins/lib");
  // script can now use require() function. An application
  // that uses v8pp::context must link against v8pp library.
  v8::HandleScope scope(context.isolate());
  auto res = context.run_file("some_file.js");
  v8::String::Utf8Value str(context.isolate(), res);
  std::cout << std::string(*str) << std::endl;
}
