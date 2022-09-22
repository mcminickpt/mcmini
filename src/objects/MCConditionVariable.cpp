#include "mcmini/objects/MCConditionVariable.h"
#include <algorithm>

std::shared_ptr<MCVisibleObject>
MCConditionVariable::copy()
{
  return std::shared_ptr<MCVisibleObject>(
    new MCConditionVariable(*this));
}

MCSystemID
MCConditionVariable::getSystemId()
{
  return this->condShadow.cond;
}

bool
MCConditionVariable::operator==(
  const MCConditionVariable &other) const
{
  return this->condShadow.cond == other.condShadow.cond;
}

bool
MCConditionVariable::operator!=(
  const MCConditionVariable &other) const
{
  return this->condShadow.cond != other.condShadow.cond;
}

bool
MCConditionVariable::isInitialized() const
{
  return this->condShadow.state ==
         MCConditionVariableShadow::initialized;
}

bool
MCConditionVariable::isDestroyed() const
{
  return this->condShadow.state ==
         MCConditionVariableShadow::destroyed;
}

void
MCConditionVariable::initialize()
{
  this->condShadow.state = MCConditionVariableShadow::initialized;
}

void
MCConditionVariable::addWaiter(tid_t tid)
{
  this->signalPolicy->addWaiter(tid);
}

void
MCConditionVariable::removeWaiter(tid_t tid)
{
  this->signalPolicy->removeWaiter(tid);
  this->wakeupPolicy->wakeThread(tid);
}

bool
MCConditionVariable::waiterCanExit(tid_t tid)
{
  return this->wakeupPolicy->threadCanExit(tid);
}

void
MCConditionVariable::sendSignalMessage()
{
  const WakeGroup newGroup =
    this->signalPolicy->receiveSignalMessage();
  this->wakeupPolicy->pushWakeupGroup(newGroup);
}

void
MCConditionVariable::sendBroadcastMessage()
{
  const WakeGroup newGroup =
    this->signalPolicy->receiveBroadcastMessage();
  this->wakeupPolicy->pushWakeupGroup(newGroup);
}

void
MCConditionVariable::destroy()
{
  this->condShadow.state = MCConditionVariableShadow::destroyed;
}
