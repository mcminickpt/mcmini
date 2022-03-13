#include "visible_operation.h"

MEMORY_ALLOC_DEF_DECL(visible_operation)

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