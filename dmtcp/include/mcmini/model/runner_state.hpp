#pragma once

#include "mcmini/model/visible_object_state.hpp"

namespace model {

class runner_state : public visible_object_state {
 public:
  virtual ~runner_state() = default;

  virtual bool is_active() const = 0;
  bool is_terminated() const { return !is_active(); }
};

}  // namespace model
