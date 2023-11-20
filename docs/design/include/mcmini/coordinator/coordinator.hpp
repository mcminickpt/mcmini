#pragma once

#include "mcmini/model/program.hpp"
#include "mcmini/real_world/process.hpp"

namespace mcmini {

/**
 * @brief A mechanism which synchronizes a McMini model of a program
 * (`mcmini::model::program`) with a process executing that modeled program
 * (`mcmini::real_world::process`).
 *
 * Model-checking algorithms in McMini operate strictly on program models
 * (`mcmini::model::program`) and not directly with the raw memory contents of
 * a running process. McMini must work to continuously synchronize the _model_
 * it maintains of a program undergoing verification with the progression of the
 * _actual process_ whose state (i.e. memory) _is represented by_ that model.
 * Below is a schema of the synchronization the tracecoordinator performs.
 *
 *  +-------+                             +----------------------+
 * | model |     kept synchronized       |     corresponding    |
 * |  of   |  <------------------------> |      process         |
 * |  the  |                             +----------------------+
 * | world |                            |        |        |
 * +-------+                           +---+    +---+    +---+
 *                                    |t_0i|   |t_1j|   |t_2k| ...
 *                                    +---+    +----+   +----+
 *                                      /        /        /
 *                                    +---+    +---+    +---+
 *                                   |????|   |????|   |????| ...
 *                                   +---+    +----+   +----+
 *
 * However, since McMini is an explicit-state model checker, a model can only be
 * constructed through the _execution_ of actual processes running in user space
 * (e.g. `fork()`-ed from a shell).
 *
 * Recall that a `mcmini::model::program` consists of the following information:
 *
 * - a mapping between the individual execution units of the program ("threads")
 * and the next step that thread will take once executed.
 *
 * A `mcmini::model::program`, among other things, must describe _how_ the model
 * in its current state can transition into a different state. But since _only_
 * the next immediate action (the "thread routine", e.g. `pthread_mutex_lock()`)
 * that will be taken by any thread `i` in that process is known _for
 * certain_ (all subsequent actions taken by any thread `i` are unknown), McMini
 * must coordinate changes in the process with changes in the model. After a
 * thread has executed its next immediate action in the process, its
 * _subsequent_ next immediate action is discovered dynamically at runtime.
 *
 *              run thread 0 ------------------------+
 *                                                  |
 *                                                 +
 *  +-------+                             +----------------------+
 * | model |     kept synchronized       |     corresponding    |
 * |  of   |  <------------------------> |      process         |
 * |  the  |                             +----------------------+
 * | world |                            |        |        |
 * +-------+     update the model      +---+    +---+    +---+
 *       +---------------------------|*t_0i*|  |t_1j|   |t_2k| ...
 *                                    +---+    +----+   +----+
 *                                      /\        /        /
 *                                    +---+    +---+    +---+
 *  discovered _after_`t_0j`------->|t_0(i+1)||????|   |????| ...
 *                                   +---+    +----+   +----+
 *                                     /         /        /
 *                                    +---+    +---+    +---+
 *                                   |????|   |????|   |????| ...
 *                                   +---+    +----+   +----+
 * The coordinator is responsible for this dynamic discovery and the
 * aforementioned synchronization.
 */
class coordinator {
 private:
 public:
  /**
   * @brief Constructs a new coordinator which traces processes
   *
   * @param initial_state the initial model from which this coordinator should
   * keep synchronization.
   * @param corresponding_process the process
   */
  coordinator(model::program &&initial_state,
              real_world::process &&corresponding_process);
  ~coordinator() = default;

  void go_to_nth_step(uint32_t n);
};

};  // namespace mcmini