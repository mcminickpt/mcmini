#pragma once

#include "mcmini/defines.h"

extern volatile void *global_shm_start;
extern MCMINI_THREAD_LOCAL tid_t tid_self;

void mc_exit(int);



