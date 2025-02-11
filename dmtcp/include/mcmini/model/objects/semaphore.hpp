#pragma once

#include <string>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace model {
namespace objects {
struct semaphore : public model::visible_object_state {
 public:
  enum state { uninitialized, initialized, destroyed };

 private:
  unsigned _count = 0;
  state current_state = state::uninitialized;

 public:
  semaphore() = default;
  ~semaphore() = default;
  semaphore(const semaphore &) = default;
  explicit semaphore(state s) : semaphore(s, 0) {}
  explicit semaphore(unsigned count) : semaphore(initialized, count) {}
  explicit semaphore(state s, unsigned count)
      : _count(count), current_state(s) {}
  void wait() { this->_count--; }
  void post() { this->_count++; }
  void destroy() { this->current_state = state::destroyed; }
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
