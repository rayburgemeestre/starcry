#pragma once

#include <memory>

class v8_wrapper_impl;
class v8_wrapper_context;
class v8_wrapper {
public:
    v8_wrapper();

    void cleanup();

    /**
     * create a separate context, if you want to clone an existing context wrapper object provide it as a parameter
     */
    std::shared_ptr<v8_wrapper_context> context(std::shared_ptr<v8_wrapper_context> clone_from = nullptr);

private:
    std::shared_ptr<v8_wrapper_impl> impl_; // TODO check: no longer necessary
};

class v8_wrapper_context_impl;

namespace v8pp {
    class context;
}
class v8_wrapper_context {
public:
    v8_wrapper_context();
    v8_wrapper_context(std::shared_ptr<v8_wrapper_context> & context);
    v8_wrapper_context(std::shared_ptr<v8pp::context> context);
    ~v8_wrapper_context();

    template <typename T = void>
    T run(const std::string & script);

    template <typename T>
    T add(std::function<T> func);

    std::shared_ptr<v8pp::context> context(); // returns subcontext

private:
    std::shared_ptr<v8pp::context> subcontext; // can also contain a clone
};
