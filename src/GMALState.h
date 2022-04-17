#ifndef GMAL_GMALSTATE_H
#define GMAL_GMALSTATE_H

struct GMALTransition;
struct GMALSharedTransition;
struct GMALState;
typedef GMALTransition*(*GMALSharedMemoryHandler)(const GMALSharedTransition*, void*, GMALState*);

#include <stdint.h>
#include "GMALShared.h"
#include "GMALMap.h"
#include "GMALSharedTransition.h"
#include "GMALStateStackItem.h"
#include "GMALObjectStore.h"
#include "GMALThread.h"
#include <typeinfo>
#include <functional>
#include <unordered_map>

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

    tid_t nextThreadId = 0;
    std::shared_ptr<GMALTransition> nextTransitions[MAX_TOTAL_THREADS_IN_PROGRAM];

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

    void growStateStack();
    void virtuallyRevertTransitionForBacktracking(const std::shared_ptr<GMALTransition>&);

    std::shared_ptr<GMALTransition> getTransitionAtIndex(int) const;
    std::shared_ptr<GMALTransition> getTransitionStackTop() const;
    std::shared_ptr<GMALTransition> getPendingTransitionForThread(tid_t) const;

    bool happensBefore(int i, int j) const;
    bool happensBeforeThread(int i, const std::shared_ptr<GMALThread>&) const;
    bool happensBeforeThread(int i, tid_t) const;
    bool threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const;

public:
    std::shared_ptr<GMALTransition> getNextTransitionForThread(GMALThread *thread);
    std::shared_ptr<GMALTransition> getNextTransitionForThread(tid_t thread);
    std::shared_ptr<GMALTransition> getFirstEnabledTransitionFromNextStack();

    bool programIsInDeadlock() const;

    objid_t createNewThread();
    objid_t createNewThread(GMALThreadShadow&);
    objid_t createMainThread();

    objid_t addNewThread(GMALThreadShadow&);

    objid_t registerNewObject(const std::shared_ptr<GMALVisibleObject>& object);

    template<typename T> std::shared_ptr<T> getObjectWithId(objid_t id) const;
    std::shared_ptr<GMALThread> getThreadWithId(tid_t id) const;

    void setNextTransitionForThread(GMALThread *, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, GMALSharedTransition*, void *);

    void growStateStackWithTransition(const std::shared_ptr<GMALTransition>&);
    void growTransitionStackRunning(const std::shared_ptr<GMALTransition>&);
    void simulateRunningTransition(const std::shared_ptr<GMALTransition>&);
    void virtuallyRunTransition(const std::shared_ptr<GMALTransition>&);
    void virtuallyRevertTransition(const std::shared_ptr<GMALTransition>&);

    /**
     * Computes the height of the transition stack
     * @return the number of elements in the transition stack
     */
    uint64_t getTransitionStackSize() const;
    uint64_t getStateStackSize() const;

    bool transitionStackIsEmpty() const;
    bool stateStackIsEmpty() const;
    uint64_t getNumProgramThreads() const;

    tid_t getThreadRunningTransitionAtIndex(int) const;
    std::shared_ptr<GMALStateStackItem> getStateItemAtIndex(int) const;
    std::shared_ptr<GMALStateStackItem> getStateStackTop() const;

    // Registering new types
    void registerVisibleOperationType(GMALType, GMALSharedMemoryHandler);
    void registerVisibleObjectWithSystemIdentity(GMALSystemID, std::shared_ptr<GMALVisibleObject>);

    template<typename Object>
    std::shared_ptr<Object>
    getVisibleObjectWithSystemIdentity(GMALSystemID systemId) {
        return objectStorage.getObjectWithSystemAddress<Object>(systemId);
    }

    void dynamicallyUpdateBacktrackSets();

    // Restarting
    void start();
    void reset();
    void moveToPreviousState();
};

#endif //GMAL_GMALSTATE_H
