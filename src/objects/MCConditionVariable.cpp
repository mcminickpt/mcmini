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
  return this->shadow.cond;
}

bool
MCConditionVariable::operator==(
  const MCConditionVariable &other) const
{
  return this->shadow.cond == other.shadow.cond;
}

bool
MCConditionVariable::operator!=(
  const MCConditionVariable &other) const
{
  return this->shadow.cond != other.shadow.cond;
}

bool
MCConditionVariable::isInitialized() const
{
  return this->shadow.state == MCConditionVariableShadow::initialized;
}

bool
MCConditionVariable::isDestroyed() const
{
  return this->shadow.state == MCConditionVariableShadow::destroyed;
}

void
MCConditionVariable::initialize()
{
  this->shadow.state = MCConditionVariableShadow::initialized;
}

void
MCConditionVariable::addWaiter(tid_t tid)
{
  this->policy->add_waiter(tid);
}

bool
MCConditionVariable::hasWaiters()
{
  return this->policy->has_waiters();
}

void
MCConditionVariable::removeWaiter(tid_t tid)
{
  this->policy->wake_thread(tid);

  // If there are any spurious wake ups allowed,
  // we always allow the thread to wake up
  // due to a spurious wake up and decrement the
  // number of future such wakeups allowed
  if (this->numRemainingSpuriousWakeups > 0) {
    this->numRemainingSpuriousWakeups--;
  }
}

bool
MCConditionVariable::waiterCanExit(tid_t tid)
{
  return this->numRemainingSpuriousWakeups > 0 ||
         this->policy->thread_can_exit(tid);
}

void
MCConditionVariable::sendSignalMessage()
{
  this->policy->receive_signal_message();
}

void
MCConditionVariable::sendBroadcastMessage()
{
  this->policy->receive_broadcast_message();
}

void
MCConditionVariable::destroy()
{
  this->shadow.state = MCConditionVariableShadow::destroyed;
}
