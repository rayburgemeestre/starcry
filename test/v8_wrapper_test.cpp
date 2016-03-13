#include "gtest/gtest.h"

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
TEST(v8_wrapper_test, test_cpp_function_in_thread) {
    auto ctx = wrapper.context();
    ctx->run("function foo() { return 1111; }");
    ASSERT_EQ(1111, ctx->run<int>("foo()"));
    auto l = [&ctx]() {
        //while (true) {
        //    v8::Locker locker(ctx->context()->isolate());
            std::cout << " result = " << ctx->run<int>("foo()") << std::endl;
        //}
    };
    std::thread t(l);
    t.join();
}
