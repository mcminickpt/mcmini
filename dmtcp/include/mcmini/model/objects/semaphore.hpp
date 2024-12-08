#pragma once

#include <string>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace model {
namespace objects {
struct semaphore : public model::visible_object_state {
 private:
  unsigned _count = 0;
  enum state { uninitialized, initialized, destroyed };

 private:
  state current_state = state::uninitialized;

 public:
  semaphore() = default;
  ~semaphore() = default;
  semaphore(const semaphore &) = default;
  explicit semaphore(unsigned count)
      : _count(count), current_state(initialized) {}
  void wait() { this->_count--; }
  void post() { this->_count++; }
  unsigned count() const { return this->_count; }
  bool will_block() const { return this->_count <= 0; }
  std::unique_ptr<visible_object_state> clone() const override {
    return extensions::make_unique<semaphore>(*this);
  }
  std::string to_string() const override {
    return "semaphore(count: " + std::to_string(_count) + ")";
  }
};
}  // namespace objects
}  // namespace model
