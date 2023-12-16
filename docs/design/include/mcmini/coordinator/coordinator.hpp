#pragma once

#include "mcmini/model/program.hpp"
#include "mcmini/real_world/process_source.hpp"

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
 * Recall that a `mcmini::model::program` consists of a mapping between the
 * individual execution units of the program ("threads") and the next step that
 * thread will take once executed.
 *
 * A `mcmini::model::program`, among other things, must describe _how_ the model
 * in its current state can transition into a different state. But since _only_
 * the next immediate action (the "thread routine", e.g. `pthread_mutex_lock()`)
 * that will be taken by any thread `i` in that process is known _for
 * certain_ (all subsequent actions taken by any thread `i` are unknown), McMini
 * must coordinate changes in the process with changes in the model. After a
 * thread has executed its next immediate action in the process, its
 * the action that the thread will _later_ run (i.e. the subsequent action that
 * follows immediately after the given one) is discovered dynamically at
 * runtime. The diagram below illustrates the progre
 *
 *              run thread 0 ------------------------+
 *                                                   |
 *                                                  +
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
 *  discovered _after_`t_0i`------->|t_0(i+1)||????|   |????| ...
 *                                   +---+    +----+   +----+
 *                                     /         /        /
 *                                    +---+    +---+    +---+
 *                                   |????|   |????|   |????| ...
 *                                   +---+    +----+   +----+
 *
 * The coordinator is responsible for this dynamic discovery and the
 * aforementioned synchronization.
 */
class coordinator {
 private:
  model::program current_state;
  std::unique_ptr<real_world::process> current_process;
  std::unique_ptr<real_world::process_source> process_source;

 public:
  /**
   * @brief Constructs a new coordinator which synchronizes processes.
   *
   * @param initial_state the state from which the coordinator begins
   * coordination with . This is often referred to as `s_0` or the "initial
   * state" in the model-checking literature.
   * @param process_source a process source which can repeatedly produce
   * processes starting at state `initial_state`. The coordinator will
   * repeatedly create new processes from this source part of its exploration.
   * @invariant: A _critical_ invariant is that _process_source_ create new
   * processed that are modeled by _initial_state_. McMini model-checking
   * algorithms rely _solely_ on the model to make determinations about how to
   * explore the state space of processes constructed by _process_source_. The
   * coordinator only ensures that the process and the state remain synchornized
   * _after_ executions take place. If the initially modeled state does not
   * correspond to the processes produced by `process_source`, the behavior is
   * undefined (most likely this would lead to deadlocks etc.).
   */
  coordinator(model::program &&initial_state,
              std::unique_ptr<real_world::process_source> &&process_source);
  ~coordinator() = default;

  /**
   *
   */
  void go_to_nth_step(uint32_t n);
};

};  // namespace mcmini