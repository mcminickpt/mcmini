#include "GMALThreadFinish.h"

GMALTransition*
GMALReadThreadFinish(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    // TODO: Potentially add asserts that the thread that just ran exists!
    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALThreadFinish(threadThatRan);
}

std::shared_ptr<GMALTransition>
GMALThreadFinish::staticCopy()
{
    // INVARIANT: Target and the thread itself are the same
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto threadStartCpy = new GMALThreadFinish(threadCpy);
    return std::shared_ptr<GMALTransition>(threadStartCpy);
}

std::shared_ptr<GMALTransition>
GMALThreadFinish::dynamicCopyInState(const GMALState *state)
{
    // INVARIANT: Target and the thread itself are the same
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    auto cpy = new GMALThreadFinish(threadInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALThreadFinish::applyToState(GMALState *state)
{
    this->target->die();
}

void
GMALThreadFinish::unapplyToState(GMALState *state)
{
    this->target->regenerate();
}

bool
GMALThreadFinish::enabledInState(const GMALState *) {
    return thread->enabled() && thread->tid != TID_MAIN_THREAD;
}

bool
GMALThreadFinish::coenabledWith(std::shared_ptr<GMALTransition> transition) {
    if (this->thread->tid == transition->getThreadId()) {
        return false;
    }
    return true;
}

bool
GMALThreadFinish::dependentWith(std::shared_ptr<GMALTransition> transition) {
    if (this->thread->tid == transition->getThreadId()) {
        return true;
    }
    return false;
}
