#include "GMALThread.h"

static_assert(std::is_trivially_copyable<GMALThreadShadow>::value,
              "The shared transition is not trivially copiable. Performing a memcpy of this type "
              "is undefined behavior according to the C++ standard.");

std::shared_ptr<GMALVisibleObject>
GMALThread::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALThread(*this));
}

GMALSystemID
GMALThread::getSystemId() {
    // TODO: This is unsafe -> we cannot rely on the identity of pthread_t
    return (GMALSystemID)threadShadow.systemIdentity;
}


GMALThreadShadow::GMALThreadState
GMALThread::getState() const
{
    return threadShadow.state;
}

bool
GMALThread::enabled() const
{
    return this->threadShadow.state == GMALThreadShadow::alive;
}

void
GMALThread::awaken()
{
    this->threadShadow.state = GMALThreadShadow::alive;
}

void
GMALThread::die()
{
    this->threadShadow.state = GMALThreadShadow::dead;
}

void
GMALThread::sleep()
{
    this->threadShadow.state = GMALThreadShadow::sleeping;
}

void
GMALThread::spawn()
{
    this->threadShadow.state = GMALThreadShadow::alive;
}

void
GMALThread::despawn()
{
    this->threadShadow.state = GMALThreadShadow::embryo;
}

void
GMALThread::regenerate()
{
    this->threadShadow.state = GMALThreadShadow::alive;
}

bool
GMALThread::isAlive() const
{
    // Note this is NOT equivalent to
    // threadShadow.state == GMALThreadShadow::alive;
    return threadShadow.state == GMALThreadShadow::alive || threadShadow.state == GMALThreadShadow::sleeping;
}

bool
GMALThread::isDead() const
{
    return threadShadow.state == GMALThreadShadow::dead;
}


void
GMALThread::print()
{
    printf("THREAD tid: %lu, id: %lu, pthread: %lu \n", this->tid, this->getObjectId(), this->threadShadow.systemIdentity);
}