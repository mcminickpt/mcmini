#include "GMALCondBroadcast.h"
#include "GMAL.h"
#include "GMALMutexTransition.h"

GMALTransition*
GMALReadCondBroadcast(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    const auto condInShm = static_cast<pthread_cond_t **>(shmData);
    const auto condSystemId = (GMALSystemID)*condInShm;
    const auto condThatExists = state->getVisibleObjectWithSystemIdentity<GMALConditionVariable>(condSystemId);

    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(condThatExists != nullptr, "Attempting to signal a condition variable that is uninitialized");
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(!condThatExists->isDestroyed(), "Attempting to signal a destroyed condition variable");

    const auto threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALCondBroadcast(threadThatRan, condThatExists);
}

std::shared_ptr<GMALTransition>
GMALCondBroadcast::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto condCpy =
            std::static_pointer_cast<GMALConditionVariable, GMALVisibleObject>(this->conditionVariable->copy());
    auto cpy = new GMALCondBroadcast(threadCpy, condCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

std::shared_ptr<GMALTransition>
GMALCondBroadcast::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALConditionVariable> condInState = state->getObjectWithId<GMALConditionVariable>(conditionVariable->getObjectId());
    auto cpy = new GMALCondBroadcast(threadInState, condInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALCondBroadcast::applyToState(GMALState *state)
{
    this->conditionVariable->wakeAllSleepingThreads();
}

bool
GMALCondBroadcast::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALCondBroadcast::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondOperation = std::dynamic_pointer_cast<GMALCondTransition, GMALTransition>(other);
    if (maybeCondOperation) {
        return *maybeCondOperation->conditionVariable == *this->conditionVariable;
    }
    return false;
}

void
GMALCondBroadcast::print()
{
    printf("thread %lu: pthread_cond_broadcast(%lu)\n", this->thread->tid, this->conditionVariable->getObjectId());
}
