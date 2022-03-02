#include "concurrent_system.h"
#include "common.h"
#include "hashtable.h"
#include "fail.h"

thread_local tid_t tid_self = TID_INVALID;

struct concurrent_system {
    tid_t tid_next; // <-- next available thread slot

    hash_table_ref transition_map;          /* Maps struct thread* to transition_ref in the transition stack */
    thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE];

    hash_table_ref mutex_map;               /* Maps pthread_mutex_t* to struct mutex* (pointer into the static array) */
    mutex locks[MAX_MUTEX_OBJECT_COUNT];    /* Memory backing the mutexes in the map */
};

tid_t
csystem_register_thread(concurrent_system_ref ref)
{
    tid_t self = ref->tid_next++;
    tid_self = self;
    thread_ref tself = &ref->threads[self];
    tself->arg = NULL;
    tself->start_routine = NULL;
    tself->owner = pthread_self();
    tself->tid = self;
    tself->is_alive = true;
    return self;
}

tid_t
csystem_register_main_thread(concurrent_system_ref ref)
{
    tid_t main = csystem_register_thread(ref);
    mc_assert(main == TID_MAIN_THREAD);
    return TID_MAIN_THREAD;
}

thread_ref
csystem_get_thread(concurrent_system_ref ref, tid_t tid)
{
    if (tid == TID_INVALID) return NULL;
    return &ref->threads[tid];
}