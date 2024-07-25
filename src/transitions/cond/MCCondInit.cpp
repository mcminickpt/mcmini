#include "transitions/cond/MCCondInit.h"
#include "misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include "mcmini_private.h"
#include <pthread.h>
#include <string.h>
#include <sys/uio.h> /* For process_vm_readv() */

using namespace std;
using namespace mcmini;

MCTransition *
MCReadCondInit(const MCSharedTransition *shmTransition, void *shmData,
               MCStack *state)
{
  // WAS: const MCSystemID condSystemId = *condInShm; condInShm=shmData;
  pthread_cond_t * condSystemId = *static_cast<pthread_cond_t **>(shmData);
  auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      condSystemId);

  // SEE:  src/transitions/cond/MCCondInit.cpp:MCReadCondInit()
  bool testIfValidCond = false;

  if (condThatExists == nullptr) {
    auto shadow = MCConditionVariableShadow(condSystemId);
    // If this has state 'undefined', then the condThatExists is still 'NULL'
    if (shadow.state == MCConditionVariable::Shadow::undefined) {
      testIfValidCond = true;
    }
    // FIXME: Allow dynamic selection of wakeup policies.
    // For now, we hard-code it here. Not great, but at least
    // we can change it relatively easily still

    // FIXME:  We're assigning a condition variable policy of arbitrary.
    //         Is this reasonable?
    auto policy = std::unique_ptr<ConditionVariablePolicy>(
      new ConditionVariableArbitraryPolicy());

    auto newCond =
      std::make_shared<ConditionVariable>(shadow, std::move(policy));
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
    // WAS: condThatExists =
    //   std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
    //     newCond);
    // This is still MCConditionVariable::Shadow::undefined;
    // We set that in the next 'if' statement.
  }

  assert(condThatExists != nullptr);

#if 1
  if (testIfValidCond) {
    condThatExists -> shadow.state = MCConditionVariable::Shadow::initialized;
  }

#else
  // We know that user has pthread_cond_init.  So, we could remove this 'if'.
  if (testIfValidCond || condThatExists -> shadow.state ==
                         MCConditionVariable::Shadow::undefined) {
    // Then create and register an MCConditionVariable object.
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
      mcprintf("\n*** ERROR (pthread_cond_init/traceId: %d): The cond var\n"
               "***     was never initialized.\n"
               "***   Use pthread_cond_init() or PTHREAD_COND_INITIALIZER.\n"
               "***   Additional McMini traces will not work with this bug.\n"
               "***   McMini is exiting now.\n\n", traceId);
      // No need to patch it with process_vm_writev.  We will stop now.
      mc_stop_model_checking(EXIT_FAILURE);
    }
    if (condThatExists -> shadow.state ==
        MCConditionVariable::Shadow::undefined) {
      condThatExists -> shadow.state = MCConditionVariable::Shadow::initialized;
    }
  }
#endif

  assert(condThatExists -> shadow.state ==
         MCConditionVariable::Shadow::initialized);

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCCondInit(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto condInit = new MCCondInit(threadCpy, condCpy);
  return std::shared_ptr<MCTransition>(condInit);
}

std::shared_ptr<MCTransition>
MCCondInit::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> mutexInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  auto cpy = new MCCondInit(threadInState, mutexInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondInit::applyToState(MCStack *state)
{
  this->conditionVariable->initialize();
}

bool
MCCondInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCCondInit::dependentWith(const MCTransition *other) const
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
MCCondInit::print() const
{
  mcprintf("thread %lu: pthread_cond_init(cond:%u, _)\n", this->thread->tid,
           countVisibleObjectsOfType(this->conditionVariable->getObjectId()));
}
