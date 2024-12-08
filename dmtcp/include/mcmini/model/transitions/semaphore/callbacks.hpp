#pragma once

#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

model::transition* sem_init_callback(runner_id_t,
                                     const volatile runner_mailbox&,
                                     model_to_system_map&);
model::transition* sem_post_callback(runner_id_t,
                                     const volatile runner_mailbox&,
                                     model_to_system_map&);
model::transition* sem_wait_callback(runner_id_t,
                                     const volatile runner_mailbox&,
                                     model_to_system_map&);
