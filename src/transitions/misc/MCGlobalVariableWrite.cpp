#include "MCGlobalVariableWrite.h"

MCTransition*
MCReadGlobalWrite(const MCSharedTransition *shmTransition, void *shmStart, MCState *state)
{
    auto data = *(MCGlobalVariableWriteData*)shmStart;
    auto threadThatRan = state->getThreadWithId(shmTransition->executor);
    auto globalVariable = state->getVisibleObjectWithSystemIdentity<MCGlobalVariable>(data.addr);

    /* New global variable */
    if (globalVariable == nullptr)
        globalVariable = std::make_shared<MCGlobalVariable>(data.addr);

    return new MCGlobalVariableWrite(threadThatRan, globalVariable, data.newValue);
}

std::shared_ptr<MCTransition>
MCGlobalVariableWrite::staticCopy() {
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto globalCpy = std::static_pointer_cast<MCGlobalVariable, MCVisibleObject>(this->global->copy());
    auto newValueCpy = (void*)this->newValue;
    return std::make_shared<MCGlobalVariableWrite>(threadCpy, globalCpy, newValueCpy);
}

std::shared_ptr<MCTransition>
MCGlobalVariableWrite::dynamicCopyInState(const MCState *state) {
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    auto globalInState = state->getObjectWithId<MCGlobalVariable>(global->getObjectId());

    // TODO: Verify if copying the value directly instead of storing with the associated object is correct
    auto newValueCpy = (void*)this->newValue;
    return std::make_shared<MCGlobalVariableWrite>(threadInState, globalInState, newValueCpy);
}

bool
MCGlobalVariableWrite::coenabledWith(std::shared_ptr<MCTransition>)
{
    return true; /* Co-enabled with anything else */
}

bool
MCGlobalVariableWrite::dependentWith(std::shared_ptr<MCTransition> transition)
{
    auto maybeGlobalVariableAccess = std::dynamic_pointer_cast<MCGlobalVariableTransition, MCTransition>(transition);
    if (maybeGlobalVariableAccess != nullptr) {
        return *this->global == *maybeGlobalVariableAccess->global;
    }
    return false;
}

void
MCGlobalVariableWrite::print()
{
    printf("thread %lu: write_global_var(%p, %p)\n", this->thread->tid, this->global->addr, this->newValue);
}
