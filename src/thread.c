#include "thread.h"

thread_ref
thread_create(void) {
    thread_ref thread = malloc(sizeof(*thread));
    if (!thread) return NULL;
    thread->is_alive = true;
    thread->start_routine = NULL;
    thread->arg = NULL;
    return thread; // owner unspecified
}
thread_ref thread_copy(thread_refc);
void thread_destroy(thread_ref);

thread_ref
thread_wrap(pthread_t pthread) {
    thread_ref ref = thread_create();
    if (ref) {
        ref->owner = pthread;
    }
    return ref;
}