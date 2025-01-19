#pragma once 

#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_signal.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_wait.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variable_enqueue_thread.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

model::transition* cond_init_callback(runner_id_t,
                                      const volatile runner_mailbox&,
                                      model_to_system_map&);

model::transition* cond_waiting_thread_enqueue_callback(runner_id_t,
                                                        const volatile runner_mailbox&,
                                                        model_to_system_map&);

model::transition* cond_wait_callback(runner_id_t,  
                                      const volatile runner_mailbox&,
                                      model_to_system_map&);
model::transition* cond_signal_callback(runner_id_t,
                                        const volatile runner_mailbox&,
                                        model_to_system_map&);
