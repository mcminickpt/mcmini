#pragma once

#include <libgen.h>

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace real_world {

struct target {
 protected:
  // The name of the program which we should exec() into with libmcmini.so
  // preloaded.
  // NOTE: Favor std::filesystem::path if C++17 is eventually supported
  // Alternatively, have McMini conditionally compile a std::filesystem::path
  std::string target_program;

  /// @brief The list of arguments to pass to the target program.
  std::vector<std::string> target_program_args;

  /// @brief A list of environment variables to set prior to `fork()/exec()`.
  std::unordered_map<std::string, std::string> setenv_vars;

  /// @brief A list of environment variable names for which `unsetenv()` should
  /// be invoked prior to a `fork()/exec()`.
  std::unordered_set<std::string> unsetenv_vars;

 public:
  target() = default;
  explicit target(const std::string &target_program)
      : target(target_program, std::vector<std::string>()) {}

  explicit target(const std::string &target_program,
                  const std::vector<std::string> &target_program_args)
      : target_program(target_program),
        target_program_args(target_program_args) {}

  const std::string &name() const { return this->target_program; }

  // The semantics
  void dont_unset_env(std::string name) {
    unsetenv_vars.erase(std::move(name));
  }
  void dont_set_env(std::string name) { setenv_vars.erase(std::move(name)); }
  void unset_env(std::string name) {
    unsetenv_vars.insert(name);
    dont_set_env(std::move(name));
  }
  void set_env(std::string name, std::string value) {
    setenv_vars.insert({name, std::move(value)});
    dont_unset_env(std::move(name));
  }

  /// @brief A convenience method for setting the `LD_PRELOAD` environment
  /// variable for `libmcmini.so`.
  void set_preload_libmcmini() {
    // NOTE: According to the man page `dirname(const char *path)` "may modify
    // the contents of `path`...", so we use the storage of the local instead.
    // We don't want to use `std::string` either since it doesn't expect its
    // contents to be modified indirectly
    std::vector<char> target_program_mutable_name(this->name().begin(),
                                                  this->name().end());
    char buf[1000];
    buf[sizeof(buf) - 1] = '\0';
    snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
             (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
             dirname(target_program_mutable_name.data()));
    this->set_env("LD_PRELOAD", buf);
  }

  /// @brief A convenience method for setting the `MCMINI_TEMPLATE_LOOP`
  /// environment variable for the child process
  ///
  /// A child process with `MCMINI_TEMPLATE_LOOP` set indicates that the child
  /// will be used as a template process for McMini
  void set_is_template() { this->set_env("MCMINI_TEMPLATE_LOOP", "1"); }

  /// @brief Creates a new process running this program
  ///
  /// @return the process id of the newly created process.
  virtual pid_t launch_dont_wait();

  /// @brief Executes the target program as a separate process
  /// and waits for that process to finish execution.
  ///
  /// @throws a `process::execution_error` is raised if the
  /// process exits unexpectedly
  virtual void launch_and_wait();

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
