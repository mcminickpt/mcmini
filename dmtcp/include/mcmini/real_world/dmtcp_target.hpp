#pragma once

#include <fstream>

#include "mcmini/real_world/target.hpp"

namespace real_world {

struct dmtcp_target : public target {
 protected:
  // Whether the environment should be propagated by `dmtcp_restart`
  bool propagate_env_through_dmtcp = true;

  // The path to the checkpoint file
  std::string ckpt_file_path;

  // A handle to the `dmtcp_env.txt` file containing each line of the evironment
  // which will be propogated to the
  std::fstream dmtcp_env_file;

 private:
  /// @brief A cache variable which is used to determine whether new environment
  /// variables have been set since the last time the file was written to
  ///
  /// To avoid writing to the environment file after each invocation, if the
  /// environment hasn't changed since the last invocation, we can simply skip
  /// the rewrite since the environment variables are as is.
  std::unordered_map<std::string, std::string> setenv_file_vars;
  std::unordered_set<std::string> unsetenv_file_vars;

 public:
  dmtcp_target() = default;
  explicit dmtcp_target(const std::string &target_program,
                        const std::vector<std::string> &target_program_args)
      : target(target_program, target_program_args) {}
  explicit dmtcp_target(const std::string &target_program,
                        const std::string &ckpt_file_path)
      : dmtcp_target(target_program, std::vector<std::string>{},
                     ckpt_file_path) {}
  explicit dmtcp_target(const std::string &target_program,
                        const std::vector<std::string> &target_program_args,
                        const std::string &ckpt_file_path)
      : target(target_program, target_program_args) {
    this->target_program_args.push_back(ckpt_file_path);
  }
  pid_t launch_dont_wait() override;
};

}  // namespace real_world
