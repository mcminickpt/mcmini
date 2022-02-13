#ifndef DPOR_DPOR_H
#define DPOR_DPOR_H

#include <stdlib.h>
#include <pthread.h>
#include "state_stack_item.h"

state_stack_ref global_stack;
static transtion_stack global_transition_stack;

void dpor_init(void);

#endif //DPOR_DPOR_H
