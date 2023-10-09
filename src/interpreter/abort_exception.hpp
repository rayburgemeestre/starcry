#include <stdexcept>
#include <string>
#include "fmt/core.h"

class abort_exception : public std::exception {
private:
  std::string what_;

public:
  explicit abort_exception(std::string msg) : exception(), what_(std::move(msg)) {}
  const char* what() const noexcept override {
    static auto msg = fmt::format("[abort exception] {}", what_);
    return msg.c_str();
  }
};