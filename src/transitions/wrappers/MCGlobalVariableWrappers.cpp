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
mcmini_write(void *addr, char *varName, uint64_t value)
{
  auto writeData = MCGlobalVariableWriteData(addr, varName, value);
  thread_post_visible_operation_hit<MCGlobalVariableWriteData>(
    typeid(MCGlobalVariableWrite), &writeData);
  thread_await_scheduler();

  // FIXME: Write-value tracking currently supports values represented as
  // uint64_t. The LLVM instrumentation pass currently normalizes integer
  // stores to this representation. Supporting arbitrary value types would
  // require a generic representation & corresponding scheduler-side handling.
}
