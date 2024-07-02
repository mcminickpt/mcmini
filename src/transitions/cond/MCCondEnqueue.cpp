#include "mcmini/transitions/cond/MCCondEnqueue.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/mcmini_private.h" /* For trace_pid, and state */
#include "mcmini/transitions/mutex/MCMutexTransition.h"
#include "mcmini/transitions/mutex/MCMutexUnlock.h"
#include "mcmini/misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include <pthread.h>
#include <string.h>
#include <sys/uio.h> /* For process_vm_readv() */

MCTransition *
MCReadCondEnqueue(const MCSharedTransition *shmTransition,
                  void *shmData, MCStack *state)
{
  // WAS: const MCSystemID condSystemId = *condInShm; condInShm=shmData;
  pthread_cond_t * condSystemId = *static_cast<pthread_cond_t **>(shmData);
  // WAS: const auto mutexSystemId = (MCSystemID)mutexInShm;
  auto shmCond = static_cast<MCSharedMemoryConditionVariable *>(shmData);
  const auto mutexSystemId = (MCSystemID)shmCond->mutex;
  const auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(mutexSystemId);
  // We can't freeze this as 'const', since we might have to init cond later.
  auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      condSystemId);

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
    // This is still MCConditionVariable::Shadow::undefined;
    // We set that in the next 'if' statement.
  }

  assert(condThatExists != nullptr);
  assert(mutexThatExists != nullptr);

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
           MCConditionVariable::Shadow::initialized ||
         condThatExists -> shadow.state ==
           MCConditionVariable::Shadow::destroyed);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists->isLocked(),
    "Attempting to wait on a condition variable with a mutex that is unlocked.");

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !condThatExists->isDestroyed(),
    "Attempting to wait on a destroyed condition variable.");

  if (condThatExists->mutex != nullptr) {
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
      *condThatExists->mutex == *mutexThatExists,
      "Attempting to associate more than one mutex with a condition\n"
      "variable is undefined behavior. Ensure that you're calling\n"
      "pthread_cond_wait() with the same mutex.");
  }

  const auto threadThatRanId = shmTransition->executor;
  const auto mutexAssociatedWithConditionVariable =
    condThatExists->mutex;

  // NOTE: It's possible (and likely) for a condition variable
  // to NOT already be associated with a mutex at this point.
  // E.g., the first call to pthread_cond_wait() will have a
  // condition variable that isn't associated with a mutex.
  //    mutexThatExists->mutexShadow.systemIdentity
  if (mutexAssociatedWithConditionVariable) {
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
      *mutexAssociatedWithConditionVariable == *mutexThatExists,
      "A different mutex was previously bound to this condition variable,\n"
      "and at least one thread is still blocked on that previous mutex.\n"
      "But pthread_cond_wait is attempting to bind a new mutex to the\n"
      "condition variable, even while some thread is still blocked on\n"
      "the first mutex.");
  }

  // NOTE: We have to associate the mutex with the condition
  // variable when the transition is encountered; otherwise,
  // we wouldn't be able to determine if, e.g., a pthread_cond_wait()
  // enqueue call were dependent with a pthread_mutex_lock(). Note
  // that we ALSO must assign the mutex when the operation is APPLIED
  // to the condition variable
  condThatExists->mutex = mutexThatExists;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCCondEnqueue(threadThatRan, condThatExists,
                           mutexThatExists);
}

std::shared_ptr<MCTransition>
MCCondEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto mutCpy = std::static_pointer_cast<MCMutex, MCVisibleObject>(
    this->mutex->copy());
  auto cpy = new MCCondEnqueue(threadCpy, condCpy, mutCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondEnqueue::dynamicCopyInState(const MCStack *state) const
{
  auto threadInState = state->getThreadWithId(thread->tid);
  auto condInState   = state->getObjectWithId<MCConditionVariable>(
    conditionVariable->getObjectId());
  auto mutCpy =
    state->getObjectWithId<MCMutex>(this->mutex->getObjectId());
  auto cpy = new MCCondEnqueue(threadInState, condInState, mutCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondEnqueue::applyToState(MCStack *state)
{
  /* Insert this thread into the waiting queue */
  this->conditionVariable->addWaiter(this->getThreadId());
  this->conditionVariable->mutex = this->mutex;
  this->mutex->unlock();
}

bool
MCCondEnqueue::coenabledWith(const MCTransition *other) const
{
  const MCCondEnqueue *maybeCondEnqueueOperation =
    dynamic_cast<const MCCondEnqueue *>(other);
  if (maybeCondEnqueueOperation) {
    /* Only one cond_wait will be able to acquire the mutex */
    return *maybeCondEnqueueOperation->conditionVariable !=
           *this->conditionVariable;
  }

  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    auto unlockMutex =
      std::make_shared<MCMutexUnlock>(this->thread, this->mutex);
    return MCTransition::coenabledTransitions(unlockMutex.get(),
                                              maybeMutexOperation);
  }

  return true;
}

bool
MCCondEnqueue::dependentWith(const MCTransition *other) const
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
    auto unlockMutex =
      std::make_shared<MCMutexUnlock>(this->thread, this->mutex);
    return MCTransition::dependentTransitions(unlockMutex.get(),
                                              maybeMutexOperation);
  }
  return false;
}

void
MCCondEnqueue::print() const
{
  mcprintf("thread %lu: pthread_cond_wait(cond:%u, mut:%u) (awake -> asleep)\n",
           this->thread->tid,
           countVisibleObjectsOfType(this->conditionVariable->getObjectId()),
           countVisibleObjectsOfType(this->mutex->getObjectId()));
}
