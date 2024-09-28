#pragma once

#include <stdint.h>

#include <memory>

namespace model {
class visible_object_state {
 public:
  virtual ~visible_object_state() = default;
  virtual std::unique_ptr<visible_object_state> clone() const = 0;
  virtual std::string to_string() const = 0;
};

}  // namespace model
