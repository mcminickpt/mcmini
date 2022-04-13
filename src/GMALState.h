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
#include "GMALObjectStore.h"
#include "GMALThread.h"
#include <typeinfo>
#include <functional>
#include <unordered_map>

using TypeInfoRef = std::reference_wrapper<const std::type_info>;
using GMALType = const std::type_info&;

struct Hasher {
    std::size_t operator()(TypeInfoRef code) const
    {
        return code.get().hash_code();
    }
};

struct EqualTo {
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

    int t_stack_top = -1;
    std::shared_ptr<GMALTransition> transitionStack[MAX_TOTAL_TRANSITIONS_IN_PROGRAM];

    int s_stack_top = -1;
    std::shared_ptr<GMALTransition> stateStack[MAX_TOTAL_STATES_IN_STATE_STACK];

    std::unordered_map<TypeInfoRef, GMALSharedMemoryHandler, Hasher, EqualTo> sharedMemoryHandlerTypeMap;

public:
    std::shared_ptr<GMALTransition> getSlotForThread(GMALThread *thread);
    std::shared_ptr<GMALTransition> getSlotForThread(tid_t thread);

    void setNextTransitionForThread(GMALThread *, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, std::shared_ptr<GMALTransition>);
    void setNextTransitionForThread(tid_t, GMALSharedTransition*);

    tid_t createNewThread();
    tid_t createMainThread();
    objid_t registerNewObject(GMALVisibleObject *object);
    objid_t softRegisterNewObject(GMALVisibleObject *object);

    template<typename T> std::shared_ptr<T> getObjectWithId(objid_t id) const;
    std::shared_ptr<GMALThread> getThreadWithId(tid_t id) const;

    uint64_t getTransitionStackSize() const;
    uint64_t getStateStackSize() const;

    // Registering new types
    void registerVisibleOperationType(GMALType, GMALSharedMemoryHandler);
};

#endif //GMAL_GMALSTATE_H
