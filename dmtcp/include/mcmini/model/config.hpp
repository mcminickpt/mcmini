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
   * libmcmini.so configured in record mode.
   */
  bool record_target_executable_only = false;

  /**
   * The time between consecutive checkpoint images when `libmcmini.so` is
   * running in record mode.
   */
  std::chrono::seconds checkpoint_period;

  std::string target_executable = "";
  std::vector<std::string> target_executable_args;
};
}  // namespace model
