#include "GMALSemaphore.h"
#include <algorithm>

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

bool
GMALSemaphore::wouldBlockIfWaitedOn()
{
    return this->semShadow.count == 0;
}

bool
GMALSemaphore::threadCanExit(tid_t tid)
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
GMALSemaphore::leaveWaitingQueue(tid_t tid)
{
    auto index = std::find(this->waitingQueue.begin(), this->waitingQueue.end(), tid);
    if (index != this->waitingQueue.end())
        this->waitingQueue.erase(index);
}

void
GMALSemaphore::enterWaitingQueue(tid_t tid)
{
    this->waitingQueue.push_back(tid);
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

unsigned int
GMALSemaphore::getCount() const
{
    return this->semShadow.count;
}
