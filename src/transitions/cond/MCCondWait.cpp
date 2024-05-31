#include "mcmini/transitions/cond/MCCondWait.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/mcmini_private.h" /* For trace_pid, and state */
#include "mcmini/transitions/mutex/MCMutexLock.h"
#include "mcmini/transitions/mutex/MCMutexTransition.h"
#include "mcmini/misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include <pthread.h>
#include <string.h>
#include <sys/uio.h> /* For process_vm_readv() */

MCTransition *
MCReadCondWait(const MCSharedTransition *shmTransition, void *shmData,
               MCStack *state)
{
  // In theory, we should never see an undefined PTHREAD_COND_INITIALIZER.
  // src/transitions/cond/MCReadCondEnqueue.cpp should trigger first.
  const auto shmCond =
    static_cast<MCSharedMemoryConditionVariable *>(shmData);
  const auto mutexInShm    = shmCond->mutex;
  pthread_cond_t * condSystemId = *static_cast<pthread_cond_t **>(shmData);
  // We can't freeze this as 'const', since we might have to init cond later.
  const auto mutexSystemId = (MCSystemID)mutexInShm;
  // We can't freeze this as 'const', since we might have to init cond later.
  auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      condSystemId);
  const auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(mutexSystemId);

  bool testIfValidCond = false;
  // SEE:  src/transitions/cond/MCCondInit.cpp:MCReadCondInit()
  if (condThatExists == nullptr) {
    auto shadow = MCConditionVariable::Shadow(condSystemId);
    // If this has state 'undefined', then the condThatExists is still 'NULL'
    if (shadow.state == MCConditionVariable::Shadow::undefined) {
      testIfValidCond = true;
    }
    // FIXME: Allow dynamic selection of wakeup policies.
    // For now, we hard-code it here. Not great, but at least
    // we can change it relatively easily still

    // FIXME:  We're assigning a condition variable policy of arbitrary.
    //         Is this reasonable?
    auto policy = std::unique_ptr<mcmini::ConditionVariablePolicy>(
      new mcmini::ConditionVariableArbitraryPolicy());

    auto newCond =
      std::make_shared<mcmini::ConditionVariable>(shadow, std::move(policy));
    // Then create and register an MCConditionVariable object.
    state->registerVisibleObjectWithSystemIdentity(condSystemId, newCond);
    // condThatExists =
    //   std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
    //     newCond);
    // Get the current cond var on the stack, along with condSystemIdentity
    //   (the pthread_cond_t struct of the user).
    condThatExists =
      state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
        condSystemId);
#if 0
    // This is an alternative.
    condThatExists =
      std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(newCond);
#endif
    // This is still MCConditionVariable::Shadow::undefined;
    // We set that in the next 'if' statement.
  }

  if (testIfValidCond || condThatExists -> shadow.state ==
                         MCConditionVariable::Shadow::undefined) {
#ifdef HAS_PROCESS_VM
    pthread_cond_t cond_initializer = PTHREAD_COND_INITIALIZER;
    // NOTE:  If PTHREAD_COND_INITIALIZER was used on a global variable,
    //        the macro is all zeros, and the data segment is all zeros.
    //        So, we can't tell if the initializer was or was not used.
    //          But if a local variable of type pthread_cond_t is not
    //        not initialized with PTHREAD_COND_INITIALIZER, and was
    //        not initialized with pthread_cond_init, then this code can
    //        truly catch an error.  pthread_cond_signal() would return
    //        error "Illegal argument", but only if the user had checked
    //        the return value of pthread_cond_signal, and then the errno.
    pthread_cond_t local_cond;
    struct iovec iov_remote_cond =
      { .iov_base = condSystemId, .iov_len = sizeof(cond_initializer) };
    struct iovec iov_local_cond =
      { .iov_base = &local_cond, .iov_len = sizeof(cond_initializer) };
    process_vm_readv(trace_pid, &iov_local_cond, 1, &iov_remote_cond, 1, 0);
    if (memcmp(&local_cond, &cond_initializer, sizeof(cond_initializer)) != 0) {
      mcprintf("\n*** ERROR (pthread_cond_wait/traceId: %d): The cond var\n"
               "***     was never initialized.\n"
               "***   Use pthread_cond_init() or PTHREAD_COND_INITIALIZER.\n"
               "***   Additional McMini traces will not work with this bug.\n"
               "***   McMini is exiting now.\n\n", traceId);
      // No need to patch it with process_vm_writev.  We will stop now.
      mc_stop_model_checking(EXIT_FAILURE);
    }
#endif
    if (condThatExists -> shadow.state ==
        MCConditionVariable::Shadow::undefined) {
      condThatExists -> shadow.state = MCConditionVariable::Shadow::initialized;
    }
  }

  assert(condThatExists -> shadow.state ==
         MCConditionVariable::Shadow::initialized);
  assert(condThatExists != nullptr);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to wait on a condition variable with an "
    "uninitialized mutex");
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !condThatExists->isDestroyed(),
    "Attempting to wait on a destroyed condition variable");

  const auto threadThatRanId = shmTransition->executor;
  auto threadThatRan = state->getThreadWithId(threadThatRanId);
  return new MCCondWait(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondWait::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto cpy = new MCCondWait(threadCpy, condCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondWait::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> condInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  auto cpy = new MCCondWait(threadInState, condInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondWait::applyToState(MCStack *state)
{
  const tid_t threadId = this->getThreadId();
  this->conditionVariable->mutex->lock(threadId);
  this->conditionVariable->removeWaiter(threadId);
  // POSIX sttandard says that the dynamic binding of a condition variable
  // to a mutex is removed when the last thread waiting on that cond var is
  // unblocked (i.e., has re-acquired the associated mutex of the cond var).
  if (! this->conditionVariable->hasWaiters()) {
    this->conditionVariable->mutex = nullptr;
  }
}

bool
MCCondWait::enabledInState(const MCStack *) const
{
  const tid_t threadId = this->getThreadId();
  return this->conditionVariable->waiterCanExit(threadId) &&
         this->conditionVariable->mutex->canAcquire(threadId);
}

bool
MCCondWait::coenabledWith(const MCTransition *other) const
{
  auto maybeCondWaitOperation =
    dynamic_cast<const MCCondWait *>(other);
  if (maybeCondWaitOperation) {
    /* Only one cond_wait will be able to acquire the mutex */
    return *maybeCondWaitOperation->conditionVariable !=
           *this->conditionVariable;
  }

  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    auto lockMutex = std::make_shared<MCMutexLock>(
      this->thread, this->conditionVariable->mutex);
    return MCTransition::coenabledTransitions(lockMutex.get(),
                                              maybeMutexOperation);
  }

  return true;
}

bool
MCCondWait::dependentWith(const MCTransition *other) const
{
  const MCCondTransition *maybeCondOperation =
    dynamic_cast<const MCCondTransition *>(other);
  if (maybeCondOperation) {
    return *maybeCondOperation->conditionVariable ==
           *this->conditionVariable;
  }

  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    auto lockMutex = std::make_shared<MCMutexLock>(
      this->thread, this->conditionVariable->mutex);
    return MCTransition::coenabledTransitions(lockMutex.get(),
                                              maybeMutexOperation);
  }
  return false;
}

void
MCCondWait::print() const
{
  printf("thread %lu: pthread_cond_wait(cond:%u, mut:%u) (asleep -> awake)\n",
      this->thread->tid,
      countVisibleObjectsOfType(this->conditionVariable->getObjectId()),
      countVisibleObjectsOfType(this->conditionVariable->mutex->getObjectId()));
}
