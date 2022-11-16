#ifndef MC_MCCONSTANTS_H
#define MC_MCCONSTANTS_H

#include <stdint.h>

#define MAX_TOTAL_THREADS_IN_PROGRAM         (20u)
#define MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM (10000u)
#define MAX_SHARED_MEMORY_ALLOCATION         (4096u)
#define MAX_TOTAL_STATE_OBJECTS_IN_PROGRAM \
  (MAX_TOTAL_THREADS_IN_PROGRAM +          \
   MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM)

#define MAX_TOTAL_TRANSITIONS_IN_PROGRAM (1500u)
#define MAX_TOTAL_STATES_IN_STATE_STACK \
  (MAX_TOTAL_TRANSITIONS_IN_PROGRAM + 1u)

typedef uint64_t tid_t;
typedef uint64_t mutid_t;
typedef uint64_t trid_t;
#define TID_MAIN_THREAD           (0ul)
#define TID_INVALID               (-1ul) // ULONG_MAX
#define TID_PTHREAD_CREATE_FAILED (-2ul) // ULONG_MAX - 1
#define MUTID_INVALID             (-1ul) // ULONG_MAX

#define FORK_IS_CHILD_PID(pid)    ((pid) == 0)
#define FORK_IS_PARENT_PID(pid)   (!(FORK_IS_CHILD_PID(pid)))

#define MC_PROGRAM_TYPE           bool
#define MC_SCHEDULER              (false)
#define MC_TARGET_PROGRAM         (true)
#define MC_IS_SCHEDULER(expr) \
  (static_cast<bool>(expr) == MC_SCHEDULER)
#define MC_IS_TARGET_PROGRAM(expr) \
  (static_cast<bool>(expr) == MC_TARGET_PROGRAM)

#ifdef MC_SHARED_LIBRARY
#define MC_CTOR __attribute__((constructor))
#else
#define MC_CTOR
#endif

#define PTHREAD_SUCCESS (0)
#define SEM_FLAG_SHARED (1)

typedef void *(*thread_routine)(void *);
typedef void (*free_function)(void *);

#endif // MC_MCCONSTANTS_H
