#ifndef INCLUDE_MCMINI_OBJECTS_MCMUTEX_HPP
#define INCLUDE_MCMINI_OBJECTS_MCMUTEX_HPP

#include "mcmini/misc/MCOptional.h"
#include "mcmini/objects/MCVisibleObject.h"

struct MCMutexShadow {
  pthread_mutex_t *systemIdentity;
  enum State { undefined, unlocked, locked, destroyed } state;
  explicit MCMutexShadow(pthread_mutex_t *systemIdentity)
    : systemIdentity(systemIdentity), state(undefined)
  {}
};

struct MCMutex : public MCVisibleObject {
private:

  MCMutexShadow mutexShadow;

public:

  inline explicit MCMutex(MCMutexShadow shadow)
    : MCVisibleObject(), mutexShadow(shadow)
  {}
  inline MCMutex(const MCMutex &mutex)
    : MCVisibleObject(mutex.getObjectId()),
      mutexShadow(mutex.mutexShadow)
  {}

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCMutex &) const;
  bool operator!=(const MCMutex &) const;

  bool canAcquire(tid_t) const;
  bool isLocked() const;
  bool isUnlocked() const;
  bool isDestroyed() const;

  void lock(tid_t);
  void unlock();
  void init();
  void deinit();
};

#endif // INCLUDE_MCMINI_OBJECTS_MCMUTEX_HPP
