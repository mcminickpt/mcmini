#include "GMALState.h"
#include <set>
#include <vector>

std::shared_ptr<GMALTransition>
GMALState::getSlotForThread(GMALThread *thread)
{
    return nextTransitions[thread->tid];
}

std::shared_ptr<GMALTransition>
GMALState::getSlotForThread(tid_t tid)
{
    return nextTransitions[tid];
}

objid_t
GMALState::registerNewObject(GMALVisibleObject *object)
{
    return objectStorage.registerNewObject(object);
}

objid_t
GMALState::softRegisterNewObject(GMALVisibleObject *object)
{
    return objectStorage.registerNewObject(object);
}

template<typename Object>
std::shared_ptr<Object>
GMALState::getObjectWithId(objid_t id) const
{
    return objectStorage.getObjectWithId<Object>(id);
}

std::shared_ptr<GMALThread>
GMALState::getThreadWithId(tid_t id) const
{
    return objectStorage.getObjectWithId<GMALThread>(id);
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
GMALState::setNextTransitionForThread(tid_t tid, GMALSharedTransition *shmData)
{
    GMALSharedMemoryHandler handlerForType = this->sharedMemoryHandlerTypeMap.find(shmData->type)->second;
    GMALTransition *newTransitionForThread = handlerForType(&shmData->data, *this);
    auto sharedPointer = std::shared_ptr<GMALTransition>(newTransitionForThread);
    this->setNextTransitionForThread(tid, sharedPointer);
}

tid_t
GMALState::createNewThread()
{
    tid_t newTid = this->nextThreadId++;
    auto thread = new GMALThread(newTid, nullptr, nullptr, pthread_self());
    objid_t objid = this->registerNewObject(thread);
    return newTid;
}

tid_t
GMALState::createMainThread()
{
    tid_t mainThreadId = this->createNewThread();
    GMAL_ASSERT(mainThreadId == TID_MAIN_THREAD);
    return mainThreadId;
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

std::shared_ptr<GMALTransition>
GMALState::getPendingTransitionForThread(tid_t tid) const
{
    return this->nextTransitions[tid];
}

std::shared_ptr<GMALTransition>
GMALState::getFirstEnabledTransitionFromNextStack()
{
    const int threadsInProgram = this->getNumProgramThreads();
    for (auto i = 0; i < threadsInProgram; i++) {
        const std::shared_ptr<GMALTransition> &nextTransitionForI = this->nextTransitions[i];
        if (nextTransitionForI->enabledInState(this)) return nextTransitionForI;
    }
    return nullptr;
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

    const int tStackSize = this->getTransitionStackSize();
    for (int k = i + 1; k < tStackSize; k++) {
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
    const int transitionStackHeight = this->getTransitionStackSize();
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
    auto remainingThreadsToProcess = std::set<tid_t>();
    for (auto i = 0; i < numThreadsBeforeBacktracking; i++) {
        nextTransitionsAtLastS[i] = this->nextTransitions[i];
        remainingThreadsToProcess.insert(static_cast<tid_t>(i));
    }

    // 4. Perform the actual backtracking here

    // Walk up the transition stack, starting at the top
    for (int i = transitionStackTopBeforeBacktracking; i >= 0 && !remainingThreadsToProcess.empty(); i--) {
        std::shared_ptr<GMALTransition> S_i = this->getTransitionAtIndex(i);
        std::shared_ptr<GMALStateStackItem> preSi = this->getStateItemAtIndex(i);

        // The set of threads at pre(S, i) that are enabled. Lazily created
        auto enabledThreadsAtPreSi = std::set<tid_t>();
        const auto numThreadsAtDepthi = this->getNumProgramThreads();

        // for all processes p
        for (auto p : remainingThreadsToProcess) {
            std::shared_ptr<GMALTransition> nextSP = nextTransitionsAtLastS[p];
            const bool shouldProcess = GMALTransition::dependentTransitions(S_i, nextSP) && !this->happensBeforeThread(i, p);

            // if there exists i such that ...
            if (shouldProcess) {
                // This is the largest such i for tid -> no need to process it again
                remainingThreadsToProcess.erase(p);

                // Compute set E
                if (enabledThreadsAtPreSi.empty()) {
                    for (auto j = 0; j < numThreadsAtDepthi; i++) {
                        auto nextAtPreSi = this->getPendingTransitionForThread(i);
                        if (nextAtPreSi->enabledInState(this)) enabledThreadsAtPreSi.insert(j);
                    }
                }

                for (auto q : enabledThreadsAtPreSi) {
                    const bool inE = q == p || this->threadsRaceAfterDepth(i, q, p);

                    // If E != empty set
                    if (inE) {
                        // Add any element in E
                        // TODO: We can selectively pick here to reduce the number of times we backtrack
                        preSi->addBacktrackingThreadIfUnsearched(q);
                    }
                }
                // E is the empty set -> add every enabled thread at pre(S, i)
                for (auto q : enabledThreadsAtPreSi)
                    preSi->addBacktrackingThreadIfUnsearched(q);
            }
        }
    }

    // Restore the previous state
    {
        for (auto i = 0; i < numThreadsBeforeBacktracking; i++)
            this->nextTransitions[i] = nextTransitionsAtLastS[i];

        this->stateStackTop = stateStackTopBeforeBacktracking;
        this->transitionStackTop = transitionStackTopBeforeBacktracking;
    }
}


void
GMALState::virtuallyRunTransition(const std::shared_ptr<GMALTransition> &transition)
{
    transition->applyToState(this);
    // Do other state updates here
}
