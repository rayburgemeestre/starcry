#include "v8_wrapper.h"

#include "libplatform/libplatform.h"
#include "v8.h"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

#include <iostream>

using namespace std;
using namespace v8;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class v8_wrapper_impl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "v8_wrapper_functions.hpp"

class v8_wrapper_impl {

public:
    v8_wrapper_impl() : context_(nullptr), platform(nullptr) {
        if (num_instances >= 1)
            throw std::logic_error("there can only be one v8_wrapper instance");

        v8::V8::InitializeICU();
        //v8::V8::InitializeExternalStartupData(argv[0]);
        std::unique_ptr<v8::Platform> p(v8::platform::CreateDefaultPlatform());
        std::swap(platform, p);
        v8::V8::InitializePlatform(platform.get());
        v8::V8::Initialize();
        //context_ = std::make_shared<v8pp::context>();
        context_ = new v8pp::context();
        num_instances++;
    }

    ~v8_wrapper_impl() {
        num_instances--;
        //context_.reset();
        delete(context_);
        platform.reset();

        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
    }

    std::shared_ptr<v8_wrapper_context> context(std::shared_ptr<v8_wrapper_context> clone_from = nullptr)
    {
        if (clone_from == nullptr)
            return make_shared<v8_wrapper_context>();
            //return make_shared<v8_wrapper_context>(); <<< this will not create a new subcontext
        else
            return make_shared<v8_wrapper_context>(clone_from);
    }

private:

    //std::shared_ptr<v8pp::context> context_;
    v8pp::context * context_;
    std::unique_ptr<v8::Platform> platform;
    static size_t num_instances;
};

size_t v8_wrapper_impl::num_instances = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class v8_wrapper
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

v8_wrapper::v8_wrapper() : impl_(std::make_shared<v8_wrapper_impl>()) {
}

std::shared_ptr<v8_wrapper_context> v8_wrapper::context(std::shared_ptr<v8_wrapper_context> clone_from)
{
    return impl_->context(clone_from);
}

void v8_wrapper::cleanup() {
    impl_.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class v8_wrapper_context
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Use to create a new context
 */
v8_wrapper_context::v8_wrapper_context()
    : subcontext(std::make_shared<v8pp::context>()) {
}
/**
* Use to create a clone from existing context
*/
v8_wrapper_context::v8_wrapper_context(std::shared_ptr<v8_wrapper_context> & context)
    : subcontext(std::make_shared<v8pp::context>(context->context()->isolate())) {
}

/**
 * Re-use existing v8pp_context!
 */
v8_wrapper_context::v8_wrapper_context(std::shared_ptr<v8pp::context> context)
    : subcontext(std::move(context)) {
}

v8_wrapper_context::~v8_wrapper_context() {
    if (subcontext)
        subcontext.reset();
}

std::shared_ptr<v8pp::context> v8_wrapper_context::context() {
    return subcontext;
}


template <typename T>
T v8_wrapper_context::run(const std::string & script) {
    try
    {
        v8::HandleScope scope(subcontext->isolate());
        auto result = subcontext->run_script(script);

        T i = v8pp::from_v8<T>(subcontext->isolate(), result);
        return i;
    }
    catch (std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        throw ex;
    }
}

template<> void v8_wrapper_context::run<void>(const std::string & script) {
    try
    {
        v8::HandleScope scope(subcontext->isolate());
        subcontext->run_script(script);
    }
    catch (std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
        throw ex;
    }
}

template int v8_wrapper_context::run(const std::string & script);
template std::string v8_wrapper_context::run(const std::string & script);
template void v8_wrapper_context::run(const std::string & script);
template float v8_wrapper_context::run(const std::string & script);
template double v8_wrapper_context::run(const std::string & script);
