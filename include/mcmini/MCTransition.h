#ifndef MC_MCTRANSITION_H
#define MC_MCTRANSITION_H

#include "mcmini/MCShared.h"
#include "mcmini/MCState.h"
#include "mcmini/objects/MCThread.h"
#include <memory>
#include <utility>

struct MCState;

/**
 * A base class representing a fundamental unit in DPOR: the
 * transition
 *
 * Strictly speaking, a *transition* "moves a concurrent system from
 * one state to a subsequent one by performing one visible
 * operation of a chosen process, followed by a _finite_ sequence
 * of invisible operations of the same process" (Flanagan,
 * Godefroid 2005). Intuitively, a transition is a description of how
 * to modify the current state in order to reach new states. When a
 * transition is executed by a thread, any visible objects affected
 * by the transition will be updated to reflect the fact that the
 * transition was executed. For example, when a `lockMutex()`
 * transition is executed, the lock the transition operates on
 * would change from being unlocked to locked.
 *
 * You don't use `MCTransition` directly; instead, you subclass
 * `MCTransition` and specify any object you need to
 * describe the transition. McMini handles providing your
 * transition subclass the correct references to any live objects,
 * but you must ensure you query for the correct objects in order
 * to copy state. At a minimum, each `MCTransition` subclass has a
 * reference to the thread that executes the transition.
 *
 * Transitions are used to move McMini from state to state in order
 * to perform searches through a concurrent system's state space.
 * McMini uses *static* copies when it needs
 * to save the states of a transition and its references for use at
 * a later date. On the other hand, McMini creates *dynamic copies*
 * of transitions when it needs the references in the transition to
 * point to "live" objects, viz. those objects whose states McMini
 * tracks.
 *
 * You write your transitions with any references you need and
 * update state assuming those references point to the correct
 * objects. McMini will create the appropriate static and dynamic
 * copies of transitions it encounters when appropriate.
 *
 * ** Important **
 *
 * McMini relies heavily on the correctness of semantics of the
 * transitions you create. For example, it relies on your
 * implementation to specify when a thread will be blocked attempting
 * to run the transition and uses this information to determine
 * when threads should be scheduled to execute their
 * transitions. If you specify that a transition *won't* block but
 * actually does, McMini could be left in a deadlock.
 */
struct MCTransition {
public:

  virtual ~MCTransition() = default;

  MCTransition(std::shared_ptr<MCThread> thread) : thread(thread) {}
  MCTransition(const MCTransition &)            = default;
  MCTransition &operator=(const MCTransition &) = default;

  static bool dependentTransitions(const MCTransition &,
                                   const MCTransition &);
  static bool dependentTransitions(const MCTransition *,
                                   const MCTransition *);

  static bool coenabledTransitions(const MCTransition &,
                                   const MCTransition &);
  static bool coenabledTransitions(const MCTransition *,
                                   const MCTransition *);

  static bool transitionsInDataRace(const MCTransition &,
                                    const MCTransition &);
  static bool transitionsInDataRace(const MCTransition *,
                                    const MCTransition *);

  static bool transitionEnabledInState(const MCState *,
                                       const MCTransition &);
  static bool transitionEnabledInState(const MCState *,
                                       const MCTransition *);

  /**
   * Creates a deep copy of this transition
   *
   * A "static" copy is a snapshot in time of a transition whose
   * references resolve to previous states of live objects.
   *
   * McMini uses static copies when it needs to keep track of
   * more than a single state of the objects it managed, e.g.
   * when filling the transition stack with new transitions.
   * Static copies of transitions store small amounts of
   * previously visited states which is necessary to accurately
   * describe the sequence of events leading up to a deadlock or
   * assertion failure.
   *
   * You must ensure you copy all of the objects references
   * needed by the transition.
   *
   * @return a transition that is equivalent to this one, with
   * each object reference pointing
   */
  virtual std::shared_ptr<MCTransition> staticCopy() const = 0;

  /**
   * Produces a deep copy of this transition which points to
   * live objects in the given state
   *
   * A "dynamic" copy is a transition whose references
   * refer to objects McMini uses as its source of truth for the
   * states of those objects.
   *
   * McMini creates dynamic copies when it needs to apply a
   * transition to the objects it keeps track of. You must ensure
   * that you access the correct references by querying the state
   * instance with `MCState::getObjectWithId()`, using the ids
   * associated with each object reference owned by this transition.
   * For example, the `MyTransition` looks up the "live" object
   * corresponding to the id of the reference that the transition
   * owns (which could be a copy of a dynamic object from a
   * previous state e.g.):
   *
   * ```
   * struct MyTransition: public MCTransition {
   * private:
   *   MyObjectRef myObj;
   * public: ...
   *
   *   std::shared_ptr<MCTransition>
   *   dynamicCopyInState(const MCState* state)
   *   {
   *      MyObjectRef dynamicMyObj =
   *          state->getObjectWithId<MyObjectRef>(myObj->getObjectId());
   *      ...
   *   }
   * };
   *
   * ```
   * @param state the state to retrieve dynamic references from
   * for the objects necessary
   * @return a transition that is equivalent to this one, with
   * each object reference pointing at "live" objects
   */
  virtual std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *state) const = 0;

  /**
   * Determines whether the thread running this transition would
   * be able to execute the transition without blocking
   *
   * You must override this method to specify the conditions
   * under which this transition is enabled. Factors that could
   * affect whether the transition is enabled include the state
   * of the thread running the transition; the states of any
   * objects the transition affects; and the state of other
   * threads, among other factors. When implementing this method
   * for your transition subclasses, you can assume that any
   * object references have been resolved to point to live
   * objects and not to copies of those objects. See
   * `MCTransition::dynamicCopyInState()` and
   * `MCTransition::staticCopy()` for more details.
   *
   * You do not need to check thread execution depths
   * when determining whether a transition is enabled: McMini
   * transparently prevents scheduling threads from executing
   * which have run past their execution depth limits (see
   * `MCState.cpp` and
   * `MCTransition::countsAgainstThreadExecutionDepth()` for more
   * details). Furthermore, McMini also checks whether the thread
   * executing this transition is enabled in the first place. Thus
   * you can assume that the thread executing the transition
   * is alive
   *
   *  ** Important **
   *
   * It is critical that the conditions under which the
   * transition are enabled match the underlying semantics of the
   * transition and any objects that the transition interacts
   * with. McMini assumes that any transition that returns `true`
   * from this method can be executed by the owning thread in the
   * underlying test program without blocking. If conditions
   * don't match the semantics of the transition, McMini might
   * deadlock waiting for a thread that will never wake up.
   *
   * By default, the method returns `true`.
   *
   * @param state a description of the state to run from
   * @return whether the thread can execute past the given
   * transition if it were tested in the state provided as an
   * argument to this method
   */
  virtual bool
  enabledInState(const MCState *state) const
  {
    return true;
  }

  /**
   * Performs the actions represented by this transition
   *
   * This method should modify the appropriate objects the
   * transition interacts with so that those objects are in a
   * state reflecting the fact that this transition has been
   * executed.
   *
   * When implementing this method for your transition
   * subclasses, you can assume that any object references have
   * been resolved to point to live objects and not to copies of
   * those objects. See `MCTransition::dynamicCopyInState()` and
   * `MCTransition::staticCopy()` for more details.
   *
   * ** Important **
   *
   * It is critical that the transition update state correctly to
   * match the semantics of the operation it represents.
   * Failing to correctly apply the transition to match the
   * semantics of the operation will again result in McMini
   * misrepresenting states, which could lead to deadlock. See
   * the discussion about `MCTransition::enabledInState()` for
   * more details on how McMini relies on the state.
   *
   * @param state the state representation to modify. Any object
   * references held onto by this instance refer to live objects
   * in this state
   */
  virtual void applyToState(MCState *state) = 0;

  /**
   * @brief Whether or not the transition can be reverted
   * in the given state
   *
   * It's possible that a transition's effects can be
   * reverted. In such cases, override this method
   * and return `true` and implement the
   * `MCTransition::unapplyToState(MCState*)` method.
   *
   * McMini will use this information
   * to dynamically determine if it can simply undo
   * the effects of a sequence of reversible transitions
   * instead of resetting all of its local state for
   * backtracking. This can improve performance as
   * McMini would not be forced to restore shadow object
   * states.
   *
   * @param state the state in which to determine reversibility
   * @return true if the transition can be reversed in the given
   * state, and false otherwise
   */
  virtual bool
  isReversibleInState(const MCState *state) const
  {
    return false;
  }

  /**
   * Reverts the actions represented by this transition
   *
   * This method should modify the appropriate objects the
   * transition interacts with so that those objects are in a
   * state reflecting the fact that this transition has been
   * _reverted_.
   *
   * As with `MCState::applyToState(MCState*)`,
   * you can assume that any object references have
   * been resolved to point to live objects and not to copies of
   * those objects. See `MCTransition::dynamicCopyInState()` and
   * `MCTransition::staticCopy()` for more details.
   *
   * ** Important **
   *
   * It is critical that the transition undo state correctly to
   * match the (reverted) semantics of the operation it represents.
   * In particular, the effect of unapplying the transition must be
   * the EXACT opposite of applying the transition.
   * Failing to correctly unapply the transition to match the
   * semantics of the operation will result in McMini
   * misrepresenting states, which could lead to deadlock. See
   * the discussion about `MCTransition::enabledInState()` for
   * more details on how McMini relies on the state.
   *
   * @param state the state representation to modify. Any object
   * references held onto by this instance refer to live objects
   * in this state
   *
   * @throws std::runtime_error if you attempt to unapply the
   * transition when it's unsupported
   */
  virtual void
  unapplyToState(MCState *state)
  {
    if (!isReversibleInState(state))
      throw std::runtime_error(
        "Attempted to revert a transition that cannot be reverted");
  }

  /**
   * Determines whether this transition is co-enabled with the
   * one given
   *
   * Two transitions `t1` and `t2` are *co-enabled* if there is
   * *some* state in which both transitions are both
   * simultaneously enabled.
   *
   * A transition need only specify which transitions it's
   * co-enabled with. McMini will check when two transitions are
   * co-enabled by asking each whether it is co-enabled with the
   * other. If both transitions answer affirmatively, the two
   * transitions are considered to be co-enabled.
   *
   * Thus, you can write any co-enabled conditions assuming only
   * the existence of any existing transition subclasses: it is
   * up to future subclasses to properly take into
   * consideration any extra conditions under which it fails to
   * be co-enabled with transitions of this type.
   *
   * Furthermore, McMini already handles the "standard" set of
   * conditions under which two transitions are coenabled. Thus,
   * you can write and conditions for determining if this
   * transitions is co-enabled with the given one assuming that
   *
   * 1. the other transition is run by a different thread
   * 2. the other transition does not create or join on the thread
   * running this transition
   *
   * By default, the method returns `true`, indicating that the
   * McMini should consider that there is always a state in which
   * this transition is enabled with any other transition
   *
   * @param other the transition to test against
   * @return whether this transition is co-enabled, in the formal
   * sense, with the given one
   */
  virtual bool
  coenabledWith(const MCTransition *other) const
  {
    return true;
  }

  /**
   * Determines whether this transition has a dependency with the
   * one given
   *
   * Two transitions `t1` and `t2` are *independent* with one
   * another when, intuitively, the order in which the two
   * transitions execute doesn't matter: executing one of
   * the two transition has no effect on whether the other
   * transition remains enabled/disabled, and you should be able
   * to execute both transitions in either order and end up in
   * the same state. Formally, transitions `t1` and `t2` are
   * *independent* if, for all states, the following two
   * statements are true:
   *
   * 1. if t1 is enabled in s and s (t1) → s', then t2 is enabled in
   * s iff t2 is enabled in s'; and
   *
   * 2. if t1 and t2 are enabled in s, then there is a unique
   * state s' such that s (t1t2) → s' and s (t2t1) → s'
   *
   * Two transitions are *dependent* with one another when they
   * are not independent with each other
   *
   * A transition subclass specifies whether is it dependent with
   * another transition based on its own, current state and that
   * of the given transition. You check the type of the transition
   * provided and decide the dependency between it and this
   * transition. You need only check the identity of the
   * transition for those transitions that you know can be
   * dependent with this one; otherwise, simply return `false`
   *
   * A transition need only specify its own dependencies. McMini
   * will check dependency between any two transitions by
   * asking each whether it is dependent with the other. If
   * either transition specifies that it's dependent with the
   * other, the transitions are considered to be dependent with
   * one another *even if* only a single transition said it was
   * dependent with the other. Thus, again, you can write any
   * dependency conditions assuming only the existence of any
   * existing transition subclasses: it is up to future
   * subclasses to properly take into consideration any
   * dependencies with transitions of this type.
   *
   * Furthermore, McMini already handles the "standard" set of
   * conditions under which two transitions are dependent. Thus,
   * you can write and conditions for determining if this
   * transitions is dependent with the given one assuming that
   *
   * 1. the other transition is run by a different thread
   * 2. the other transition does not create or join on the thread
   * running this transition
   *
   * By default, the method returns `true`, indicating that the
   * McMini should consider it in potential conflict with all
   * other transitions.
   *
   * @param other the transition to test dependency against
   * @return whether this transition is dependent, in the formal
   * sense, with the given one
   */
  virtual bool
  dependentWith(const MCTransition *other) const
  {
    return true;
  }

  /**
   * Whether this transition in a data race with
   * the one given
   *
   * Two transitions are in a data race if there is a conflict
   * with both being enabled at the same time.
   *
   * @param other the transition to test against for a data race
   * @returns whether this transition is in a data race with the
   * transition specified
   */
  virtual bool
  isRacingWith(const MCTransition *other) const
  {
    return false;
  }

  /**
   * Whether this transition guarantees that the program
   * is free from deadlock
   *
   * When a transition is disabled, it indicates that a thread
   * can no longer make any progress. In most cases this should
   * indicate that the transition cannot be run without the
   * thread ending up being blocked. However, in the the case of a
   * transition that has a process-wide implication of
   * terminating the target process (such as a call
   * to exit()), allowing McMini to run such a transition
   * complicates the task of performing its model checking since
   * McMini would have to re-advance the target program to the
   * position to the point before the transition was run each
   * time it was hit. This is also expensive: allowing such a
   * transition to proceed means more fork()s and process
   * simulations
   *
   * Thus, such a transition is marked as disabled *even
   * though* it's actually enabled. To prevent McMini from
   * marking a state in which one (or more) threads running this
   * transition as its next visible operation as a deadlock,
   * override this method and return `true`. This tells McMini
   * that such a transition is not allowed to run for the reasons
   * described above.
   *
   * @returns whether a thread that has this transition as the
   * next transition it runs guarantees that the program
   * will not be in a deadlock
   *
   */
  virtual bool
  ensuresDeadlockIsImpossible() const
  {
    return false;
  }

  /**
   * Determines whether this transition should be considered
   * when determining the number of transitions a thread has run
   *
   * The *thread execution depth* of a thread determines how
   * many visible operations that thread has run from the
   * perspective of the *source code*. Threads can be limited to
   * run for a maxmimum number of steps before becoming
   * permanently disabled by McMini; the per-thread execution
   * depth is what is compared against the max depth McMini allows
   *
   * Typically, each wrapper function that the thread encounters
   * while executing will count as a single transition; however,
   * some transitions, such as `sem_wait()`, are broken up into more
   * than one visible operation. Since we strive for each visible
   * operation function call to count as a *single* transition
   * from the perspective of the user, as it is more intuitive to
   * simply count the number of system calls a thread has made to
   * determine how far a thread has gone, you override this method
   * to specify whether this transition counts against the
   * execution depth of the thread.
   *
   * @return whether this transition should be considered
   * when determining the number of transitions a thread has run
   */
  virtual bool
  countsAgainstThreadExecutionDepth() const
  {
    return true;
  }

  /**
   * Returns the id of thread that executes this transition
   *
   * @return the id for the thread that executes
   * this transition
   */
  inline tid_t
  getThreadId() const
  {
    return this->thread->tid;
  }

  /**
   * @brief Whether or not the thread
   * running this transition can currently
   * execute
   *
   * A thread may not be able to execute
   * a transition if it is e.g. asleep
   * or an embryo thread. In the case that
   * the thread is an embryo, it should not
   * be able to run any transitions
   *
   * @return true is the thread is in the
   * alive state, and false otherwise
   */
  inline bool
  threadIsEnabled() const
  {
    return this->thread->enabled();
  }

  // FIXME: De-couple printing from the interface
  virtual void
  print() const
  {}

protected:

  /**
   * @brief
   *
   */
  std::shared_ptr<MCThread> thread;

private:

  static bool transitionsCoenabledCommon(const MCTransition *t1,
                                         const MCTransition *t2);
  static bool transitionsDependentCommon(const MCTransition *t1,
                                         const MCTransition *t2);
};

#endif // MC_MCTRANSITION_H
