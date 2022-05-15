#include "GMALCondSignal.h"
#include "GMAL.h"
#include "transitions/mutex/GMALMutexTransition.h"

GMALTransition*
GMALReadCondSignal(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    const auto condInShm = static_cast<pthread_cond_t **>(shmData);
    const auto condSystemId = (GMALSystemID)*condInShm;
    const auto condThatExists = state->getVisibleObjectWithSystemIdentity<GMALConditionVariable>(condSystemId);

    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(condThatExists != nullptr, "Attempting to signal a condition variable that is uninitialized");
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(!condThatExists->isDestroyed(), "Attempting to signal a destroyed condition variable");
    
    const auto threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALCondSignal(threadThatRan, condThatExists);
}

std::shared_ptr<GMALTransition>
GMALCondSignal::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto condCpy =
            std::static_pointer_cast<GMALConditionVariable, GMALVisibleObject>(this->conditionVariable->copy());
    auto cpy = new GMALCondSignal(threadCpy, condCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

std::shared_ptr<GMALTransition>
GMALCondSignal::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALConditionVariable> condInState = state->getObjectWithId<GMALConditionVariable>(conditionVariable->getObjectId());
    auto cpy = new GMALCondSignal(threadInState, condInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALCondSignal::applyToState(GMALState *state)
{
    /* Here's where the algorithm can change for signal */
    /* For simplicity, we assume that the first thread can be awoken */
    this->conditionVariable->wakeFirstThreadIfPossible();
}

bool
GMALCondSignal::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALCondSignal::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondOperation = std::dynamic_pointer_cast<GMALCondTransition, GMALTransition>(other);
    if (maybeCondOperation) {
        return *maybeCondOperation->conditionVariable == *this->conditionVariable;
    }
    return false;
}

void
GMALCondSignal::print()
{
    printf("thread %lu: pthread_cond_signal(%lu)\n", this->thread->tid, this->conditionVariable->getObjectId());
}
