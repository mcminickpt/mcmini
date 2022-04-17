//
// Created by parallels on 4/10/22.
//

#include "GMALThreadStart.h"

GMALTransition*
GMALReadThreadStart(const GMALSharedTransition *shmTransition, void *shmData, GMALState *programState)
{
    // Should never be called
    GMAL_FATAL();
}

std::shared_ptr<GMALTransition>
GMALThreadStart::staticCopy() {
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto threadStartCpy = new GMALThreadStart(threadCpy);
    return std::shared_ptr<GMALTransition>(threadStartCpy);
}

std::shared_ptr<GMALTransition>
GMALThreadStart::dynamicCopyInState(const GMALState *state) {
    // INVARIANT: Target and the thread itself are the same
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    auto cpy = new GMALThreadStart(threadInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALThreadStart::applyToState(GMALState *) {
    // Nothing to do
    this->thread->spawn();
}

void
GMALThreadStart::unapplyToState(GMALState *) {
    // Nothing to do
    this->thread->despawn();
}
