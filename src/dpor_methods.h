#ifndef DPOR_DPOR_METHODS_H
#define DPOR_DPOR_METHODS_H

#include "transition.h"

void dpor_init(void);

int transition_happens_before(struct transition, struct transition);
int transition_happens_after(struct transition, struct transition);
int transitions_dependent(struct transition, struct transition);
int transitions_independent(struct transition, struct transition);

#endif //DPOR_DPOR_METHODS_H
