#pragma once

#include <cstdint>

namespace model_checking {
struct stats {
  uint32_t trace_id = 1;
  uint32_t total_transitions = 0;
};
}  // namespace model_checking
