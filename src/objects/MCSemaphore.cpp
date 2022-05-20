#include "MCSemaphore.h"

MCSystemID
MCSemaphore::getSystemId()
{
    return this->semShadow.sem;
}

std::shared_ptr<MCVisibleObject>
MCSemaphore::copy()
{
    return std::shared_ptr<MCVisibleObject>(new MCSemaphore(*this));
}

bool
MCSemaphore::wouldBlockIfWaitedOn()
{
    return this->semShadow.count == 0;
}

void
MCSemaphore::deinit()
{
    this->semShadow.state = MCSemaphoreShadow::undefined;
}

void
MCSemaphore::init()
{
    this->semShadow.state = MCSemaphoreShadow::initialized;
}

void
MCSemaphore::post()
{
    this->semShadow.count++;
}

void
MCSemaphore::wait()
{
    this->semShadow.count--;
}

bool
MCSemaphore::operator==(const MCSemaphore &other) const
{
    return this->semShadow.sem == other.semShadow.sem;
}

bool
MCSemaphore::operator!=(const MCSemaphore &other) const
{
    return this->semShadow.sem != other.semShadow.sem;
}

bool
MCSemaphore::isDestroyed() const
{
    return this->semShadow.state == MCSemaphoreShadow::destroyed;
}

unsigned int
MCSemaphore::getCount() const
{
    return this->semShadow.count;
}
