#ifndef INCLUDE_MCMINI_OBJECTS_MCRWLOCK_HPP
#define INCLUDE_MCMINI_OBJECTS_MCRWLOCK_HPP

#include "mcmini/misc/MCOptional.h"
#include "mcmini/objects/MCVisibleObject.h"
#include <pthread.h>
#include <unordered_set>

struct MCRWLockShadow {
  pthread_rwlock_t *systemIdentity;
  enum State {
    undefined,
    unlocked,
    reader_owned,
    writer_owned,
    destroyed
  } state;

  explicit MCRWLockShadow(pthread_rwlock_t *systemIdentity)
    : systemIdentity(systemIdentity), state(undefined)
  {}
};

struct MCRWLock : public MCVisibleObject {
private:

  enum Type {
    writer_preferred,
    reader_preferred,
    mostly_writer_preferred,
    mostly_reader_preferred
  } type;

  MCRWLockShadow shadow;
  MCOptional<tid_t> writer = MCOptional<tid_t>::nil();
  std::unordered_set<tid_t> readers;

public:

  inline explicit MCRWLock(MCRWLockShadow shadow)
    : MCVisibleObject(), shadow(shadow)
  {}
  inline MCRWLock(const MCRWLock &rwlock)
    : MCVisibleObject(rwlock.getObjectId()), shadow(rwlock.shadow)
  {}

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCRWLock &) const;
  bool operator!=(const MCRWLock &) const;

  bool canAcquireAsReader(tid_t) const;
  bool canAcquireAsWriter(tid_t) const;
  bool isReaderLocked() const;
  bool isWriterLocked() const;
  bool isUnlocked() const;
  bool isDestroyed() const;

  void reader_lock(tid_t);
  void writer_lock(tid_t);
  void unlock(tid_t);
  void init();
  void deinit();
};

#endif // INCLUDE_MCMINI_OBJECTS_MCRWLOCK_HPP
