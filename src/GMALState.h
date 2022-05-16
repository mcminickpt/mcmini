#ifndef GMAL_GMALSTATE_H
#define GMAL_GMALSTATE_H

struct GMALTransition;
struct GMALSharedTransition;
struct GMALState;
typedef GMALTransition*(*GMALSharedMemoryHandler)(const GMALSharedTransition*, void*, GMALState*);

#include "GMALObjectStore.h"
#include "GMALShared.h"
#include "GMALSharedTransition.h"
#include "GMALStateConfiguration.h"
#include "GMALStateStackItem.h"
#include "objects/GMALThread.h"
#include <typeinfo>
#include <functional>
#include <unordered_map>
#include <vector>

using TypeInfoRef = std::reference_wrapper<const std::type_info>;
using GMALType = const std::type_info&;

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

class GMALState {
private:
    GMALObjectStore objectStorage;

    /**
     * The maximum number of transitions that any given thread is allowed to execute
     */
    const GMALStateConfiguration configuration;

    tid_t nextThreadId = 0;
    std::shared_ptr<GMALTransition> nextTransitions[MAX_TOTAL_THREADS_IN_PROGRAM];

    /**
     * Maps, for each thread, data associated with the given thread
     */
    uint32_t threadDepthData[MAX_TOTAL_THREADS_IN_PROGRAM];

    /**
     * A pointer to the top-most element in the transition stack
     */
    int transitionStackTop = -1;
    std::shared_ptr<GMALTransition> transitionStack[MAX_TOTAL_TRANSITIONS_IN_PROGRAM];

    /**
     * A pointer to the top-most element in the state stack
     */
    int stateStackTop = -1;

    /**
     * The current backtracking states at this particular moment in time
     */
    std::shared_ptr<GMALStateStackItem> stateStack[MAX_TOTAL_STATES_IN_STATE_STACK];

    /**
     * A collection of shared object types that the scheduler knows how to handle
     */
    std::unordered_map<TypeInfoRef, GMALSharedMemoryHandler, TypeHasher, TypesEqual>
    sharedMemoryHandlerTypeMap;

    /* Maps thread ids to their respective object ids */
    std::unordered_map<tid_t, objid_t> threadIdMap;

private:

    bool transitionIsEnabled(const std::shared_ptr<GMALTransition>&);


    bool happensBefore(int i, int j) const;
    bool happensBeforeThread(int i, const std::shared_ptr<GMALThread>&) const;
    bool happensBeforeThread(int i, tid_t) const;
    bool threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const;

    void growStateStack();
    void growStateStackWithTransition(const std::shared_ptr<GMALTransition>&);
    void growTransitionStackRunning(const std::shared_ptr<GMALTransition>&);
    void virtuallyRunTransition(const std::shared_ptr<GMALTransition>&);


    /**
     * Inserts a backtracking point given a context of insertion (where in
     * the transition/state stacks to insert into etc.)
     */
    void dynamicallyUpdateBacktrackSetsHelper(const std::shared_ptr<GMALTransition> &S_i,
                                              const std::shared_ptr<GMALStateStackItem> &preSi,
                                              const std::shared_ptr<GMALTransition> &nextSP,
                                              const std::unordered_set<tid_t> &enabledThreadsAtPreSi,
                                              int i, int p);

    void incrementThreadTransitionCountIfNecessary(const std::shared_ptr<GMALTransition>&);
    void decrementThreadTransitionCountIfNecessary(const std::shared_ptr<GMALTransition>&);
    uint32_t totalThreadExecutionDepth() const;

public:

    GMALState(GMALStateConfiguration config) : configuration(config) {}

    tid_t getThreadRunningTransitionAtIndex(int) const;
    std::shared_ptr<GMALTransition> getPendingTransitionForThread(tid_t) const;
    std::shared_ptr<GMALTransition> getTransitionAtIndex(int) const;
    std::shared_ptr<GMALTransition> getTransitionStackTop() const;
    std::shared_ptr<GMALStateStackItem> getStateItemAtIndex(int) const;
    std::shared_ptr<GMALStateStackItem> getStateStackTop() const;

    std::shared_ptr<GMALTransition> getNextTransitionForThread(GMALThread *thread);
    std::shared_ptr<GMALTransition> getNextTransitionForThread(tid_t thread) const;
    void setNextTransitionForThread(GMALThread *, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, GMALSharedTransition*, void *);

    std::shared_ptr<GMALTransition> getFirstEnabledTransitionFromNextStack();
    std::unordered_set<tid_t> getEnabledThreadsInState();

    objid_t createNewThread();
    objid_t createNewThread(GMALThreadShadow&);
    objid_t createMainThread();
    objid_t addNewThread(GMALThreadShadow&);

    objid_t registerNewObject(const std::shared_ptr<GMALVisibleObject>& object);
    std::shared_ptr<GMALThread> getThreadWithId(tid_t id) const;

    template<typename Object>
    std::shared_ptr<Object>
    getObjectWithId(objid_t id) const
    {
        return objectStorage.getObjectWithId<Object>(id);
    }

    template<typename Object>
    std::shared_ptr<Object>
    getVisibleObjectWithSystemIdentity(GMALSystemID systemId) {
        return objectStorage.getObjectWithSystemAddress<Object>(systemId);
    }

    void simulateRunningTransition(const std::shared_ptr<GMALTransition>&, GMALSharedTransition*, void *);

    uint64_t getTransitionStackSize() const;
    uint64_t getStateStackSize() const;
    bool transitionStackIsEmpty() const;
    bool stateStackIsEmpty() const;

    // Registering new types
    void registerVisibleOperationType(GMALType, GMALSharedMemoryHandler);
    void registerVisibleObjectWithSystemIdentity(GMALSystemID, std::shared_ptr<GMALVisibleObject>);

    void dynamicallyUpdateBacktrackSets();

    bool programIsInDeadlock() const;
    bool programAchievedForwardProgressGoals() const;
    uint64_t getNumProgramThreads() const;

    bool isTargetTraceIdForGDB(trid_t) const;
    std::vector<tid_t> getThreadIdTraceOfTransitionStack() const;

    // Restarting
    void start();
    void reset();

    void reflectStateAtTransitionDepth(uint32_t);

    void moveToPreviousState();

    // TODO: De-couple priting from the state stack + transitions somehow
    /* Printing */
    void printTransitionStack() const;
    void printNextTransitions() const;
    void printForwardProgressViolations() const;
};

#endif //GMAL_GMALSTATE_H
