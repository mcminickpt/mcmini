#ifndef MC_MCCONSTANTS_H
#define MC_MCCONSTANTS_H

#include <stdint.h>

#define MAX_TOTAL_THREADS_IN_PROGRAM         (20u)
#define MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM (10000u)
#define MAX_SHARED_MEMORY_ALLOCATION         (4096u)
#define MAX_TOTAL_STATE_OBJECTS_IN_PROGRAM \
  (MAX_TOTAL_THREADS_IN_PROGRAM +          \
   MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM)

// MAX_TOTAL_TRANSITIONS_IN_PROGRAM is different from
//   ENV_MAX_TRANSITIONS_DEPTH_LIMIT
#define MAX_TOTAL_TRANSITIONS_IN_PROGRAM ((int)1e9)
#define MAX_TOTAL_STATES_IN_STATE_STACK ((int)1e6)
/**
 * A configuration constant which specifies that a branch may
 * execute as many transitions as it wants, given that the
 * total number of transitions in the search tree does not exceed
 * the default bound set by MAX_TOTAL_TRANSITIONS_IN_PROGRAM
 */
#define MC_STATE_CONFIG_MAX_TRANSITIONS_DEPTH_LIMIT_DEFAULT (1500)

typedef uint64_t tid_t;
typedef uint64_t mutid_t;
typedef uint64_t trid_t;
#define TID_MAIN_THREAD           (0ul)
#define TID_INVALID               (-1ul) // ULONG_MAX
#define TID_PTHREAD_CREATE_FAILED (-2ul) // ULONG_MAX - 1
#define MUTID_INVALID             (-1ul) // ULONG_MAX

#define FORK_IS_CHILD_PID(pid)    ((pid) == 0)
#define FORK_IS_PARENT_PID(pid)   (!(FORK_IS_CHILD_PID(pid)))

/**
 * A configuration constant which specifies that threads
 * may execute as many transitions as they would like (i.e. are
 * not limited to an execution depth)
 */
#define MC_STATE_CONFIG_MAX_DEPTH_PER_THREAD_DEFAULT (UINT64_MAX)
#define MC_STATE_CONFIG_PRINT_AT_TRACE  (UINT64_MAX)

/* The livelock algorithm for pattern detection uses the following constants:
 *   1. LLOCK_MAX_PATTERN_SIZE: Maximum size that a candidate pattern can grow
 *                              before returning no livelock
 *   2. LLOCK_MAX_SCAN_DEPTH: Number of transitions for which McMini scans
 *                            for confirming a livelock
 *   3. LLOCK_MIN_PATTERN_REPEATS: Minimum number of times a pattern must
 *                                 repeat to be declared livelock
 *   4. LLOCK_MIN_SCAN_DEPTH: The pattern size times the number of repetitions
 *                            must be at least this value, in order to be
 *                            declared livelock
 */
#define LLOCK_MAX_PATTERN_SIZE (20)
#define LLOCK_MIN_PATTERN_REPEATS (3)
#define LLOCK_MIN_SCAN_DEPTH (50)
#define LLOCK_INCREASED_MAX_TRANSITIONS_DEPTH (LLOCK_MAX_PATTERN_SIZE * LLOCK_MIN_PATTERN_REPEATS)
#define LLOCK_MAX_SCAN_DEPTH (LLOCK_INCREASED_MAX_TRANSITIONS_DEPTH)

#ifdef MC_SHARED_LIBRARY
#define MC_CONSTRUCTOR __attribute__((constructor))
#else
#define MC_CONSTRUCTOR
#endif

#define PTHREAD_SUCCESS (0)
#define SEM_FLAG_SHARED (1)

typedef void *(*thread_routine)(void *);
typedef void (*free_function)(void *);

#endif // MC_MCCONSTANTS_H
