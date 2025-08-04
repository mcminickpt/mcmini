#pragma once

#include <cstdint>

namespace logging {
enum severity_level : uint32_t {
  nothing,
  very_verbose,
  verbose,
  debug,
  info,
  unexpected,
  error,
  critical,
  abort,
  everything
};
}  // namespace logging
