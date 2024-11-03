#include "mcmini/mem.h"
#include "mcmini/model/exception.hpp"
#include "mcmini/model/transitions/thread/callbacks.hpp"

using namespace model;
using namespace objects;

model::transition* thread_create_callback(runner_id_t p,
                                          const volatile runner_mailbox& rmb,
                                          model_to_system_map& m) {
  pthread_t new_thread;
  memcpy_v(&new_thread, static_cast<const volatile void*>(&rmb.cnts),
           sizeof(pthread_t));
  if (!m.contains_runner((void*)new_thread))
    m.observe_runner(
        (void*)new_thread, new thread(thread::embryo),
        [](runner_id_t id) { return new transitions::thread_start(id); });
  const runner_id_t new_thread_id = m.get_model_of_runner((void*)new_thread);
  return new transitions::thread_create(p, new_thread_id);
}

model::transition* thread_exit_callback(runner_id_t p,
                                        const volatile runner_mailbox& rmb,
                                        model_to_system_map& m) {
  return new transitions::thread_exit(p);
}

model::transition* thread_join_callback(runner_id_t p,
                                        const volatile runner_mailbox& rmb,
                                        model_to_system_map& m) {
  pthread_t target;
  memcpy_v(&target, static_cast<const volatile void*>(&rmb.cnts),
           sizeof(pthread_t));
  const state::runner_id_t target_id = m.get_model_of_runner((void*)target);
  if (target_id == model::invalid_rid) {
    throw std::runtime_error("Attemping to join on an unmodeled thread");
  }
  return new transitions::thread_join(p, target_id);
}
