#pragma once

#include <exception>
#include <stdexcept>

namespace model {

/// @brief An error that is raised when a program encounters undefined behavior.
struct undefined_behavior : public std::exception {};
}  // namespace model