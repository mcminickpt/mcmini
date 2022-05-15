#include "GMALExitTransition.h"

GMALTransition* GMALReadExitTransition(const GMALSharedTransition *shmTransition, void *shmStart, GMALState *programState)
{
    auto executor = programState->getThreadWithId(shmTransition->executor);
    auto exitCode = *(int*)shmStart;
    return new GMALExitTransition(executor, exitCode);
}

std::shared_ptr<GMALTransition>
GMALExitTransition::staticCopy() {
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto threadStartCpy = new GMALExitTransition(threadCpy, exitCode);
    return std::shared_ptr<GMALTransition>(threadStartCpy);
}

std::shared_ptr<GMALTransition>
GMALExitTransition::dynamicCopyInState(const GMALState *state) {
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    auto cpy = new GMALExitTransition(threadInState, exitCode);
    return std::shared_ptr<GMALTransition>(cpy);
}

bool
GMALExitTransition::dependentWith(std::shared_ptr<GMALTransition>)
{
    return false;
}

bool
GMALExitTransition::enabledInState(const GMALState *)
{
    return false; // Never enabled
}

void
GMALExitTransition::print()
{
    printf("thread %lu: exit(%u)\n", this->thread->tid, this->exitCode);
}

bool
GMALExitTransition::ensuresDeadlockIsImpossible()
{
    return true;
}

bool
GMALExitTransition::countsAgainstThreadExecutionDepth()
{
    return false;
}

