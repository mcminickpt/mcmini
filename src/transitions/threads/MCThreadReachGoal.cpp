#include "MCThreadReachGoal.h"

MCTransition*
MCReadThreadReachGoal(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCThreadReachGoal(threadThatRan);
}

std::shared_ptr<MCTransition>
MCThreadReachGoal::staticCopy() {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    return std::make_shared<MCThreadReachGoal>(threadCpy);
}

std::shared_ptr<MCTransition>
MCThreadReachGoal::dynamicCopyInState(const MCState *state) {
    // INVARIANT: Target and the thread itself are the same
    auto threadInState = state->getThreadWithId(thread->tid);
    return std::make_shared<MCThreadReachGoal>(threadInState);
}

void
MCThreadReachGoal::applyToState(MCState *) {
    this->thread->maybeStarved = false;
    this->thread->maybeStarvedAndBlocked = false;
}

bool
MCThreadReachGoal::coenabledWith(std::shared_ptr<MCTransition> transition)
{
    return true;
}

bool
MCThreadReachGoal::dependentWith(std::shared_ptr<MCTransition> transition)
{
    return false;
}

void
MCThreadReachGoal::print()
{
    printf("thread %lu: GOAL()\n", this->thread->tid);
}

bool
MCThreadReachGoal::countsAgainstThreadExecutionDepth()
{
    return false;
}
