#include "GMALGlobalVariableWrite.h"

GMALTransition*
GMALReadGlobalWrite(const GMALSharedTransition *shmTransition, void *shmStart, GMALState *state)
{
    auto data = *(GMALGlobalVariableWriteData*)shmStart;
    auto threadThatRan = state->getThreadWithId(shmTransition->executor);
    auto globalVariable = state->getVisibleObjectWithSystemIdentity<GMALGlobalVariable>(data.addr);

    /* New global variable */
    if (globalVariable == nullptr)
        globalVariable = std::make_shared<GMALGlobalVariable>(data.addr);

    return new GMALGlobalVariableWrite(threadThatRan, globalVariable, data.newValue);
}

std::shared_ptr<GMALTransition>
GMALGlobalVariableWrite::staticCopy() {
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto globalCpy = std::static_pointer_cast<GMALGlobalVariable, GMALVisibleObject>(this->global->copy());
    auto newValueCpy = (void*)this->newValue;
    return std::make_shared<GMALGlobalVariableWrite>(threadCpy, globalCpy, newValueCpy);
}

std::shared_ptr<GMALTransition>
GMALGlobalVariableWrite::dynamicCopyInState(const GMALState *state) {
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    auto globalInState = state->getObjectWithId<GMALGlobalVariable>(global->getObjectId());

    // TODO: Verify if copying the value directly instead of storing with the associated object is correct
    auto newValueCpy = (void*)this->newValue;
    return std::make_shared<GMALGlobalVariableWrite>(threadInState, globalInState, newValueCpy);
}

bool
GMALGlobalVariableWrite::coenabledWith(std::shared_ptr<GMALTransition>)
{
    return true; /* Co-enabled with anything else */
}

bool
GMALGlobalVariableWrite::dependentWith(std::shared_ptr<GMALTransition> transition)
{
    auto maybeGlobalVariableAccess = std::dynamic_pointer_cast<GMALGlobalVariableTransition, GMALTransition>(transition);
    if (maybeGlobalVariableAccess != nullptr) {
        return *this->global == *maybeGlobalVariableAccess->global;
    }
    return false;
}

void
GMALGlobalVariableWrite::print()
{
    printf("thread %lu: write_global_var(%p, %p)\n", this->thread->tid, this->global->addr, this->newValue);
}
