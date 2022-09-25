#include "mcmini/transitions/threads/MCThreadCreate.h"

MCTransition *
MCReadThreadCreate(const MCSharedTransition *shmTransition,
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

  auto newThread     = state->getThreadWithId(newThreadId);
  auto threadThatRan = state->getThreadWithId(threadThatRanId);
  return new MCThreadCreate(threadThatRan, newThread);
}

std::shared_ptr<MCTransition>
MCThreadCreate::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto targetThreadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->target->copy());
  auto threadStartCpy =
    new MCThreadCreate(threadCpy, targetThreadCpy);
  return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCThreadCreate::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCThread> targetInState =
    state->getThreadWithId(target->tid);
  auto cpy = new MCThreadCreate(threadInState, targetInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCThreadCreate::applyToState(MCState *state)
{
  this->target->spawn();
}

void
MCThreadCreate::unapplyToState(MCState *state)
{
  this->target->despawn();
}

bool
MCThreadCreate::isReversibleInState(const MCState *state) const
{
  return false;
}

bool
MCThreadCreate::coenabledWith(const MCTransition *transition) const
{
  tid_t targetThreadId = transition->getThreadId();
  if (this->thread->tid == targetThreadId ||
      this->target->tid == targetThreadId) {
    return false;
  }
  return true;
}

bool
MCThreadCreate::dependentWith(const MCTransition *transition) const
{
  tid_t targetThreadId = transition->getThreadId();
  if (this->thread->tid == targetThreadId ||
      this->target->tid == targetThreadId) {
    return true;
  }
  return false;
}

bool
MCThreadCreate::doesCreateThread(tid_t tid) const
{
  return this->target->tid == tid;
}

void
MCThreadCreate::print() const
{
  printf("thread %lu: pthread_create(%lu)\n", this->thread->tid,
         this->target->tid);
}