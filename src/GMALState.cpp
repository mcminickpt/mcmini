#include "GMALState.h"
#include "GMALTransitionFactory.h"
#include <memory>
#include <unordered_set>
#include <vector>

bool
GMALState::transitionIsEnabled(const std::shared_ptr<GMALTransition> &transition)
{
    // Decorator
    auto threadRunningTransition = transition->getThreadId();
    auto numExecutionsOfTransitionInState = this->threadTransitionCounts[threadRunningTransition];
    auto threadEnabledAccordingToConfig = numExecutionsOfTransitionInState < this->configuration.maxThreadExecutionDepth;
    return threadEnabledAccordingToConfig && transition->enabledInState(this);
}

std::shared_ptr<GMALTransition>
GMALState::getNextTransitionForThread(GMALThread *thread)
{
    return nextTransitions[thread->tid];
}

std::shared_ptr<GMALTransition>
GMALState::getNextTransitionForThread(tid_t thread)
{
    return nextTransitions[thread];
}

objid_t
GMALState::registerNewObject(const std::shared_ptr<GMALVisibleObject>& object)
{
    objid_t newObj = objectStorage.registerNewObject(object);
    GMALSystemID objID = object->getSystemId();
    objectStorage.mapSystemAddressToShadow(objID, newObj);
    return newObj;
}

std::shared_ptr<GMALThread>
GMALState::getThreadWithId(tid_t tid) const
{
    objid_t threadObjectId = threadIdMap.find(tid)->second;
    return objectStorage.getObjectWithId<GMALThread>(threadObjectId);
}

void
GMALState::setNextTransitionForThread(GMALThread *thread, std::shared_ptr<GMALTransition> transition)
{
    this->setNextTransitionForThread(thread->tid, transition);
}

void
GMALState::setNextTransitionForThread(tid_t tid, std::shared_ptr<GMALTransition> transition)
{
    this->nextTransitions[tid] = transition;
}

void
GMALState::setNextTransitionForThread(tid_t tid, GMALSharedTransition *shmTypeInfo, void *shmData)
{
    // TODO: Assert when the type doesn't exist
    auto maybeHandler = this->sharedMemoryHandlerTypeMap.find(shmTypeInfo->type);
    if (maybeHandler == this->sharedMemoryHandlerTypeMap.end()) {
        return;
    }

    GMALSharedMemoryHandler handlerForType = maybeHandler->second;
    GMALTransition *newTransitionForThread = handlerForType(shmTypeInfo, shmData, this);
    GMAL_FATAL_ON_FAIL(newTransitionForThread != nullptr);

    auto sharedPointer = std::shared_ptr<GMALTransition>(newTransitionForThread);
    this->setNextTransitionForThread(tid, sharedPointer);
}

tid_t
GMALState::createNewThread(GMALThreadShadow &shadow)
{
    tid_t newTid = this->nextThreadId++;
    auto rawThread = new GMALThread(newTid, shadow);
    auto thread = std::shared_ptr<GMALThread>(rawThread);
    objid_t newObjId = this->registerNewObject(thread);

    // TODO: Encapsulate transferring object ids
    thread->id = newObjId;
    this->threadIdMap.insert({newTid, newObjId});
    return newTid;
}

tid_t
GMALState::createNewThread()
{
    auto shadow = GMALThreadShadow(nullptr, nullptr, pthread_self());
    return this->createNewThread(shadow);
}

tid_t
GMALState::createMainThread()
{
    tid_t mainThreadId = this->createNewThread();
    GMAL_ASSERT(mainThreadId == TID_MAIN_THREAD);

    // Creating the main thread -> bring it into the spawned state
    auto mainThread = getThreadWithId(mainThreadId);
    mainThread->spawn();

    return mainThreadId;
}

tid_t
GMALState::addNewThread(GMALThreadShadow &shadow)
{
    tid_t newThread = this->createNewThread(shadow);
    auto thread = getThreadWithId(newThread);
    auto threadStart = GMALTransitionFactory::createInitialTransitionForThread(thread);
    this->setNextTransitionForThread(newThread, threadStart);
    return newThread;
}

uint64_t
GMALState::getTransitionStackSize() const
{
    if (this->transitionStackTop < 0) return 0;
    return this->transitionStackTop + 1;
}

uint64_t
GMALState::getStateStackSize() const
{
    if (this->stateStackTop < 0) return 0;
    return this->stateStackTop + 1;
}

uint64_t
GMALState::getNumProgramThreads() const
{
    return this->nextThreadId;
}

void
GMALState::registerVisibleOperationType(GMALType type, GMALSharedMemoryHandler handler)
{
    this->sharedMemoryHandlerTypeMap.insert({type, handler});
}

inline std::shared_ptr<GMALTransition>
GMALState::getTransitionAtIndex(int i) const
{
    return this->transitionStack[i];
}

std::shared_ptr<GMALTransition>
GMALState::getTransitionStackTop() const
{
    return this->transitionStack[this->transitionStackTop];
}

inline tid_t
GMALState::getThreadRunningTransitionAtIndex(int i) const
{
    return this->transitionStack[i]->getThreadId();
}

inline std::shared_ptr<GMALStateStackItem>
GMALState::getStateItemAtIndex(int i) const
{
    return this->stateStack[i];
}

std::shared_ptr<GMALStateStackItem>
GMALState::getStateStackTop() const
{
    return this->stateStack[this->stateStackTop];
}


std::shared_ptr<GMALTransition>
GMALState::getPendingTransitionForThread(tid_t tid) const
{
    return this->nextTransitions[tid];
}

std::shared_ptr<GMALTransition>
GMALState::getFirstEnabledTransitionFromNextStack()
{
    const auto threadsInProgram = this->getNumProgramThreads();
    for (auto i = 0; i < threadsInProgram; i++) {
        const std::shared_ptr<GMALTransition> &nextTransitionForI = this->nextTransitions[i];
        if (this->transitionIsEnabled(nextTransitionForI)) return nextTransitionForI;
    }
    return nullptr;
}

bool
GMALState::programIsInDeadlock()
{
    /*
     * We artificially restrict deadlock reports to those in which the total
     * thread depth is at most the total allowed by the program
     */
    if (this->totalThreadExecutionDepth() > this->configuration.maxThreadExecutionDepth) {
        return false;
    }

    const auto numThreads = this->getNumProgramThreads();
    for (tid_t tid = 0; tid < numThreads; tid++) {
        auto nextTransitionForTid = this->getNextTransitionForThread(tid);

        if (nextTransitionForTid->ensuresDeadlockIsImpossible())
            return false;

        // We don't use the wrapper here (this->transitionIsEnabled)
        // because we only care about if the schedule *could* keep going:
        // we wouldn't be in deadlock if we artificially restricted the threads
        if (nextTransitionForTid->enabledInState(this))
            return false;
    }
    return true;
}

bool
GMALState::transitionStackIsEmpty() const
{
    return this->transitionStackTop < 0;
}

bool
GMALState::stateStackIsEmpty() const
{
    return this->stateStackTop < 0;
}

bool
GMALState::happensBefore(int i, int j) const
{
    GMAL_ASSERT(i >= 0 && j >= 0);
    if (i > j) return false;
    {
        std::shared_ptr<GMALTransition> t_i = this->getTransitionAtIndex(i);
        std::shared_ptr<GMALTransition> t_j = this->getTransitionAtIndex(j);
        if (GMALTransition::dependentTransitions(t_i, t_j))
            return true;
    }

    const uint64_t stackCount = this->getTransitionStackSize();
    auto dfs_stack = std::vector<int>();
    dfs_stack.push_back(i);

    while (!dfs_stack.empty()) {
        int i_search = dfs_stack.back(); dfs_stack.pop_back();
        std::shared_ptr<GMALTransition> ti_search = this->getTransitionAtIndex(i_search);

        if (i_search == j) {
            return true;
        }
        else if (i_search > j) {
            // We only increase from here, but we may have pushed something onto the stack that is less than j, so we have to search it
            continue;
        }
        else {
            for (int k = i_search + 1; k < stackCount; k++) {
                std::shared_ptr<GMALTransition> tk_search = this->getTransitionAtIndex(k);
                if (GMALTransition::dependentTransitions(ti_search, tk_search))
                    dfs_stack.push_back(k);
            }
        }
    }
    return false;
}

bool
GMALState::happensBeforeThread(int i, const std::shared_ptr<GMALThread>& p) const
{
    return this->happensBeforeThread(i, p->tid);
}

bool
GMALState::happensBeforeThread(int i, tid_t p) const
{
    tid_t tidI = this->getTransitionAtIndex(i)->getThreadId();
    if (p == tidI) return true;

    const auto tStackSize = this->getTransitionStackSize();
    for (auto k = i + 1; k < tStackSize; k++) {
        std::shared_ptr<GMALTransition> S_k = this->getTransitionAtIndex(k);

        // Check threads_equal first (much less costly than happens before)
        if (p == S_k->getThreadId() && happensBefore(i, k))
            return true;
    }
    return false;
}

bool
GMALState::threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const
{
    // We want to search the entire transition stack in this case
    const auto transitionStackHeight = this->getTransitionStackSize();
    for (int j = depth + 1; j < transitionStackHeight; j++) {
        if (q == this->getThreadRunningTransitionAtIndex(j) && this->happensBeforeThread(j, p))
            return true;
    }
    return false;
}

void
GMALState::dynamicallyUpdateBacktrackSets()
{
    // 1. Save current state info
    const int stateStackTopBeforeBacktracking = this->stateStackTop;
    const int transitionStackTopBeforeBacktracking = this->transitionStackTop;
    const uint64_t numThreadsBeforeBacktracking = this->getNumProgramThreads();
    const uint64_t transitionStackHeight = this->getTransitionStackSize();

    // 2. State assertions before continuing
    GMAL_ASSERT(transitionStackHeight < MAX_TOTAL_TRANSITIONS_IN_PROGRAM);

    // 3. Map which thread ids still need to be processed
    std::shared_ptr<GMALTransition> nextTransitionsAtLastS[MAX_TOTAL_THREADS_IN_PROGRAM];
    auto remainingThreadsToProcess = std::unordered_set<tid_t>();

    for (auto i = 0; i < numThreadsBeforeBacktracking; i++) {
        nextTransitionsAtLastS[i] = this->nextTransitions[i];
        remainingThreadsToProcess.insert(static_cast<tid_t>(i));
    }

    int detachedTransitionStackTop = transitionStackTopBeforeBacktracking + 1;

    // 4. Perform the actual backtracking here

    // Walk up the transition stack, starting at the top
    for (int i = transitionStackTopBeforeBacktracking; i >= 0 && !remainingThreadsToProcess.empty(); i--, detachedTransitionStackTop--) {

        std::shared_ptr<GMALTransition> S_i = this->getTransitionAtIndex(i);
        std::shared_ptr<GMALStateStackItem> preSi = this->getStateItemAtIndex(i);

        // Undo the transition so that the state reflects
        this->virtuallyRevertTransitionForBacktracking(S_i);

        // The set of threads at pre(S, i) that are enabled. Lazily created
        auto enabledThreadsAtPreSi = std::unordered_set<tid_t>();
        auto processedThreads = std::unordered_set<tid_t>();
        const auto numThreadsAtDepthi = this->getNumProgramThreads();

        // for all processes p
        for (auto p : remainingThreadsToProcess) {
            std::shared_ptr<GMALTransition> nextSP = nextTransitionsAtLastS[p];
            const bool shouldProcess = GMALTransition::dependentTransitions(S_i, nextSP) && GMALTransition::coenabledTransitions(S_i, nextSP) && !this->happensBeforeThread(i, p);

            // if there exists i such that ...
            if (shouldProcess) {
                // This is the largest such i for tid -> no need to process it again
                processedThreads.insert(p);

                // Compute set E
                if (enabledThreadsAtPreSi.empty()) {
                    for (auto j = 0; j < numThreadsAtDepthi; j++) {
                        auto nextAtPreSi = this->getPendingTransitionForThread(j);

                        // We use the enabled wrapper function here since we should try to run
                        // a thread that has already run the maximum number of times
                        if (this->transitionIsEnabled(nextAtPreSi)) enabledThreadsAtPreSi.insert(j);
                    }
                }

                bool EIsEmpty = true;
                for (auto q : enabledThreadsAtPreSi) {
                    const bool inE = q == p || this->threadsRaceAfterDepth(i, q, p);

                    // If E != empty set
                    if (inE) {
                        // Add any element in E
                        // TODO: We can selectively pick here to reduce the number of times we backtrack
                        preSi->addBacktrackingThreadIfUnsearched(q);
                        EIsEmpty = false;
                        break;
                    }
                }
                if (EIsEmpty) {
                    // E is the empty set -> add every enabled thread at pre(S, i)
                    for (auto q : enabledThreadsAtPreSi)
                        preSi->addBacktrackingThreadIfUnsearched(q);
                }
            }
        }

        // Filter out the processed threads from those we still have to process
        for (auto &p : processedThreads) {
            remainingThreadsToProcess.erase(p);
        }
    }

    // Restore the previous state
    {
        for (auto i = detachedTransitionStackTop; i <= transitionStackTopBeforeBacktracking; i++)
            this->virtuallyRunTransition(this->getTransitionAtIndex(i));

        for (auto i = 0; i < numThreadsBeforeBacktracking; i++)
            this->nextTransitions[i] = nextTransitionsAtLastS[i];

        this->stateStackTop = stateStackTopBeforeBacktracking;
        this->transitionStackTop = transitionStackTopBeforeBacktracking;
    }
}

void
GMALState::virtuallyRunTransition(const std::shared_ptr<GMALTransition> &transition)
{
    std::shared_ptr<GMALTransition> dynamicCopy = transition->dynamicCopyInState(this);
    dynamicCopy->applyToState(this);
    // Do other state updates here
    this->incrementThreadTransitionCountIfNecessary(transition);
}

void
GMALState::virtuallyRevertTransitionForBacktracking(const std::shared_ptr<GMALTransition> &transition)
{
    std::shared_ptr<GMALTransition> dynamicCopy = transition->dynamicCopyInState(this);
    dynamicCopy->unapplyToState(this);

    // Do other state update here
    this->decrementThreadTransitionCountIfNecessary(transition);

    tid_t threadRunningTransition = dynamicCopy->getThreadId();
    this->nextTransitions[threadRunningTransition] = dynamicCopy;
}

void
GMALState::simulateRunningTransition(const std::shared_ptr<GMALTransition> &transition)
{
    this->growStateStackWithTransition(transition);
    this->growTransitionStackRunning(transition);
    this->virtuallyRunTransition(transition);
}

void
GMALState::incrementThreadTransitionCountIfNecessary(const std::shared_ptr<GMALTransition> &transition)
{
    if (transition->countsAgainstThreadExecutionDepth()) {
        auto threadRunningTransition = transition->getThreadId();
        this->threadTransitionCounts[threadRunningTransition]++;
    }
}

void
GMALState::decrementThreadTransitionCountIfNecessary(const std::shared_ptr<GMALTransition> &transition)
{
    if (transition->countsAgainstThreadExecutionDepth()) {
        auto threadRunningTransition = transition->getThreadId();
        this->threadTransitionCounts[threadRunningTransition]--;
    }
}

uint32_t
GMALState::totalThreadExecutionDepth() const
{
    /*
     * The number of transitions total into the search we find ourselves -> we don't
     * backtrack in the case that we are over the total depth allowed by the configuration
    */
    auto totalThreadTransitionDepth = static_cast<uint32_t>(0);
    for (auto i = 0; i < this->nextThreadId; i++)
        totalThreadTransitionDepth += this->threadTransitionCounts[i];
    return totalThreadTransitionDepth;
}

void
GMALState::growTransitionStackRunning(const std::shared_ptr<GMALTransition> &transition)
{
    auto transitionCopy = transition->staticCopy();
    auto threadRunningTransition = transition->getThreadId();
    this->transitionStackTop++;
    this->transitionStack[this->transitionStackTop] = transitionCopy;
}

void
GMALState::growStateStack()
{
    auto newState = std::make_shared<GMALStateStackItem>();
    this->stateStackTop++;
    this->stateStack[this->stateStackTop] = newState;
}


void
GMALState::growStateStackWithTransition(const std::shared_ptr<GMALTransition> &transition)
{
    GMAL_ASSERT(this->stateStackTop >= 0);

    // Insert the thread that ran the transition into the
    // done set of the current top-most state
    auto oldStop = this->stateStack[this->stateStackTop];
    auto threadRunningTransition = transition->getThreadId();
    oldStop->markBacktrackThreadSearched(threadRunningTransition);
    this->growStateStack();
}

void
GMALState::start()
{
    this->growStateStack();
}

void
GMALState::reset()
{
    this->stateStackTop = -1;
    this->transitionStackTop = -1;
    this->nextThreadId = 0;
}

void
GMALState::moveToPreviousState()
{
    if (!transitionStackIsEmpty()) {
        std::shared_ptr<GMALTransition> transitionTop = this->getTransitionStackTop();
        this->virtuallyRevertTransitionForBacktracking(transitionTop);
        this->transitionStackTop--;
    }
    this->stateStackTop--;
}

void
GMALState::registerVisibleObjectWithSystemIdentity(GMALSystemID systemId, std::shared_ptr<GMALVisibleObject> object)
{
    // TODO: This can be simplified easily
    objid_t id = this->objectStorage.registerNewObject(object);
    this->objectStorage.mapSystemAddressToShadow(systemId, id);
    object->id = id;
}

void
GMALState::printTransitionStack() const
{
    printf("\t THREAD BACKTRACE\n");
    for (int i = 0; i <= this->transitionStackTop; i++) {
        this->getTransitionAtIndex(i)->print();
    }
    printf("\t END\n");
}

void
GMALState::printNextTransitions() const
{
    printf("\t THREAD STATES\n");
    auto numThreads = this->getNumProgramThreads();
    for (auto i = 0; i < numThreads; i++) {
        this->getPendingTransitionForThread(i)->print();
    }
    printf("\t END\n");
}

bool
GMALState::isTargetTraceIdForGDB(trid_t trid) const
{
    return this->configuration.gdbDebugTraceNumber == trid;
}

std::vector<tid_t>
GMALState::getThreadIdTraceOfTransitionStack() const
{
    auto trace = std::vector<tid_t>();
    const auto transitionStackHeight = this->getTransitionStackSize();
    for (auto i = 0; i < transitionStackHeight; i++)
        trace.push_back(this->getTransitionAtIndex(i)->getThreadId());
    return trace;
}