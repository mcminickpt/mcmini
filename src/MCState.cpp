#include "mcmini/MCState.h"
#include "mcmini/MCTransitionFactory.h"
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

extern "C" {
#include "mcmini/MCCommon.h"
}

using namespace std;

bool
MCState::transitionIsEnabled(const MCTransition &transition)
{
  // We artificially restrict threads from running that have
  // run for more than their fair share of transitions. Note that
  // in the case that the thread is in a critical section for
  // GOAL() statements this is explicitly ignored
  const tid_t tid                = transition.getThreadId();
  const MCThreadData &threadData = getThreadDataForThread(tid);
  const unsigned numExecutions   = threadData.getExecutionDepth();
  const bool threadNotRestrictedByThreadExecutionDepth =
    numExecutions < this->configuration.maxThreadExecutionDepth;
  return threadNotRestrictedByThreadExecutionDepth &&
         MCTransition::transitionEnabledInState(this, transition);
}

MCTransition &
MCState::getNextTransitionForThread(tid_t thread) const
{
  return *this->nextTransitions[thread];
}

objid_t
MCState::registerNewObject(
  const std::shared_ptr<MCVisibleObject> &object)
{
  objid_t newObj   = objectStorage.registerNewObject(object);
  MCSystemID objID = object->getSystemId();
  objectStorage.mapSystemAddressToShadow(objID, newObj);
  return newObj;
}

std::shared_ptr<MCThread>
MCState::getThreadWithId(tid_t tid) const
{
  objid_t threadObjectId = threadIdMap.find(tid)->second;
  return objectStorage.getObjectWithId<MCThread>(threadObjectId);
}

void
MCState::setNextTransitionForThread(
  MCThread *thread, std::shared_ptr<MCTransition> transition)
{
  this->setNextTransitionForThread(thread->tid, transition);
}

void
MCState::setNextTransitionForThread(
  tid_t tid, std::shared_ptr<MCTransition> transition)
{
  this->nextTransitions[tid] = transition;
}

void
MCState::setNextTransitionForThread(tid_t tid,
                                    MCSharedTransition *shmTypeInfo,
                                    void *shmData)
{
  // TODO: Assert when the type doesn't exist
  auto maybeHandler =
    this->sharedMemoryHandlerTypeMap.find(shmTypeInfo->type);
  if (maybeHandler == this->sharedMemoryHandlerTypeMap.end()) {
    return;
  }

  MCSharedMemoryHandler handlerForType = maybeHandler->second;
  MCTransition *newTransitionForThread =
    handlerForType(shmTypeInfo, shmData, this);
  MC_FATAL_ON_FAIL(newTransitionForThread != nullptr);

  auto sharedPointer =
    std::shared_ptr<MCTransition>(newTransitionForThread);
  this->setNextTransitionForThread(tid, sharedPointer);
}

tid_t
MCState::createNewThread(MCThreadShadow &shadow)
{
  tid_t newTid     = this->nextThreadId++;
  auto rawThread   = new MCThread(newTid, shadow);
  auto thread      = std::shared_ptr<MCThread>(rawThread);
  objid_t newObjId = this->registerNewObject(thread);

  // TODO: Encapsulate transferring object ids
  //    thread->id = newObjId;
  this->threadIdMap.insert({newTid, newObjId});
  return newTid;
}

tid_t
MCState::createNewThread()
{
  auto shadow = MCThreadShadow(nullptr, nullptr, pthread_self());
  return this->createNewThread(shadow);
}

tid_t
MCState::createMainThread()
{
  tid_t mainThreadId = this->createNewThread();
  MC_ASSERT(mainThreadId == TID_MAIN_THREAD);

  // Creating the main thread -> bring it into the spawned state
  auto mainThread = getThreadWithId(mainThreadId);
  mainThread->spawn();

  return mainThreadId;
}

tid_t
MCState::addNewThread(MCThreadShadow &shadow)
{
  tid_t newThread = this->createNewThread(shadow);
  auto thread     = getThreadWithId(newThread);
  auto threadStart =
    MCTransitionFactory::createInitialTransitionForThread(thread);
  this->setNextTransitionForThread(newThread, threadStart);
  return newThread;
}

uint64_t
MCState::getTransitionStackSize() const
{
  if (this->transitionStackTop < 0) return 0;
  return this->transitionStackTop + 1;
}

uint64_t
MCState::getStateStackSize() const
{
  if (this->stateStackTop < 0) return 0;
  return this->stateStackTop + 1;
}

uint64_t
MCState::getNumProgramThreads() const
{
  return this->nextThreadId;
}

void
MCState::registerVisibleOperationType(MCType type,
                                      MCSharedMemoryHandler handler)
{
  this->sharedMemoryHandlerTypeMap.insert({type, handler});
}

MCStateStackItem &
MCState::getDepartingStateForTransitionAtIndex(int i) const
{
  return this->getStateItemAtIndex(i);
}

MCStateStackItem &
MCState::getResultantStateForTransitionAtIndex(int i) const
{
  return this->getStateItemAtIndex(i + 1);
}

MCTransition &
MCState::getTransitionAtIndex(int i) const
{
  return *this->transitionStack[i];
}

MCTransition &
MCState::getTransitionStackTop() const
{
  return this->getTransitionAtIndex(this->transitionStackTop);
}

tid_t
MCState::getThreadRunningTransitionAtIndex(int i) const
{
  return this->transitionStack[i]->getThreadId();
}

MCStateStackItem &
MCState::getStateItemAtIndex(int i) const
{
  return *this->stateStack[i];
}

MCStateStackItem &
MCState::getStateStackTop() const
{
  return this->getStateItemAtIndex(this->stateStackTop);
}

MCOptional<int>
MCState::getDeepestDPORBranchPoint()
{
  for (int j = this->stateStackTop; j >= 0; j--) {
    const auto &s = this->getStateItemAtIndex(j);
    if (s.hasThreadsToBacktrackOn()) return MCOptional<int>::some(j);
  }
  return MCOptional<int>::nil();
}

const MCTransition *
MCState::getFirstEnabledTransition()
{
  const uint64_t threadsInProgram = this->getNumProgramThreads();
  const MCStateStackItem &sTop    = getStateStackTop();
  for (uint64_t i = 0; i < threadsInProgram; i++) {
    const MCTransition &nextTransitionForI =
      this->getNextTransitionForThread(i);
    const bool transitionIsEnabled =
      this->transitionIsEnabled(nextTransitionForI);

    // We never run transitions contained
    // in the sleep set. Note that new state
    // spaces can be initialized with non-empty
    // sleep sets if previous states passed
    // their state members on
    const bool transitionIsInSleepSet = sTop.threadIsInSleepSet(i);
    if (transitionIsEnabled && !transitionIsInSleepSet)
      return &nextTransitionForI;
  }
  return nullptr;
}

std::unordered_set<tid_t>
MCState::getCurrentlyEnabledThreads()
{
  std::unordered_set<tid_t> enabledThreadsInState;
  const uint32_t numThreads = this->getNumProgramThreads();
  for (uint32_t i = 0; i < numThreads; i++) {
    MCTransition &next = this->getNextTransitionForThread(i);
    if (MCTransition::transitionEnabledInState(this, next))
      enabledThreadsInState.insert(i);
  }
  return enabledThreadsInState;
}

bool
MCState::isInDeadlock() const
{
  /*
   * We artificially restrict deadlock reports to those in which the
   * total thread depth is at most the total allowed by the program
   */
  if (this->totalThreadExecutionDepth() >
      this->configuration.maxThreadExecutionDepth) {
    return false;
  }

  const auto numThreads = this->getNumProgramThreads();
  for (tid_t tid = 0; tid < numThreads; tid++) {
    const MCTransition &nextTransitionForTid =
      this->getNextTransitionForThread(tid);

    if (nextTransitionForTid.ensuresDeadlockIsImpossible())
      return false;

    // We don't use the wrapper here (this->transitionIsEnabled)
    // because we only care about if the schedule *could* keep going:
    // we wouldn't be in deadlock if we artificially restricted the
    // threads
    if (MCTransition::transitionEnabledInState(this,
                                               nextTransitionForTid))
      return false;
  }
  return true;
}

bool
MCState::hasADataRaceWithNewTransition(
  const MCTransition &transition) const
{
  /*
   * There is a data race if, at any point in the program,
   * there are two threads that are racing with each other
   */
  const tid_t threadRunningTransition = transition.getThreadId();
  const uint64_t numThreads           = this->getNumProgramThreads();

  for (auto i = 0; i < numThreads; i++) {
    if (i == threadRunningTransition) continue;
    const MCTransition &nextTransitionForThreadI =
      this->getNextTransitionForThread(i);

    if (MCTransition::transitionsInDataRace(nextTransitionForThreadI,
                                            transition))
      return true;
  }

  return false;
}

bool
MCState::transitionStackIsEmpty() const
{
  return this->transitionStackTop < 0;
}

bool
MCState::stateStackIsEmpty() const
{
  return this->stateStackTop < 0;
}

bool
MCState::happensBefore(int i, int j) const
{
  MC_ASSERT(i >= 0 && j >= 0);
  const tid_t tid        = getThreadRunningTransitionAtIndex(i);
  const MCClockVector cv = clockVectorForTransitionAtIndex(j);
  return i <= cv.valueForThread(tid).value_or(0);
}

bool
MCState::happensBeforeThread(int i, tid_t p) const
{
  const tid_t tid        = getThreadRunningTransitionAtIndex(i);
  const MCClockVector cv = getThreadDataForThread(p).getClockVector();
  return i <= cv.valueForThread(tid).value_or(0);
}

bool
MCState::threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const
{
  // We want to search the entire transition stack in this case
  const auto transitionStackHeight = this->getTransitionStackSize();
  for (int j = depth + 1; j < transitionStackHeight; j++) {
    if (q == this->getThreadRunningTransitionAtIndex(j) &&
        this->happensBeforeThread(j, p))
      return true;
  }
  return false;
}

void
MCState::dynamicallyUpdateBacktrackSets()
{
  /*
   * Updating the backtrack sets is accomplished as follows
   * (under the given assumptions)
   *
   * ASSUMPTIONS
   *
   * 1. The state reflects last(S) for the transition stack
   *
   * 2. The thread that ran last is at the top of the transition
   * stack (this should always be true)
   *
   * 3. The next transition for the thread that ran the most
   * recent transition in the transition stack (the transition at the
   * top of the stack) has been properly updated to reflect what that
   * thread will do next
   *
   * WLOG, assume there are `n` transitions in the transition stack
   * and `k` threads that are known to exist at the time of updating
   * the backtrack sets. Note this implies that there are `n+1` items
   * in the state stack (since there is always the initial state + 1
   * for every subsequent transition thereafter)
   *
   * Let
   *  S_i = ith backtracking state item
   *  T_i = ith transition
   *  N_p = the next transition for thread p (next(s, p))
   *
   * ALGORITHM:
   *
   * 1. First, get a reference to the transition at the top
   * of the transition stack (i.e. the most recent transition)
   * as well as the thread that ran that transition. WLOG suppose that
   * thread has a thread id `i`.
   *
   * This transition will be used to test against the transitions
   * queued as running "next" for all of the **other** threads
   * that exist
   *
   *  2. Test whether a backtrack point is needed at state
   *  S_n for the other threads by comparing N_p, for all p != i.
   *
   *  3. Get a reference to N_i and traverse the transition stack
   *  to determine if a backtrack point is needed anywhere for
   *  thread `i`
   */
  const uint64_t num_threads = this->getNumProgramThreads();

  std::unordered_set<tid_t> thread_ids;
  for (tid_t i = 0; i < num_threads; i++) thread_ids.insert(i);

  // 3. Determine the i
  const MCTransition &tStackTop  = this->getTransitionStackTop();
  const tid_t mostRecentThreadId = tStackTop.getThreadId();
  const MCTransition &nextTransitionForMostRecentThread =
    this->getNextTransitionForThread(mostRecentThreadId);
  thread_ids.erase(mostRecentThreadId);

  // O(# threads)
  {
    const MCTransition &S_n = this->getTransitionStackTop();
    MCStateStackItem &s_n =
      this->getStateItemAtIndex(this->transitionStackTop);
    const std::unordered_set<tid_t> enabledThreadsAt_s_n =
      s_n.getEnabledThreadsInState();

    for (tid_t tid : thread_ids) {
      const MCTransition &nextSP =
        this->getNextTransitionForThread(tid);
      this->dynamicallyUpdateBacktrackSetsHelper(
        S_n, s_n, nextSP, this->transitionStackTop, (int)tid);
    }
  }

  // O(transition stack size)

  // It only remains to add backtrack points at the necessary
  // points for thread `mostRecentThreadId`. We start at one step
  // below the top since we know that transition to not be co-enabled
  // (since it was, by assumption, run by `mostRecentThreadId`
  for (int i = this->transitionStackTop - 1; i >= 0; i--) {
    const MCTransition &S_i = this->getTransitionAtIndex(i);
    MCStateStackItem &preSi = this->getStateItemAtIndex(i);
    const bool shouldStop   = dynamicallyUpdateBacktrackSetsHelper(
        S_i, preSi, nextTransitionForMostRecentThread, i,
        (int)mostRecentThreadId);
    /*
     * Stop when we find the first such i; this
     * will be the maxmimum `i` since we're searching
     * backwards
     */
    if (shouldStop) break;
  }
}

bool
MCState::dynamicallyUpdateBacktrackSetsHelper(
  const MCTransition &S_i, MCStateStackItem &preSi,
  const MCTransition &nextSP, int i, int p)
{
  const unordered_set<tid_t> enabledThreadsAtPreSi =
    preSi.getEnabledThreadsInState();
  const bool shouldProcess =
    MCTransition::dependentTransitions(S_i, nextSP) &&
    MCTransition::coenabledTransitions(S_i, nextSP) &&
    !this->happensBeforeThread(i, p);

  // if there exists i such that ...
  if (shouldProcess) {
    std::unordered_set<tid_t> E;

    for (tid_t q : enabledThreadsAtPreSi) {
      const bool inE = q == p || this->threadsRaceAfterDepth(i, q, p);
      const bool isInSleepSet = preSi.threadIsInSleepSet(q);

      // If E != empty set
      if (inE && !isInSleepSet) E.insert(q);
    }

    if (E.empty()) {
      // E is the empty set -> add every enabled thread at pre(S, i)
      for (tid_t q : enabledThreadsAtPreSi)
        if (!preSi.threadIsInSleepSet(q))
          preSi.addBacktrackingThreadIfUnsearched(q);
    } else {
      for (tid_t q : E) {
        // If there is a thread in preSi that we
        // are already backtracking AND which is contained
        // in the set E, chose that thread to backtrack
        // on. This is equivalent to not having to do
        // anything
        if (preSi.isBacktrackingOnThread(q)) return shouldProcess;
      }
      preSi.addBacktrackingThreadIfUnsearched(*E.begin());
    }
  }
  return shouldProcess;
}

void
MCState::virtuallyApplyTransition(const MCTransition &transition)
{
  shared_ptr<MCTransition> dynamicCopy =
    transition.dynamicCopyInState(this);
  dynamicCopy->applyToState(this);
}

void
MCState::virtuallyUnapplyTransition(const MCTransition &transition)
{
  shared_ptr<MCTransition> dynamicCopy =
    transition.dynamicCopyInState(this);
  dynamicCopy->unapplyToState(this);
}

void
MCState::virtuallyRunTransition(const MCTransition &transition)
{
  const tid_t tid = transition.getThreadId();
  this->virtuallyApplyTransition(transition);
  this->incrementThreadDepthIfNecessary(transition);
  this->getThreadDataForThread(tid).pushNewLatestExecutionPoint(
    this->transitionStackTop);
}

void
MCState::virtuallyRerunTransitionAtIndex(int i)
{
  MC_ASSERT(i >= 0);
  const MCTransition &transition = this->getTransitionAtIndex(i);
  const tid_t tid                = transition.getThreadId();
  this->virtuallyApplyTransition(transition);
  this->incrementThreadDepthIfNecessary(transition);
  this->getThreadDataForThread(tid).pushNewLatestExecutionPoint(i);
  MCClockVector cv = clockVectorForTransitionAtIndex(i);
  this->getThreadDataForThread(tid).setClockVector(cv);
}

void
MCState::virtuallyRevertTransitionAtIndex(int i)
{
  MC_ASSERT(i >= 0);
  const MCTransition &transition = this->getTransitionAtIndex(i);
  const tid_t tid                = transition.getThreadId();
  this->virtuallyUnapplyTransition(transition);
  this->decrementThreadDepthIfNecessary(transition);
  this->getThreadDataForThread(tid).popLatestExecutionPoint();
  MCClockVector cv = clockVectorForTransitionAtIndex(i);
  this->getThreadDataForThread(tid).setClockVector(cv);
}

bool
MCState::canRunInReverseToStateAtIndex(uint32_t stateStackIndex) const
{
  if (irreversibleStatesStack.empty()) return true;
  return stateStackIndex >= irreversibleStatesStack.top();
}

void
MCState::simulateRunningTransition(
  const MCTransition &transition,
  MCSharedTransition *shmTransitionTypeInfo, void *shmTransitionData)
{
  // NOTE: You must grow the transition stack before
  // the state stack for clock vector updates
  // to occur properly
  this->growTransitionStackRunning(transition);
  this->growStateStackRunningTransition(transition);
  this->virtuallyRunTransition(transition);

  tid_t tid = transition.getThreadId();
  this->setNextTransitionForThread(tid, shmTransitionTypeInfo,
                                   shmTransitionData);
}

void
MCState::incrementThreadDepthIfNecessary(
  const MCTransition &transition)
{
  if (transition.countsAgainstThreadExecutionDepth()) {
    const tid_t tid          = transition.getThreadId();
    MCThreadData &threadData = getThreadDataForThread(tid);
    threadData.incrementExecutionDepth();
  }
}

void
MCState::decrementThreadDepthIfNecessary(
  const MCTransition &transition)
{
  if (transition.countsAgainstThreadExecutionDepth()) {
    const tid_t tid          = transition.getThreadId();
    MCThreadData &threadData = getThreadDataForThread(tid);
    threadData.decrementExecutionDepthIfNecessary();
  }
}

uint32_t
MCState::totalThreadExecutionDepth() const
{
  /*
   * The number of transitions total into the search we find ourselves
   * -> we don't backtrack in the case that we are over the total
   * depth allowed by the configuration
   */
  uint32_t totalThreadTransitionDepth = 0;
  for (tid_t i = 0; i < this->nextThreadId; i++) {
    const uint32_t depth =
      getThreadDataForThread(i).getExecutionDepth();
    totalThreadTransitionDepth += depth;
  }

  return totalThreadTransitionDepth;
}

MCThreadData &
MCState::getThreadDataForThread(tid_t tid)
{
  return this->threadData[tid];
}

const MCThreadData &
MCState::getThreadDataForThread(tid_t tid) const
{
  return this->threadData[tid];
}

void
MCState::growTransitionStackRunning(const MCTransition &transition)
{
  auto transitionCopy = transition.staticCopy();
  this->transitionStackTop++;
  this->transitionStack[this->transitionStackTop] = transitionCopy;
}

void
MCState::growStateStack()
{
  this->growStateStackWith(MCClockVector::newEmptyClockVector(),
                           false);
}

void
MCState::growStateStackWith(const MCClockVector &cv, bool revertible)
{
  auto newState = std::make_shared<MCStateStackItem>(cv, revertible);
  this->stateStackTop++;
  this->stateStack[this->stateStackTop] = newState;
}

void
MCState::growStateStackRunningTransition(const MCTransition &t)
{
  MC_ASSERT(this->stateStackTop >= 0);

  const bool transitionIsRevertible   = t.isReversibleInState(this);
  const tid_t threadRunningTransition = t.getThreadId();
  const unordered_set<tid_t> enabledThreads =
    getCurrentlyEnabledThreads();
  MCThreadData &threadData =
    getThreadDataForThread(threadRunningTransition);
  MCStateStackItem &oldSTop = getStateStackTop();

  // NOTE: Compute the clock vector BEFORE growing the state
  // stack. The clock vectors in the state stack *prior to* expansion
  // are searched
  MCClockVector cv            = transitionStackMaxClockVector(t);
  cv[threadRunningTransition] = this->transitionStackTop;

  this->growStateStackWith(cv, transitionIsRevertible);

  MCStateStackItem &newSTop              = getStateStackTop();
  const unordered_set<tid_t> oldSleepSet = oldSTop.getSleepSet();

  // INVARIANT: For each thread `p`, if such a thread is contained
  // in `oldSleepSet`, then next(oldSTop, p) MUST be the transition
  // that would be contained in that sleep set.
  for (const tid_t &tid : oldSleepSet) {
    const MCTransition &tidNext = getNextTransitionForThread(tid);
    if (!MCTransition::dependentTransitions(tidNext, t))
      newSTop.addThreadToSleepSet(tid);
  }

  // `threadRunningTransition` is *about to* execute.
  // We don't want to add `threadRunningTransition` before
  // computing `oldSleepSet` above
  oldSTop.markThreadsEnabledInState(enabledThreads);
  oldSTop.addThreadToSleepSet(threadRunningTransition);
  oldSTop.markBacktrackThreadSearched(threadRunningTransition);

  // The DPOR variant with clock vectors points to
  // the thread at the top of the sequence after running
  // the transition (S' = S.t). Since state stack growth occurs
  // *after* the transition stack grows, the index
  // will be what the new current top of the transition stack points
  // to
  threadData.setClockVector(cv);

  // Push the transition
  if (!transitionIsRevertible)
    irreversibleStatesStack.push(this->stateStackTop);
}

MCClockVector
MCState::transitionStackMaxClockVector(const MCTransition &transition)
{
  MCClockVector cv = MCClockVector::newEmptyClockVector();

  // The pseudocode stores clock vectors in the transition
  // stack, but this data can be stored equivalently in the
  // stack stack by noting that the state stack is always
  // one larger than the transition stack (hence tStackIndex)
  for (int i = 1; i <= this->stateStackTop; i++) {
    const int tStackIndex = i - 1;
    const MCTransition &t = getTransitionAtIndex(tStackIndex);

    if (MCTransition::dependentTransitions(t, transition)) {
      const MCStateStackItem &s   = getStateItemAtIndex(i);
      const MCClockVector clock_i = s.getClockVector();
      cv                          = MCClockVector::max(clock_i, cv);
    }
  }
  return cv;
}

MCClockVector
MCState::clockVectorForTransitionAtIndex(int i) const
{
  // The clock vector for transition `i` resides in
  // *resulting* state after the transition is executed
  return this->getResultantStateForTransitionAtIndex(i)
    .getClockVector();
}

void
MCState::start()
{
  this->growStateStack();
}

void
MCState::reset()
{
  // NOTE:!!!! DO NOT clear out the transition stack's contents
  // in this method. If you eventually decide to, remember that
  // backtracking now relies on the fact that the transition stack
  // remains unchanged to resimulate the simulation
  // back to the current state
  this->stateStackTop      = -1;
  this->transitionStackTop = -1;
  this->nextThreadId       = 0;
}

void
MCState::reflectStateAtTransitionIndex(uint32_t index)
{
  MC_ASSERT(index <= this->transitionStackTop);

  /*
   * Note that this is the number of threads at the *current*
   * (last(S)) state It's possible that some of these threads will be
   * in the embryo state at depth _depth_
   */
  const uint64_t numThreadsToProcess = this->getNumProgramThreads();

  // The `irreversibleStatesStack` has indices into the state stack
  // while the `index` points into the transition stack. Thus
  // we add 1 to be in the same "coordinate space".
  const uint32_t stateStackIndex = index + 1;
  const bool canRunReverseOperationsToIndex =
    canRunInReverseToStateAtIndex(stateStackIndex);

  /* The transition stack at this point is untouched */

  if (!canRunReverseOperationsToIndex) {
    // 1. Reset the state of all of the objects
    this->objectStorage.resetObjectsToInitialStateInStore();

    // Zero the thread depth counts
    for (tid_t tid = 0; tid < this->nextThreadId; tid++)
      getThreadDataForThread(tid).resetExecutionData();

    /*
     * Then, replay the transitions in the transition stack forward in
     * time up until the specified depth. Note we include the _depth_
     * value itself
     */
    for (uint32_t i = 0u; i <= index; i++)
      this->virtuallyRerunTransitionAtIndex(i);
  } else {

    // In the case we can revert the transitions in the transition
    // stack, all we need to do is revert the transitions
    // up until the index, making sure _not_ to revert the transition
    // at _index_
    for (tid_t tid = 0; tid < this->nextThreadId; tid++)
      getThreadDataForThread(tid).popExecutionPointsGreaterThan(
        index);

    for (uint32_t i = this->transitionStackTop; i > index; i--)
      this->virtuallyRevertTransitionAtIndex(i);
  }

  {
    /*
     * Finally, fill in the set of next transitions by
     * following the transition stack from the top to _depth_ since
     * this implicitly holds what each thread *was* doing next
     *
     * To reduce the number of dynamic copies, we can simply keep
     * track of the _smallest_ index in the transition stack *greater
     * than _depth_* for each thread, as that would have to be the
     * most recent transition that that thread would have wanted to
     * run next at transition depth _depth_
     */

    std::unordered_map<tid_t, uint32_t> threadToClosestIndexMap;
    for (int i = this->transitionStackTop; i > index; i--) {
      const tid_t tid = this->getThreadRunningTransitionAtIndex(i);
      threadToClosestIndexMap[tid] = i;
    }

    for (const pair<tid_t, uint32_t> &elem :
         threadToClosestIndexMap) {
      const tid_t tid            = elem.first;
      const uint32_t latestIndex = elem.second;
      const MCThreadData &tData  = this->getThreadDataForThread(tid);
      const MCTransition &transition =
        this->getTransitionAtIndex(latestIndex);
      const auto dynamicCopy = transition.dynamicCopyInState(this);
      this->setNextTransitionForThread(tid, dynamicCopy);
    }

    /*
     * For threads that didn't run after depth _depth_, we still need
     * to update those transitions to reflect the new dynamic state
     * since the objects those transitions refer to are no longer
     * valid. But if we can reverse the transitions,
     */
    if (!canRunReverseOperationsToIndex) {
      for (tid_t tid = 0; tid < numThreadsToProcess; tid++) {
        if (threadToClosestIndexMap.count(tid) == 0) {
          this->setNextTransitionForThread(
            tid,
            this->getNextTransitionForThread(tid).dynamicCopyInState(
              this));
        }
      }
    }
  }

  // Keep the set of irreversible states up to date
  irreversibleStatesStack.popGreaterThan(stateStackIndex);

  {
    /* Reset where we now are in the transition/state stacks */
    this->transitionStackTop = index;
    this->stateStackTop      = index + 1;
  }
}

void
MCState::registerVisibleObjectWithSystemIdentity(
  MCSystemID systemId, std::shared_ptr<MCVisibleObject> object)
{
  objid_t id = this->objectStorage.registerNewObject(object);
  this->objectStorage.mapSystemAddressToShadow(systemId, id);
}

void
MCState::printTransitionStack() const
{
  printf("THREAD BACKTRACE\n");
  for (int i = 0; i <= this->transitionStackTop; i++) {
    this->getTransitionAtIndex(i).print();
  }
  for (int i = 0; i <= this->transitionStackTop; i++) {
    const tid_t tid = this->getTransitionAtIndex(i).getThreadId();
    printf("%lu, ", tid);
  }
  printf("\nEND\n");
  mcflush();
}

void
MCState::printNextTransitions() const
{
  printf("THREAD STATES\n");
  auto numThreads = this->getNumProgramThreads();
  for (auto i = 0; i < numThreads; i++) {
    this->getNextTransitionForThread(i).print();
  }
  printf("END\n");
  mcflush();
}

bool
MCState::isTargetTraceIdForGDB(trid_t trid) const
{
  return this->configuration.gdbDebugTraceNumber == trid;
}

bool
MCState::isTargetTraceIdForStackContents(trid_t trid) const
{
  return this->configuration.stackContentDumpTraceNumber == trid;
}

std::vector<tid_t>
MCState::getThreadIdBacktrace() const
{
  auto trace                       = std::vector<tid_t>();
  const auto transitionStackHeight = this->getTransitionStackSize();
  for (auto i = 0; i < transitionStackHeight; i++)
    trace.push_back(this->getTransitionAtIndex(i).getThreadId());
  return trace;
}

MCStateConfiguration
MCState::getConfiguration() const
{
  return this->configuration;
}
