#ifndef MC_MCSTATE_H
#define MC_MCSTATE_H

struct MCTransition;
struct MCSharedTransition;
struct MCState;
struct MCStateStackItem;
typedef MCTransition*(*MCSharedMemoryHandler)(const MCSharedTransition*, void*, MCState*);

#include "MCObjectStore.h"
#include "MCShared.h"
#include "MCSharedTransition.h"
#include "MCStateConfiguration.h"
#include "MCStateStackItem.h"
#include "MCClockVector.hpp"
#include "MCThreadData.hpp"
#include "objects/MCThread.h"
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
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

    /**
     * @brief 
     * 
     */
    MCObjectStore objectStorage;


    /**
     * @brief 
     * 
     */
    const MCStateConfiguration configuration;

    /**
     * @brief 
     * 
     */
    tid_t nextThreadId = 0;
    std::shared_ptr<MCTransition> nextTransitions[MAX_TOTAL_THREADS_IN_PROGRAM];

    /**
     * @brief 
     * 
     */
    MCThreadData threadData[MAX_TOTAL_THREADS_IN_PROGRAM];

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

    bool transitionIsEnabled(const MCTransition&);

    bool happensBefore(int i, int j) const;
    bool happensBeforeThread(int i, const std::shared_ptr<MCThread>&) const;
    bool happensBeforeThread(int i, tid_t) const;
    bool threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const;

    void growStateStack();
    void growStateStackWithTransition(const MCTransition&);
    void growTransitionStackRunning(const MCTransition&);
    void virtuallyRunTransition(const MCTransition&);
    void virtuallyApplyTransition(const MCTransition&);
    void virtuallyRerunTransitionAtIndex(int);
    MCClockVector transitionStackMaxClockVector(const MCTransition&);
    MCClockVector clockVectorForTransitionAtIndex(int i) const;

    /**
     * Inserts a backtracking point given a context of insertion (where in
     * the transition/state stacks to insert into etc.)
     */
    bool dynamicallyUpdateBacktrackSetsHelper(const
      MCTransition &S_i,
      MCStateStackItem &preSi,
      const MCTransition &nextSP,
      const std::unordered_set<tid_t> &enabledThreadsAtPreSi,
      int i, int p);

    void incrementThreadTransitionCountIfNecessary(const MCTransition&);
    void updateLatestExecutionPointForThread(tid_t, uint32_t);
    uint32_t totalThreadExecutionDepth() const;
    
    MCThreadData &getThreadDataForThread(tid_t tid);
    const MCThreadData &getThreadDataForThread(tid_t tid) const;

public:

    MCState(MCStateConfiguration config) : configuration(config) {}

    tid_t getThreadRunningTransitionAtIndex(int) const;

    MCTransition &getPendingTransitionForThread(tid_t) const;
    MCTransition &getTransitionAtIndex(int) const;
    MCTransition &getTransitionStackTop() const;
    MCStateStackItem &getStateItemAtIndex(int) const;
    MCStateStackItem &getStateStackTop() const;

    MCTransition &getNextTransitionForThread(MCThread *thread);
    MCTransition &getNextTransitionForThread(tid_t thread) const;

    void setNextTransitionForThread(MCThread *, std::shared_ptr<MCTransition>);
    void setNextTransitionForThread(tid_t, std::shared_ptr<MCTransition>);
    void setNextTransitionForThread(tid_t, MCSharedTransition*, void *);

    const MCTransition *getFirstEnabledTransitionFromNextStack();
    std::unordered_set<tid_t> computeEnabledThreads();

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

    void simulateRunningTransition(const MCTransition&, MCSharedTransition*, void *);

    uint64_t getTransitionStackSize() const;
    uint64_t getStateStackSize() const;
    bool transitionStackIsEmpty() const;
    bool stateStackIsEmpty() const;

    // Registering new types
    void registerVisibleOperationType(MCType, MCSharedMemoryHandler);
    void registerVisibleObjectWithSystemIdentity(MCSystemID, std::shared_ptr<MCVisibleObject>);

    void dynamicallyUpdateBacktrackSets();

    bool programIsInDeadlock() const;
    bool programAchievedForwardProgressGoals() const;
    bool programHasADataRaceWithNewTransition(const MCTransition&) const;

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
};

#endif //MC_MCSTATE_H
