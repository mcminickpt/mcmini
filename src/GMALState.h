#ifndef GMAL_GMALSTATE_H
#define GMAL_GMALSTATE_H

struct GMALTransition;
struct GMALSharedTransition;
struct GMALState;
typedef GMALTransition*(*GMALSharedMemoryHandler)(void*, const GMALState&);

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

struct PointerHasher {
    std::size_t operator()(void *code) const
    {
        return (std::size_t)(code);
    }
};

struct PointersEqual {
    bool operator()(void *lhs, void *rhs) const
    {
        return lhs == rhs;
    }
};

class GMALState {
private:
    GMALObjectStore objectStorage;

//    std::unordered_map<void*, objid_t, PointerHasher, PointersEqual> systemVisibleObjectMap;

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

    std::unordered_map<TypeInfoRef, GMALSharedMemoryHandler, TypeHasher, TypesEqual> sharedMemoryHandlerTypeMap;

private:

    tid_t getThreadRunningTransitionAtIndex(int) const;
    std::shared_ptr<GMALTransition> getTransitionAtIndex(int) const;
    std::shared_ptr<GMALTransition> getPendingTransitionForThread(tid_t) const;
    std::shared_ptr<GMALStateStackItem> getStateItemAtIndex(int) const;

    bool happensBefore(int i, int j) const;
    bool happensBeforeThread(int i, const std::shared_ptr<GMALThread>&) const;
    bool happensBeforeThread(int i, tid_t) const;
    bool threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const;

public:
    std::shared_ptr<GMALTransition> getSlotForThread(GMALThread *thread);
    std::shared_ptr<GMALTransition> getSlotForThread(tid_t thread);
    std::shared_ptr<GMALTransition> getFirstEnabledTransitionFromNextStack();

    tid_t createNewThread();
    tid_t createMainThread();

    objid_t registerNewObject(GMALVisibleObject *object);
    objid_t softRegisterNewObject(GMALVisibleObject *object);

    template<typename T> std::shared_ptr<T> getObjectWithId(objid_t id) const;
    std::shared_ptr<GMALThread> getThreadWithId(tid_t id) const;

    void setNextTransitionForThread(GMALThread *, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, GMALSharedTransition*);

    void virtuallyRunTransition(const std::shared_ptr<GMALTransition>&);

    /**
     * Computes the height of the transition stack
     * @return the number of elements in the transition stack
     */
    uint64_t getTransitionStackSize() const;

    uint64_t getStateStackSize() const;
    uint64_t getNumProgramThreads() const;


    // Registering new types
    void registerVisibleOperationType(GMALType, GMALSharedMemoryHandler);

    void dynamicallyUpdateBacktrackSets();
};

#endif //GMAL_GMALSTATE_H
