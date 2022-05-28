#ifndef MC_MCTHREAD_H
#define MC_MCTHREAD_H

#include <pthread.h>
#include "MCVisibleObject.h"
#include "MCShared.h"
#include <memory>

struct MCThreadShadow {
    void * arg;
    thread_routine startRoutine;
    pthread_t systemIdentity;
    enum MCThreadState {
        embryo, alive, sleeping, dead
    } state;

    MCThreadShadow(void *arg, thread_routine startRoutine, pthread_t systemIdentity) :
            arg(arg), startRoutine(startRoutine), systemIdentity(systemIdentity), state(embryo) {}
};

struct MCThread : public MCVisibleObject {
private:
    MCThreadShadow threadShadow;

    bool _hasEncounteredThreadProgressGoal = false;

public:
    /* Threads are unique in that they have *two* ids */
    const tid_t tid;

    /**
     * Whether or not the thread is currently executing within
     * the context of a "GOAL() critical section".
     *
     * To support the detection of starvation while model checking,
     * it is necessary to add critical sections of code within which
     * the thread execution limit of the thread is ignored. When a
     * thread enters such a critical section, this value is set to
     * `true` and is read to allow the thread to continue to execute
     * even if the thread has reached its execution limit
     */
    bool isInThreadCriticalSection = false;

    inline
    MCThread(tid_t tid, void *arg, thread_routine startRoutine, pthread_t systemIdentity) :
    MCVisibleObject(), threadShadow(MCThreadShadow(arg, startRoutine, systemIdentity)), tid(tid) {}

    inline explicit MCThread(tid_t tid, MCThreadShadow shadow) : MCVisibleObject(), threadShadow(shadow), tid(tid) {}
    inline MCThread(const MCThread &thread)
    : MCVisibleObject(thread.getObjectId()), threadShadow(thread.threadShadow), tid(thread.tid),
      _hasEncounteredThreadProgressGoal(thread._hasEncounteredThreadProgressGoal) {}

    std::shared_ptr<MCVisibleObject> copy() override;
    MCSystemID getSystemId() override;

    bool operator ==(const MCThread&) const;

    // Managing thread state
    MCThreadShadow::MCThreadState getState() const;

    bool enabled() const;
    bool isAlive() const;
    bool isDead() const;

    void awaken();
    void sleep();

    void regenerate();
    void die();
    void spawn();
    void despawn();

    inline void
    markEncounteredThreadProgressPost()
    {
        _hasEncounteredThreadProgressGoal = true;
    }

    inline bool
    hasEncounteredThreadProgressGoal() const
    {
        return _hasEncounteredThreadProgressGoal;
    }
};

#endif //MC_MCTHREAD_H
