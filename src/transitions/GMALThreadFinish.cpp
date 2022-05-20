#include "MCThreadFinish.h"

MCTransition*
MCReadThreadFinish(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    // TODO: Potentially add asserts that the thread that just ran exists!
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCThreadFinish(threadThatRan);
}

std::shared_ptr<MCTransition>
MCThreadFinish::staticCopy()
{
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto threadStartCpy = new MCThreadFinish(threadCpy);
    return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCThreadFinish::dynamicCopyInState(const MCState *state)
{
    // INVARIANT: Target and the thread itself are the same
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    auto cpy = new MCThreadFinish(threadInState);
    return std::shared_ptr<MCTransition>(cpy);
}

void
MCThreadFinish::applyToState(MCState *state)
{
    this->target->die();
}

bool
MCThreadFinish::enabledInState(const MCState *) {
    return thread->enabled() && thread->tid != TID_MAIN_THREAD;
}

bool
MCThreadFinish::coenabledWith(std::shared_ptr<MCTransition> transition) {
    if (this->thread->tid == transition->getThreadId()) {
        return false;
    }
    return true;
}

bool
MCThreadFinish::dependentWith(std::shared_ptr<MCTransition> transition) {
    if (this->thread->tid == transition->getThreadId()) {
        return true;
    }
    return false;
}

void
MCThreadFinish::print()
{
    printf("thread %lu: exits\n", this->thread->tid);
}

bool
MCThreadFinish::ensuresDeadlockIsImpossible()
{
    return this->thread->tid == TID_MAIN_THREAD;
}

bool
MCThreadFinish::countsAgainstThreadExecutionDepth()
{
    return false;
}
