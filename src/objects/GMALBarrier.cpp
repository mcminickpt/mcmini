#include "GMALBarrier.h"

GMALSystemID
GMALBarrier::getSystemId()
{
    return this->barrierShadow.systemIdentity;
}

std::shared_ptr<GMALVisibleObject>
GMALBarrier::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALBarrier(*this));
}

void
GMALBarrier::print()
{
    printf("Barrier id: %lu pthread_barrier_t: %p, waiting: %d wait_count: %d\n", this->getObjectId(),
           this->barrierShadow.systemIdentity, this->barrierShadow.numThreadsWaiting, this->barrierShadow.waitCount);
}

void
GMALBarrier::deinit()
{
    this->barrierShadow.state = GMALBarrierShadow::undefined;
}

void
GMALBarrier::init()
{
    this->barrierShadow.state = GMALBarrierShadow::initialized;
}

void
GMALBarrier::wait(tid_t tid)
{
    this->threadsWaitingOnBarrier.insert(tid);
}

void
GMALBarrier::leave(tid_t tid)
{
    this->threadsWaitingOnBarrier.erase(tid);
}

bool
GMALBarrier::operator==(const GMALBarrier &other) const
{
    return this->barrierShadow.systemIdentity == other.barrierShadow.systemIdentity;
}

bool
GMALBarrier::operator!=(const GMALBarrier &other) const
{
    return this->barrierShadow.systemIdentity != other.barrierShadow.systemIdentity;
}

bool
GMALBarrier::wouldBlockIfWaitedOn()
{
    return this->threadsWaitingOnBarrier.size() < this->barrierShadow.waitCount;
}

bool
GMALBarrier::isWaitingOnBarrier(tid_t tid)
{
    return this->threadsWaitingOnBarrier.count(tid) > 0;
}
