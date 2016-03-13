#pragma once

// for adding functions etc., we need to pull in v8pp (and v8) in it's entirely
// (or we need to forward declare all possible function signatures)

#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

template <typename T>
inline void add_fun(std::shared_ptr<v8_wrapper_context> context, const std::string &name, T func) {
    auto ctx = context->context();
    v8::HandleScope scope(ctx->isolate());
    ctx->set(name.c_str(), v8pp::wrap_function(ctx->isolate(), name.c_str(), func));
}
template <typename T>
inline void add_fun(std::shared_ptr<v8pp::context> ctx, const std::string &name, T func) {
    v8::HandleScope scope(ctx->isolate());
    ctx->set(name.c_str(), v8pp::wrap_function(ctx->isolate(), name.c_str(), func));
}
