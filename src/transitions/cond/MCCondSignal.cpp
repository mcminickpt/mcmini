#include "MCCondSignal.h"
#include "MCMINI_Private.h"
#include "transitions/mutex/MCMutexTransition.h"

MCTransition*
MCReadCondSignal(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    const auto condInShm = static_cast<pthread_cond_t **>(shmData);
    const auto condSystemId = (MCSystemID)*condInShm;
    const auto condThatExists = state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(condSystemId);

    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(condThatExists != nullptr, "Attempting to signal a condition variable that is uninitialized");
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(!condThatExists->isDestroyed(), "Attempting to signal a destroyed condition variable");
    
    const auto threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCCondSignal(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondSignal::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto condCpy =
            std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(this->conditionVariable->copy());
    auto cpy = new MCCondSignal(threadCpy, condCpy);
    return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondSignal::dynamicCopyInState(const MCState *state)
{
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<MCConditionVariable> condInState = state->getObjectWithId<MCConditionVariable>(conditionVariable->getObjectId());
    auto cpy = new MCCondSignal(threadInState, condInState);
    return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondSignal::applyToState(MCState *state)
{
    /* Here's where the algorithm can change for signal */
    /* For simplicity, we assume that the first thread can be awoken */
    this->conditionVariable->wakeFirstThreadIfPossible();
}

bool
MCCondSignal::coenabledWith(std::shared_ptr<MCTransition> other)
{
    return true;
}

bool
MCCondSignal::dependentWith(std::shared_ptr<MCTransition> other)
{
    auto maybeCondOperation = std::dynamic_pointer_cast<MCCondTransition, MCTransition>(other);
    if (maybeCondOperation) {
        return *maybeCondOperation->conditionVariable == *this->conditionVariable;
    }
    return false;
}

void
MCCondSignal::print()
{
    printf("thread %lu: pthread_cond_signal(%lu)\n", this->thread->tid, this->conditionVariable->getObjectId());
}
