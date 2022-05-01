#ifndef GMAL_GMALBARRIER_H
#define GMAL_GMALBARRIER_H

#include "GMALVisibleObject.h"
#include <unordered_set>

struct GMALBarrierShadow {
    pthread_barrier_t *systemIdentity;
    const unsigned int waitCount;
    enum GMALBarrierState {
        undefined, initialized, destroyed
    } state;

    GMALBarrierShadow(pthread_barrier_t *systemIdentity, unsigned int waitCount) : systemIdentity(systemIdentity), waitCount(waitCount), state(undefined) {}
};

struct GMALBarrier : public GMALVisibleObject {
private:
    std::unordered_set<tid_t> threadsWaitingOnBarrier;
    GMALBarrierShadow barrierShadow;
    inline explicit GMALBarrier(GMALBarrierShadow shadow, objid_t id) : GMALVisibleObject(id), barrierShadow(shadow) {}

public:
    inline explicit GMALBarrier(GMALBarrierShadow shadow) : GMALVisibleObject(), barrierShadow(shadow) {}
    inline GMALBarrier(const GMALBarrier &barrier) : GMALVisibleObject(barrier.getObjectId()), barrierShadow(barrier.barrierShadow) {}

    std::shared_ptr<GMALVisibleObject> copy() override;
    GMALSystemID getSystemId() override;

    bool operator ==(const GMALBarrier&) const;
    bool operator !=(const GMALBarrier&) const;

    bool wouldBlockIfWaitedOn();
    void deinit();
    void init();
    void wait(tid_t);
    void leave(tid_t);
    bool isWaitingOnBarrier(tid_t);

    void print() override;
};

#endif //GMAL_GMALBARRIER_H
