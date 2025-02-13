#include "mcmini/model/transition_registry.hpp"

#include "mcmini/model/transitions/condition_variables/callbacks.hpp"
#include "mcmini/model/transitions/mutex/callbacks.hpp"
#include "mcmini/model/transitions/semaphore/callbacks.hpp"
#include "mcmini/model/transitions/thread/callbacks.hpp"
#include "mcmini/spy/checkpointing/transitions.h"

using namespace model;

transition_registry transition_registry::default_registry() {
  transition_registry tr;
  tr.register_transition(MUTEX_INIT_TYPE, &mutex_init_callback);
  tr.register_transition(MUTEX_LOCK_TYPE, &mutex_lock_callback);
  tr.register_transition(MUTEX_UNLOCK_TYPE, &mutex_unlock_callback);
  tr.register_transition(THREAD_CREATE_TYPE, &thread_create_callback);
  tr.register_transition(THREAD_EXIT_TYPE, &thread_exit_callback);
  tr.register_transition(THREAD_JOIN_TYPE, &thread_join_callback);
  tr.register_transition(COND_INIT_TYPE, &cond_init_callback);
  tr.register_transition(COND_ENQUEUE_TYPE,
                         &cond_waiting_thread_enqueue_callback);
  tr.register_transition(COND_WAIT_TYPE, &cond_wait_callback);
  tr.register_transition(COND_SIGNAL_TYPE, &cond_signal_callback);
  tr.register_transition(COND_BROADCAST_TYPE, &cond_broadcast_callback);
  tr.register_transition(COND_DESTROY_TYPE, &cond_destroy_callback);
  tr.register_transition(SEM_INIT_TYPE, &sem_init_callback);
  tr.register_transition(SEM_DESTROY_TYPE, &sem_destroy_callback);
  tr.register_transition(SEM_POST_TYPE, &sem_post_callback);
  tr.register_transition(SEM_WAIT_TYPE, &sem_wait_callback);
  return tr;
}
