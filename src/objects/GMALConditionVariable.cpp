#include "GMALConditionVariable.h"
#include <algorithm>

std::shared_ptr<GMALVisibleObject>
GMALConditionVariable::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALConditionVariable(*this));
}

GMALSystemID
GMALConditionVariable::getSystemId()
{
    return this->condShadow.cond;
}

bool
GMALConditionVariable::operator==(const GMALConditionVariable &other) const
{
    return this->condShadow.cond == other.condShadow.cond;
}

bool
GMALConditionVariable::operator!=(const GMALConditionVariable &other) const
{
    return this->condShadow.cond != other.condShadow.cond;
}

bool
GMALConditionVariable::isInitialized() const
{
    return this->condShadow.state == GMALConditionVariableShadow::initialized;
}

bool
GMALConditionVariable::isDestroyed() const
{
    return this->condShadow.state == GMALConditionVariableShadow::destroyed;
}

void
GMALConditionVariable::initialize()
{
    this->condShadow.state = GMALConditionVariableShadow::initialized;
}

void
GMALConditionVariable::destroy()
{
    this->condShadow.state = GMALConditionVariableShadow::destroyed;
}

void
GMALConditionVariable::enterSleepingQueue(tid_t tid)
{
    /* Ensure that the thread is not already in the queue */
    this->sleepQueue.push_back(tid);
}

void
GMALConditionVariable::wakeThread(tid_t tid)
{
    this->removeSleepingThread(tid);
    this->wakeQueue.push_back(tid);
}

void
GMALConditionVariable::removeSleepingThread(tid_t tid)
{
    auto _ = std::remove(this->sleepQueue.begin(), this->sleepQueue.end(), tid);
}

void
GMALConditionVariable::removeWakingThread(tid_t tid)
{
    auto _ = std::remove(this->wakeQueue.begin(), this->wakeQueue.end(), tid);
}

void
GMALConditionVariable::wakeFirstThreadIfPossible()
{
    if (this->sleepQueue.empty()) return;

    auto front = this->sleepQueue.front();
    this->sleepQueue.pop_front();
    this->wakeQueue.push_back(front);
}

void
GMALConditionVariable::wakeAllSleepingThreads()
{
    for (const auto tid : this->sleepQueue)
        this->wakeQueue.push_back(tid);
    this->sleepQueue.clear();
}

void
GMALConditionVariable::removeThread(tid_t tid)
{
    this->removeSleepingThread(tid);
    this->removeWakingThread(tid);
}

bool
GMALConditionVariable::threadIsInWaitingQueue(tid_t tid)
{
    return std::find(this->wakeQueue.begin(), this->wakeQueue.end(), tid) != this->wakeQueue.end();
}

bool
GMALConditionVariable::threadCanExit(tid_t tid)
{
    return this->threadIsInWaitingQueue(tid);
}