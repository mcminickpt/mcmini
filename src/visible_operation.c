#include "visible_operation.h"

MEMORY_ALLOC_DEF_DECL(visible_operation)

visible_operation_ref
visible_operation_copy(visible_operation_refc other)
{
    if (!other) return NULL;
    visible_operation_ref copy = visible_operation_alloc();
    copy->type = other->type;
    switch (other->type) {
        case THREAD_LIFECYCLE:
            copy->thread_operation = thread_operation_copy(other->thread_operation);
        case MUTEX:
            copy->mutex_operation = mutex_operation_copy(other->mutex_operation);
        default:
#warning ""
            return NULL;
    }
    return copy;
}

void
visible_operation_destroy(visible_operation_ref op)
{
    if (!other) return NULL;
    visible_operation_ref copy = visible_operation_alloc();
    copy->type = other->type;
    switch (other->type) {
        case THREAD_LIFECYCLE:
            thread_operation_destroy(op->thread_operation);
        case MUTEX:
            mutex_operation_destroy(op->mutex_operation);
        default:
#warning ""
            return NULL;
    }
    return copy;
}