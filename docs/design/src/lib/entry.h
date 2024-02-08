#ifndef INCLUDE_ENTRY_HPP
#define INCLUDE_ENTRY_HPP
#include <stddef.h>

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

#endif

