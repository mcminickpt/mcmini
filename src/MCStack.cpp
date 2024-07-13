#include "mcmini/MCStack.h"
#include "mcmini/MCTransitionFactory.h"
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

extern "C" {
#include "mcmini/MCCommon.h"
}
#include "mcmini/mcmini_private.h"

using namespace std;

int traceSeq[1000] = {-1};
static int traceSeqIdx = 1;  // traceSeq[0] is for thread 0 'starts'. Skip it.
static int lastEndOfTraceId = -1;

void setEndOfTraceSeq() {
  lastEndOfTraceId = traceId;
}
void resetTraceSeqArray() {
  traceSeqIdx = 0;  // traceSeq[0] is for thread 0 'starts'. Skip it.
}
unsigned int traceSeqLength() {
  unsigned int i;
  for (i = 0; traceSeq[i] != -1; i++);
  return i;
}

static void trace_string_to_int_array(char *str, int *traceArray,
                                      long traceArrayLen) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == ',') {
      str[i] = ' ';
    }
  }

  unsigned int traceArrayIdx = 0;
  while (1) {
    if (!isdigit(*str) && *str != ' ' && *str != '\0') {
      mcprintf("\n*** Input traceSeq contains delimiiter"
               " not in ',' or ' ': %c\n\n", *str);
      mc_stop_model_checking(EXIT_FAILURE);
    }

    char *endptr;
    long num = strtol(str, &endptr, 10);
    if (str == endptr) {
      break; // We've seen all the numbers in the array.
    }
    str = endptr;
    for (; *str == ' '; str++);

    traceArray[traceArrayIdx++] = num;
    if (traceArrayIdx >= traceArrayLen) {
      mcprintf("\n*** traceSeq string too long;"
               " Raise size of 'traceSeq' array\n\n");
      mc_stop_model_checking(EXIT_FAILURE);
    }
    traceArray[traceArrayIdx] = -1;
  }
}

static int getNextTraceSeqEntry(int index) {
  if (lastEndOfTraceId == static_cast<int>(traceId)) {
    // We have gone past traceSeq.
    return -1;
  }
  static bool print_at_trace_seq = (getenv(ENV_PRINT_AT_TRACE_SEQ) != NULL);
  if (!print_at_trace_seq) {
    // This triggers. Why?  It's the same process; same ENV_PRINT_AT_TRACE_SEQ
    print_at_trace_seq = (getenv(ENV_PRINT_AT_TRACE_SEQ) != NULL);
  }
  if (print_at_trace_seq) {
    static bool initialized = false;
    int buflen = sizeof(traceSeq) / sizeof(traceSeq[0]);
    static int traceSeqLen = -1;
    if (! initialized) {
      trace_string_to_int_array(getenv(ENV_PRINT_AT_TRACE_SEQ), traceSeq, buflen);
      for (int i = 0; i < buflen; i++) {
        if (traceSeq[i] == -1) {
          traceSeqLen = i;
          break;
        }
      }
      initialized = true;
    }
    if (lastEndOfTraceId < static_cast<int>(traceId) &&
        index < buflen && traceSeq[index] == -1) {
      if (getenv(ENV_VERBOSE)) {
        mcprintf(
           "**************************************************************\n"
           "*** END OF trace sequence                                    *\n"
           "*** ... continuing beyond trace sequence: At transition # %d%s *\n"
           "**************************************************************\n\n",
           traceSeqLen, (traceSeqLen>=10 ? "" : " "));
      }
      print_at_trace_seq = false;
      setEndOfTraceSeq();
      return -1; // -1 means end of traceSeq; Continue as mormal.
    } else {
      return traceSeq[index];
    }
  } else {
    return -2; // -2 means no ENV_PRINT_AT_TRACE_SEQ; Continue as normal
  }
}

// =====================================================================
/************************************************************************
 * Below here, we are choosing the next enabled transitions to explore. *
 ************************************************************************/
bool
MCStack::transitionIsEnabled(const MCTransition &transition)
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
MCStack::getNextTransitionForThread(tid_t thread) const
{
  return *this->nextTransitions[thread];
}

objid_t
MCStack::registerNewObject(
  const std::shared_ptr<MCVisibleObject> &object)
{
  objid_t newObj   = objectStorage.registerNewObject(object);
  MCSystemID objID = object->getSystemId();
  objectStorage.mapSystemAddressToShadow(objID, newObj);
  return newObj;
}

std::shared_ptr<MCThread>
MCStack::getThreadWithId(tid_t tid) const
{
  objid_t threadObjectId = threadIdMap.find(tid)->second;
  return objectStorage.getObjectWithId<MCThread>(threadObjectId);
}

void
MCStack::setNextTransitionForThread(
  MCThread *thread, std::shared_ptr<MCTransition> transition)
{
  this->setNextTransitionForThread(thread->tid, transition);
}

void
MCStack::setNextTransitionForThread(
  tid_t tid, std::shared_ptr<MCTransition> transition)
{
  this->nextTransitions[tid] = transition;
}

void
MCStack::setNextTransitionForThread(tid_t tid,
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
MCStack::createNewThread(MCThreadShadow &shadow)
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

tid_t MCStack::createNewThread() {
  auto shadow = MCThreadShadow(nullptr, nullptr, pthread_self());
  return this->createNewThread(shadow);
}

tid_t MCStack::createMainThread() {
  MCThreadShadow shadow(nullptr, nullptr, pthread_self());
  shadow.state = MCThreadShadow::alive;
  tid_t mainThreadId = this->createNewThread(shadow);
  MC_ASSERT(mainThreadId == TID_MAIN_THREAD);
  return mainThreadId;
}

void MCStack::restoreInitialTrace() {
  // Reset the modeled state of all objects
  this->nextThreadId = 0;
  this->objectStorage = MCObjectStore();

  // Reset what we believe to be the next steps for each thread. In this
  // case we're starting from the beginning so `thread 0` is executing
  // the start transition
  for (unsigned int i = 0; i < MAX_TOTAL_THREADS_IN_PROGRAM; i++) {
    this->nextTransitions[i] = nullptr;
    this->threadData[i] = MCThreadData();
  }

  this->createMainThread();
  const auto mainThread = getThreadWithId(TID_MAIN_THREAD);
  const auto threadStart =
      MCTransitionFactory::createInitialTransitionForThread(mainThread);
  this->setNextTransitionForThread(TID_MAIN_THREAD, threadStart);

  // Reset the transition/state stacks. NOTE: Clearing the contents isn't
  // strictly necessary since these indices determine the bounds

  // traceSeqIdx = 1; // getFirstEnabledTransition will already be at user main()
  traceSeqIdx = 0; // getFirstEnabledTransition will increment before user main()
  lastEndOfTraceId = -1; // We support 'mcmini back' only for 'traceId == 0'.

  this->nextThreadId = 1;
  this->stateStackTop = -1;
  this->transitionStackTop = -1;
  this->irreversibleStatesStack = MCSortedStack();
  this->growStateStack();
}

tid_t
MCStack::addNewThread(MCThreadShadow &shadow)
{
  tid_t newThread = this->createNewThread(shadow);
  auto thread     = getThreadWithId(newThread);
  auto threadStart =
    MCTransitionFactory::createInitialTransitionForThread(thread);
  this->setNextTransitionForThread(newThread, threadStart);
  return newThread;
}

uint64_t
MCStack::getTransitionStackSize() const
{
  if (this->transitionStackTop < 0) return 0;
  return this->transitionStackTop + 1;
}

uint64_t
MCStack::getStateStackSize() const
{
  if (this->stateStackTop < 0) return 0;
  return this->stateStackTop + 1;
}

uint64_t
MCStack::getNumProgramThreads() const
{
  return this->nextThreadId;
}

void
MCStack::registerVisibleOperationType(MCType type,
                                      MCSharedMemoryHandler handler)
{
  this->sharedMemoryHandlerTypeMap.insert({type, handler});
}

MCStackItem &
MCStack::getDepartingStateForTransitionAtIndex(int i) const
{
  return this->getStateItemAtIndex(i);
}

MCStackItem &
MCStack::getResultantStateForTransitionAtIndex(int i) const
{
  return this->getStateItemAtIndex(i + 1);
}

MCTransition &
MCStack::getTransitionAtIndex(int i) const
{
  return *this->transitionStack[i];
}

MCTransition &
MCStack::getTransitionStackTop() const
{
  return this->getTransitionAtIndex(this->transitionStackTop);
}

tid_t
MCStack::getThreadRunningTransitionAtIndex(int i) const
{
  return this->transitionStack[i]->getThreadId();
}

MCStackItem &
MCStack::getStateItemAtIndex(int i) const
{
  return *this->stateStack[i];
}

MCStackItem &
MCStack::getStateStackTop() const
{
  return this->getStateItemAtIndex(this->stateStackTop);
}

int
MCStack::getDeepestDPORBranchPoint()
{
  for (int j = this->stateStackTop; j >= 0; j--) {
    const auto &s = this->getStateItemAtIndex(j);
    if (s.hasThreadsToBacktrackOn()) return j;
  }
  return FIRST_BRANCH;
}

// Extend current branch by exploring first enabled transition.
const MCTransition *MCStack::getFirstEnabledTransition() {
  int nextTraceEntry = getNextTraceSeqEntry(traceSeqIdx++);
  if (nextTraceEntry >= 0) {
    // FIXME: This is more C++ obfuscation.
    //   We use '&' to convert from 'reference variable' to 'ptr'.
    // This happens because we're using 'reference variables' instead of ptrs.
    // The normal C++ rule is that if we can return nullptr, then the
    //   type should be 'ptr', and _not_ 'reference variable'.
    // We were forced to use a ptr return value for this function, because
    //   this function can return nullptr.  But now, we're doing a dance
    //   with reference variables in order to "pretend" that we will
    //   never return 'nullptr', when in fact we can return 'nullptr'.
    //   It would be better to use ptrs everywhere instead of a
    //   mixture of ptr and reference variable, with little dances to convert.
    return &(this->getNextTransitionForThread(nextTraceEntry));
  }

  const uint32_t numThreads = this->getNumProgramThreads();
  for (uint32_t i = 0; i < numThreads; i++) {
    const MCTransition &nextTransition = this->getNextTransitionForThread(i);
    const bool transitionIsEnabled = this->transitionIsEnabled(nextTransition);

    // We never run transitions contained
    // in the sleep set. Note that new state
    // spaces can be initialized with non-empty
    // sleep sets if previous states passed
    // their state members on
    const MCStackItem &sTop = getStateStackTop();
    const bool transitionIsInSleepSet = sTop.threadIsInSleepSet(i);
    if (transitionIsEnabled && !transitionIsInSleepSet)
      // FIXME:  The syntax makes it seem as though we are returning
      //         an address from this local call frame.
      //         Since nextTransition could be NULL in other situations,
      //         it's better to replace the reference variable by a ptr.
      return &nextTransition;
  }
  return nullptr;
}

// =====================================================================
/*****************************************************************
 * Below here, we are computing the enabled sets to be stored in *
 * backtrack sets in the history (the stack).                    *
 *****************************************************************/
std::unordered_set<tid_t>
MCStack::getCurrentlyEnabledThreads()
{
  std::unordered_set<tid_t> enabledThreadsInState;
  // FIXME:  We are returning an unordered set.
  //         An address to it might be more performant, but the set is small.

#if 0
  static bool print_at_trace_seq = (getenv(ENV_PRINT_AT_TRACE_SEQ) != NULL);
  if (print_at_trace_seq) {
    static int traceSeqIdx = 1; // traceSeq[0] is for thread 0 'starts'. Skip it.
    if (traceSeqIdx <= 1) {
      trace_string_to_int_array(getenv(ENV_PRINT_AT_TRACE_SEQ), traceSeq,
                                       sizeof(traceSeq) / sizeof(traceSeq[0]));
    }
    if (traceSeq[traceSeqIdx] == -1) {
      mcprintf("*** END OF trace sequence\n");
      mcprintf("*** ... continuing beyond trace sequence\n");
      print_at_trace_seq = false;
      assert(traceId == 0);
      setenv(ENV_PRINT_AT_TRACE_ID, "0", 1); // but his was also set earlier
    } else {
      enabledThreadsInState.insert(traceSeq[traceSeqIdx++]);
      return enabledThreadsInState;
    }
  }
#else
  static int traceSeqIdx = 1; // traceSeq[0] is for thread 0 'starts'. Skip it.
  int nextTraceEntry = getNextTraceSeqEntry(traceSeqIdx++);
  if (nextTraceEntry >= 0) {
    enabledThreadsInState.insert(nextTraceEntry);
    return enabledThreadsInState;
  }
#endif

  const uint32_t numThreads = this->getNumProgramThreads();
  for (uint32_t i = 0; i < numThreads; i++) {
    MCTransition &nextTransition = this->getNextTransitionForThread(i);
    if (MCTransition::transitionEnabledInState(this, nextTransition))
      enabledThreadsInState.insert(i);
  }
  return enabledThreadsInState;
}

bool
MCStack::isInDeadlock() const
{
  /*
   * FIXME:  This 'if' statement is being commented out.
   *         In the future, we could consider a new flag, --max-total-depth
   *         So, the code here is kept, to be re-purposed.
   */
  /*
   * We artificially restrict deadlock reports to those in which the
   * total thread depth is at most the total allowed by the program.
   *
   * if (this->totalThreadExecutionDepth() >
   *     this->configuration.maxThreadExecutionDepth) {
   *   return false;
   * }
   */

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
MCStack::hasADataRaceWithNewTransition(
  const MCTransition &transition) const
{
  /*
   * There is a data race if, at any point in the program,
   * there are two threads that are racing with each other
   */
  const tid_t threadRunningTransition = transition.getThreadId();
  const uint64_t numThreads           = this->getNumProgramThreads();

  for (uint64_t i = 0; i < numThreads; i++) {
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
MCStack::transitionStackIsEmpty() const
{
  return this->transitionStackTop < 0;
}

bool
MCStack::stateStackIsEmpty() const
{
  return this->stateStackTop < 0;
}

bool
MCStack::happensBefore(int i, int j) const
{
  MC_ASSERT(i >= 0 && j >= 0);
  const tid_t tid        = getThreadRunningTransitionAtIndex(i);
  const MCClockVector cv = clockVectorForTransitionAtIndex(j);
  return i <= (int)cv.valueForThread(tid).value_or(0);
}

bool
MCStack::happensBeforeThread(int i, tid_t p) const
{
  const tid_t tid        = getThreadRunningTransitionAtIndex(i);
  const MCClockVector cv = getThreadDataForThread(p).getClockVector();
  return i <= (int)cv.valueForThread(tid).value_or(0);
}

bool
MCStack::threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const
{
  // We want to search the entire transition stack in this case
  const int transitionStackHeight = this->getTransitionStackSize();
  for (int j = depth + 1; j < transitionStackHeight; j++) {
    if (q == this->getThreadRunningTransitionAtIndex(j) &&
        this->happensBeforeThread(j, p))
      return true;
  }
  return false;
}

void
MCStack::dynamicallyUpdateBacktrackSets()
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
    MCStackItem &s_n =
      this->getStateItemAtIndex(this->transitionStackTop);
    const std::unordered_set<tid_t> enabledThreadsAt_s_n =
      s_n.getEnabledThreadsInState();

    for (tid_t tid : thread_ids) {
      const MCTransition &nextSP =
        this->getNextTransitionForThread(tid);
      this->dynamicallyUpdateBacktrackSetsHelper(
        S_n, s_n, nextSP, this->transitionStackTop, tid);
    }
  }

  // O(transition stack size)

  // It only remains to add backtrack points at the necessary
  // points for thread `mostRecentThreadId`. We start at one step
  // below the top since we know that transition to not be co-enabled
  // (since it was, by assumption, run by `mostRecentThreadId`
  for (int i = this->transitionStackTop - 1; i >= 0; i--) {
    const MCTransition &S_i = this->getTransitionAtIndex(i);
    MCStackItem &preSi = this->getStateItemAtIndex(i);
    const bool shouldStop   = dynamicallyUpdateBacktrackSetsHelper(
        S_i, preSi, nextTransitionForMostRecentThread, i,
        mostRecentThreadId);
    /*
     * Stop when we find the first such i; this
     * will be the maxmimum `i` since we're searching
     * backwards
     */
    if (shouldStop) break;
  }
}

bool
MCStack::dynamicallyUpdateBacktrackSetsHelper(
  const MCTransition &S_i, MCStackItem &preSi,
  const MCTransition &nextSP, int i, tid_t p)
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
MCStack::virtuallyApplyTransition(const MCTransition &transition)
{
  shared_ptr<MCTransition> dynamicCopy =
    transition.dynamicCopyInState(this);
  dynamicCopy->applyToState(this);
}

void
MCStack::virtuallyUnapplyTransition(const MCTransition &transition)
{
  shared_ptr<MCTransition> dynamicCopy =
    transition.dynamicCopyInState(this);
  dynamicCopy->unapplyToState(this);
}

void
MCStack::virtuallyRunTransition(const MCTransition &transition)
{
  const tid_t tid = transition.getThreadId();
  this->virtuallyApplyTransition(transition);
  this->incrementThreadDepthIfNecessary(transition);
  this->getThreadDataForThread(tid).pushNewLatestExecutionPoint(
    this->transitionStackTop);
}

void
MCStack::virtuallyRerunTransitionAtIndex(int i)
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
MCStack::virtuallyRevertTransitionAtIndex(int i)
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
MCStack::canRunInReverseToStateAtIndex(uint32_t stateStackIndex) const
{
  if (irreversibleStatesStack.empty()) return true;
  return stateStackIndex >= irreversibleStatesStack.top();
}

void
MCStack::simulateRunningTransition(
  const MCTransition &transition,
  MCSharedTransition *shmTransitionTypeInfo, void *shmTransitionData)
{
  // NOTE: You must grow the transition stack before
  // the state stack for clock vector updates
  // to occur properly
  this->growTransitionStackRunning(transition);
  this->growStateStackRunningTransition(transition);

  // NOTE: After applying the transition, this `MCStack`
  // object has moved into the NEXT state, meaning that
  // e.g. asking the question "which threads are enabled
  // now" would ultimate be being asked about the state
  // that follows AFTER `transition` is executed
  this->virtuallyRunTransition(transition);

  tid_t tid = transition.getThreadId();
  this->setNextTransitionForThread(tid, shmTransitionTypeInfo,
                                   shmTransitionData);
}

void
MCStack::incrementThreadDepthIfNecessary(
  const MCTransition &transition)
{
  if (transition.countsAgainstThreadExecutionDepth()) {
    const tid_t tid          = transition.getThreadId();
    MCThreadData &threadData = getThreadDataForThread(tid);
    threadData.incrementExecutionDepth();
  }
}

void
MCStack::decrementThreadDepthIfNecessary(
  const MCTransition &transition)
{
  if (transition.countsAgainstThreadExecutionDepth()) {
    const tid_t tid          = transition.getThreadId();
    MCThreadData &threadData = getThreadDataForThread(tid);
    threadData.decrementExecutionDepthIfNecessary();
  }
}

uint32_t
MCStack::totalThreadExecutionDepth() const
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
MCStack::getThreadDataForThread(tid_t tid)
{
  return this->threadData[tid];
}

const MCThreadData &
MCStack::getThreadDataForThread(tid_t tid) const
{
  return this->threadData[tid];
}

void
MCStack::growTransitionStackRunning(const MCTransition &transition)
{
  auto transitionCopy = transition.staticCopy();
  this->transitionStackTop++;
  this->transitionStack[this->transitionStackTop] = transitionCopy;
}

void
MCStack::growStateStack()
{
  this->growStateStackWith(MCClockVector::newEmptyClockVector(),
                           false);
}

void
MCStack::growStateStackWith(const MCClockVector &cv, bool revertible)
{
  auto newState = std::make_shared<MCStackItem>(cv, revertible);
  this->stateStackTop++;
  this->stateStack[this->stateStackTop] = newState;
}

void
MCStack::growStateStackRunningTransition(const MCTransition &t)
{
  MC_ASSERT(this->stateStackTop >= 0);

  const bool transitionIsRevertible   = t.isReversibleInState(this);
  const tid_t threadRunningTransition = t.getThreadId();
  const unordered_set<tid_t> enabledThreads =
    getCurrentlyEnabledThreads();
  MCThreadData &threadData =
    getThreadDataForThread(threadRunningTransition);
  MCStackItem &oldSTop = getStateStackTop();

  // NOTE: Compute the clock vector BEFORE growing the state
  // stack. The clock vectors in the state stack *prior to* expansion
  // are searched
  MCClockVector cv            = transitionStackMaxClockVector(t);
  cv[threadRunningTransition] = this->transitionStackTop;

  this->growStateStackWith(cv, transitionIsRevertible);

  MCStackItem &newSTop              = getStateStackTop();
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
MCStack::transitionStackMaxClockVector(const MCTransition &transition)
{
  MCClockVector cv = MCClockVector::newEmptyClockVector();

  // The pseudocode stores clock vectors in the transition
  // stack, but this data can be stored equivalently in the
  // stack by noting that the state stack is always one
  // larger than the transition stack (hence tStackIndex).
  for (int i = 1; i <= this->stateStackTop; i++) {
    const int tStackIndex = i - 1;
    const MCTransition &t = getTransitionAtIndex(tStackIndex);

    if (MCTransition::dependentTransitions(t, transition)) {
      const MCStackItem &s   = getStateItemAtIndex(i);
      const MCClockVector clock_i = s.getClockVector();
      cv                          = MCClockVector::max(clock_i, cv);
    }
  }
  return cv;
}

MCClockVector
MCStack::clockVectorForTransitionAtIndex(int i) const
{
  // The clock vector for transition `i` resides in
  // *resulting* state after the transition is executed
  return this->getResultantStateForTransitionAtIndex(i)
    .getClockVector();
}

void
MCStack::start()
{
  this->growStateStack();
}

void
MCStack::reset()
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
MCStack::reflectStateAtTransitionIndex(uint32_t index)
{
  MC_ASSERT((int)index <= this->transitionStackTop);

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
    for (int i = this->transitionStackTop; i > (int)index; i--) {
      const tid_t tid = this->getThreadRunningTransitionAtIndex(i);
      threadToClosestIndexMap[tid] = i;
    }

    for (const pair<tid_t, uint32_t> elem :
         threadToClosestIndexMap) {
      const tid_t tid            = elem.first;
      const uint32_t latestIndex = elem.second;
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
MCStack::registerVisibleObjectWithSystemIdentity(
  MCSystemID systemId, std::shared_ptr<MCVisibleObject> object)
{
  objid_t id = this->objectStorage.registerNewObject(object);
  this->objectStorage.mapSystemAddressToShadow(systemId, id);
}

std::shared_ptr<MCVisibleObject>
MCStack::getObjectWithId(objid_t objectId) {
  return objectStorage.getObjectWithId(objectId);
}

void
MCStack::printThreadSchedule() const
{
  for (int i = 0; i <= this->transitionStackTop; i++) {
    const tid_t tid = this->getTransitionAtIndex(i).getThreadId();
    mcprintf("%lu, ", tid);
  }
  mcprintf("\n");
}

void
MCStack::printTransitionStack() const
{
  mcprintf("THREAD BACKTRACE\n");
  for (int i = 0; i <= this->transitionStackTop; i++) {
    this->getTransitionAtIndex(i).print();
  }
  MCStack::printThreadSchedule();
  mcprintf("END\n");
  mcflush();
}

void
MCStack::printNextTransitions() const
{
  mcprintf("THREAD PENDING OPERATIONS\n");
  auto numThreads = this->getNumProgramThreads();
  for (uint64_t i = 0; i < numThreads; i++) {
    this->getNextTransitionForThread(i).print();
  }
  mcprintf("END\n");
  mcflush();
}

std::vector<tid_t>
MCStack::getThreadIdBacktrace() const
{
  auto trace                       = std::vector<tid_t>();
  const uint64_t transitionStackHeight = this->getTransitionStackSize();
  for (uint64_t i = 0; i < transitionStackHeight; i++)
    trace.push_back(this->getTransitionAtIndex(i).getThreadId());
  return trace;
}

MCStackConfiguration
MCStack::getConfiguration() const
{
  return this->configuration;
}
