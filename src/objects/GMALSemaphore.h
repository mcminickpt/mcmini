#ifndef GMAL_GMALSEMAPHORE_H
#define GMAL_GMALSEMAPHORE_H

#include <semaphore.h>
#include <deque>
#include "GMALVisibleObject.h"

struct GMALSemaphoreShadow {
    sem_t *sem;
    unsigned int count;
    enum State {
        undefined, initialized, destroyed
    } state;

    GMALSemaphoreShadow(sem_t *sem, unsigned int count) : sem(sem), count(count), state(undefined) {}
};

class GMALSemaphore : public GMALVisibleObject {
private:

    /* */
    std::deque<tid_t> waitingQueue;

    GMALSemaphoreShadow semShadow;
    inline explicit GMALSemaphore(GMALSemaphoreShadow shadow, objid_t id) : GMALVisibleObject(id), semShadow(shadow) {}

public:
    inline explicit GMALSemaphore(GMALSemaphoreShadow shadow) : GMALVisibleObject(), semShadow(shadow) {}
    inline GMALSemaphore(const GMALSemaphore &sem) : GMALVisibleObject(sem.getObjectId()), semShadow(sem.semShadow) {}

    std::shared_ptr<GMALVisibleObject> copy() override;
    GMALSystemID getSystemId() override;

    bool operator ==(const GMALSemaphore&) const;
    bool operator !=(const GMALSemaphore&) const;

    unsigned int getCount() const;
    bool isDestroyed() const;
    bool wouldBlockIfWaitedOn();
    bool threadCanExit(tid_t);
    void leaveWaitingQueue(tid_t);
    void enterWaitingQueue(tid_t);

    void deinit();
    void init();
    void post();
    void wait();
};


#endif //GMAL_GMALSEMAPHORE_H
