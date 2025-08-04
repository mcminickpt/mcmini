#pragma once

#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/model/transition.hpp"

model::transition* process_exit_callback(runner_id_t,
                                         const volatile runner_mailbox&,
                                         model_to_system_map&);
model::transition* process_abort_callback(runner_id_t,
                                          const volatile runner_mailbox&,
                                          model_to_system_map&);
