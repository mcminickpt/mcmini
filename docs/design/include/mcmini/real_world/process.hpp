#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::real_world {

/**
 * @brief A proxy for a multi-threaded process running on the CPU.
 *
 * A `mcmini::real_world::process` is a proxy for running code. It represents a
 * static snapshot of a process.
 *
 * A process can be conceptualized as a "multi-dimensional" forward iterator;
 * that is, a process can undergo changes only following the arrow of time
 * forward. Backwards progress is not possible for a
 * `mcmini::real_world::process`, which mirrors the forward iteration of the
 * instruction streams.
 *
 * Processes are split into different threads of execution. Each separate thread
 * of execution in the proxied process is uniquely represented with an id. Each
 * new thread is assigned a unique id (one for every thread creation call)
 *
 * @note The `process` could be generalized to an "object which has independent
 * sequences of execution." The current implementation of McMini only supports
 * model-checking a single process, but this could easily be extended in the
 * future with the same type of abstraction.
 */
struct process {
 public:
  using tid_t = mcmini::model::thread::tid_t;

 public:
  process();

  /**
   * @brief Schedule the thread with id `tid` for execution.
   *
   * This method signals the proxy process to resume execution of the thread of
   * execution with id `tid` and waits for the thread to reach its next visible
   * operation.
   *
   * @note The process may not actually contain thread `tid`, or the
   * thread with the id `tid` may be blocked or asleep, or the process itself
   * may be in a deadlock. In such scenarios, the proxy process will not
   * respond and the method will never return. It is up to the caller to ensure
   * that scheduling thread `tid` for execution will not block.
   */
  void execute_thread(tid_t tid);
};

}  // namespace mcmini::real_world