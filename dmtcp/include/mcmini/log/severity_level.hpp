#pragma once

#include <cstdint>

namespace mcmini {
namespace log {
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
}  // namespace log
}  // namespace mcmini
