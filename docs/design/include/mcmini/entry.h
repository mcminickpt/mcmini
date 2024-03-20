#ifndef INCLUDE_ENTRY_H
#define INCLUDE_ENTRY_H
#include <stddef.h>
#include <stdint.h>
#include "mcmini/shared_sem.h"
#include "mcmini/shared_transition.h"
#define MAX_TOTAL_THREADS_IN_PROGRAM         (20u)

/**
 * @brief The size of the shared memory allocation in bytes
 */
//TODO: What is the size of shared memory,how the transiyion info is stored?
extern const size_t shmAllocationSize;

/**
 * @brief The address at which the shared memory mailbox begins to
 * allow threads in a trace process to communicate with the scheduler
 */
extern void *shmStart;

extern shared_sem (
  *trace_list)[MAX_TOTAL_THREADS_IN_PROGRAM];

/**
 * @brief Initializes the variables in the global `trace_list`
 */
void initialize_trace_list();




#endif

