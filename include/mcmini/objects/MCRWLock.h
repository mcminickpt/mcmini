#ifndef INCLUDE_MCMINI_OBJECTS_MCRWLOCK_HPP
#define INCLUDE_MCMINI_OBJECTS_MCRWLOCK_HPP

#include "mcmini/misc/MCOptional.h"
#include "mcmini/objects/MCVisibleObject.h"
#include <pthread.h>
#include <queue>
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

  MCRWLockShadow shadow;
  MCOptional<tid_t> active_writer = MCOptional<tid_t>::nil();
  std::vector<tid_t> active_readers;

  std::queue<tid_t> reader_queue = std::queue<tid_t>();
  std::queue<tid_t> writer_queue = std::queue<tid_t>();

  // INVARIANT: Represents an interleaving of
  // the reader and writer queues. The acquisition
  // queue is used for locks with a Type of no_preference
  std::queue<tid_t> acquire_queue = std::queue<tid_t>();

public:

  enum Type {
    writer_preferred,
    reader_preferred,
    no_preference
  } type = Type::no_preference;

  inline explicit MCRWLock(MCRWLockShadow shadow, Type type)
    : MCVisibleObject(), shadow(shadow), type(type)
  {}
  inline MCRWLock(const MCRWLock &rwlock)
    : MCVisibleObject(rwlock.getObjectId()), shadow(rwlock.shadow),
      type(rwlock.type), active_writer(rwlock.active_writer),
      active_readers(rwlock.active_readers),
      reader_queue(rwlock.reader_queue),
      writer_queue(rwlock.writer_queue),
      acquire_queue(rwlock.acquire_queue)
  {}

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCRWLock &) const;
  bool operator!=(const MCRWLock &) const;

  bool canAcquireAsReader(tid_t) const;
  bool canAcquireAsWriter(tid_t) const;
  bool hasEnqueuedWriters() const;
  bool hasEnqueuedReaders() const;

  bool isReaderLocked() const;
  bool isWriterLocked() const;
  bool isUnlocked() const;
  bool isDestroyed() const;

  void enqueue_as_reader(tid_t);
  void enqueue_as_writer(tid_t);
  void reader_lock(tid_t);
  void writer_lock(tid_t);
  void unlock(tid_t);
  void init();
  void deinit();
};

#endif // INCLUDE_MCMINI_OBJECTS_MCRWLOCK_HPP
