#ifndef GMAL_GMALTHREAD_H
#define GMAL_GMALTHREAD_H

#include <pthread.h>
#include "GMALVisibleObject.h"
#include "GMALShared.h"
#include <memory>

struct GMALThreadShadow {
    void * arg;
    thread_routine startRoutine;
    pthread_t systemIdentity;
    enum GMALThreadState {
        embryo, alive, sleeping, dead
    } state;

    GMALThreadShadow(void *arg, thread_routine startRoutine, pthread_t systemIdentity) :
            arg(arg), startRoutine(startRoutine), systemIdentity(systemIdentity), state(embryo) {}
};

struct GMALThread : public GMALVisibleObject {
private:
    GMALThreadShadow threadShadow;
public:
    /* Threads are unique in that they have *two* ids */
    const tid_t tid;

    inline
    GMALThread(tid_t tid, void *arg, thread_routine startRoutine, pthread_t systemIdentity) :
    GMALVisibleObject(), threadShadow(GMALThreadShadow(arg, startRoutine, systemIdentity)), tid(tid) {}

    inline explicit GMALThread(tid_t tid, GMALThreadShadow shadow) : GMALVisibleObject(), threadShadow(shadow), tid(tid) {}
    inline GMALThread(const GMALThread &thread) : GMALThread(thread.tid, thread.threadShadow) {}

    std::shared_ptr<GMALVisibleObject> copy() override;
    GMALSystemID getSystemId() override;

    bool operator ==(const GMALThread&) const;

    // Managing thread state
    GMALThreadShadow::GMALThreadState getState() const;

    bool enabled();
    bool isAlive() const;
    bool isDead() const;

    void awaken();
    void sleep();

    void regenerate();
    void die();
    void spawn();
    void despawn();
};

#endif //GMAL_GMALTHREAD_H
