#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::real_world {

/**
 * @brief A proxy for a process running on the CPU.
 *
 * A `mcmini::real_world::process` is a proxy for running code. It represents a
 * static snapshot of executable code at a given moment in time as it runs under
 * McMini's.
 *
 * A process can be conceptualized as a "multi-dimensional" forward iterator;
 * that is, a process can undergo changes only following the arrow of time
 * forward. Backwards progress is not possible for a
 * `mcmini::real_world::process`, which mirrors the forward execution of the
 * instruction streams.
 *
 * A process is a collection of different  _runners_. of execution. Each
 * separate thread of execution in the proxied process is uniquely represented
 * with an id. Each new thread created in by process during execution is
 * assigned a unique id (one for every thread creation call) and is tracked by
 * the process.
 */
struct process {
 public:
  using runner_id_t = uint32_t;

 public:
  /**
   * @brief Schedule the runner with id `id` for execution.
   *
   * This method signals the proxy process to resume execution of the runner
   * with id `mcmini_runner_id`. The method blocks until the runner reaches the
   * next semantically interesting point of execution according to that runner.
   *
   * @note The process may not actually contain a runner with id
   * `mcmini_runner_id`, or the runner with the id `mcmini_runner_id` may be
   * blocked or asleep, or the runner itself may be in a deadlock. In such
   * scenarios, the proxy process will not respond and the method will never
   * return. It is up to the caller to ensure that scheduling runner
   * `mcmini_runner_id` for execution will not block forever.
   */
  void execute_runner(runner_id_t mcmini_runner_id);
};

}  // namespace mcmini::real_world