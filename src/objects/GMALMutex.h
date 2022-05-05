#ifndef GMAL_GMALMUTEX_H
#define GMAL_GMALMUTEX_H

#include "GMALVisibleObject.h"

struct GMALMutexShadow {
    pthread_mutex_t *systemIdentity;
    enum GMALMutexState {
        undefined, unlocked, locked, destroyed
    } state;
    explicit GMALMutexShadow(pthread_mutex_t *systemIdentity) : systemIdentity(systemIdentity), state(undefined) {}
};

struct GMALMutex : public GMALVisibleObject {
private:
    GMALMutexShadow mutexShadow;
    inline explicit GMALMutex(GMALMutexShadow shadow, objid_t id) : GMALVisibleObject(id), mutexShadow(shadow) {}

public:
    inline explicit GMALMutex(GMALMutexShadow shadow) : GMALVisibleObject(), mutexShadow(shadow) {}
    inline GMALMutex(const GMALMutex &mutex) : GMALVisibleObject(mutex.getObjectId()), mutexShadow(mutex.mutexShadow) {}

    std::shared_ptr<GMALVisibleObject> copy() override;
    GMALSystemID getSystemId() override;

    bool operator ==(const GMALMutex&) const;
    bool operator !=(const GMALMutex&) const;

    bool canAcquire(tid_t) const;
    bool isLocked() const;
    bool isUnlocked() const;
    bool isDestroyed() const;

    void lock();
    void unlock();
    void init();
    void deinit();
};

#endif //GMAL_GMALMUTEX_H
