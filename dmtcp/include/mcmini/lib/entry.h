#pragma once

#include "mcmini/defines.h"

extern volatile void *global_shm_start;
extern MCMINI_THREAD_LOCAL runner_id_t tid_self;

void mc_prevent_addr_randomization(void);
void mc_install_sig_handlers(void);
runner_id_t mc_register_this_thread(void);
