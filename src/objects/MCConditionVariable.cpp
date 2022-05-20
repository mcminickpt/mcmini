#include "MCConditionVariable.h"
#include <algorithm>

std::shared_ptr<MCVisibleObject>
MCConditionVariable::copy()
{
    return std::shared_ptr<MCVisibleObject>(new MCConditionVariable(*this));
}

MCSystemID
MCConditionVariable::getSystemId()
{
    return this->condShadow.cond;
}

bool
MCConditionVariable::operator==(const MCConditionVariable &other) const
{
    return this->condShadow.cond == other.condShadow.cond;
}

bool
MCConditionVariable::operator!=(const MCConditionVariable &other) const
{
    return this->condShadow.cond != other.condShadow.cond;
}

bool
MCConditionVariable::isInitialized() const
{
    return this->condShadow.state == MCConditionVariableShadow::initialized;
}

bool
MCConditionVariable::isDestroyed() const
{
    return this->condShadow.state == MCConditionVariableShadow::destroyed;
}

void
MCConditionVariable::initialize()
{
    this->condShadow.state = MCConditionVariableShadow::initialized;
}

void
MCConditionVariable::destroy()
{
    this->condShadow.state = MCConditionVariableShadow::destroyed;
}

void
MCConditionVariable::enterSleepingQueue(tid_t tid)
{
    /* Ensure that the thread is not already in the queue */
    this->sleepQueue.push_back(tid);
}

void
MCConditionVariable::wakeThread(tid_t tid)
{
    this->removeSleepingThread(tid);
    this->wakeQueue.push_back(tid);
}

void
MCConditionVariable::removeSleepingThread(tid_t tid)
{
    auto _ = std::remove(this->sleepQueue.begin(), this->sleepQueue.end(), tid);
}

void
MCConditionVariable::removeWakingThread(tid_t tid)
{
    auto _ = std::remove(this->wakeQueue.begin(), this->wakeQueue.end(), tid);
}

void
MCConditionVariable::wakeFirstThreadIfPossible()
{
    if (this->sleepQueue.empty()) return;

    auto front = this->sleepQueue.front();
    this->sleepQueue.pop_front();
    this->wakeQueue.push_back(front);
}

void
MCConditionVariable::wakeAllSleepingThreads()
{
    for (const auto tid : this->sleepQueue)
        this->wakeQueue.push_back(tid);
    this->sleepQueue.clear();
}

void
MCConditionVariable::removeThread(tid_t tid)
{
    this->removeSleepingThread(tid);
    this->removeWakingThread(tid);
}

bool
MCConditionVariable::threadIsInWaitingQueue(tid_t tid)
{
    return std::find(this->wakeQueue.begin(), this->wakeQueue.end(), tid) != this->wakeQueue.end();
}

bool
MCConditionVariable::threadCanExit(tid_t tid)
{
    return this->threadIsInWaitingQueue(tid);
}