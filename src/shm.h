#ifndef DPOR_SHM_H
#define DPOR_SHM_H

#include "decl.h"
#include "visible_operation.h"
#include "transition.h"

STRUCT_DECL(shm_visible_operation)
struct shm_visible_operation {
    visible_operation_type type;
    union {
        mutex_operation mutex_operation;
        thread_operation thread_operation;
    };
};

STRUCT_DECL(shm_transition)
struct shm_transition {
    thread thread;
    shm_visible_operation operation;
};

transition_ref create_transition_from_shm(shm_transition_ref);


#endif //DPOR_SHM_H
