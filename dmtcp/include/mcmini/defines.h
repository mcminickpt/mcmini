#pragma once

#include <stdint.h>

#define MCMINI_INLINE
#define MCMINI_LIBRARY_ENTRY_POINT
#define MCMINI_EXPORT __attribute__((visibility(default)))
#define MCMINI_PRIVATE __attribute__((visibility(hidden)))

#ifdef __cplusplus
#define MCMINI_THREAD_LOCAL thread_local
#else
#define MCMINI_THREAD_LOCAL _Thread_local
#endif

#ifdef __cplusplus
#define MCMINI_NO_RETURN [[noreturn]]
#else
#define MCMINI_NO_RETURN __attribute__((__noreturn__))
#endif

#define MAX_TOTAL_TRANSITIONS_IN_PROGRAM (1500u)
#define MAX_TOTAL_STATES_IN_STATE_STACK (MAX_TOTAL_TRANSITIONS_IN_PROGRAM + 1u)
#define MAX_TOTAL_THREADS_IN_PROGRAM (20u)
#define MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM (10000u)
#define MAX_TOTAL_STATE_OBJECTS_IN_PROGRAM \
  (MAX_TOTAL_THREADS_IN_PROGRAM + MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM)

#define THREAD_SHM_OFFSET (128)

typedef uint16_t runner_id_t;
typedef uint64_t trid_t;
#define RUNNER_ID_MAX UINT16_MAX
#define RID_MAIN_THREAD ((runner_id_t)0)
#define RID_INVALID ((runner_id_t)-1)
#define RID_PTHREAD_CREATE_FAILED ((runner_id_t)-2)

#define FORK_IS_CHILD_PID(pid) ((pid) == 0)
#define FORK_IS_PARENT_PID(pid) (!(FORK_IS_CHILD_PID(pid)))

#define PTHREAD_SUCCESS (0)
#define SEM_FLAG_SHARED (1)

typedef void *(*thread_routine)(void *);
typedef void (*free_function)(void *);
