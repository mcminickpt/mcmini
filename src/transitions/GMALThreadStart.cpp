//
// Created by parallels on 4/10/22.
//

#include "GMALThreadStart.h"

GMALTransition*
GMALReadThreadStart(void *shmStart, const GMALState &programState)
{
    std::shared_ptr<GMALThread> thread = programState.getThreadWithId(0);
    return new GMALThreadStart(thread);
}

std::shared_ptr<GMALTransition>
GMALThreadStart::staticCopy() {
    return nullptr;
}

std::shared_ptr<GMALTransition>
GMALThreadStart::dynamicCopyInState(const GMALState *state) {
    // INVARIANT: Target and the thread itself are the same
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    auto cpy = new GMALThreadStart(threadInState);
    auto sharedCpy = std::shared_ptr<GMALThreadStart>(cpy);
    return std::static_pointer_cast<GMALThreadStart, GMALTransition>(sharedCpy);
}

void
GMALThreadStart::applyToState(GMALState *) {

}

void
GMALThreadStart::unapplyToState(GMALState *) {

}
