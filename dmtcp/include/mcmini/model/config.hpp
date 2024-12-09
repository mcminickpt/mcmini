#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "mcmini/defines.h"

namespace model {

struct config {
  /**
   * The maximum number of transitions that can be run
   * by any single thread while running the model checker
   */
  uint64_t max_thread_execution_depth;

  /**
   * The trace id to stop the model checker at
   * to print the contents of the transition stack
   */
  trid_t target_trace_id;

  /**
   * Whether model checking should halt at the first encountered deadlock
   */
  bool stop_at_first_deadlock = false;

  /**
   * Informs McMini that the target executable should be run under DMTCP with
   * `libmcmini.so` configured in record mode.
   */
  bool record_target_executable_only = false;

  /**
   * Informs McMini that model checking with the checkpoint file should occur
   * using `multithreaded_fork()` + a template process instead of
   * `dmtcp_restart` to create new branches
   */
  bool use_multithreaded_fork = false;

  /**
   * The time between consecutive checkpoint images when `libmcmini.so` is
   * running in record mode.
   */
  std::chrono::seconds checkpoint_period;

  /**
   * The path to the checkpoint file that should be used to begin deep debugging
   * from.
   */
  std::string checkpoint_file = "";

  std::string target_executable = "";
  std::vector<std::string> target_executable_args;
};
}  // namespace model
