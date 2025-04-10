#include "mcmini/mem.h"
#include "mcmini/model/exception.hpp"
#include "mcmini/model/transitions/condition_variables/callbacks.hpp"
#include "../include/mcmini/misc/cond/cond_var_arbitrary_policy.hpp"

using namespace model;
using namespace objects;

model::transition* cond_init_callback(runner_id_t p, 
                                      const volatile runner_mailbox& rmb,
                                      model_to_system_map& m) {
  // Fetch the remote object
  pthread_cond_t* remote_cond;
  memcpy_v(&remote_cond, (volatile void*)rmb.cnts, sizeof(pthread_cond_t*));

  // Locate the corresponding model of this object
  if (!m.contains(remote_cond)) {
    // FIXME: Allow dynamic selection of wakeup policies.
    // For now, we hard-code it here. Not great, but at least
    // we can change it relatively easily still
    ConditionVariablePolicy* policy = new ConditionVariableArbitraryPolicy();
      m.observe_object(remote_cond, 
      new condition_variable(
        condition_variable::state::cv_initialized, policy));  
  }
    
  state::objid_t const cond = m.get_model_of_object(remote_cond);
  return new transitions::condition_variable_init(p, cond);
}

model::transition* cond_waiting_thread_enqueue_callback(runner_id_t p,
                                                        const volatile runner_mailbox& rmb,
                                                        model_to_system_map& m){
  pthread_cond_t* remote_cond;
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_cond, (volatile void*)rmb.cnts, sizeof(pthread_cond_t*));
  memcpy_v(&remote_mut, (volatile void*)(rmb.cnts + sizeof(pthread_cond_t*)), sizeof(pthread_mutex_t*));

  if (!m.contains(remote_cond))
    throw undefined_behavior_exception(
        "Attempting to wait on an uninitialized condition variable");

  if (!m.contains(remote_mut))
    throw undefined_behavior_exception(
        "Attempting to wait on a condition variable with an uninitialized mutex");
  
  state::objid_t const cond = m.get_model_of_object(remote_cond);
  state::objid_t const mut = m.get_model_of_object(remote_mut);
  return new transitions::condition_variable_enqueue_thread(p, cond, mut);
}

model::transition* cond_wait_callback(runner_id_t p, 
                                      const volatile runner_mailbox& rmb,
                                      model_to_system_map& m) {
  pthread_cond_t* remote_cond;
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_cond, (volatile void*)rmb.cnts, sizeof(pthread_cond_t*));
  memcpy_v(&remote_mut, (volatile void*)(rmb.cnts + sizeof(pthread_cond_t*)), sizeof(pthread_mutex_t*));

  // Locate the corresponding model of this object
  if (!m.contains(remote_cond))
    throw undefined_behavior_exception(
        "Attempting to wait on an uninitialized condition variable");

  if (!m.contains(remote_mut))
    throw undefined_behavior_exception(
        "Attempting to wait on a condition variable with an uninitialized mutex");


  state::objid_t const cond = m.get_model_of_object(remote_cond);
  state::objid_t const mut = m.get_model_of_object(remote_mut);
  return new transitions::condition_variable_wait(p, cond, mut);
}

model::transition* cond_signal_callback(runner_id_t p, 
                                        const volatile runner_mailbox& rmb,
                                        model_to_system_map& m) {
  pthread_cond_t* remote_cond;
  memcpy_v(&remote_cond, (volatile void*)rmb.cnts, sizeof(pthread_cond_t*)); 

  // Locate the corresponding model of this object
  if (!m.contains(remote_cond))
    throw undefined_behavior_exception(
        "Attempting to signal an uninitialized condition variable");

  state::objid_t const cond = m.get_model_of_object(remote_cond);
  return new transitions::condition_variable_signal(p, cond);
}

model::transition* cond_broadcast_callback(runner_id_t p, 
                                           const volatile runner_mailbox& rmb,
                                           model_to_system_map& m) {
  pthread_cond_t* remote_cond;
  memcpy_v(&remote_cond, (volatile void*)rmb.cnts, sizeof(pthread_cond_t*)); 

  // Locate the corresponding model of this object
  if (!m.contains(remote_cond))
  throw undefined_behavior_exception(
  "Attempting to broadcast on an uninitialized condition variable");

  state::objid_t const cond = m.get_model_of_object(remote_cond);
  return new transitions::condition_variable_broadcast(p, cond);
}

model::transition* cond_destroy_callback(runner_id_t p, 
                                         const volatile runner_mailbox& rmb,
                                         model_to_system_map& m) {
  pthread_cond_t* remote_cond;
  memcpy_v(&remote_cond, (volatile void*)rmb.cnts, sizeof(pthread_cond_t*)); 

  // Locate the corresponding model of this object
  if (!m.contains(remote_cond))
  throw undefined_behavior_exception(
  "Attempting to destroy an uninitialized condition variable");

  state::objid_t const cond = m.get_model_of_object(remote_cond);
  return new transitions::condition_variable_destroy(p, cond);
}
