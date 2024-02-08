#include "mcmini/objects/MCBarrier.h"

MCSystemID
MCBarrier::getSystemId()
{
  return this->barrierShadow.systemIdentity;
}

std::shared_ptr<MCVisibleObject>
MCBarrier::copy()
{
  return std::shared_ptr<MCVisibleObject>(new MCBarrier(*this));
}

void
MCBarrier::deinit()
{
  this->barrierShadow.state = MCBarrierShadow::undefined;
}

void
MCBarrier::init()
{
  this->barrierShadow.state = MCBarrierShadow::initialized;
}

void
MCBarrier::wait(tid_t tid)
{
  if (this->isWaitingOnBarrier(tid)) {
    return; /* Nothing to do in the case the thread is already waiting
             */
  }
  auto &waitingSet = this->waitingSetForParity();
  waitingSet.insert(tid);

  if (waitingSet.size() == this->barrierShadow.waitCount) {
    this->isEven = !this->isEven; // Swap parities for future threads
  }
}

void
MCBarrier::leave(tid_t tid)
{
  // A thread will be waiting in one of the two sets...
  // Which one is unclear but we don't really care so long
  // as we remove it
  this->threadsWaitingOnBarrierOdd.erase(tid);
  this->threadsWaitingOnBarrierEven.erase(tid);
}

bool
MCBarrier::operator==(const MCBarrier &other) const
{
  return this->barrierShadow.systemIdentity ==
         other.barrierShadow.systemIdentity;
}

bool
MCBarrier::operator!=(const MCBarrier &other) const
{
  return this->barrierShadow.systemIdentity !=
         other.barrierShadow.systemIdentity;
}

std::unordered_set<tid_t> &
MCBarrier::waitingSetForParity()
{
  return isEven ? this->threadsWaitingOnBarrierEven
                : this->threadsWaitingOnBarrierOdd;
}

bool
MCBarrier::wouldBlockIfWaitedOn(tid_t tid)
{
  // TODO: This logic doesn't work if more than N threads
  // are interacting with the barrier. A possible solution
  // might be to have a counter which identifies the barrier
  // "level". We'd have to map thread ids to their level
  if (this->isWaitingOnBarrier(tid)) {
    auto threadHasEvenParity  = this->hasEvenParity(tid);
    auto barrierHasEvenParity = this->isEven;
    if (threadHasEvenParity == barrierHasEvenParity) {
      return this->waitingSetForParity().size() <
             this->barrierShadow.waitCount;
    }
    else {
      // This means that the barrier has successfully been lifted
      return false;
    }
  }
  else {
    return this->waitingSetForParity().size() <
           this->barrierShadow.waitCount;
  }
}

bool
MCBarrier::isWaitingOnBarrier(tid_t tid)
{
  return this->threadsWaitingOnBarrierEven.count(tid) > 0 ||
         this->threadsWaitingOnBarrierOdd.count(tid) > 0;
}

bool
MCBarrier::hasEvenParity(tid_t tid)
{
  if (this->threadsWaitingOnBarrierEven.count(tid) > 0) {
    return true;
  }
  return false;
}

unsigned int
MCBarrier::getCount()
{
  return this->barrierShadow.waitCount;
}
