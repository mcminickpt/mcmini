#pragma once

#include "mcmini/defines.h"

extern volatile void *global_shm_start;
extern MCMINI_THREAD_LOCAL runner_id_t tid_self;

void mc_exit(int);
runner_id_t mc_register_this_thread(void);



