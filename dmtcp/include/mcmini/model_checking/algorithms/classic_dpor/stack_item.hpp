#pragma once
#include <unordered_set>
#include <utility>
#include <vector>

#include "mcmini/defines.h"
#include "mcmini/model_checking/algorithms/classic_dpor/clock_vector.hpp"

namespace model_checking {

/**
 * @brief Associates information relevant to DPOR about a state it has searched
 *
 * The `stack_item` is a light-weight representation of a state explored by
 * McMini. McMini is a stateless model checker, meaning that is does not
 * "remember" or cache the previous states of the visible objects it manages.
 * However, DPOR requires that some information be persisted during the
 * state-space search in order to function correctly.
 *
 * Each `stack_item` persists the state's backtracking set, done set, and sleep
 * set. Each state explored by McMini has exactly one representative
 * `stack_item`. McMini will adjust the state's trio of sets as it continues to
 * explore the state space and execute more transitions
 */
struct stack_item final {
 private:
  /**
   * @brief The clock vector associated with the
   * transition _resulting_ in this state
   */
  const clock_vector cv;

  /// @brief The transition which DPOR decided to schedule from this state.
  ///
  /// @note the item does not own this transition; it is instead owned by the
  /// trace of the model which the DPOR algorithm manipulates. It is up to the
  const model::transition *out_transition = nullptr;

  /**
   * @brief A collection of threads that are scheduled to be run from this state
   * to continune to state-space search
   *
   * @invariant a thread is contained in exactly one of either the backtracking
   * set, the done set, or the sleep set
   */
  std::unordered_set<runner_id_t> backtrack_set;

  /**
   * @brief A collection of threads that have executed from this state so far
   *
   * @invariant a thread is contained in exactly one of either the backtracking
   * set, the done set, or the sleep set
   */
  std::unordered_set<runner_id_t> done_set;

  /**
   * @brief A collection of threads that do
   * not need to be executed from this state
   *
   * The technical definition of a sleep set is
   * a collection of transitions. However, we can
   * exploit a clever observation
   *
   * @invariant a thread is contained in
   * exactly one of either the backtracking set,
   * the done set, or the sleep set
   *
   * @invariant for any given thread, at most
   * one transition can be contained in the sleep
   * set. If such a transition exists, it is the
   * _next_ transition in this state that will be
   * run by that thread
   */
  std::unordered_set<runner_id_t> sleep_set;

  /**
   * @brief A cache of threads that are enabled in this state
   *
   * DPOR needs to know which threads are enabled in an
   * arbitrary state in the current DFS search. It is important
   * that the state(s) of the visible object(s) McMini
   * manages reflect what they were at a particular moment in
   * time during the state-space search since a transition's
   * status for being enabled can change between state changes
   * (consider e.g. a mutex lock becoming disabled after a different
   * thread acquires the mutex before this thread does).
   *
   * Since DPOR performs stateless model checking, McMini can only
   * represent a *single* state at any given time. Thus, we can either
   *
   * A. Re-generate the past states of objects by reseting the current
   * state and re-playing the transitions in the transition stack
   * leading up to that state; or
   *
   * B. Compute which threads were enabled when the state was
   * encountered and store them for later use
   *
   * The former is very expensive and would complicate the implementation even
   * further. Thus, we opt for the latter choice.
   */
  std::unordered_set<runner_id_t> enabled_runners;

 public:
  stack_item() : stack_item(clock_vector()) {}
  stack_item(clock_vector cv) : cv(std::move(cv)) {}
  stack_item(clock_vector cv, std::unordered_set<runner_id_t> enabled_runners)
      : stack_item(std::move(cv), nullptr, std::move(enabled_runners)) {}
  stack_item(clock_vector cv, const model::transition *out_transition,
             std::unordered_set<runner_id_t> enabled_runners)
      : cv(std::move(cv)),
        out_transition(out_transition),
        enabled_runners(std::move(enabled_runners)) {}

  bool backtrack_set_empty() const { return backtrack_set.empty(); }
  bool backtrack_set_contains(runner_id_t id) const {
    return backtrack_set.count(id) != 0;
  }
  bool sleep_set_contains(runner_id_t rid) const {
    return sleep_set.count(rid) != 0;
  }
  bool done_set_contains(runner_id_t rid) const {
    return done_set.count(rid) != 0;
  }
  bool has_enabled_runners() const { return !this->enabled_runners.empty(); }

  /**
   * @brief Places the given thread into the backtrack set.
   *
   * If the thread is already contained in the done set, the thread will
   * not be re-added to the backtracking set and this method acts as a no-op.
   * Otherwise the given thread will be added to the backtracking set associated
   * with this state and will be scheduled to execute from this state.
   *
   * @param rid the thread to add to the backtracking set
   *
   * @note McMini will ensure that the given thread is running a transition
   * that's enabled. This, however, is not enforced by this method; rather,
   * threads added to the backtrack set using this method are simply cached for
   * later use by McMini and DPOR
   */
  void insert_into_backtrack_set_unless_completed(runner_id_t rid) {
    if (!done_set_contains(rid)) backtrack_set.insert(rid);
  }
  void insert_into_sleep_set(runner_id_t rid) { sleep_set.insert(rid); }

  /**
   * @brief Moves a thread into the done set
   *
   * When a thread is moved into the done set, the thread will not be scheduled
   * to execute from this state in future depth-first state-space searches that
   * include this state.
   *
   * @param rid the thread to move to the done set
   */
  void mark_searched(runner_id_t rid) {
    this->done_set.insert(rid);
    this->backtrack_set.erase(rid);
  }

  /**
   * @brief Caches the threads in the given set
   * for later use by DPOR when determining enabled
   * threads in this state
   *
   * @param runners a collection of runners that are enabled in this state
   *
   * @note McMini will ensure that threads marked enabled in this state are
   * indeed enabled. This, however, is not enforced by this method; rather,
   * threads marked as enabled are simply cached for later use by McMini and
   * DPOR
   */
  void mark_enabled_in_state(const std::unordered_set<runner_id_t> &runners) {
    for (const runner_id_t rid : runners) this->enabled_runners.insert(rid);
  }

  runner_id_t backtrack_set_pop() {
    if (backtrack_set_empty())
      throw std::runtime_error("There are no more threads to backtrack on");
    runner_id_t backtrack_thread = *this->backtrack_set.begin();
    this->mark_searched(backtrack_thread);
    return backtrack_thread;
  }

  runner_id_t backtrack_set_pop_first() {
    if (backtrack_set_empty())
      throw std::runtime_error("There are no more threads to backtrack on");

    // NOTE: We arbitrarily always pick the smallest thread to provide
    // determinism
    runner_id_t backtrack_thread = *this->backtrack_set.begin();
    for (const runner_id_t rid : this->backtrack_set)
      backtrack_thread = std::min(rid, backtrack_thread);

    this->mark_searched(backtrack_thread);
    return backtrack_thread;
  }

  clock_vector get_clock_vector() const { return this->cv; }
  runner_id_t get_first_enabled_runner() const {
    runner_id_t r = RUNNER_ID_MAX;
    for (runner_id_t p : this->enabled_runners) {
      r = std::min(r, p);
    }
    return r;
  }
  const std::unordered_set<runner_id_t> &get_enabled_runners() const {
    return this->enabled_runners;
  }
  const std::unordered_set<runner_id_t> &get_sleep_set() const {
    return this->sleep_set;
  }
  const std::unordered_set<runner_id_t> &get_backtrack_set() const {
    return this->backtrack_set;
  }
  const model::transition *get_out_transition() const {
    return this->out_transition;
  }
  void set_out_transition(const model::transition *out) {
    this->out_transition = out;
  }
};

}  // namespace model_checking
