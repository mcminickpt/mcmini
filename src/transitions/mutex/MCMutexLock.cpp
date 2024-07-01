#include "mcmini/transitions/mutex/MCMutexLock.h"
#include "mcmini/transitions/mutex/MCMutexUnlock.h"
#include "mcmini/transitions/threads/MCThreadCreate.h"
#include "mcmini/mcmini_private.h" /* For trace_pid, and state */
#include <pthread.h>
#include <string.h>
#include <memory>
#include <sys/uio.h> /* For process_vm_readv() */

MCTransition *
MCReadMutexLock(const MCSharedTransition *shmTransition,
                void *shmData, MCStack *state)
{
  auto mutexInShm = static_cast<MCMutexShadow *>(shmData);

  auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(
             (MCSystemID)mutexInShm->systemIdentity);

  // SEE:  src/transitions/mutex/MCMutexInit.cpp:MCReadMutexInit()
  bool testIfValidMutex = false;
  if (mutexThatExists == nullptr) {
    auto newMutex = new MCMutex(*mutexInShm);
    // If this has state 'undefined', then the mutexThatExists is still 'NULL'
    if (newMutex->mutexShadow.state == MCMutexShadow::undefined) {
      testIfValidMutex = true;
    }
    auto newMutexSharedPtr =
      std::shared_ptr<MCVisibleObject>(newMutex);
    // systemId is the pthread_mutex_t struct of the user).
    auto systemId = (MCSystemID)mutexInShm->systemIdentity;
    // This 'state' refers to 'MCStack' and not 'state' of mutex.
    // Register the mutex on the stack.
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newMutexSharedPtr);
    mutexThatExists =
      state->getVisibleObjectWithSystemIdentity<MCMutex>(systemId);
#if 0
    // This is an alternative:
    mutexThatExists =
      std::static_pointer_cast<MCMutex, MCVisibleObject>(newMutexSharedPtr);
#endif
    // This is still MCMutexShadow::undefined;
    // We set that in the next 'if' statement.
  }

  assert(mutexThatExists != nullptr);

  // NOTE:  MCMutexShadow != MCMutex::Shadow ; unlike MCCondShadow
  if (testIfValidMutex || mutexThatExists -> mutexShadow.state ==
                          MCMutexShadow::undefined) {
#ifdef HAS_PROCESS_VM
    // NOTE:  If the mutex is in the data or heap segment, then it will be
    // pre-initialized to all zero, which is same as PTHREAD_MUTEX_INITIALIZER.
    // So, uninitialized mutex'es won't be detected in this situation.
    pthread_mutex_t mutex_initializer = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t local_mut;
    struct iovec iov_remote_mut =
      { .iov_base = mutexInShm->systemIdentity,
        .iov_len = sizeof(mutex_initializer) };
    struct iovec iov_local_mut =
      { .iov_base = &local_mut, .iov_len = sizeof(mutex_initializer) };
    process_vm_readv(trace_pid, &iov_local_mut, 1, &iov_remote_mut, 1, 0);
    if (memcmp(&local_mut, &mutex_initializer, sizeof(mutex_initializer)) != 0){
      mcprintf("\n*** ERROR (pthread_mutex_lock/traceId: %d): The mutex was\n"
               "***     never initialized.\n"
               "***   Use pthread_mutex_init() or PTHREAD_MUTEX_INITIALIZER.\n"
               "***   Additional McMini traces will not work with this bug.\n"
               "***   McMini is exiting now.\n\n", traceId);
      // No need to patch it with process_vm_writev.  We will stop now.
      mc_stop_model_checking(EXIT_FAILURE);
    }
#endif
    if (mutexThatExists -> mutexShadow.state == MCMutexShadow::undefined) {
      mutexThatExists -> mutexShadow.state = MCMutexShadow::unlocked;
    }
  }

  // The mutex might have been locked or unlocked when this was called.
  assert(mutexThatExists -> mutexShadow.state == MCMutexShadow::unlocked ||
         mutexThatExists -> mutexShadow.state == MCMutexShadow::locked);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to lock an uninitialized mutex");
  if (mutexThatExists->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to lock a mutex that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCMutexLock(threadThatRan, mutexThatExists);
}

std::shared_ptr<MCTransition>
MCMutexLock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto mutexCpy = std::static_pointer_cast<MCMutex, MCVisibleObject>(
    this->mutex->copy());
  return std::make_shared<MCMutexLock>(threadCpy, mutexCpy);
}

std::shared_ptr<MCTransition>
MCMutexLock::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCMutex> mutexInState =
    state->getObjectWithId<MCMutex>(mutex->getObjectId());
  return std::make_shared<MCMutexLock>(threadInState, mutexInState);
}

void
MCMutexLock::applyToState(MCStack *state)
{
  this->mutex->lock(this->getThreadId());
}

void
MCMutexLock::unapplyToState(MCStack *state)
{
  this->mutex->unlock();
}

bool
MCMutexLock::isReversibleInState(const MCStack *state) const
{
  return false;
}

bool
MCMutexLock::enabledInState(const MCStack *state) const
{
  return this->mutex->canAcquire(this->getThreadId());
}

bool
MCMutexLock::coenabledWith(const MCTransition *transition) const
{
  {
    const MCMutexUnlock *maybeMutexUnlock =
      dynamic_cast<const MCMutexUnlock *>(transition);
    if (maybeMutexUnlock) {
      return *maybeMutexUnlock->mutex != *this->mutex;
    }
  }
  return true;
}

bool
MCMutexLock::dependentWith(const MCTransition *transition) const
{
  {
    const MCMutexLock *maybeMutexLock =
      dynamic_cast<const MCMutexLock *>(transition);
    if (maybeMutexLock) {
      return *maybeMutexLock->mutex == *this->mutex;
    }
  }

  {
    const MCMutexUnlock *maybeMutexUnlock =
      dynamic_cast<const MCMutexUnlock *>(transition);
    if (maybeMutexUnlock) {
      return *maybeMutexUnlock->mutex == *this->mutex;
    }
  }

  return false;
}

void
MCMutexLock::print() const
{
  mcprintf("thread %lu: pthread_mutex_lock(mut:%u)\n", this->thread->tid,
           countVisibleObjectsOfType(this->mutex->getObjectId()));
}
