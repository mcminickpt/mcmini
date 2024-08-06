#pragma once

#include <ostream>
#include <string>
#include <vector>

namespace real_world {

struct target {
 private:
  // The name of the program which we should exec() into with libmcmini.so
  // preloaded.
  // NOTE: Favor std::filesystem::path if C++17 is eventually supported
  // Alternatively, have McMini conditionally compile a std::filesystem::path
  std::string target_program;

  /// @brief The list of arguments to pass to the target program.
  std::vector<std::string> target_program_args;

 public:
  explicit target(const std::string &target_program)
      : target(target_program, std::vector<std::string>()) {}

  explicit target(const std::string &target_program,
                  const std::vector<std::string> &target_program_args)
      : target_program(target_program),
        target_program_args(target_program_args) {}

  const std::string &name() const { return this->target_program; }

  /// @brief Turn this process into the target via `execvp()`
  ///
  /// @note this function only returns on failure of `execvp(2)` and sets
  /// errno
  void execvp(bool with_ld_preload = true) const;

  friend std::ostream &operator<<(std::ostream &os, const target &target) {
    os << target.target_program;
    for (const std::string &arg : target.target_program_args) os << " " << arg;
    return os;
  }
};

}  // namespace real_world
