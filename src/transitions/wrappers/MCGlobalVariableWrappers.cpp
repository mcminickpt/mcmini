#include "MCGlobalVariableWrappers.h"
#include "transitions/MCTransitionsShared.h"
#include "transitions/misc/MCGlobalVariableRead.h"
#include "transitions/misc/MCGlobalVariableWrite.h"

extern "C" {
#include "transitions/wrappers/MCSharedLibraryWrappers.h"
}

void *
mcmini_read(void *addr)
{
    thread_post_visible_operation_hit<void*>(typeid(MCGlobalVariableRead), &addr);
    thread_await_MC_scheduler();
    return addr;
}

void
mcmini_write(void *addr, void *newValue)
{
    auto writeData = MCGlobalVariableWriteData(addr, newValue);
    thread_post_visible_operation_hit<MCGlobalVariableWriteData>(typeid(MCGlobalVariableRead), &writeData);
    thread_await_MC_scheduler();

    // TODO: How do we perform the write here?
    /* */
}

