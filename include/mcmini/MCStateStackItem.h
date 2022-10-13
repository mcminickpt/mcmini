#ifndef MC_MCSTATESTACKITEM_H
#define MC_MCSTATESTACKITEM_H

#include "mcmini/MCClockVector.hpp"
#include "mcmini/MCShared.h"
#include "mcmini/MCTransition.h"
#include <unordered_set>
#include <utility>
#include <vector>

/**
 * @brief Associates information relevant to
 * DPOR about a state it has searched
 *
 * The `MCStateStackItem` is a light-weight
 * representation of a state explored by McMini.
 * McMini is a stateless model checker, meaning
 * that is does not "remember" or cache the previous
 * states of the visible objects it manages. However,
 * DPOR requires that some information be persisted
 * during the state-space search in order to function
 * correctly.
 *
 * Each `MCStateStackItem` persists the state's
 * backtracking set, done set, and sleep set.
 * Each state explored by McMini has exactly one
 * representative `MCStateStackItem`. McMini will
 * adjust the state's trio of sets as it continues
 * to explore the state space and execute
 * more transitions
 */
struct MCStateStackItem final {
private:

  /**
   * @brief A collection of threads that
   * are scheduled to be run from this state
   * to continune to state-space search
   *
   * @invariant a thread is contained in
   * exactly one of either the backtracking set,
   * the done set, or the sleep set
   */
  std::unordered_set<tid_t> backtrackSet;

  /**
   * @brief A collection of threads that
   * have executed from this state so far
   *
   * @invariant a thread is contained in
   * exactly one of either the backtracking set,
   * the done set, or the sleep set
   */
  std::unordered_set<tid_t> doneSet;

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
  std::unordered_set<tid_t> sleepSet;

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
   * B. Compute which threads were enabled when the state was
   * encountered and store them for later use
   *
   * The former is very expensive and would complicate McMini's
   * implementation even further. Thus, we opt for the latter choice.
   */
  std::unordered_set<tid_t> enabledThreads;

  /**
   * @brief The clock vector associated with the
   * transition resulting in this state
   */
  const MCClockVector clockVector;

  /**
   * @brief Whether or not the transition
   * that creates this state can be reverted
   * from this state
   */
  const bool spawningTransitionCanRevertState;

public:

  MCStateStackItem()
    : MCStateStackItem(MCClockVector::newEmptyClockVector(), false)
  {}

  MCStateStackItem(const MCClockVector &cv,
                   const bool reversibleInState)
    : clockVector(cv),
      spawningTransitionCanRevertState(reversibleInState)
  {}

  /**
   * @brief Puts the given thread into the
   * backtrack set for the state represented
   * by this item
   *
   * If the thread is already contained in
   * the state's done set, the thread will
   * not be re-added to the backtracking set
   * and this method acts as a no-op. Otherwise
   * the given thread will be added to the
   * backtracking set associated with this state
   * and will be scheduled to execute from this state.
   *
   * @param tid the thread to add to the
   * backtracking set
   *
   * @note McMini will ensure that the given
   * thread is running a transition that's
   * enabled. This, however, is not enforced
   * by this method; rather, threads added
   * to the backtrack set using this method
   * are simply cached for later use by McMini
   * and DPOR
   */
  void addBacktrackingThreadIfUnsearched(tid_t tid);

  /**
   * @brief Moves a thread into the state's
   * done set
   *
   * When a thread is moved into the state's
   * done set, the thread will not be scheduled
   * to execute from this state in future
   * depth-first state-space searches that include
   * this state
   *
   * @param tid the thread to move to the done set
   */
  void markBacktrackThreadSearched(tid_t tid);

  /**
   * @brief Caches the threads in the given set
   * for later use by DPOR when determining enabled
   * threads in this state
   *
   * @param threads a collection of thread ids
   * that are enabled in this state
   *
   * @note McMini will ensure that threads marked
   * enabled in this state are indeed enabled. This,
   * however, is not enforced by this method; rather,
   * threads marked as enabled are simply cached
   * for later use by McMini and DPOR
   */
  void
  markThreadsEnabledInState(const std::unordered_set<tid_t> &threads);

  MCClockVector getClockVector() const;
  std::unordered_set<tid_t> getEnabledThreadsInState() const;
  std::unordered_set<tid_t> getSleepSet() const;

  /**
   * @brief Inserts the given thread into the sleep
   * set of this state
   */
  void addThreadToSleepSet(tid_t);

  /**
   * @brief Whether or not there are any
   * threads in the backtracking set in this
   * state
   *
   * @return true if there are threads to
   * backtrack on in this state, false otherwise
   */
  bool hasThreadsToBacktrackOn() const;

  /**
   * @brief Whether or not the given thread
   * is contained in the backtracking set of this
   * state
   *
   * @return true if the thread is
   * contained in the backtracking set
   * for this state
   */
  bool isBacktrackingOnThread(tid_t) const;

  /**
   * @brief Whether or not the given thread
   * is contained in this state's sleep set
   *
   * @return true if the thread is contained in
   * the sleep set, and false otherwise
   */
  bool threadIsInSleepSet(tid_t tid) const;

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  bool isRevertible() const;

  /**
   * @brief Removes and returns a thread
   * from the backtracking set
   *
   * @return a thread to backtrack on. The
   * thread is automatically moved to the
   * done set
   */
  tid_t popThreadToBacktrackOn();
};

#endif // MC_MCSTATESTACKITEM_H
