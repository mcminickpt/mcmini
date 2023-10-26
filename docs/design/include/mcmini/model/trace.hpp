#pragma once

#include <stdint.h>

namespace mcmini::model {

/**
 *
 *
 */
class trace final {
 public:
  trace() = default;

  trace subtrace(uint32_t subtrace_index) const;

  // Subtrace
  // Pop off the top
  // Push onto
  // is empty?
  // how many are there ?
  // get the element at a particular index
};

}  // namespace mcmini::model