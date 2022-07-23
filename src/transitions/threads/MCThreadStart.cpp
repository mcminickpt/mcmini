#include "MCThreadStart.h"

MCTransition*
MCReadThreadStart(const MCSharedTransition *shmTransition, void *shmData, MCState *programState)
{
    // Should never be called
    MC_FATAL();
}

std::shared_ptr<MCTransition>
MCThreadStart::staticCopy() const {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto threadStartCpy = new MCThreadStart(threadCpy);
    return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCThreadStart::dynamicCopyInState(const MCState *state) const {
    // INVARIANT: Target and the thread itself are the same
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    auto cpy = new MCThreadStart(threadInState);
    return std::shared_ptr<MCTransition>(cpy);
}

void
MCThreadStart::applyToState(MCState *) {
    // Nothing to do
    this->thread->spawn();
}

bool
MCThreadStart::coenabledWith(const MCTransition *transition) const
{
    if (this->thread->tid == transition->getThreadId()) {
        return false;
    }

    // Technically, if the other transition is *never* enabled,
    // we could return false here. Such a transition is hard to
    // imagine though...
    return true;
}

bool
MCThreadStart::dependentWith(const MCTransition *transition) const
{
    return this->thread->tid == transition->getThreadId();
}

void
MCThreadStart::print() const
{
    printf("thread %lu: starts\n", this->thread->tid);
}

bool
MCThreadStart::countsAgainstThreadExecutionDepth() const
{
    return false;
}
