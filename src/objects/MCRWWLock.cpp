#include "mcmini/objects/MCRWWLock.h"
#include <algorithm>

using namespace std;

std::shared_ptr<MCVisibleObject>
MCRWWLock::copy()
{
  return std::make_shared<MCRWWLock>(*this);
}

MCSystemID
MCRWWLock::getSystemId()
{
  return this->shadow.systemIdentity;
}

bool
MCRWWLock::operator==(const MCRWWLock &other) const
{
  return this->shadow.systemIdentity == other.shadow.systemIdentity;
}

bool
MCRWWLock::operator!=(const MCRWWLock &other) const
{
  return this->shadow.systemIdentity != other.shadow.systemIdentity;
}

bool
MCRWWLock::hasEnqueuedType1Writers() const
{
  return !this->writer1_queue.empty();
}

bool
MCRWWLock::hasEnqueuedType2Writers() const
{
  return !this->writer2_queue.empty();
}

bool
MCRWWLock::hasEnqueuedReaders() const
{
  return !this->reader_queue.empty();
}

bool
MCRWWLock::isReaderLocked() const
{
  return !this->active_readers.empty();
}

bool
MCRWWLock::isWriter1Locked() const
{
  return this->active_writer1.hasValue();
}

bool
MCRWWLock::isWriter2Locked() const
{
  return this->active_writer2.hasValue();
}

bool
MCRWWLock::isWriterLocked() const
{
  return isWriter1Locked() || isWriter2Locked();
}

bool
MCRWWLock::canAcquireAsReader(tid_t tid) const
{
  if (this->type == Type::writer1_preferred &&
      hasEnqueuedType1Writers()) {
    return false;
  }

  if (this->type == Type::writer2_preferred &&
      hasEnqueuedType2Writers()) {
    return false;
  }

  if (this->type == Type::no_preference) {
    return !isWriterLocked() && this->acquire_queue.front() == tid;
  }

  return !isWriterLocked() && this->reader_queue.front() == tid;
}

bool
MCRWWLock::canAcquireAsWriter1(tid_t tid) const
{
  if (this->type == Type::writer2_preferred &&
      hasEnqueuedType2Writers()) {
    return false;
  }

  if (this->type == Type::reader_preferred && hasEnqueuedReaders()) {
    return false;
  }

  if (this->type == Type::no_preference) {
    return isUnlocked() && this->acquire_queue.front() == tid;
  }

  return isUnlocked() && this->writer1_queue.front() == tid;
}

bool
MCRWWLock::canAcquireAsWriter2(tid_t tid) const
{
  if (this->type == Type::writer1_preferred &&
      hasEnqueuedType1Writers()) {
    return false;
  }

  if (this->type == Type::reader_preferred && hasEnqueuedReaders()) {
    return false;
  }

  if (this->type == Type::no_preference) {
    return isUnlocked() && this->acquire_queue.front() == tid;
  }

  return isUnlocked() && this->writer2_queue.front() == tid;
}

bool
MCRWWLock::isUnlocked() const
{
  return !isWriterLocked() && !isReaderLocked();
}

bool
MCRWWLock::isDestroyed() const
{
  return this->shadow.state == MCRWWLockShadow::State::destroyed;
}

void
MCRWWLock::reader_lock(tid_t tid)
{
  this->active_readers.push_back(tid);

  MC_ASSERT(!this->reader_queue.empty());
  MC_ASSERT(!this->acquire_queue.empty());

  // Remove reader from the reader_queue
  MC_ASSERT(this->reader_queue.front() == tid);

  this->reader_queue.pop();
  this->acquire_queue.pop();
}

void
MCRWWLock::writer1_lock(tid_t tid)
{
  MC_ASSERT(!this->active_writer1.hasValue());
  MC_ASSERT(!this->writer1_queue.empty());

  // Remove reader from the reader_queue
  MC_ASSERT(this->writer1_queue.front() == tid);

  this->writer1_queue.pop();
  this->acquire_queue.pop();
  this->active_writer1 = MCOptional<tid_t>::some(tid);
}

void
MCRWWLock::writer2_lock(tid_t tid)
{
  MC_ASSERT(!this->active_writer2.hasValue());
  MC_ASSERT(!this->writer2_queue.empty());

  // Remove reader from the reader_queue
  MC_ASSERT(this->writer2_queue.front() == tid);

  this->writer2_queue.pop();
  this->acquire_queue.pop();
  this->active_writer2 = MCOptional<tid_t>::some(tid);
}

void
MCRWWLock::enqueue_as_reader(tid_t tid)
{
  this->reader_queue.push(tid);
  this->acquire_queue.push(tid);
}

void
MCRWWLock::enqueue_as_writer1(tid_t tid)
{
  this->writer1_queue.push(tid);
  this->acquire_queue.push(tid);
}

void
MCRWWLock::enqueue_as_writer2(tid_t tid)
{
  this->writer2_queue.push(tid);
  this->acquire_queue.push(tid);
}

void
MCRWWLock::unlock(tid_t tid)
{
  // INVARIANT: The thread unlocking should be
  // a reader or writer based on the state of the rwlock
  if (isWriter1Locked()) {
    MC_ASSERT(active_writer1.unwrapped() == tid);
    this->active_writer1 = MCOptional<tid_t>::nil();
  } else if (isWriter2Locked()) {
    MC_ASSERT(active_writer2.unwrapped() == tid);
    this->active_writer2 = MCOptional<tid_t>::nil();
  } else if (!active_readers.empty()) {
    MC_ASSERT(isReaderLocked());
    const vector<tid_t>::iterator iter = std::find(
      this->active_readers.begin(), this->active_readers.end(), tid);
    MC_ASSERT(iter != this->active_readers.end());
    this->active_readers.erase(iter);
  } else {
    MC_FATAL();
  }
}

void
MCRWWLock::init()
{
  this->shadow.state = MCRWWLockShadow::State::unlocked;
}

void
MCRWWLock::deinit()
{
  this->shadow.state = MCRWWLockShadow::State::destroyed;
}