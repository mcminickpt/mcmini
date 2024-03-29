#pragma once

#include <istream>
#include <memory>

#include "mcmini/model/transition.hpp"
#include "mcmini/real_world/runner.hpp"

namespace real_world {

/**
 * @brief A proxy for a process running on the CPU.
 *
 * A `real_world::process` is a proxy for running code. It represents a
 * static snapshot of executable code at a given moment in time as it runs under
 * McMini's.
 *
 * A process can be conceptualized as a "multi-dimensional" forward iterator;
 * that is, a process can undergo changes by _executing_ operations which cannot
 * be undone. Backwards progress is not possible for a
 * `real_world::process`, which matches the forward execution of the
 * instruction streams.
 *
 * A process is a collection of different  _runners_ of execution. Each
 * separate _runner_ in the proxied process is represented with a unique id.
 * New runners may appear during the  during execution is assigned a unique
 * id (one for every thread creation call) and is tracked by the process.
 */
struct process {
 public:
  using runner_id_t = uint32_t;
  std::unordered_map<runner_id_t, std::unique_ptr<runner>> runners;

 public:
  virtual ~process() = default;

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
   *
   * @returns a stream containing the serialized response from the process
   * represented by this proxy. The stream must contain as its first element a
   * `model::transition_registry::rttid`. The McMini coordinator will
   * use this identifier to invoke the appropriate callback function to
   * transform the remaining contents of the stream into its model.
   */
  virtual std::istream &execute_runner(runner_id_t mcmini_runner_id) = 0;

  // TODO: We assume at the moment that the number of runners is fixed and that
  // every call to `execute_runner` above is valid. Eventually, to support more
  // complicated runners (e.g. entire processes, a runner representing multiple
  // threads, etc.) the idea of "adding" a new slot for a runner dynamically
  // might be needed. runner_id_t add_runner(std::unique_ptr<runner>
  // new_runner);
};

}  // namespace real_world