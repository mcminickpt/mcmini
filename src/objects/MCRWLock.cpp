#include "mcmini/objects/MCRWLock.h"

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
  return this->writer_queue.empty();
}

bool
MCRWLock::hasEnqueuedReaders() const
{
  return this->reader_queue.empty();
}

bool
MCRWLock::canAcquireAsReader(tid_t tid) const
{
  if (this->type == Type::writer_preferred && hasEnqueuedWriters())
    return false;

  return !isWriterLocked() && this->reader_queue.front() == tid;
}

bool
MCRWLock::canAcquireAsWriter(tid_t tid) const
{
  if (this->type == Type::reader_preferred && hasEnqueuedReaders())
    return false;
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
  this->active_readers.insert(tid);
}

void
MCRWLock::writer_lock(tid_t tid)
{
  MC_ASSERT(!this->active_writer.hasValue());

  if (!this->writer_queue.empty()) {
    MC_ASSERT(!this->writer_queue.front() == tid);
    this->writer_queue.pop();
  }
  this->active_writer = MCOptional<tid_t>::some(tid);
}

void
MCRWLock::enqueue_as_reader(tid_t tid)
{
  this->reader_queue.push(tid);
}

void
MCRWLock::enqueue_as_writer(tid_t tid)
{
  this->writer_queue.push(tid);
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
  } else if (active_readers.count(tid) > 0) {
    MC_ASSERT(isReaderLocked());
    active_readers.erase(tid);
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