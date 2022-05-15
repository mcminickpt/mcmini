#include "GMALThreadReachGoal.h"

GMALTransition*
GMALReadThreadReachGoal(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALThreadReachGoal(threadThatRan);
}

std::shared_ptr<GMALTransition>
GMALThreadReachGoal::staticCopy() {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    return std::make_shared<GMALThreadReachGoal>(threadCpy);
}

std::shared_ptr<GMALTransition>
GMALThreadReachGoal::dynamicCopyInState(const GMALState *state) {
    // INVARIANT: Target and the thread itself are the same
    auto threadInState = state->getThreadWithId(thread->tid);
    return std::make_shared<GMALThreadReachGoal>(threadInState);
}

void
GMALThreadReachGoal::applyToState(GMALState *) {
    this->thread->markEncounteredThreadProgressPost();
}

bool
GMALThreadReachGoal::coenabledWith(std::shared_ptr<GMALTransition> transition)
{
    return true;
}

bool
GMALThreadReachGoal::dependentWith(std::shared_ptr<GMALTransition> transition)
{
    return false;
}

void
GMALThreadReachGoal::print()
{
    printf("thread %lu: GOAL()\n", this->thread->tid);
}

bool
GMALThreadReachGoal::countsAgainstThreadExecutionDepth()
{
    return false;
}
