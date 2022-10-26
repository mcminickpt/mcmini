#include "mcmini/transitions/wrappers/MCGlobalVariableWrappers.h"
#include "mcmini/transitions/MCTransitionsShared.h"
#include "mcmini/transitions/misc/MCGlobalVariableRead.h"
#include "mcmini/transitions/misc/MCGlobalVariableWrite.h"

extern "C" {
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"
}

void *
mcmini_read(void *addr)
{
  thread_post_visible_operation_hit<void *>(
    typeid(MCGlobalVariableRead), &addr);
  thread_await_scheduler();
  return addr;
}

void
mcmini_write(void *addr, void *newValue)
{
  auto writeData = MCGlobalVariableWriteData(addr, newValue);
  thread_post_visible_operation_hit<MCGlobalVariableWriteData>(
    typeid(MCGlobalVariableWrite), &writeData);
  thread_await_scheduler();

  // FIXME: We don't really support writes in general. We'd
  // need to define a template here that defined the write in
  // general terms. What's tricky is on the side of the scheduler:
  // each template specialization would need its own "read" handler,
  // ideally itself defined as a template
}
