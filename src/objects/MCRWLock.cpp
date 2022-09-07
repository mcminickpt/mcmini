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
MCRWLock::canAcquireAsReader(tid_t tid) const
{
  return !isWriterLocked();
}

bool
MCRWLock::canAcquireAsWriter(tid_t tid) const
{
  return isUnlocked();
}

bool
MCRWLock::isReaderLocked() const
{
  return !this->readers.empty();
}

bool
MCRWLock::isWriterLocked() const
{
  return this->writer.hasValue();
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
  this->readers.insert(tid);
}

void
MCRWLock::writer_lock(tid_t tid)
{
  this->writer = MCOptional<tid_t>::some(tid);
}

void
MCRWLock::unlock(tid_t tid)
{
  // INVARIANT: The thread unlocking should be
  // a reader or writer based on the state of the
  // rwlock
  if (writer.hasValue() && writer.unwrapped() == tid) {
    MC_ASSERT(isWriterLocked());
  } else if (readers.count(tid) > 0) {
    MC_ASSERT(isReaderLocked());
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