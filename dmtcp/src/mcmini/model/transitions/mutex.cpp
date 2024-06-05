#include "mcmini/mem.h"
#include "mcmini/model/exception.hpp"
#include "mcmini/model/transitions/mutex/callbacks.hpp"

using namespace model;
using namespace objects;

model::transition* mutex_init_callback(runner_id_t p,
                                       const volatile runner_mailbox& rmb,
                                       model_to_system_map& m) {
  // Fetch the remote object
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_mut, (volatile void*)rmb.cnts, sizeof(pthread_mutex_t*));

  // Locate the corresponding model of this object
  if (!m.contains(remote_mut))
    m.observe_object(remote_mut, new mutex(mutex::state_type::uninitialized));

  state::objid_t const mut = m.get_model_of_object(remote_mut);
  return new transitions::mutex_init(p, mut);
}

model::transition* mutex_lock_callback(runner_id_t p,
                                       const volatile runner_mailbox& rmb,
                                       model_to_system_map& m) {
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_mut, (volatile void*)rmb.cnts, sizeof(pthread_mutex_t*));

  // TODO: add code from Gene's PR here
  if (!m.contains(remote_mut))
    throw undefined_behavior_exception(
        "Attempting to lock an uninitialized mutex");

  state::objid_t const mut = m.get_model_of_object(remote_mut);
  return new transitions::mutex_lock(p, mut);
}

model::transition* mutex_unlock_callback(runner_id_t p,
                                         const volatile runner_mailbox& rmb,
                                         model_to_system_map& m) {
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_mut, (volatile void*)rmb.cnts, sizeof(pthread_mutex_t*));

  // TODO: add code from Gene's PR here
  if (!m.contains(remote_mut))
    throw undefined_behavior_exception(
        "Attempting to lock an uninitialized mutex");

  state::objid_t const mut = m.get_model_of_object(remote_mut);
  return new transitions::mutex_unlock(p, mut);
}
