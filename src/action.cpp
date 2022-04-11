//
// Created by parallels on 4/6/22.
//

#include "GMALShared.h"


using namespace std;


// GOAL: Make adding new types easy -> fixed set of operations

#include <stdint.h>

struct GMALObjectStore;
struct GMALState;

//struct GMALMutex : public GMALVisibleObject {
//private:
//    pthread_mutex_t *mutex;
//    enum class GMALMutexState {
//        locked, unlocked, unknown, destroyed
//    } state;
//public:
//    inline GMALMutex(pthread_mutex_t *mutex) : GMALVisibleObject(), mutex(mutex), state(GMALMutexState::unknown) {}
//};


const GMALTypeID kThreadTypeID = 0;
const GMALTypeID kThreadStartTypeID = 0;



// How to add a type to GMAL:
//
// - Define the POD simple C shadow struct + unique identifier for the type
//
//
// - Define the POD visible operations (what type is it + what is it doing)
//

// GOALS:
//
// 1. Prevent the need to modify the source code (good API design)
// 2. Define the objects, the transitions, how they're related to the
// other transitions, etc. and that's it (plus unique identifiers for
// each transition, object, etc.) <-
// 3. Tell the system to track these visible operations
//

class GMALState;

struct GMALTransition {
protected:
    GMALRef<GMALThread> thread;

    friend GMALState;
public:
    GMALTransition(GMALRef<GMALThread> thread) : thread(std::move(thread)) {}
    virtual void applyToState(GMALState &state) {}
    virtual void unapplyToState(GMALState &state) {}
    virtual bool enabledInState(const GMALState &state) { return true; }
    virtual bool conenabledWith(const GMALTransition &other) { return true; }
    virtual bool dependentWith(const GMALTransition &other) { return true; }
};

class GMALState {
private:
    GMALObjectStore objectStorage;
    GMALTransition *nextTransitions[MAX_TOTAL_THREADS_PER_SCHEDULE];

    // Maps thread object IDs to their storage in the nextTransitions array
    GMALMap<objid_t, GMALTransition*> threadMap;
public:

    inline GMALTransition*
    getSlotForThread(GMALRef<GMALThread> thread)
    {
        return threadMap.valueForKey(thread.getRef()->id);
    }

    inline GMALTransition*
    getSlotForThread(GMALThread *thread)
    {
        return nextTransitions[0];
//        return threadMap.valueForKey(thread->id);
    }

    inline objid_t
    registerNewObject(GMALVisibleObject *object)
    {
        return objectStorage.registerNewObject(object);
    }

    inline objid_t
    softRegisterNewObject(GMALVisibleObject *object)
    {
        return objectStorage.registerNewObject(object);
    }

    inline std::shared_ptr<GMALVisibleObject>
    getObjectWithId(objid_t id)
    {
        return objectStorage.getObjectWithId(id);
    }

    inline objid_t
    createNewThread()
    {
        auto thread = new GMALThread(nullptr, nullptr, pthread_self());
        objid_t objid = this->registerNewObject(thread);
        return objid;
    }

    inline objid_t
    createMainThread()
    {
        objid_t mainThreadId = this->createNewThread();
        GMAL_ASSERT(mainThreadId == 0);
        return mainThreadId;
    }
};

// 8 bytes
GMALTransition *shm_child_result; // 0x400

// GMALTransition *     // 0x408
// |
// |
// ^
GMALTransition *shm_child_result2;

struct GMALThreadTransition : public GMALTransition {
public:
    GMALRef<GMALThread> target;
    GMALThreadTransition(GMALRef<GMALThread> running, GMALRef<GMALThread> target) : GMALTransition(running), target(target) {}
    GMALThreadTransition(GMALRef<GMALThread> runningThread) : GMALThreadTransition(runningThread, runningThread) {}
};

struct GMALThreadStart : public GMALThreadTransition {
public:
    inline explicit GMALThreadStart(GMALRef<GMALThread> thread) : GMALThreadTransition(std::move(thread)) {}

    bool
    dependentWith(const GMALTransition &other) override
    {


        return true;
    }

    void
    applyToState(GMALState &state) override
    {

    }

    void
    unapplyToState(GMALState &state) override
    {

    }

};


class GMALTransitionFactory final {
public:
    GMALTransitionFactory() = delete;

    static inline void
    createInitialTransitionForThread(GMALThread *thread, GMALTransition *transition)
    {
        *transition = GMALThreadStart(GMAL_PASS_DYNAMIC<GMALThread>(thread));
    }
};

static GMALState gmalProgramState;

void
gmal_init()
{
    gmalProgramState.createMainThread();

    auto thread = gmalProgramState.getObjectWithId(0);

    GMALTransition *slotForThread = gmalProgramState.getSlotForThread(thread);
    GMALTransitionFactory::createInitialTransitionForThread(thread, slotForThread);

    GMALMutex mut = GMALMutex(nullptr);
    objid_t mutid = gmalProgramState.registerNewObject(mut);

    GMALMutex *mt = (GMALMutex*)gmalProgramState.getObjectWithId(mutid);

    GMALThreadStart *start = (GMALThreadStart *)slotForThread;

    GMALRef<GMALThread> a = start->target;

    GMALThread *t = a.getRef();

    GMALThread random = GMALThread(nullptr, nullptr, pthread_self());

    *shm_child_result = GMALThreadStart(GMAL_PASS_STATIC(&random));
}
