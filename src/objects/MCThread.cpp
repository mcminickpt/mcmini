#include "mcmini/objects/MCThread.h"

static_assert(MC_IS_TRIVIALLY_COPYABLE(MCThreadShadow),
              "The shared transition is not trivially copiable. "
              "Performing a memcpy of this type "
              "is undefined behavior according to the C++ standard.");

std::shared_ptr<MCVisibleObject>
MCThread::copy()
{
  return std::shared_ptr<MCVisibleObject>(new MCThread(*this));
}

MCSystemID
MCThread::getSystemId()
{
  // TODO: This is unsafe -> we cannot rely on the identity of
  // pthread_t
  return (MCSystemID)threadShadow.systemIdentity;
}

MCThreadShadow::MCThreadState
MCThread::getState() const
{
  return threadShadow.state;
}

bool
MCThread::enabled() const
{
  return this->threadShadow.state == MCThreadShadow::alive;
}

void
MCThread::awaken()
{
  this->threadShadow.state = MCThreadShadow::alive;
}

void
MCThread::die()
{
  this->threadShadow.state = MCThreadShadow::dead;
}

void
MCThread::sleep()
{
  this->threadShadow.state = MCThreadShadow::sleeping;
}

void
MCThread::spawn()
{
  this->threadShadow.state = MCThreadShadow::alive;
}

void
MCThread::despawn()
{
  this->threadShadow.state = MCThreadShadow::embryo;
}

void
MCThread::regenerate()
{
  this->threadShadow.state = MCThreadShadow::alive;
}

bool
MCThread::isAlive() const
{
  // Note this is NOT equivalent to
  // threadShadow.state == MCThreadShadow::alive;
  return threadShadow.state == MCThreadShadow::alive ||
         threadShadow.state == MCThreadShadow::sleeping;
}

bool
MCThread::isDead() const
{
  return threadShadow.state == MCThreadShadow::dead;
}