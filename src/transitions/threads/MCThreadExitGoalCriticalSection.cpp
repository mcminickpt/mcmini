#include "MCThreadExitGoalCriticalSection.h"

MCTransition*
MCReadThreadExitGoalCriticalSection(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCThreadExitGoalCriticalSection(threadThatRan);
}

std::shared_ptr<MCTransition>
MCThreadExitGoalCriticalSection::staticCopy() {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    return std::make_shared<MCThreadExitGoalCriticalSection>(threadCpy);
}

std::shared_ptr<MCTransition>
MCThreadExitGoalCriticalSection::dynamicCopyInState(const MCState *state) {
    // INVARIANT: Target and the thread itself are the same
    auto threadInState = state->getThreadWithId(thread->tid);
    return std::make_shared<MCThreadExitGoalCriticalSection>(threadInState);
}

void
MCThreadExitGoalCriticalSection::applyToState(MCState *) {
    this->thread->isInThreadCriticalSection = false;
}

void
MCThreadExitGoalCriticalSection::print()
{
    printf("thread %lu: EXIT GOAL CRITICAL SECTION\n", this->thread->tid);
}