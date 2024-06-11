#pragma once

#include <pthread.h>

typedef enum {
  TEMPLATE,
  MODEL_CHKR_CNTRL,
  RECORD,
  TEMPLATE_FROM_CHECKPOINT,
} mode;

extern mode libmcmini_mode;

typedef enum mutex_state {
  UNINITIALIZED,
  UNLOCKED,
  LOCKED,
  DESTROYED
} mutex_state;

typedef struct rec_list {
  mutex_state state;
  pthread_mutex_t *mutex;
  struct rec_list *next;
} rec_list;

extern rec_list *head;
extern rec_list *current;
extern pthread_mutex_t rec_list_lock;

/// @brief Retrieves the stored state for the given mutex
/// @return a pointer to the node in the list formed by `head`,
/// or `NULL` if the mutex is not found
///
/// @note you must acquire `rec_list_lock` before calling this function
rec_list *find_mutex(pthread_mutex_t *);

/// @brief Adds a new element to the list `head`.
///
/// @note you must acquire `rec_list_lock` before calling this function
rec_list *add_rec_entry(pthread_mutex_t *, mutex_state);
