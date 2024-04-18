#include "mcmini/transitions/cond/MCCondEnqueue.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/mutex/MCMutexTransition.h"
#include "mcmini/transitions/mutex/MCMutexUnlock.h"
#include "mcmini/misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include <pthread.h>
#include <string.h>

MCTransition *
MCReadCondEnqueue(const MCSharedTransition *shmTransition,
                  void *shmData, MCStack *state)
{
  // FIXME:  MCCondSignal.cpp does:
  //       const auto condInShm    = static_cast<pthread_cond_t **>(shmData);
  //   Shouldn't we do it that way also here, instead of
  //   hiding the design with 'auto'?  - Gene
  const auto shmCond =
    static_cast<MCSharedMemoryConditionVariable *>(shmData);
  const auto condInShm     = shmCond->cond;
  const auto mutexInShm    = shmCond->mutex;
  const auto condSystemId  = (MCSystemID)condInShm;
  const auto mutexSystemId = (MCSystemID)mutexInShm;
  // We can't freeze this as 'const', since we might nave to init cond later.
  auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      condSystemId);
  const auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(mutexSystemId);

  /*** BAD DESIGN:
   *** McMini has two state variables:  MCStack and mcmini::ConditionVariable::Shadow;.state
   ***   where using MCConditionVariableShadow = mcmini::ConditionVariable::Shadow
   *** (You can distinguish them by observinnmg that one uses thpe 'auto' and the other uses tyhpe 'auto'.
   ***  You just havve to figure out nhhow the two non-equal thpes 'auto' are different.)
   *** McMmini has two condIn Shm:
   ***  condInShm2 = static_cast<pthread_cond_t **>(shmData); and condInShm = shmCond->cond;
   *** (You can distinguish them by observinnmg that one uses thpe 'auto' and the other uses tyhpe 'auto'.
   ***  You just havve to figure out nhhow the two non-equal thpes 'auto' are different.)
   ***/
  // *** We imitate the spaghetti code here, to get down to the 'state' for ConmInit (not CondEnqueue).
  // *** The XXX2 variables are taken from MCCondinit,cpp (same dir);  XXX2 is used to avoid name conflicts
  auto condInShm2 = static_cast<pthread_cond_t **>(shmData);
  auto systemId2  = (MCSystemID)*condInShm2;
  auto shadow2 = MCConditionVariableShadow(*condInShm2);
  auto state2 = shadow2.state;

  /*** BAD DESIGN:  mutexInShm->state works; but condInShm->state fails.
   ***              The code cleverly uses 'auto' to make sure that
   ***              developers can never be sure about what are the
   ***              the types for mutexInShm and condInShm.
   ***              This is the principle of 'data hiding' in OOD.  :-)
   ***              So, the design rule seems to be: don't extend
   ***              any code to support 'cond'.
   ***  We want:  MCConditionVariable->shadow.state, but by using the
   ***            the principle of 'data hiding', we use 'auto' to make
   ***            sure that it's impossible to decide which of
   ***            these variables has that type.
   ***/
  if (state2 == MCConditionVariableShadow::undefined) {
    // Then create and register an MCConditionVariable object.
    pthread_cond_t cond_initializer = PTHREAD_COND_INITIALIZER;
    // mutexinShm->systemIdentity, but condInShm already means 'pthread_cond_t *'
    // (Weird!  Why create a special type for 'pthread_cond_t *' ?)
    if (memcmp(condInShm, &cond_initializer,
               sizeof(cond_initializer)) != 0) {
      memcpy(condInShm, &cond_initializer,
             sizeof(cond_initializer));
      mcprintf("WARNING: A mutex wasn't initialized by pthread_cond_init()\n"
               "* We are initializing it now to PTHREAD_COND_INITIALIZER\n"
               "* You can ignore this if target code also initialized it to\n"
               "*   PTHREAD_COND_INITIALIZER\n");
    }

    // SEE:  src/transitions/cond/MCCondInit.cpp:MCReadCondInit()
    if (condThatExists == nullptr) {
      auto shadow = MCConditionVariableShadow(*condInShm2);

      // FIXME: Allow dynamic selection of wakeup policies.
      // For now, we hard-code it here. Not great, but at least
      // we can change it relatively easily still

      // FIXME:  We're assigning a condition variable policy of arbitrary.
      //         Is tnhis reasonable?
      auto policy = std::unique_ptr<mcmini::ConditionVariablePolicy>(
        new mcmini::ConditionVariableArbitraryPolicy());

      auto newCond =
        std::make_shared<mcmini::ConditionVariable>(shadow, std::move(policy));
      state->registerVisibleObjectWithSystemIdentity(systemId2, newCond);
      condThatExists =
        std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
          newCond);
    }
  }

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    condThatExists != nullptr, "Attempting to wait on a condition "
                               "variable that is uninitialized.");
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to wait on a condition variable with an "
    "uninitialized mutex.");
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists->isLocked(),
    "Attempting to wait on a condition variable with a "
    "mutex that is already unlocked.");
  // FIXME:  This stmt is repeated below.  Shouldn't we delete his one? - Gene
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
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !condThatExists->isDestroyed(),
    "Attempting to wait on a destroyed condition variable.");

  const auto threadThatRanId = shmTransition->executor;
  const auto mutexAssociatedWithConditionVariable =
    condThatExists->mutex;

  // NOTE: It's possible (and likely) for a condition variable
  // to NOT already be associated with a mutex at this point.
  // E.g., the first call to pthread_cond_wait() will have a
  // condition variable that isn't associated with a mutex.
  if (mutexAssociatedWithConditionVariable) {
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
      *mutexAssociatedWithConditionVariable == *mutexThatExists,
      "A mutex has already been associated with this condition "
      "variable. Attempting to wait on a condition variable using "
      "more "
      "than one mutex is undefined");
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
  printf("thread %lu: pthread_cond_wait(cond:%u, mut:%u) (awake -> asleep)\n",
         this->thread->tid,
         countVisibleObjectsOfType(this->conditionVariable->getObjectId()),
         countVisibleObjectsOfType(this->mutex->getObjectId()));
}
