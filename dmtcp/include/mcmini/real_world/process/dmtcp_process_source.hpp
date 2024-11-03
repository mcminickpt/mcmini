#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "mcmini/defines.h"
#include "mcmini/forwards.hpp"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/process/local_linux_process.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/real_world/target.hpp"

namespace real_world {

/**
 * @brief A factory which produces `real_world::local_linux_process` by forking
 * a process stored from a checkpoint image via `dmtcp_restart`
 *
 * A `dmtcp_process_source` is responsible for creating new processes by forking
 * and `exec()`-ing into a template process (`dmtcp_restart`) and then
 * repeatedly forking the new process to create new processes. The processes
 * that are vended are duplicates of the template process.
 */
class dmtcp_process_source : public process_source {
 private:
  std::string ckpt_file;
  bool has_transferred_recorded_objects = false;

private:
  pid_t make_new_branch();

public:
  dmtcp_process_source(const std::string &ckpt_file);
  virtual ~dmtcp_process_source();

 public:
  std::unique_ptr<process> make_new_process() override;
};

};  // namespace real_world
