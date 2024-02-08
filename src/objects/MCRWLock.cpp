#include "mcmini/objects/MCRWLock.h"
#include <algorithm>

using namespace std;

std::shared_ptr<MCVisibleObject>
MCRWLock::copy()
{
  return std::make_shared<MCRWLock>(*this);
}

MCSystemID
MCRWLock::getSystemId()
{
  return this->shadow.systemIdentity;
}

bool
MCRWLock::operator==(const MCRWLock &other) const
{
  return this->shadow.systemIdentity == other.shadow.systemIdentity;
}

bool
MCRWLock::operator!=(const MCRWLock &other) const
{
  return this->shadow.systemIdentity != other.shadow.systemIdentity;
}

bool
MCRWLock::hasEnqueuedWriters() const
{
  return !this->writer_queue.empty();
}

bool
MCRWLock::hasEnqueuedReaders() const
{
  return !this->reader_queue.empty();
}

bool
MCRWLock::canAcquireAsReader(tid_t tid) const
{
  if (this->type == Type::writer_preferred && hasEnqueuedWriters()) {
    return false;
  }

  if (this->type == Type::no_preference) {
    return !isWriterLocked() && this->acquire_queue.front() == tid;
  }
  return !isWriterLocked() && this->reader_queue.front() == tid;
}

bool
MCRWLock::canAcquireAsWriter(tid_t tid) const
{
  if (this->type == Type::reader_preferred && hasEnqueuedReaders()) {
    return false;
  }

  if (this->type == Type::no_preference) {
    return isUnlocked() && this->acquire_queue.front() == tid;
  }

  return isUnlocked() && this->writer_queue.front() == tid;
}

bool
MCRWLock::isReaderLocked() const
{
  return !this->active_readers.empty();
}

bool
MCRWLock::isWriterLocked() const
{
  return this->active_writer.hasValue();
}

bool
MCRWLock::isUnlocked() const
{
  return !isWriterLocked() && !isReaderLocked();
}

bool
MCRWLock::isDestroyed() const
{
  return this->shadow.state == MCRWLockShadow::State::destroyed;
}

void
MCRWLock::reader_lock(tid_t tid)
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
MCRWLock::writer_lock(tid_t tid)
{
  MC_ASSERT(!this->active_writer.hasValue());
  MC_ASSERT(!this->writer_queue.empty());

  // Remove reader from the reader_queue
  MC_ASSERT(this->writer_queue.front() == tid);

  this->writer_queue.pop();
  this->acquire_queue.pop();
  this->active_writer = MCOptional<tid_t>::some(tid);
}

void
MCRWLock::enqueue_as_reader(tid_t tid)
{
  this->reader_queue.push(tid);
  this->acquire_queue.push(tid);
}

void
MCRWLock::enqueue_as_writer(tid_t tid)
{
  this->writer_queue.push(tid);
  this->acquire_queue.push(tid);
}

void
MCRWLock::unlock(tid_t tid)
{
  // INVARIANT: The thread unlocking should be
  // a reader or writer based on the state of the rwlock
  if (active_writer.hasValue()) {
    MC_ASSERT(active_writer.unwrapped() == tid);
    MC_ASSERT(isWriterLocked());
    this->active_writer = MCOptional<tid_t>::nil();
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
MCRWLock::init()
{
  this->shadow.state = MCRWLockShadow::State::unlocked;
}

void
MCRWLock::deinit()
{
  this->shadow.state = MCRWLockShadow::State::destroyed;
}