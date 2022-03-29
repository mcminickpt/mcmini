#include "visible_operation.h"
#include "fail.h"
#include <stdio.h>

MEMORY_ALLOC_DEF_DECL(visible_operation)
PRETTY_PRINT_DEF_DECL(visible_operation)

void
visible_operation_pretty_off(visible_operation_refc vop, unsigned int off)
{
    switch (vop->type) {
        case MUTEX:
            mutex_operation_pretty_off(&vop->mutex_operation, off);
            break;
        case THREAD_LIFECYCLE:
            thread_operation_pretty_off(&vop->thread_operation, off);
            break;
        default:
            mc_unimplemented();
    }
}


bool
visible_operation_is_thread_operation(visible_operation_refc ref)
{
    return ref != NULL ? ref->type == THREAD_LIFECYCLE : false;
}

mutex_operation_ref
visible_operation_unsafely_as_mutex_operation(visible_operation_ref ref)
{
    return ref != NULL ? &ref->mutex_operation : NULL;
}

thread_operation_ref
visible_operation_unsafely_as_thread_operation(visible_operation_ref ref)
{
    return (ref != NULL) ? &ref->thread_operation : NULL;
}

visible_operation
dynamic_visible_operation_get_snapshot(dynamic_visible_operation_ref dref)
{
    if (!dref) return (visible_operation){};

//    switch (dref->type) {
//        case THREAD_LIFECYCLE:;
//            return (visible_operation){ .type = dref->type, .mutex_operation};
//        case MUTEX:;
//            return (visible_operation){ .type = dref->type, .mutex_operation};
//        default:
//            abort();
//    }
}