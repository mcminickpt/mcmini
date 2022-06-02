#ifndef MC_MCSTATE_H
#define MC_MCSTATE_H

struct MCTransition;
struct MCSharedTransition;
struct MCState;
typedef MCTransition*(*MCSharedMemoryHandler)(const MCSharedTransition*, void*, MCState*);

#include "MCObjectStore.h"
#include "MCShared.h"
#include "MCSharedTransition.h"
#include "MCStateConfiguration.h"
#include "MCStateStackItem.h"
#include "objects/MCThread.h"
#include <typeinfo>
#include <functional>
#include <unordered_map>
#include <vector>

using TypeInfoRef = std::reference_wrapper<const std::type_info>;
using MCType = const std::type_info&;

struct TypeHasher {
    std::size_t operator()(TypeInfoRef code) const
    {
        return code.get().hash_code();
    }
};

struct TypesEqual {
    bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const
    {
        return lhs.get() == rhs.get();
    }
};

class MCState {
private:
    MCObjectStore objectStorage;

    /**
     * The maximum number of transitions that any given thread is allowed to execute
     */
    const MCStateConfiguration configuration;

    tid_t nextThreadId = 0;
    std::shared_ptr<MCTransition> nextTransitions[MAX_TOTAL_THREADS_IN_PROGRAM];

    /**
     * Maps, for each thread, data associated with the given thread
     */
    uint32_t currentThreadDepthData[MAX_TOTAL_THREADS_IN_PROGRAM];
    uint32_t maxThreadDepthData[MAX_TOTAL_THREADS_IN_PROGRAM];
    uint32_t transitionsSinceLastCandidateStarvedThread[MAX_TOTAL_THREADS_IN_PROGRAM];
    uint32_t threadDepthAtLastCandidateStarvingPoint[MAX_TOTAL_THREADS_IN_PROGRAM];


    /**
     * A pointer to the top-most element in the transition stack
     */
    int transitionStackTop = -1;
    std::shared_ptr<MCTransition> transitionStack[MAX_TOTAL_TRANSITIONS_IN_PROGRAM];

    /**
     * A pointer to the top-most element in the state stack
     */
    int stateStackTop = -1;

    /**
     * The current backtracking states at this particular moment in time
     */
    std::shared_ptr<MCStateStackItem> stateStack[MAX_TOTAL_STATES_IN_STATE_STACK];

    /**
     * A collection of shared object types that the scheduler knows how to handle
     */
    std::unordered_map<TypeInfoRef, MCSharedMemoryHandler, TypeHasher, TypesEqual>
    sharedMemoryHandlerTypeMap;

    /* Maps thread ids to their respective object ids */
    std::unordered_map<tid_t, objid_t> threadIdMap;

private:

    bool threadHasExhaustedTransitionBudget(tid_t) const;
    bool transitionIsEnabled(const std::shared_ptr<MCTransition>&) const;

    bool happensBefore(int i, int j) const;
    bool happensBeforeThread(int i, const std::shared_ptr<MCThread>&) const;
    bool happensBeforeThread(int i, tid_t) const;
    bool threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const;

    void growStateStack();
    void growStateStackWithTransition(const std::shared_ptr<MCTransition>&);
    void growTransitionStackRunning(const std::shared_ptr<MCTransition>&);
    void virtuallyRunTransition(const std::shared_ptr<MCTransition>&);
    void dispatchExtraLivenessTransitionsToThreadsIfNecessary();

    /**
     * Inserts a backtracking point given a context of insertion (where in
     * the transition/state stacks to insert into etc.)
     */
    void dynamicallyUpdateBacktrackSetsHelper(const std::shared_ptr<MCTransition> &S_i,
                                              const std::shared_ptr<MCStateStackItem> &preSi,
                                              const std::shared_ptr<MCTransition> &nextSP,
                                              const std::unordered_set<tid_t> &enabledThreadsAtPreSi,
                                              int i, int p);

    void incrementThreadTransitionCountIfNecessary(const std::shared_ptr<MCTransition>&);
    void incrementTransitionsSinceNewCandidateStarvedThreadIfNecessary(const std::shared_ptr<MCTransition>&);
    uint32_t totalThreadExecutionDepth() const;

public:

    MCState(MCStateConfiguration config) : configuration(config) {
        for (unsigned int & i : this->maxThreadDepthData) {
            i = config.maxThreadExecutionDepth;
        }

        memset(currentThreadDepthData, 0, sizeof(currentThreadDepthData));
        memset(transitionsSinceLastCandidateStarvedThread, 0, sizeof(transitionsSinceLastCandidateStarvedThread));
    }

    tid_t getThreadRunningTransitionAtIndex(int) const;

    std::shared_ptr<MCTransition> getPendingTransitionForThread(tid_t) const;
    std::shared_ptr<MCTransition> getTransitionAtIndex(int) const;
    std::shared_ptr<MCTransition> getTransitionStackTop() const;
    std::shared_ptr<MCStateStackItem> getStateItemAtIndex(int) const;
    std::shared_ptr<MCStateStackItem> getStateStackTop() const;

    std::shared_ptr<MCTransition> getNextTransitionForThread(MCThread *thread);
    std::shared_ptr<MCTransition> getNextTransitionForThread(tid_t thread) const;
    uint32_t getMaximumExecutionDepthForThread(tid_t) const;
    uint32_t getCurrentExecutionDepthForThread(tid_t) const;
    uint32_t getExecutionDepthForThreadWhenLastStarvedAndBlocked(tid_t) const;
    void setNextTransitionForThread(MCThread *, std::shared_ptr<MCTransition>);
    void setNextTransitionForThread(tid_t, std::shared_ptr<MCTransition>);
    void setNextTransitionForThread(tid_t, MCSharedTransition*, void *);
    void setMaximumExecutionDepthForThread(tid_t, uint32_t);

    std::shared_ptr<MCTransition> getFirstEnabledTransitionFromNextStack();
    std::unordered_set<tid_t> getEnabledThreadsInState();

    objid_t createNewThread();
    objid_t createNewThread(MCThreadShadow&);
    objid_t createMainThread();
    objid_t addNewThread(MCThreadShadow&);

    objid_t registerNewObject(const std::shared_ptr<MCVisibleObject>& object);
    std::shared_ptr<MCThread> getThreadWithId(tid_t id) const;

    template<typename Object>
    std::shared_ptr<Object>
    getObjectWithId(objid_t id) const
    {
        return objectStorage.getObjectWithId<Object>(id);
    }

    template<typename Object>
    std::shared_ptr<Object>
    getVisibleObjectWithSystemIdentity(MCSystemID systemId) {
        return objectStorage.getObjectWithSystemAddress<Object>(systemId);
    }

    void simulateRunningTransition(const std::shared_ptr<MCTransition>&, MCSharedTransition*, void *);

    uint64_t getTransitionStackSize() const;
    uint64_t getStateStackSize() const;
    bool transitionStackIsEmpty() const;
    bool stateStackIsEmpty() const;

    // Registering new types
    void registerVisibleOperationType(MCType, MCSharedMemoryHandler);
    void registerVisibleObjectWithSystemIdentity(MCSystemID, std::shared_ptr<MCVisibleObject>);

    void dynamicallyUpdateBacktrackSets();

    bool hasMaybeStarvedThread() const;
    bool hasMaybeStarvedAndBlockedThread() const;
    tid_t getCandidateStarvedThread() const;
    void setNewCandidateStarvedThread(tid_t);

    bool programIsInDeadlock() const;
    bool programAchievedForwardProgressGoals() const;
    bool programMaybeAchievedForwardProgressGoals() const;
    bool programHasADataRaceWithNewTransition(const std::shared_ptr<MCTransition>&) const;

    MCStateConfiguration getConfiguration() const;

    uint64_t getNumProgramThreads() const;

    bool isTargetTraceIdForGDB(trid_t) const;
    bool isTargetTraceIdForStackContents(trid_t) const;
    std::vector<tid_t> getThreadIdTraceOfTransitionStack() const;

    // Restarting
    void start();
    void reset();

    void reflectStateAtTransitionDepth(uint32_t);

    // TODO: De-couple priting from the state stack + transitions somehow
    /* Printing */
    void printTransitionStack() const;
    void printNextTransitions() const;
    void printForwardProgressViolations() const;
    void printThreadExecutionDepths() const;
};

#endif //MC_MCSTATE_H
