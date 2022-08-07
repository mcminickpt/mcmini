#include "mcmini/transitions/threads/MCThreadJoin.h"

MCTransition *
MCReadThreadJoin(const MCSharedTransition *shmTransition,
                 void *shmData, MCState *state)
{
  // TODO: Potentially add asserts that the thread that just ran
  // exists!
  auto newThreadData = static_cast<MCThreadShadow *>(shmData);

  auto threadThatExists =
    state->getVisibleObjectWithSystemIdentity<MCThread>(
      (MCSystemID)newThreadData->systemIdentity);
  tid_t newThreadId     = threadThatExists != nullptr
                            ? threadThatExists->tid
                            : state->addNewThread(*newThreadData);
  tid_t threadThatRanId = shmTransition->executor;

  auto joinThread    = state->getThreadWithId(newThreadId);
  auto threadThatRan = state->getThreadWithId(threadThatRanId);
  return new MCThreadJoin(threadThatRan, joinThread);
}

std::shared_ptr<MCTransition>
MCThreadJoin::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto targetThreadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->target->copy());
  auto threadStartCpy = new MCThreadJoin(threadCpy, targetThreadCpy);
  return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCThreadJoin::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCThread> targetInState =
    state->getThreadWithId(target->tid);
  auto cpy = new MCThreadJoin(threadInState, targetInState);
  return std::shared_ptr<MCTransition>(cpy);
}

bool
MCThreadJoin::enabledInState(const MCState *) const
{
  return target->isDead();
}

void
MCThreadJoin::applyToState(MCState *state)
{
  // A thread join will only be executed by
  // a thread that's awake whose target thread
  // is already dead. Thus, we don't need
  // to update the thread's state at all.
  // As a sanity check, we put an assert
  MC_ASSERT(target->isDead());
}

void
MCThreadJoin::unapplyToState(MCState *state)
{
  // See above comment. The same
  // applies for state reversal: the thread
  // had to be awake before executing
  // this transition
}

bool
MCThreadJoin::isReversibleInState(const MCState *state) const
{
  return true;
}

bool
MCThreadJoin::coenabledWith(const MCTransition *transition) const
{
  tid_t targetThreadId = transition->getThreadId();
  if (this->thread->tid == targetThreadId ||
      this->target->tid == targetThreadId) {
    return false;
  }
  return true;
}

bool
MCThreadJoin::dependentWith(const MCTransition *transition) const
{
  tid_t targetThreadId = transition->getThreadId();
  if (this->thread->tid == targetThreadId ||
      this->target->tid == targetThreadId) {
    return true;
  }
  return false;
}

bool
MCThreadJoin::joinsOnThread(tid_t tid) const
{
  return this->target->tid == tid;
}

bool
MCThreadJoin::joinsOnThread(
  const std::shared_ptr<MCThread> &thread) const
{
  return this->target->tid == thread->tid;
}

void
MCThreadJoin::print() const
{
  printf("thread %lu: pthread_join(%lu, _)\n", this->thread->tid,
         this->target->tid);
}