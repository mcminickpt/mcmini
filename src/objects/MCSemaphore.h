#ifndef MC_MCSEMAPHORE_H
#define MC_MCSEMAPHORE_H

#include <semaphore.h>
#include <deque>
#include "MCVisibleObject.h"

struct MCSemaphoreShadow {
    sem_t *sem;
    unsigned int count;
    enum State {
        undefined, initialized, destroyed
    } state;

    MCSemaphoreShadow(sem_t *sem, unsigned int count) : sem(sem), count(count), state(undefined) {}
};

class MCSemaphore : public MCVisibleObject {
private:

    /* */
    std::deque<tid_t> waitingQueue;

    MCSemaphoreShadow semShadow;
    inline explicit MCSemaphore(MCSemaphoreShadow shadow, objid_t id) : MCVisibleObject(id), semShadow(shadow) {}

public:
    inline explicit MCSemaphore(MCSemaphoreShadow shadow) : MCVisibleObject(), semShadow(shadow) {}
    inline MCSemaphore(const MCSemaphore &sem) : MCVisibleObject(sem.getObjectId()), semShadow(sem.semShadow) {}

    std::shared_ptr<MCVisibleObject> copy() override;
    MCSystemID getSystemId() override;

    bool operator ==(const MCSemaphore&) const;
    bool operator !=(const MCSemaphore&) const;

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


#endif //MC_MCSEMAPHORE_H
