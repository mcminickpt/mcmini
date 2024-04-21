#include "mcmini/transitions/cond/MCCondBroadcast.h"
#include "mcmini/mcmini_private.h" /* For trace_pid, and state */
#include "mcmini/transitions/mutex/MCMutexTransition.h"
#include "mcmini/misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include <pthread.h>
#include <string.h>
#include <sys/uio.h> /* For process_vm_readv() */

MCTransition *
MCReadCondBroadcast(const MCSharedTransition *shmTransition,
                    void *shmData, MCStack *state)
{
  // WAS: const MCSystemID condSystemId = *condInShm; condInShm=shmData;
  pthread_cond_t * condSystemId = *static_cast<pthread_cond_t **>(shmData);
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
      mcprintf("\n*** ERROR (pthread_cond_broadcast/traceId: %d): The cond var\n"
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
    !condThatExists->isDestroyed(),
    "Attempting to signal a destroyed condition variable");

  const auto threadThatRanId = shmTransition->executor;
  auto threadThatRan = state->getThreadWithId(threadThatRanId);
  return new MCCondBroadcast(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondBroadcast::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  return std::make_shared<MCCondBroadcast>(threadCpy, condCpy);
}

std::shared_ptr<MCTransition>
MCCondBroadcast::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> condInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  return std::make_shared<MCCondBroadcast>(threadInState,
                                           condInState);
}

void
MCCondBroadcast::applyToState(MCStack *state)
{
  this->conditionVariable->sendBroadcastMessage();
}

bool
MCCondBroadcast::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCCondBroadcast::dependentWith(const MCTransition *other) const
{
  const MCCondTransition *maybeCondOperation =
    dynamic_cast<const MCCondTransition *>(other);
  if (maybeCondOperation) {
    return *maybeCondOperation->conditionVariable ==
           *this->conditionVariable;
  }
  return false;
}

void
MCCondBroadcast::print() const
{
  printf("thread %lu: pthread_cond_broadcast(cond:%u)\n", this->thread->tid,
         countVisibleObjectsOfType(this->conditionVariable->getObjectId()));
}
