#include "GMALSemaphore.h"

GMALSystemID
GMALSemaphore::getSystemId()
{
    return this->semShadow.sem;
}

std::shared_ptr<GMALVisibleObject>
GMALSemaphore::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALSemaphore(*this));
}

void
GMALSemaphore::print()
{
    printf("SEMAPHORE id: %lu sem_t: %p, count: %d\n", this->getObjectId(),
           this->semShadow.sem, this->semShadow.count);
}

bool
GMALSemaphore::wouldBlockIfWaitedOn()
{
    return this->semShadow.count == 0;
}

void
GMALSemaphore::deinit()
{
    this->semShadow.state = GMALSemaphoreShadow::undefined;
}

void
GMALSemaphore::init()
{
    this->semShadow.state = GMALSemaphoreShadow::initialized;
}

void
GMALSemaphore::post()
{
    this->semShadow.count++;
}

void
GMALSemaphore::wait()
{
    this->semShadow.count--;
}

bool
GMALSemaphore::operator==(const GMALSemaphore &other) const
{
    return this->semShadow.sem == other.semShadow.sem;
}

bool
GMALSemaphore::operator!=(const GMALSemaphore &other) const
{
    return this->semShadow.sem != other.semShadow.sem;
}

bool
GMALSemaphore::isDestroyed() const
{
    return this->semShadow.state == GMALSemaphoreShadow::destroyed;
}
