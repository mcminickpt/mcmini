#include "MCThreadRequestNewGoal.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

MCTransition*
MCReadThreadRequestNewGoal(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCThreadRequestNewGoal(threadThatRan);
}

std::shared_ptr<MCTransition>
MCThreadRequestNewGoal::staticCopy() {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    return std::make_shared<MCThreadRequestNewGoal>(threadCpy);
}

std::shared_ptr<MCTransition>
MCThreadRequestNewGoal::dynamicCopyInState(const MCState *state) {
    // INVARIANT: Target and the thread itself are the same
    auto threadInState = state->getThreadWithId(thread->tid);
    return std::make_shared<MCThreadRequestNewGoal>(threadInState);
}

void
MCThreadRequestNewGoal::applyToState(MCState *state)
{
    if (state->hasMaybeStarvedThread()) {
        return;
    }

    this->thread->markThreadAsMaybeStarved();

    const uint64_t numThreads = state->getNumProgramThreads();
    const uint64_t globalMaxExecutionDepth = state->getConfiguration().maxThreadExecutionDepth;
    const uint64_t extraLivenessTransitions = state->getConfiguration().extraLivenessTransitions;

    for (uint64_t i = 0; i < numThreads; i++) {
        const uint64_t threadLocalExecutionDepth = state->getCurrentExecutionDepthForThread(i);
        const uint64_t newMaxExecutionDepth = MAX(globalMaxExecutionDepth,
                                                  threadLocalExecutionDepth + extraLivenessTransitions);
        state->setMaximumExecutionDepthForThread(i, newMaxExecutionDepth);
    }
}

bool
MCThreadRequestNewGoal::enabledInState(const MCState *state)
{
    if (state->hasMaybeStarvedThread()) {
        return true;
    }

    tid_t thread = this->getThreadId();
    return state->getCurrentExecutionDepthForThread(thread) < state->getConfiguration().maxThreadExecutionDepth;
}


bool
MCThreadRequestNewGoal::coenabledWith(std::shared_ptr<MCTransition> transition)
{
    return true;
}

bool
MCThreadRequestNewGoal::dependentWith(std::shared_ptr<MCTransition> transition)
{
    return false;
}

void
MCThreadRequestNewGoal::print()
{
    printf("thread %lu: REQUEST()\n", this->thread->tid);
}

bool
MCThreadRequestNewGoal::countsAgainstThreadExecutionDepth()
{
    return false;
}
