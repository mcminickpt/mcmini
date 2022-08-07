#ifndef MC_MCBARRIER_H
#define MC_MCBARRIER_H

#include "mcmini/objects/MCVisibleObject.h"
#include <unordered_set>

struct MCBarrierShadow {
  pthread_barrier_t *systemIdentity;
  const unsigned int waitCount;
  enum MCBarrierState { undefined, initialized, destroyed } state;

  MCBarrierShadow(pthread_barrier_t *systemIdentity,
                  unsigned int waitCount)
    : systemIdentity(systemIdentity), waitCount(waitCount),
      state(undefined)
  {
  }
};

struct MCBarrier : public MCVisibleObject {
private:
  std::unordered_set<tid_t> threadsWaitingOnBarrierOdd;
  std::unordered_set<tid_t> threadsWaitingOnBarrierEven;

  std::unordered_set<tid_t> &waitingSetForParity();

  bool isEven = false;

  MCBarrierShadow barrierShadow;
  inline explicit MCBarrier(MCBarrierShadow shadow, objid_t id)
    : MCVisibleObject(id), barrierShadow(shadow)
  {
  }

public:
  inline explicit MCBarrier(MCBarrierShadow shadow)
    : MCVisibleObject(), barrierShadow(shadow)
  {
  }
  inline MCBarrier(const MCBarrier &barrier)
    : MCVisibleObject(barrier.getObjectId()),
      barrierShadow(barrier.barrierShadow)
  {
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCBarrier &) const;
  bool operator!=(const MCBarrier &) const;

  unsigned int getCount();
  bool wouldBlockIfWaitedOn(tid_t);
  void deinit();
  void init();
  void wait(tid_t);
  void leave(tid_t);
  bool isWaitingOnBarrier(tid_t);
  bool hasEvenParity(tid_t);
};

#endif // MC_MCBARRIER_H
