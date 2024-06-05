#pragma once

#include <exception>
#include <stdexcept>

namespace model {

/// @brief An error that is raised when a program encounters undefined behavior.
struct undefined_behavior_exception : public std::runtime_error {
  undefined_behavior_exception(const char *what) : std::runtime_error(what) {}
};
}  // namespace model
