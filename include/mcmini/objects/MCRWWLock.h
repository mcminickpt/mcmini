#ifndef INCLUDE_MCMINI_OBJECTS_MCRWWLOCK_HPP
#define INCLUDE_MCMINI_OBJECTS_MCRWWLOCK_HPP

#include "mcmini/export/rwwlock.h"
#include "mcmini/misc/MCOptional.h"
#include "mcmini/objects/MCVisibleObject.h"
#include <pthread.h>
#include <queue>
#include <unordered_set>

struct MCRWWLockShadow {
  pthread_rwwlock_t *systemIdentity;
  enum State {
    undefined,
    unlocked,
    reader_owned,
    writer1_owned,
    writer2_owned,
    destroyed
  } state;

  explicit MCRWWLockShadow(pthread_rwwlock_t *systemIdentity)
    : systemIdentity(systemIdentity), state(undefined)
  {}
};

struct MCRWWLock : public MCVisibleObject {
private:

  MCRWWLockShadow shadow;
  MCOptional<tid_t> active_writer1 = MCOptional<tid_t>::nil();
  MCOptional<tid_t> active_writer2 = MCOptional<tid_t>::nil();
  std::vector<tid_t> active_readers;

  std::queue<tid_t> reader_queue  = std::queue<tid_t>();
  std::queue<tid_t> writer1_queue = std::queue<tid_t>();
  std::queue<tid_t> writer2_queue = std::queue<tid_t>();

  // INVARIANT: Represents an interleaving of
  // the reader and writer queues. The acquisition
  // queue is used for locks with a Type of no_preference
  std::queue<tid_t> acquire_queue = std::queue<tid_t>();

public:

  /*
    For simplicity, we ignore the numerous possible
    thread orderings available and simply assume
    that the double-writer lock has only preferences
    between the two writers, for the readers, or no
    preference
  */
  enum Type {
    writer1_preferred,
    writer2_preferred,
    reader_preferred,
    no_preference
  } type = Type::no_preference;

  inline explicit MCRWWLock(MCRWWLockShadow shadow, Type type)
    : MCVisibleObject(), shadow(shadow), type(type)
  {}
  inline MCRWWLock(const MCRWWLock &rwwlock)
    : MCVisibleObject(rwwlock.getObjectId()), shadow(rwwlock.shadow),
      type(rwwlock.type), active_writer1(rwwlock.active_writer1),
      active_writer2(rwwlock.active_writer2),
      active_readers(rwwlock.active_readers),
      reader_queue(rwwlock.reader_queue),
      writer1_queue(rwwlock.writer1_queue),
      writer2_queue(rwwlock.writer2_queue),
      acquire_queue(rwwlock.acquire_queue)
  {}

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCRWWLock &) const;
  bool operator!=(const MCRWWLock &) const;

  bool canAcquireAsReader(tid_t) const;
  bool canAcquireAsWriter1(tid_t) const;
  bool canAcquireAsWriter2(tid_t) const;
  bool canAcquireAsWriter(tid_t) const;
  bool hasEnqueuedType1Writers() const;
  bool hasEnqueuedType2Writers() const;
  bool hasEnqueuedWriters() const;
  bool hasEnqueuedReaders() const;

  bool isReaderLocked() const;
  bool isWriterLocked() const;
  bool isWriter1Locked() const;
  bool isWriter2Locked() const;
  bool isUnlocked() const;
  bool isDestroyed() const;

  void enqueue_as_reader(tid_t);
  void enqueue_as_writer1(tid_t);
  void enqueue_as_writer2(tid_t);
  void reader_lock(tid_t);
  void writer1_lock(tid_t);
  void writer2_lock(tid_t);
  void unlock(tid_t);
  void init();
  void deinit();
};

#endif // INCLUDE_MCMINI_OBJECTS_MCRWWLOCK_HPP
