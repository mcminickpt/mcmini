#include "mcmini/mem.h"
#include "mcmini/model/exception.hpp"
#include "mcmini/model/transitions/semaphore/callbacks.hpp"
#include "mcmini/model/transitions/semaphore/sem_init.hpp"
#include "mcmini/model/transitions/semaphore/sem_post.hpp"
#include "mcmini/model/transitions/semaphore/sem_wait.hpp"

using namespace model;
using namespace objects;

model::transition* sem_init_callback(runner_id_t p,
                                     const volatile runner_mailbox& rmb,
                                     model_to_system_map& m) {
  // Fetch the remote object
  int count;
  sem_t* remote_sem;
  memcpy_v(&remote_sem, (volatile void*)rmb.cnts, sizeof(sem_t*));
  memcpy_v(&count, (volatile void*)(rmb.cnts + sizeof(sem_t*)), sizeof(int));

  // Locate the corresponding model of this object
  if (!m.contains(remote_sem)) m.observe_object(remote_sem, new semaphore());

  const state::objid_t model_sem = m.get_model_of_object(remote_sem);
  return new transitions::sem_init(p, model_sem, count);
}

model::transition* sem_post_callback(runner_id_t p,
                                     const volatile runner_mailbox& rmb,
                                     model_to_system_map& m) {
  sem_t* remote_sem;
  memcpy_v(&remote_sem, (volatile void*)rmb.cnts, sizeof(sem_t*));

  // TODO: Add code from Gene's PR here to check for initialized semaphores
  if (!m.contains(remote_sem))
    throw undefined_behavior_exception(
        "Attempting to post to an uninitialized semaphore");

  const state::objid_t model_sem = m.get_model_of_object(remote_sem);
  return new transitions::sem_post(p, model_sem);
}

model::transition* sem_wait_callback(runner_id_t p,
                                     const volatile runner_mailbox& rmb,
                                     model_to_system_map& m) {
  sem_t* remote_sem;
  memcpy_v(&remote_sem, (volatile void*)rmb.cnts, sizeof(sem_t*));

  // TODO: Add code from Gene's PR here to check for initialized semaphores
  if (!m.contains(remote_sem))
    throw undefined_behavior_exception(
        "Attempting to wait on an uninitialized semaphore");

  const state::objid_t model_sem = m.get_model_of_object(remote_sem);
  return new transitions::sem_wait(p, model_sem);
}
