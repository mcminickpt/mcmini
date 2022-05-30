#include "MCThreadEnterGoalCriticalSection.h"

MCTransition*
MCReadThreadEnterGoalCriticalSection(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCThreadEnterGoalCriticalSection(threadThatRan);
}

std::shared_ptr<MCTransition>
MCThreadEnterGoalCriticalSection::staticCopy() {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    return std::make_shared<MCThreadEnterGoalCriticalSection>(threadCpy);
}

std::shared_ptr<MCTransition>
MCThreadEnterGoalCriticalSection::dynamicCopyInState(const MCState *state) {
    // INVARIANT: Target and the thread itself are the same
    auto threadInState = state->getThreadWithId(thread->tid);
    return std::make_shared<MCThreadEnterGoalCriticalSection>(threadInState);
}

void
MCThreadEnterGoalCriticalSection::applyToState(MCState *) {
//    this->thread->isInThreadCriticalSection = true;
}

void
MCThreadEnterGoalCriticalSection::print()
{
    printf("thread %lu: EXIT GOAL CRITICAL SECTION\n", this->thread->tid);
}