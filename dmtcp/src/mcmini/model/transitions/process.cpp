#include "mcmini/model/transitions/process/abort.hpp"
#include "mcmini/model/transitions/process/callbacks.hpp"
#include "mcmini/model/transitions/process/exit.hpp"

using namespace model;

transition* process_exit_callback(runner_id_t id,
                                  const volatile runner_mailbox& mb,
                                  model_to_system_map& map) {
  return new transitions::process_exit(id);
}

transition* process_abort_callback(runner_id_t id,
                                   const volatile runner_mailbox& mb,
                                   model_to_system_map& map) {
  return new transitions::process_abort(id);
}
