#include "MCSemaphore.h"
#include <algorithm>

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

bool
MCSemaphore::threadCanExit(tid_t tid)
{
    if (wouldBlockIfWaitedOn())
        return false;

    /* Different strategies */

    /* Strategy A: Thread can exit if it is waiting at all */
    return std::find(this->waitingQueue.begin(), this->waitingQueue.end(), tid) != this->waitingQueue.end();

    /* Strategy B: Thread can exit if it was the first one waiting (FIFO) */
    //return std::find(this->waitingQueue.begin(), this->waitingQueue.end(), tid) == this->waitingQueue.begin();

    /* Strategy B: Thread can exit if it was the first one waiting (FIFO) */
    //return std::find(this->waitingQueue.begin(), this->waitingQueue.end(), tid) == this->waitingQueue.begin();
}

void
MCSemaphore::leaveWaitingQueue(tid_t tid)
{
    auto index = std::find(this->waitingQueue.begin(), this->waitingQueue.end(), tid);
    if (index != this->waitingQueue.end())
        this->waitingQueue.erase(index);
}

void
MCSemaphore::enterWaitingQueue(tid_t tid)
{
    this->waitingQueue.push_back(tid);
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
