#include "visible_operation.h"

MEMORY_ALLOC_DEF_DECL(visible_operation)

visible_operation_ref
visible_operation_copy(visible_operation_refc op)
{
    if (!op) return NULL;
    visible_operation_ref copy = visible_operation_alloc();
    copy->type = op->type;
    switch (op->type) {
        case THREAD_LIFECYCLE:
            copy->thread_operation = *thread_operation_copy(&op->thread_operation);
            break;
        case MUTEX:
            copy->mutex_operation = *mutex_operation_copy(&op->mutex_operation);
            break;
        default:
            // TODO: Implement for semaphores, etc.
            return NULL;
    }
    return copy;
}

void
visible_operation_destroy(visible_operation_ref op)
{
    if (!op) return;
    switch (op->type) {
        case THREAD_LIFECYCLE:
            thread_operation_destroy(&op->thread_operation);
            break;
        case MUTEX:
            mutex_operation_destroy(&op->mutex_operation);
            break;
        default:
              // TODO: Implement for semaphores, etc.
            return;
    }
}

bool
visible_operation_is_mutex_operation(visible_operation_refc ref)
{
   return ref != NULL ? ref->type == MUTEX : false;
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