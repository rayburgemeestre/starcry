#include "gtest/gtest.h"

#if 1 == 1
#include "v8_wrapper.h"


v8_wrapper wrapper;

TEST(v8_wrapper_test, test_run_script) {
    auto ctx = wrapper.context();
    ctx->run("var i = 1001;");
    ASSERT_EQ(1001, ctx->run<int>("i"));
    ASSERT_EQ(500.5, ctx->run<float>("i = i / 2; i"));
    ASSERT_EQ(250.25, ctx->run<double>("i / 2"));
    ASSERT_EQ(42, ctx->run<int>("11 * 4 - 2"));
    ASSERT_EQ("Hello world", ctx->run<std::string>("'Hello ' + 'world'"));
}

TEST(v8_wrapper_test, test_multiple_contexts) {
    auto ctx = wrapper.context();
    auto ctx2 = wrapper.context();
    ctx->run("var i = 1001;");
    ctx2->run("var i = 1002;");
    ASSERT_EQ(1001, ctx->run<int>("i"));
    ASSERT_EQ(1002, ctx2->run<int>("i"));
    ctx->run("i++");
    ASSERT_EQ(1002, ctx->run<int>("i"));
    ASSERT_EQ(1002, ctx2->run<int>("i"));
}

TEST(v8_wrapper_test, test_cloned_contexts) {
    auto ctx = wrapper.context();
    auto ctx2 = wrapper.context(ctx);
    ctx->run("var i = 1001;");
    ctx2->run("var i = 1002;");
    ASSERT_EQ(1002, ctx->run<int>("i"));
    ASSERT_EQ(1002, ctx2->run<int>("i"));
    ctx2->run("i++");
    ASSERT_EQ(1003, ctx->run<int>("i"));
    ASSERT_EQ(1003, ctx2->run<int>("i"));
}


int test() {
    return 1111;
}
int test2() {
    return 2222;
}

#include "v8_wrapper_functions.hpp"

TEST(v8_wrapper_test, test_cpp_function_in_v8) {
    auto ctx = wrapper.context();
    auto ctx2 = wrapper.context();

    add_fun(ctx, "foo", &test);
    add_fun(ctx2, "foo", &test2);

    ASSERT_EQ(1111, ctx->run<int>("foo()"));
    ASSERT_EQ(1111, ctx->run<int>("foo()"));
    ASSERT_EQ(2222, ctx2->run<int>("foo()"));

    add_fun(ctx2, "foo", &test);
    ASSERT_EQ(1111, ctx2->run<int>("foo()"));
}

/*
#include <thread>

TEST(v8_wrapper_test, test_cpp_function_in_thread) {
    auto ctx = wrapper.context();

    add_fun(ctx, "foo", &test);

    ASSERT_EQ(1111, ctx->run<int>("foo()"));
    auto l = [&ctx]() {
        while (true) {
            //v8::Locker locker(ctx->context()->isolate());
            std::cout << " result = " << ctx->run<int>("foo()") << std::endl;
        }
    };

    std::thread t(l);
    t.join();
}
 */

TEST(v8_wrapper_test, test_context_remember) {
    auto ctx = wrapper.context();
    v8::HandleScope scope(ctx->context()->isolate());
    add_fun(ctx, "test", &test);
    ctx->run("function foo() { return 1111; }");
    ASSERT_EQ(1111, ctx->run<int>("foo()"));
    ASSERT_EQ(1111, ctx->run<int>("foo()"));
}


// there is a problem with this block, debug mode will give info
// uncommenting the locker will trigger an assertion in its destructor
#include <thread>
auto ctx = wrapper.context();
TEST(v8_wrapper_test, test_cpp_function_in_thread) {

    v8::HandleScope scope(ctx->context()->isolate());
    auto global = v8::ObjectTemplate::New(ctx->context()->isolate());
    v8::Handle<v8::Context> impl = v8::Context::New(ctx->context()->isolate(), nullptr, global);

    ctx->run("function foo() { return 1111; }");
    //ASSERT_EQ(1111, ctx->run<int>("foo()"));
    auto l = []() {
        for (int i=0; i< 1024; i++) {
            v8::Locker locker(ctx->context()->isolate());
            v8::HandleScope scope(ctx->context()->isolate());
            std::cout << " SDF " << ctx->run<int>("foo()") << std::endl;
            /*{
                v8::Local<v8::String> source = v8::String::NewFromUtf8(ctx->context()->isolate(), "foo()", v8::NewStringType::kNormal).ToLocalChecked();
                v8::Local<v8::Script> script = v8::Script::Compile(source);
                v8::Local<v8::Value> result;
                if (!script.IsEmpty()) {
                    result = script->Run();
                    std::cout << " i = " << result->Int32Value() << std::endl;
                }
            }*/
        }
    };
    std::thread t(l);
    t.join();
}

#include "v8.h"
struct array_buffer_allocator : v8::ArrayBuffer::Allocator
{
    void* Allocate(size_t length)
    {
        return calloc(length, 1);
    }
    void* AllocateUninitialized(size_t length)
    {
        return malloc(length);
    }
    void Free(void* data, size_t length)
    {
        free(data);
    }
};
static array_buffer_allocator array_buffer_allocator_;

class x {

public:
    x() {
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = &array_buffer_allocator_;
        isolate = v8::Isolate::New(create_params);
        isolate->Enter();
        v8::HandleScope scope(isolate);
        global = v8::ObjectTemplate::New(isolate);
        impl = v8::Context::New(isolate, nullptr, global);
        impl->Enter();
        impl_.Reset(isolate, impl);
    }
    ~x() {
        //v8::Local<v8::Context> impl = v8pp::to_local(isolate, impl_);
        //impl->Exit();
        impl_.Reset();
        isolate->Exit();
        isolate->Dispose();
    }
    v8::Isolate* isolate;
    v8::Handle<v8::ObjectTemplate> global;
    v8::Handle<v8::Context> impl;
    v8::Persistent<v8::Context> impl_;
};
TEST(v8_wrapper_test, test_cpp_function_in_thread_2) {
    x xx;

    v8::HandleScope scope(xx.isolate);
    auto global = v8::ObjectTemplate::New(xx.isolate);
    v8::Handle<v8::Context> impl = v8::Context::New(xx.isolate, nullptr, global);

    //impl->Enter();
    //xx.impl_.Reset(xx.isolate, impl);
    auto &isolate = xx.isolate;
    {
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "var i = 1111;", v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(source);
        v8::Local<v8::Value> result;
        if (!script.IsEmpty()) {
            result = script->Run();
        }
    }
    auto l = [&]() {
        for (int i=0; i<1024; i++) {
            v8::Locker locker(isolate);
            v8::HandleScope scope(isolate);
            {
                v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "i", v8::NewStringType::kNormal).ToLocalChecked();
                v8::Local<v8::Script> script = v8::Script::Compile(source);
                v8::Local<v8::Value> result;
                if (!script.IsEmpty()) {
                    result = script->Run();
                    std::cout << " i = " << result->Int32Value() << std::endl;
                }
            }
        }
    };
    //impl->Exit();
    std::thread t(l);
    t.join();
}
#else

#include "libplatform/libplatform.h"
#include "v8.h"
#include <thread>

struct array_buffer_allocator : v8::ArrayBuffer::Allocator
{
    void* Allocate(size_t length)
    {
        return calloc(length, 1);
    }
    void* AllocateUninitialized(size_t length)
    {
        return malloc(length);
    }
    void Free(void* data, size_t length)
    {
        free(data);
    }
};
static array_buffer_allocator array_buffer_allocator_;
#include "v8pp/context.hpp"

TEST(v8_wrapper_test, test_cpp_function_in_thread_3) {
    v8::V8::InitializeICU();
    auto platform = v8::platform::CreateDefaultPlatform();
    v8::V8::InitializePlatform(platform);
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = &array_buffer_allocator_;
    auto isolate = v8::Isolate::New(create_params);
    isolate->Enter();
    v8::HandleScope scope(isolate);
    auto global = v8::ObjectTemplate::New(isolate);
    v8::Handle<v8::Context> impl = v8::Context::New(isolate, nullptr, global);
    impl->Enter();
    v8::Persistent<v8::Context> impl_;
    impl_.Reset(isolate, impl);
    {
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "var i = 1111;", v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(source);
        v8::Local<v8::Value> result;
        if (!script.IsEmpty()) {
        result = script->Run();
        }
    }
//    auto l = [&]() {
//        for (int i=0; i<1024; i++) {
//            v8::Locker locker(isolate);
//            v8::HandleScope scope(isolate);
//            {
//                v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "i", v8::NewStringType::kNormal).ToLocalChecked();
//                v8::Local<v8::Script> script = v8::Script::Compile(source);
//                v8::Local<v8::Value> result;
//                if (!script.IsEmpty()) {
//                    result = script->Run();
//                    std::cout << " i = " << result->Int32Value() << std::endl;
//                }
//            }
//        }
//    };
//    //impl->Exit();
//    std::thread t(l);
//    t.join();

    {
//        v8::Local<v8::Context> impl = v8pp::to_local(isolate, impl_);
//        impl->Exit();
//        impl_.Reset();
//        isolate->Exit();
//        isolate->Dispose();
    }
}
#endif
