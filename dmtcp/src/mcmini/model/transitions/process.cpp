#include "mcmini/mem.h"
#include "mcmini/model/transitions/process/abort.hpp"
#include "mcmini/model/transitions/process/callbacks.hpp"
#include "mcmini/model/transitions/process/exit.hpp"

using namespace model;

transition* process_exit_callback(runner_id_t id,
                                  const volatile runner_mailbox& mb,
                                  model_to_system_map& map) {
  int exit_code = -1;
  memcpy_v(&exit_code, (volatile void*)mb.cnts, sizeof(exit_code));
  return new transitions::process_exit(id, exit_code);
}

transition* process_abort_callback(runner_id_t id,
                                   const volatile runner_mailbox& mb,
                                   model_to_system_map& map) {
  return new transitions::process_abort(id);
}
