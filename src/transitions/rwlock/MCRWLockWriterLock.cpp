#include "mcmini/transitions/rwlock/MCRWLockWriterLock.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/rwlock/MCRWLockReaderLock.h"
#include <pthread.h>
#include <string.h>
#include <sys/uio.h> /* For process_vm_readv() */

MCTransition *
MCReadRWLockWriterLock(const MCSharedTransition *shmTransition,
                       void *shmData, MCStack *state)
{
  // SEE: src/transitions/rwlock/MCRWLockInit.cpp
  // WAS: auto rwlockInShm = static_cast<MCRWLockShadow *>(shmData);
  auto systemId =
    (MCSystemID)static_cast<MCRWLockShadow *>(shmData)->systemIdentity;
  // rwLock appears to be the analog of rwlockThatExists.
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWLock>(systemId);

  bool testIfValidRwlock = false;
  if (rwLock == nullptr) {
    auto newRWLock = std::make_shared<MCRWLock>(
      *static_cast<MCRWLockShadow *>(shmData), MCRWLock::Type::no_preference);
    if (newRWLock->shadow.state == MCRWLockShadow::State::undefined) {
      testIfValidRwlock = true;
    }
    // Then create and register an MCConditionVariable object.
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newRWLock);
    rwLock = newRWLock;
  }

  assert(rwLock != nullptr);

  if (testIfValidRwlock || rwLock->shadow.state ==
                           MCRWLockShadow::State::undefined) {
#ifdef HAS_PROCESS_VM
    pthread_rwlock_t rwlock_initializer = PTHREAD_RWLOCK_INITIALIZER;
    // NOTE:  If PTHREAD_RWLOCK_INITIALIZER was used on a global variable,
    //        the macro is all zeros, and the data segment is all zeros.
    //        So, we can't tell if the initializer was or was not used.
    //          But if a local variable of type pthread_rwlock_t is not
    //        not initialized with PTHREAD_RWLOCK_INITIALIZER, and was
    //        not initialized with pthread_rwlock_init, then this code can
    //        truly catch an error.  pthread_rwlock_rwlock() would return
    //        error "Illegal argument", but only if the user had checked
    //        the return value of pthread_rwloc_wrlock, and then the errno.
    pthread_rwlock_t local_rwlock;
    struct iovec iov_remote_rwlock =
      { .iov_base = systemId, .iov_len = sizeof(rwlock_initializer) };
    struct iovec iov_local_rwlock =
      { .iov_base = &local_rwlock, .iov_len = sizeof(rwlock_initializer) };
    process_vm_readv(trace_pid, &iov_local_rwlock, 1, &iov_remote_rwlock, 1, 0);
    if (memcmp(&local_rwlock,
               &rwlock_initializer, sizeof(rwlock_initializer)) != 0) {
      mcprintf("\n*** ERROR (pthread_rwlock_wrlock/traceId: %d): The rwlock\n"
               "***     was never initialized.\n"
               "***   Use pthread_rwloc_init() or PTHREAD_RWLOCK_INITIALIZER.\n"
               "***   Additional McMini traces will not work with this bug.\n"
               "***   McMini is exiting now.\n\n", traceId);
      // No need to patch it with process_vm_writev.  We will stop now.
      mc_stop_model_checking(EXIT_FAILURE);
    }
#endif
    if (rwLock -> shadow.state ==
        MCRWLockShadow::State::undefined) {
      rwLock -> shadow.state = MCRWLockShadow::State::unlocked;
    }
  }

  assert(rwLock -> shadow.state == MCRWLockShadow::State::unlocked);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr, "Attempting to lock an uninitialized rw-lock");

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !rwLock->isDestroyed(), "Attempting to lock a destroyed rw-lock");

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWLockWriterLock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockWriterLock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockWriterLock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockWriterLock::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockWriterLock>(threadInState,
                                              liveRWLock);
}

void
MCRWLockWriterLock::applyToState(MCStack *state)
{
  this->rwlock->writer_lock(this->getThreadId());
}

bool
MCRWLockWriterLock::enabledInState(const MCStack *state) const
{
  return this->rwlock->canAcquireAsWriter(this->getThreadId());
}

bool
MCRWLockWriterLock::coenabledWith(const MCTransition *other) const
{
  {
    const MCRWLockReaderLock *maybeReaderLock =
      dynamic_cast<const MCRWLockReaderLock *>(other);
    if (maybeReaderLock) {
      return *maybeReaderLock->rwlock != *this->rwlock;
    }
  }

  {
    const MCRWLockWriterLock *maybeWriterLock =
      dynamic_cast<const MCRWLockWriterLock *>(other);
    if (maybeWriterLock) {
      return *maybeWriterLock->rwlock != *this->rwlock;
    }
  }
  return true;
}

bool
MCRWLockWriterLock::dependentWith(const MCTransition *other) const
{
  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockWriterLock::print() const
{
  mcprintf("thread %lu: pthread_rwlock_wrlock(rwl:%u) (lock)\n",
           this->thread->tid,
           countVisibleObjectsOfType(this->rwlock->getObjectId()));
}
