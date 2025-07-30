#pragma once

#include <algorithm>
#include <string>

#include "mcmini/log/severity_level.hpp"

namespace mcmini {
namespace log {
severity_level parse_severity(const std::string& levelStr) {
  std::string s(levelStr);
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  if (s == "nothing") return nothing;
  if (s == "very_verbose") return very_verbose;
  if (s == "verbose") return verbose;
  if (s == "debug") return debug;
  if (s == "info") return info;
  if (s == "unexpected") return unexpected;
  if (s == "error") return error;
  if (s == "critical") return critical;
  if (s == "abort") return abort;
  if (s == "everything") return everything;
  return nothing;
}
}  // namespace log
}  // namespace mcmini
