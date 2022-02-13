#ifndef DPOR_DPOR_METHODS_H
#define DPOR_DPOR_METHODS_H

#include "transition.h"
#include "state_stack_item.h"
#include "common.h"

int transition_happens_before(struct transition, struct transition);
int transition_happens_after(struct transition, struct transition);
int transitions_dependent(struct transition, struct transition);
int transitions_independent(struct transition, struct transition);

void dynamically_update_backtrack_sets(struct state_stack_item);

#endif //DPOR_DPOR_METHODS_H
