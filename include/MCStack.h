#ifndef INCLUDE_MCMINI_MCSTATE_HPP
#define INCLUDE_MCMINI_MCSTATE_HPP

struct MCTransition;
struct MCSharedTransition;
struct MCStack;
struct MCStackItem;
typedef MCTransition *(*MCSharedMemoryHandler)(
  const MCSharedTransition *, void *, MCStack *);

#include "mcmini/MCClockVector.hpp"
#include "mcmini/MCObjectStore.h"
#include "mcmini/MCShared.h"
#include "mcmini/MCSharedTransition.h"
#include "mcmini/MCStackConfiguration.h"
#include "mcmini/MCStackItem.h"
#include "mcmini/MCThreadData.hpp"
#include "mcmini/misc/MCSortedStack.hpp"
#include "mcmini/misc/MCTypes.hpp"
#include "mcmini/objects/MCThread.h"

#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>


/**
 * @brief Set 'lastEnedOfTraceId = traceId'.
 * getNextTraceSeqEntry(...) will now always return '-1'.
 */
void setEndOfTraceSeq();

/**
 * @brief When '-p<traceSeq>' is invoked, then the traceSeq needs to be
 * reset when a trace is completed.
 */
void resetTraceSeqArray();

/**
 * @brief Return number of terms in traceSeq[] until '-1'.
 * This is used to decide if 'depth' is beyod the traceSeq in mcini_private.cpp.
 */
int traceSeqLength();

/* This is returned by getDeepestDPORBranchPoint() if this is the
 * first (origina) branch.  mc_do_model_checking() uses this to detect
 * when we have backtracked to before the beginning of theoriginal branch
 * (i.e., when mc_do_model_checking() is finished.
 */
#define FIRST_BRANCH (-1)

/**
 * @brief A reflection of the state of the program under which McMini
 * is model checking
 *
 * The MCStack class represents
 *
 * FIXME: This class has grown too large. We need to split it
 * up into more manageable chunks
 */
class MCStack {
public:
  std::shared_ptr<MCVisibleObject> getObjectWithId(objid_t);

private:

  /**
   * @field objectStorage: Holds all objects known to McMini that
   * exist (or have existed) in the test program
   *
   * @field configuration: A struct holding configuration parameters
   * for the current execution
   */
  MCObjectStore objectStorage;

  const MCStackConfiguration configuration;

  /**
   * @brief Tracks, for each thread known to
   * McMini to have existed at some point during
   * the execution of the program, what each thread is
   * ABOUT to execute as its next transition
   */
  tid_t nextThreadId = 0;
  std::shared_ptr<MCTransition>
    nextTransitions[MAX_TOTAL_THREADS_IN_PROGRAM];

  /**
   * @brief
   *
   */
  MCThreadData threadData[MAX_TOTAL_THREADS_IN_PROGRAM];

  /**
   * A pointer to the top-most element in the transition stack
   */
  int transitionStackTop = -1;
  std::shared_ptr<MCTransition>
    transitionStack[MAX_TOTAL_TRANSITIONS_IN_PROGRAM];

  /**
   * A pointer to the top-most element in the state stack
   */
  int stateStackTop = -1;
  std::shared_ptr<MCStackItem>
    stateStack[MAX_TOTAL_STATES_IN_STATE_STACK];

  /**
   * @brief Associates a handler function that McMini
   * invokes when threads in the program hit wrapper
   * functions for each transition type supported by McMini
   *
   * You register handlers with each transition subclass
   * using the method `MCStack::registerVisibleOperationType()`
   * to tell McMini how data written by each wrapper function
   * should be processed to create the corresponding objects
   * mcMini knows how to handle
   */
  std::unordered_map<TypeInfoRef, MCSharedMemoryHandler, TypeHasher,
                     TypesEqual>
    sharedMemoryHandlerTypeMap;

  /**
   * @brief Maps thread ids to their respective object ids
   */
  std::unordered_map<tid_t, objid_t> threadIdMap;

  /**
   * @brief A stack of indices into the state stack, which identifies
   * states at which the transition can be reverted for state
   * regeneration
   *
   * @invariant For elements e_i and e_j at indices
   * i and j, e_i < e_j <--> i < j
   */
  MCSortedStack irreversibleStatesStack;

private:

  /**
   * @brief A decorator around MCTransition::enabledInState()
   * that adds additional transparent checks to determine
   * whether a transition is enabled
   *
   * McMini may aritifically restrict enabled transitions
   * from running in certain circumstances. For example,
   * if the thread has run past the number of transitions
   * allocated to it (i.e. if it has run past the maxmimum
   * execution depth allowed for any given thread). Although
   * the next transition for that thread may be enabled,
   * McMini will artificially consider that transition to
   * be disabled to prevent the thread from running any further.
   *
   * @return whether or not if the given transition can be chosen for
   * execution
   */
  bool transitionIsEnabled(const MCTransition &) const;

  /**
   * @brief Determines, given two indices in the transition stack,
   * whether or not there is a "happens-before" relation (according to
   * the DPOR definition) between those two points, given the current
   * transition stack
   *
   * @param i the index into the transition stack in the left-hand
   * side of the relation
   * @param j the index into the transition stack in the right-hand
   * side of the relation
   * @return true if there is a happens-before relation between the
   * transitions at indices `i` and `j` in the transition stack
   * @return false if i > j or if there is not such a happens-before
   * relation
   */
  bool happensBefore(int i, int j) const;

  /**
   * @brief Determines, given an index in the transition stack and a
   * thread id whether or not there is a "happens-before" relation
   * (according to the DPOR definition) between that thread and the
   * point in the transition stack given the current transition stack
   *
   * @param i the index into the transition stack in the left-hand
   * side of the relation
   * @param tid the thread id to check in the happens-before relation
   * @return whether or not there is a happens-before relation between
   * the transitions at indices `i` and `j` in the transition stack
   */
  bool happensBeforeThread(int i, tid_t tid) const;

  /**
   * @brief Determines whether there is a race condition between
   * threads `q` and `p` such that DPOR dictates that that order
   * in which those threads executed should be checked in reverse
   *
   * This method is a helper function corresponding to the fifth line
   * of the DPOR pseudocode and determines membership in set `E` as
   * described in the algorithm
   *
   * @param depth an index into the t
   * @param q the first thread in the check
   * @param p the second thread in the check
   * @return whether or not thread `q` is in set `E`
   */
  bool threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const;

  /**
   * @brief Pushes a new (default) item onto the state stack with an
   * empty clock vector
   */
  void growStateStack();

  /**
   * @brief Directly pushes a new item onto the state stack
   *
   * This simply pushes a new item onto the state stack directly. You
   * usually shouldn't call this method directly, as its role is that
   * of a helper; rather, you should call
   * `growStateStackRunningTransition()` in order that the proper
   * clock vector, sleep set, and done set updates are propagated
   * properly
   *
   * @param cv The clock vector injected into the state. See the
   * pseudocode of section 5 in the original DPOR paper, where clock
   * vectors are pushed onto the transition stack
   *
   * TODO: We should push clock vectors onto the transition stack
   * rather than into the states those transitions result in. This
   * would make the algorithm closer to that of the original
   * pseudocode
   *
   * @param revertible whether or not the transition that can be
   * thought of as executing from the _previous state_ and resulting
   * in the new state has side effects that can be reverted. See
   * `MCTransition::isReversibleInState()` for more details
   */
  void growStateStackWith(const MCClockVector &cv, bool revertible);

  /**
   * @brief Pushes a new item onto the state stack by executing the
   * given transition
   *
   * A new state object (MCStackItem) is also pushed onto
   * the stack. More importantly, this method performs updates to the
   * *current* state at the top of the state stack (representing
   * what the current trace looks like _right now_). In particular,
   * the thread which executed is added to the sleep set of the
   * previous state and is marked as having executed from that state.
   * Furthermore, the clock vector for the new state (and for the
   * thread which executed) is computed and assigned appropriately
   *
   * @param t a reference to the transition that should be executed
   * against the current state. The transition itself is not directly
   * pushed onto the transition stack; instead, a static copy of the
   * transition is pushed onto the transition stack recording the
   * current state of the object references that the transition is
   * currently holding onto. For more details on static and dynamic
   * object references, see the documentation of
   * MCTransition::staticCopy().
   */
  void growStateStackRunningTransition(const MCTransition &t);

  /**
   * @brief Pushes a new transition onto the transition stack
   *
   * @param t a reference to the transition to push onto the
   * transition stack. The transition itself is not directly pushed
   * onto the stack; instead, a static copy of the transition is
   * pushed onto the state stack, recording the current state of the
   * references to objects that the transition is currently holding
   * onto. For more details on static and references to dynamic
   * objects, see the documentation of MCTransition::staticCopy().
   */
  void growTransitionStackRunning(const MCTransition &t);

  /**
   * @brief Performs the actual execution of the given transition on
   * the "live" objects of the current state
   *
   * When a transition is applied, the state is updated (according to
   * the particular transition subclass and implementation of the
   * `MCTransition::applyToState()`) to reflect the fact that the
   * transition was executed by the given thread. Intuitively, you
   * can imagine that the thread executing the transition finished
   * calling its wrapper function
   */
  void virtuallyApplyTransition(const MCTransition &);

  /**
   * @brief Executes a transition using the "live" objects of the
   * current state and additionally performs other state updates for
   * the thread executing the transition
   *
   * When a transition is executed in the target program, McMini first
   * applies the effect of the transition on the state
   * (MCStack::virtuallyApplyTransition()) and then updates the
   * per-thread data of the thread which executed the transition
   */
  void virtuallyRunTransition(const MCTransition &);

  /**
   * @brief Fully re-execute a transition, updating any per-thread
   * data structures as if the transition were executed anew
   */
  void virtuallyRerunTransitionAtIndex(int);

  /**
   * @brief Performs the actual un-execution of the given transition
   * on the "live" objects of the current state
   *
   * When a transition is unapplied, the state is reversed the last
   * visited state (according to the particular transition subclass
   * and implementation of the `MCTransition::unapplyToState()`) to
   * reflect the fact that the transition was reversed by the given
   * thread. Intuitively, you can imagine that the thread "undo-ing"
   * the transition it just executed to reach its current state
   */
  void virtuallyUnapplyTransition(const MCTransition &);

  /**
   * @brief Reverses a transition in the transition stack using the
   * "live" objects of the current state and additionally performs
   * other state updates for the thread executing the transition
   *
   * When a transition is reversed to regenerate past object states
   * for backtracking, McMini first unapplies the effect of the
   * transition on the state (MCStack::virtuallyUnapplyTransition())
   * and then updates the per-thread data of the thread which executed
   * the transition
   */
  void virtuallyRevertTransitionAtIndex(int);

  /**
   * @brief Computes the maximum clock vector from all clock vectors
   * in the transition stack whose transition is dependent with the
   * given one
   *
   * The maxmimum clock vector is needed for backtracking the involves
   * clock vectors. The "maxmium" clock vector of the transition stack
   * is the component-wise maxmium of all clock vectors associated
   * with the transitions in the stack that are dependent with the See
   * the pseudocode using clock vectors in section 4 of the original
   * DPOR paper for more details
   *
   * @param t the transition to compute the maxmium clock vector
   * against
   * @return MCClockVector
   */
  MCClockVector transitionStackMaxClockVector(const MCTransition &t);

  /**
   * @brief Fetches the clock vector associated with the `i`th
   * transition in the transition stack, if such a clock vector exists
   *
   * @param i the index in the transition stack to which the returned
   * clock vector correpsonds
   */
  MCClockVector clockVectorForTransitionAtIndex(int i) const;

  /**
   * Inserts a backtrack point given a context of insertion (where
   * in the transition/state stacks to insert into etc.)
   *
   * The parameters to this helper method mirror the notation that
   * appears in Flanagan and Godefroid's DPOR paper
   *
   * @param S_i the `i`th transition in the transition stack
   * @param preSi the state from which `S_i` executes from
   * @param nextSP the next transition of a particular thread `p` when
   * performing the logic checks in DPOR
   */
  bool dynamicallyUpdateBacktrackSetsHelper(
    const MCTransition &S_i, MCStackItem &preSi,
    const MCTransition &nextSP, int i, tid_t p);

  void incrementThreadDepthIfNecessary(const MCTransition &);
  void decrementThreadDepthIfNecessary(const MCTransition &);

  /**
   * @brief Computes the number of execution steps _which count
   * against the execution depth of the thread_ of all threads in the
   * program
   *
   * @return uint32_t the total number of "atomic" transitions all
   * threads have executed, roughly corresponding to the number of
   * distinct function calls were made to wrapper functions
   */
  uint32_t totalThreadExecutionDepth() const;

  /**
   * @brief Whether or not the state can be regenerated by running
   * backwards in time as opposed to forwards in time
   *
   * Transitions which have been marked as reversible can be
   * re-executed "in reverse"; i.e., their side effects on the object
   * references maintained by the state can be applied in reverse to
   * undo the effects of applying the operation. This is useful when
   * previous states need to be restored to continue searching a new
   * branch in the state space.
   *
   * @return whether or not the state at the given index can be
   * regenerated by running the transitions executed since that state
   * was encountered in reverse (i.e., undo-ing their effects). If
   * this method returns `false`, then at least one of those
   * transitions has been labeled as irreversible
   */
  bool canRunInReverseToStateAtIndex(uint32_t) const;

  MCThreadData &getThreadDataForThread(tid_t tid);
  const MCThreadData &getThreadDataForThread(tid_t tid) const;

public:

  MCStack(MCStackConfiguration config) : configuration(config) {}

  // MARK: Transition stack

  MCTransition &getTransitionAtIndex(int) const;
  MCTransition &getTransitionStackTop() const;
  uint64_t getTransitionStackSize() const;
  bool transitionStackIsEmpty() const;
  tid_t getThreadRunningTransitionAtIndex(int) const;

  /**
   * @brief Maps the transition stack to the threads which executed at
   * each corresponding point in the stack
   *
   * Each transition in the transition stack was executed by a unique
   * thread which is numbered and tracked by McMini. This method
   * computes a list which determines which threads executed and the
   * order they were executed in.
   *
   * @return a list of threads encoding the sequence of threads which
   * executed according to the transition stack
   */
  std::vector<tid_t> getThreadIdBacktrace() const;

  // MARK: State stack

  MCStackItem &getStateItemAtIndex(int) const;
  MCStackItem &getStateStackTop() const;
  uint64_t getStateStackSize() const;
  bool stateStackIsEmpty() const;

  /**
   * @brief Retrieves the index of the state closest to the top of the
   * state stack that contains at least one backtrack point
   *
   * @return the index into the state stack containing an MCStackItem
   * with at least one thread to backtrack on, or FIRST_BRANCH if no such
   * state exists
   */
  int getDeepestDPORBranchPoint();

  /**
   * @brief Retrieves the state (represented by the item in the state
   * stack) from which the transition at the given index executes
   *
   * Every transition moves a concurrent system from one state to
   * another. This method retrieves the so-called *departing* state,
   * viz. the state *from which* the transition executes.
   *
   * @return A reference to the object in the state stack representing
   * the state (and any metadata McMini is keeping track of for that
   * state) that the transition executed from
   */
  MCStackItem &getDepartingStateForTransitionAtIndex(int) const;

  /**
   * @brief Retrieves the state (represented by the item in the state
   * stack) into which the transition at the given index moves the
   * concurrent system
   *
   * Every transition moves a concurrent system from one state to
   * another. This method retrieves the so-called *resultant* state,
   * viz. the state *to which* the transition moves the concurrent
   * system. More simply, it is the state of the system AFTER the
   * transition executes.
   *
   * @return A reference to the object in the state stack representing
   * the state (and any metadata McMini is keeping track of for that
   * state) to which the transition brought the concurrent system
   */
  MCStackItem &getResultantStateForTransitionAtIndex(int) const;

  // MARK: Next "List" (what each thread runs next)

  MCTransition &getNextTransitionForThread(tid_t thread) const;
  const MCTransition *getFirstEnabledTransition();

  void setNextTransitionForThread(MCThread *,
                                  std::shared_ptr<MCTransition>);
  void setNextTransitionForThread(tid_t,
                                  std::shared_ptr<MCTransition>);
  void setNextTransitionForThread(tid_t, MCSharedTransition *,
                                  void *);

  // FIXME: The logic testing a configuration probably should be
  // adjusted
  MCStackConfiguration getConfiguration() const;

  uint64_t getNumProgramThreads() const;
  std::unordered_set<tid_t> getCurrentlyEnabledThreads();

  // MARK: Object Creation
  // FIXME: This should not be a part of the state. Object creation
  // should happen externally in a future design
  objid_t createNewThread();
  objid_t createNewThread(MCThreadShadow &);
  objid_t createMainThread();
  objid_t addNewThread(MCThreadShadow &);
  std::shared_ptr<MCThread> getThreadWithId(tid_t id) const;

  objid_t
  registerNewObject(const std::shared_ptr<MCVisibleObject> &object);

  template<typename Object>
  std::shared_ptr<Object>
  getObjectWithId(objid_t id) const
  {
    return objectStorage.getObjectWithId<Object>(id);
  }

  template<typename Object>
  std::shared_ptr<Object>
  getVisibleObjectWithSystemIdentity(MCSystemID systemId)
  {
    return objectStorage.getObjectWithSystemAddress<Object>(systemId);
  }

  // MARK: Adding new types
  // FIXME: Again, this should be broken out. I have some ideas
  // about what we need. Most of the logic itself ca (and should) stay
  // relatively in-tact, but the organization needs to be improved
  // greatly
  void registerVisibleOperationType(MCType, MCSharedMemoryHandler);
  void registerVisibleObjectWithSystemIdentity(
    MCSystemID, std::shared_ptr<MCVisibleObject>);

  // FIXME: This is too closely tied to the
  // mailbox containing the actual data where the next
  // transition is run. Essentially, this function does too many
  // things
  void simulateRunningTransition(const MCTransition &,
                                 MCSharedTransition *, void *);
  void dynamicallyUpdateBacktrackSets();

  bool isInDeadlock() const;
  bool hasADataRaceWithNewTransition(const MCTransition &) const;

  inline bool
  isTargetTraceIdForPrintBacktrace(trid_t trid) const
  {
    return this->configuration.printBacktraceAtTraceNumber == trid;
  }
  // Silly C++ style; Don't let other code know the configuration,
  // and then punch holdes to disclose private invormation.
  inline int
  traceIdForPrintBacktrace() const
  {
    return this->configuration.printBacktraceAtTraceNumber;
  }

  // Restarting
  // TODO: This is extremely unclear. I'm going to work on this...
  void start();
  void reset();

  /**
   * @brief Mutates this state object such that all program state is
   * reflective of how it was when the given transition was at the top
   * of the transition stack
   *
   * At any given point, this state object can only represent a
   * *single* state of a program, which is essentially the collective
   * state of visible objects, threads, and the transitions that those
   * threads are about to (and have) executed. Thus, after a sequence
   * of transitions S is executed against this state, the state will
   * be that of last(S) (in the notation of DPOR).
   *
   * But DPOR requires that we visit previously visited states in
   * order to search a new branch of the state space (in particular,
   * at a particular point in the transition stack [perhaps right
   * before some transition T]). You invoke this method when you need
   * the object and thread states to be as they were at a particular
   * point in time
   *
   * @param tIndex The index in the transition stack that describes
   * the last transition in the sequence (starting from the bottom of
   * the transition stack) that should have been executed in the
   * program this state reflects. In other words, the state becomes
   * last(transition_stack[0...tIndex])
   */
  void reflectStateAtTransitionIndex(uint32_t tIndex);

  void restoreInitialTrace();

  // TODO: De-couple priting from the state stack + transitions
  void printThreadSchedule() const;
  void printTransitionStack() const;
  void printNextTransitions() const;
};

#endif // INCLUDE_MCMINI_MCSTATE_HPP
