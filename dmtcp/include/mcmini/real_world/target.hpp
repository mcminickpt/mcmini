#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
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

  /// @brief A list of environment variables to set prior to fork()/exec()
  std::unordered_map<std::string, std::string> environment_vars;

 public:
  explicit target(const std::string &target_program)
      : target(target_program, std::vector<std::string>()) {}

  explicit target(const std::string &target_program,
                  const std::vector<std::string> &target_program_args)
      : target_program(target_program),
        target_program_args(target_program_args) {}

  const std::string &name() const { return this->target_program; }

  void set_env(const char *name, const char *value) {
    environment_vars.insert({std::string(name), std::string(value)});
  }

  /// @brief Creates a new process running this program
  ///
  /// @return the process id of the newly created process.
  pid_t launch_dont_wait();

  /// @brief Executes the target program as a separate process
  /// and waits for that process to finish execution.
  ///
  /// @throws a `process::execution_exception` is raised if the
  /// process exits unexpectedly
  void launch_and_wait();

  /// @brief Turn this process into the target via `execvp()`
  ///
  /// @note this function only returns on failure of `execvp(2)` and sets
  /// errno
  void execvp() const;

  friend std::ostream &operator<<(std::ostream &os, const target &target) {
    os << target.target_program;
    for (const std::string &arg : target.target_program_args) os << " " << arg;
    return os;
  }
};

}  // namespace real_world
