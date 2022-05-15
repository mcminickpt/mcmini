#include "GMALGlobalVariableWrappers.h"
#include "transitions/GMALTransitionsShared.h"
#include "transitions/misc/GMALGlobalVariableRead.h"
#include "transitions/misc/GMALGlobalVariableWrite.h"

extern "C" {
#include "transitions/wrappers/GMALSharedLibraryWrappers.h"
}

void *
gmal_read(void *addr)
{
    thread_post_visible_operation_hit<void*>(typeid(GMALGlobalVariableRead), &addr);
    thread_await_gmal_scheduler();
    return addr;
}

void
gmal_write(void *addr, void *newValue)
{
    auto writeData = GMALGlobalVariableWriteData(addr, newValue);
    thread_post_visible_operation_hit<GMALGlobalVariableWriteData>(typeid(GMALGlobalVariableRead), &writeData);
    thread_await_gmal_scheduler();

    // TODO: How do we perform the write here?
    /* */
}

