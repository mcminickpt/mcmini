#include "GMALMutex.h"

bool
GMALMutex::operator==(const GMALMutex &other) const
{
    return this->mutexShadow.systemIdentity == other.mutexShadow.systemIdentity;
}

bool
GMALMutex::operator!=(const GMALMutex &other) const
{
    return this->mutexShadow.systemIdentity != other.mutexShadow.systemIdentity;
}

GMALSystemID
GMALMutex::getSystemId()
{
    return (GMALSystemID)mutexShadow.systemIdentity;
}

std::shared_ptr<GMALVisibleObject>
GMALMutex::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALMutex(*this));
}

bool
GMALMutex::isLocked() const
{
    return this->mutexShadow.state == GMALMutexShadow::locked;
}

bool
GMALMutex::isUnlocked() const
{
    return this->mutexShadow.state == GMALMutexShadow::unlocked;
}


bool
GMALMutex::isDestroyed() const
{
    return this->mutexShadow.state == GMALMutexShadow::destroyed;
}

void
GMALMutex::lock()
{
    this->mutexShadow.state = GMALMutexShadow::locked;
}

void
GMALMutex::unlock()
{
    this->mutexShadow.state = GMALMutexShadow::unlocked;
}

void
GMALMutex::init()
{
    this->mutexShadow.state = GMALMutexShadow::unlocked;
}

void
GMALMutex::deinit()
{
    this->mutexShadow.state = GMALMutexShadow::undefined;
}

bool
GMALMutex::canAcquire(tid_t thread) const
{
    return this->isUnlocked(); /* Eventually, more complicated logic with recursive mutexes */
}