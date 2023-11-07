#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::real_world {

/**
 * @brief A proxy for a multi-threaded process running on the CPU.
 *
 * A `mcmini::real_world::process` is a local placeholder that provides an
 * interface to
 *
 * A process can be conceptualized as a "multi-dimensional" forward iterator;
 * that is, a process can under changes only moving forward, following the arrow
 * of time forward. Backwards progress is not possible for a
 * `mcmini::real_world::process`, which mirrors the forward iteration of the
 * instruction streams.
 *
 */
struct process {
 public:
  process();

  /**
   *
   */
  void execute_thread();
};

}  // namespace mcmini::real_world