#ifndef MC_MCMUTEX_H
#define MC_MCMUTEX_H

#include "MCVisibleObject.h"
#include "misc/MCOptional.h"

struct MCMutexShadow {
    pthread_mutex_t *systemIdentity;
    enum State {
        undefined, unlocked, locked, destroyed
    } state;
    explicit MCMutexShadow(pthread_mutex_t *systemIdentity) : systemIdentity(systemIdentity), state(undefined) {}
};

struct MCMutex : public MCVisibleObject {
private:

    MCMutexShadow mutexShadow;
//    inline explicit MCMutex(MCMutexShadow shadow, objid_t id) : MCVisibleObject(id), mutexShadow(shadow) {}

public:
    inline explicit MCMutex(MCMutexShadow shadow) : MCVisibleObject(), mutexShadow(shadow) {}
    inline MCMutex(const MCMutex &mutex) : MCVisibleObject(mutex.getObjectId()), mutexShadow(mutex.mutexShadow) {}

    std::shared_ptr<MCVisibleObject> copy() override;
    MCSystemID getSystemId() override;

    bool operator ==(const MCMutex&) const;
    bool operator !=(const MCMutex&) const;

    bool canAcquire(tid_t) const;
    bool isLocked() const;
    bool isUnlocked() const;
    bool isDestroyed() const;

    void lock(tid_t);
    void unlock();
    void init();
    void deinit();
};

#endif //MC_MCMUTEX_H
