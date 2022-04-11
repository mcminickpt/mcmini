#ifndef GMAL_GMALCONSTANTS_H
#define GMAL_GMALCONSTANTS_H

#include <stdint.h>

#define MAX_TOTAL_THREADS_IN_PROGRAM (10u)
#define MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM (10000u)
#define MAX_SHARED_MEMORY_ALLOCATION (4096u)
#define MAX_TOTAL_STATE_OBJECTS_IN_PROGRAM (MAX_TOTAL_THREADS_IN_PROGRAM + MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM)

typedef uint64_t tid_t;
typedef uint64_t mutid_t;
#define TID_MAIN_THREAD (0ul)
#define TID_INVALID (-1ul) // ULONG_MAX
#define TID_PTHREAD_CREATE_FAILED (-2ul) // ULONG_MAX - 1
#define MUTID_INVALID (-1ul) // ULONG_MAX

#define FORK_IS_CHILD_PID(pid) ((pid) == 0)
#define FORK_IS_PARENT_PID(pid) (!(FORK_IS_CHILD_PID(pid)))

#define GMAL_PROGRAM_TYPE bool
#define GMAL_SCHEDULER (false)
#define GMAL_SOURCE_PROGRAM (true)
#define GMAL_IS_SCHEDULER(expr) (static_cast<bool>(expr) == GMAL_SCHEDULER)
#define GMAL_IS_SOURCE_PROGRAM(expr) (static_cast<bool>(expr) == GMAL_SOURCE_PROGRAM)

#define PTHREAD_SUCCESS (0)
#define SEM_FLAG_SHARED (1)

typedef void*(*thread_routine)(void*);
typedef void(*free_function)(void*);

#endif //GMAL_GMALCONSTANTS_H
