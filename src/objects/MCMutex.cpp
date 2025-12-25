#include "objects/MCMutex.h"

bool
MCMutex::operator==(const MCMutex &other) const
{
  return this->mutexShadow.systemIdentity ==
         other.mutexShadow.systemIdentity;
}

bool
MCMutex::operator!=(const MCMutex &other) const
{
  return this->mutexShadow.systemIdentity !=
         other.mutexShadow.systemIdentity;
}

MCSystemID
MCMutex::getSystemId()
{
  return (MCSystemID)mutexShadow.systemIdentity;
}

std::shared_ptr<MCVisibleObject>
MCMutex::copy()
{
  return std::shared_ptr<MCVisibleObject>(new MCMutex(*this));
}

bool
MCMutex::isLocked() const
{
  return this->mutexShadow.state == MCMutexShadow::locked;
}

bool
MCMutex::isUnlocked() const
{
  return this->mutexShadow.state == MCMutexShadow::unlocked;
}

bool
MCMutex::isDestroyed() const
{
  return this->mutexShadow.state == MCMutexShadow::destroyed;
}

void
MCMutex::lock(tid_t newOwner)
{
  this->mutexShadow.state = MCMutexShadow::locked;
  this->mutexShadow.owner = newOwner;
}

void
MCMutex::unlock()
{
  this->mutexShadow.state = MCMutexShadow::unlocked;
  this->mutexShadow.owner = TID_INVALID;
}

void
MCMutex::init()
{
  this->mutexShadow.state = MCMutexShadow::unlocked;
  this->mutexShadow.owner = TID_INVALID;
}

void
MCMutex::deinit()
{
  this->mutexShadow.state = MCMutexShadow::undefined;
}

tid_t
MCMutex::ownerTid() const
{
  return this->mutexShadow.owner;
}

bool
MCMutex::canAcquire(tid_t thread) const
{
  return this->isUnlocked(); /* Eventually, more complicated logic
                                with recursive mutexes */
}
