#pragma once

#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/transitions/mutex/mutex_init.hpp"
#include "mcmini/model/transitions/mutex/mutex_lock.hpp"
#include "mcmini/model/transitions/mutex/mutex_unlock.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

model::transition* mutex_init_callback(runner_id_t,
                                       const volatile runner_mailbox&,
                                       model_to_system_map&);
model::transition* mutex_lock_callback(runner_id_t,
                                       const volatile runner_mailbox&,
                                       model_to_system_map&);
model::transition* mutex_unlock_callback(runner_id_t,
                                         const volatile runner_mailbox&,
                                         model_to_system_map&);
