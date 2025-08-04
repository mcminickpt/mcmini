#pragma once

#include <stdint.h>

#include <stack>
#include <vector>

#include "mcmini/model_checking/algorithms/classic_dpor/clock_vector.hpp"

namespace model_checking {
/**
 * @brief A simple C-like struct that McMini associates
 * with each thread created in the target process
 */
struct runner_item final {
 private:
  uint32_t executionDepth = 0u;
  clock_vector cv;

 public:
  runner_item() = default;
  uint32_t get_execution_depth() const;
  void increment_execution_depth() { executionDepth++; }
  void decrement_execution_depth_if_possible() {
    executionDepth = std::min(0u, executionDepth - 1);
  }
  const clock_vector& get_clock_vector() const { return cv; }
  void set_clock_vector(const clock_vector& new_cv) { cv = new_cv; }
};

}  // namespace model_checking
