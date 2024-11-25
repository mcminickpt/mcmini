#pragma once

#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/transitions/thread/thread_create.hpp"
#include "mcmini/model/transitions/thread/thread_exit.hpp"
#include "mcmini/model/transitions/thread/thread_join.hpp"
#include "mcmini/model/transitions/thread/thread_start.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

model::transition* thread_create_callback(runner_id_t,
                                          const volatile runner_mailbox&,
                                          model_to_system_map&);
model::transition* thread_exit_callback(runner_id_t,
                                        const volatile runner_mailbox&,
                                        model_to_system_map&);
model::transition* thread_join_callback(runner_id_t,
                                        const volatile runner_mailbox&,
                                        model_to_system_map&);
