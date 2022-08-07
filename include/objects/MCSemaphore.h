#ifndef MC_MCSEMAPHORE_H
#define MC_MCSEMAPHORE_H

#include "mcmini/objects/MCVisibleObject.h"
#include <deque>
#include <semaphore.h>

struct MCSemaphoreShadow {
  sem_t *sem;
  unsigned int count;

  enum State { undefined, initialized, destroyed } state;

  MCSemaphoreShadow(sem_t *sem, unsigned int count)
    : sem(sem), count(count), state(undefined)
  {
  }
};

class MCSemaphore : public MCVisibleObject {
private:
  std::deque<tid_t> waitingQueue;

  /**
   * The total number of spurious wake ups that
   * are allowed for threads waiting on the semaphore
   */
  unsigned spuriousWakeupCount = 0;

  /*
   * Whether a thread that is allowed to
   * wake on the condition variable by virtue of its position
   * in the sleeping queue should instead
   * awake because it was awoken by a spurious wake-up
   */
  const bool preferSpuriousWakeupsWhenPossible = true;

  MCSemaphoreShadow semShadow;
  inline explicit MCSemaphore(MCSemaphoreShadow shadow, objid_t id)
    : MCVisibleObject(id), semShadow(shadow)
  {
  }

public:
  inline explicit MCSemaphore(MCSemaphoreShadow shadow)
    : MCVisibleObject(), semShadow(shadow)
  {
  }
  inline MCSemaphore(const MCSemaphore &sem)
    : MCVisibleObject(sem.getObjectId()), semShadow(sem.semShadow),
      waitingQueue(sem.waitingQueue),
      spuriousWakeupCount(sem.spuriousWakeupCount)
  {
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCSemaphore &) const;
  bool operator!=(const MCSemaphore &) const;

  unsigned int getCount() const;
  bool isDestroyed() const;
  bool wouldBlockIfWaitedOn();
  bool threadCanExitWithSpuriousWakeup(tid_t) const;
  bool threadCanExitBasedOnSleepPosition(tid_t) const;
  bool threadCanExit(tid_t);
  void leaveWaitingQueue(tid_t);
  void enterWaitingQueue(tid_t);

  void deinit();
  void init();
  void post();
  void wait();
};

#endif // MC_MCSEMAPHORE_H
