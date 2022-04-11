#ifndef GMAL_GMALTHREAD_H
#define GMAL_GMALTHREAD_H

#include <pthread.h>
#include "GMALVisibleObject.h"
#include "GMALShared.h"

struct GMALThreadShadow {
    void * volatile arg;
    thread_routine startRoutine;
    pthread_t systemIdentity;
    enum GMALThreadState {
        sleeping, alive, dead
    } state;
};

struct GMALThread : public GMALVisibleObject {
private:
    GMALThreadShadow threadShadow;
public:
    /* Threads are unique in that they have *two* ids */
    const tid_t tid;

    inline
    GMALThread(tid_t tid, void *arg, thread_routine startRoutine, pthread_t systemIdentity) : GMALVisibleObject(), threadShadow(GMALThreadShadow()), tid(tid)
    {
        threadShadow.arg = arg;
        threadShadow.systemIdentity = systemIdentity;
        threadShadow.startRoutine = startRoutine;
    }
    inline explicit GMALThread(tid_t tid, GMALThreadShadow shadow) : GMALVisibleObject(), threadShadow(shadow), tid(tid) {}
};

#endif //GMAL_GMALTHREAD_H
