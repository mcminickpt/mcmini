#include "mcmini/objects/MCSemaphore.h"
#include <algorithm>

MCSystemID
MCSemaphore::getSystemId()
{
  return this->semShadow.sem;
}

std::shared_ptr<MCVisibleObject>
MCSemaphore::copy()
{
  return std::make_shared<MCSemaphore>(*this);
}

bool
MCSemaphore::wouldBlockIfWaitedOn()
{
  return this->semShadow.count <= 0;
}

bool
MCSemaphore::threadCanExitWithSpuriousWakeup(tid_t tid) const
{
  return this->spuriousWakeupCount > 0;
}

bool
MCSemaphore::threadCanExitBasedOnSleepPosition(tid_t tid) const
{
  /* Strategy A: Thread can exit if it is waiting at all */
  // return std::find(this->waitingQueue.begin(),
  // this->waitingQueue.end(), tid) != this->waitingQueue.end();

  /* Strategy B: Thread can exit if it was the first one waiting
   * (FIFO) */
  return std::find(this->waitingQueue.begin(),
                   this->waitingQueue.end(),
                   tid) == this->waitingQueue.begin();

  /* Strategy C: Thread can exit if it was the last one waiting (LIFO)
   */
  // return std::find(this->waitingQueue.begin(),
  // this->waitingQueue.end(), tid) == this->waitingQueue.end() - 1;
}

bool
MCSemaphore::threadCanExit(tid_t tid)
{
  return !wouldBlockIfWaitedOn() &&
         this->threadCanExitBasedOnSleepPosition(tid);
}

void
MCSemaphore::leaveWaitingQueue(tid_t tid)
{
  auto index = std::find(this->waitingQueue.begin(),
                         this->waitingQueue.end(), tid);
  if (index != this->waitingQueue.end())
    this->waitingQueue.erase(index);

  /**
   * If we woke up because of a spurious wake up,
   * we should also update how the semaphore continues
   * to operate/allow for spurious wake-ups. The logic
   * for being allowed to wake-up
   */
  if (threadCanExitWithSpuriousWakeup(tid)) {
    if (threadCanExitBasedOnSleepPosition(tid)) {
      if (preferSpuriousWakeupsWhenPossible) {
        this->spuriousWakeupCount--;
      }
    }
    else {
      this->spuriousWakeupCount--;
    }
  }
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
  if (this->semShadow.count > 0) { this->semShadow.count--; }
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
