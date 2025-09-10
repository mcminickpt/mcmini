#include "transitions/wrappers/MCGlobalVariableWrappers.h"
#include "transitions/MCTransitionsShared.h"
#include "transitions/misc/MCGlobalVariableRead.h"
#include "transitions/misc/MCGlobalVariableWrite.h"

extern "C" {
#include "transitions/wrappers/MCSharedLibraryWrappers.h"
}

void *
mcmini_read(void *addr, char *varName)
{
  auto readData = MCGlobalVariableReadData(addr, varName);
  thread_post_visible_operation_hit<MCGlobalVariableReadData>(
    typeid(MCGlobalVariableRead), &readData);
  thread_await_scheduler();
  return addr;
}

void
mcmini_write(void *addr, char *varName)
{
  auto writeData = MCGlobalVariableWriteData(addr, varName);
  thread_post_visible_operation_hit<MCGlobalVariableWriteData>(
    typeid(MCGlobalVariableWrite), &writeData);
  thread_await_scheduler();

  // FIXME: We don't really support writes in general. We'd
  // need to define a template here that defined the write in
  // general terms. What's tricky is on the side of the scheduler:
  // each template specialization would need its own "read" handler,
  // ideally itself defined as a template
}
