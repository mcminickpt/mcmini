#ifndef MC_MCTHREAD_H
#define MC_MCTHREAD_H

#include "mcmini/MCShared.h"
#include "mcmini/objects/MCVisibleObject.h"
#include <memory>
#include <pthread.h>

struct MCThreadShadow {
  void *arg;
  thread_routine startRoutine;
  pthread_t systemIdentity;
  enum MCThreadState { embryo, alive, sleeping, dead } state;

  MCThreadShadow(void *arg, thread_routine startRoutine,
                 pthread_t systemIdentity)
    : arg(arg), startRoutine(startRoutine),
      systemIdentity(systemIdentity), state(embryo)
  {
  }
};

struct MCThread : public MCVisibleObject {
private:
  MCThreadShadow threadShadow;

public:
  /* Threads are unique in that they have *two* ids */
  const tid_t tid;

  inline MCThread(tid_t tid, void *arg, thread_routine startRoutine,
                  pthread_t systemIdentity)
    : MCVisibleObject(),
      threadShadow(MCThreadShadow(arg, startRoutine, systemIdentity)),
      tid(tid)
  {
  }

  inline explicit MCThread(tid_t tid, MCThreadShadow shadow)
    : MCVisibleObject(), threadShadow(shadow), tid(tid)
  {
  }
  inline MCThread(const MCThread &thread)
    : MCVisibleObject(thread.getObjectId()),
      threadShadow(thread.threadShadow), tid(thread.tid)
  {
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCThread &) const;

  // Managing thread state
  MCThreadShadow::MCThreadState getState() const;

  bool enabled() const;
  bool isAlive() const;
  bool isDead() const;

  void awaken();
  void sleep();

  void regenerate();
  void die();
  void spawn();
  void despawn();
};

#endif // MC_MCTHREAD_H
