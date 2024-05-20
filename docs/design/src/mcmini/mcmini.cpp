#include "mcmini/mcmini.hpp"

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/lib/shared_transition.h"
#include "mcmini/mem.h"
#include "mcmini/misc/ddt.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/misc/volatile_mem_streambuf.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model/transitions/mutex/mutex_init.hpp"
#include "mcmini/model/transitions/mutex/mutex_lock.hpp"
#include "mcmini/model/transitions/mutex/mutex_unlock.hpp"
#include "mcmini/model/transitions/thread/thread_create.hpp"
#include "mcmini/model/transitions/thread/thread_exit.hpp"
#include "mcmini/model/transitions/thread/thread_start.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor.hpp"
#include "mcmini/real_world/process/fork_process_source.hpp"
#include "mcmini/real_world/shm.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>

using namespace extensions;
using namespace model;
using namespace real_world;

void display_usage() {
  std::cout << "mcmini [options] <program>" << std::endl;
  std::exit(EXIT_FAILURE);
}

void finished_trace_classic_dpor(const coordinator& c) {
  static uint32_t trace_id = 0;

  std::stringstream ss;
  const auto& program_model = c.get_current_program_model();
  ss << "TRACE " << trace_id << "\n";
  for (const auto& t : program_model.get_trace()) {
    ss << "thread " << t->get_executor() << ": " << t->to_string() << "\n";
  }
  ss << "\nNEXT THREAD OPERATIONS\n";
  for (const auto& tpair : program_model.get_pending_transitions()) {
    ss << "thread " << tpair.first << ": " << tpair.second->to_string() << "\n";
  }
  std::cout << ss.str();
  std::cout.flush();
  trace_id++;
}

std::unique_ptr<model::transition> mutex_init_callback(
    state::runner_id_t p, const volatile runner_mailbox& rmb,
    model_to_system_map& m) {
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_mut, (volatile void*)rmb.cnts, sizeof(pthread_mutex_t*));
  state::objid_t mut =
      m.observe_remote_process_handle(remote_mut, objects::mutex::make());
  return make_unique<transitions::mutex_init>(p, mut);
}

std::unique_ptr<model::transition> mutex_lock_callback(
    state::runner_id_t p, const volatile runner_mailbox& rmb,
    model_to_system_map& m) {
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_mut, (volatile void*)rmb.cnts, sizeof(pthread_mutex_t*));
  state::objid_t mut =
      m.observe_remote_process_handle(remote_mut, objects::mutex::make());
  return make_unique<transitions::mutex_lock>(p, mut);
}

std::unique_ptr<model::transition> mutex_unlock_callback(
    state::runner_id_t p, const volatile runner_mailbox& rmb,
    model_to_system_map& m) {
  pthread_mutex_t* remote_mut;
  memcpy_v(&remote_mut, (volatile void*)rmb.cnts, sizeof(pthread_mutex_t*));
  state::objid_t mut =
      m.observe_remote_process_handle(remote_mut, objects::mutex::make());
  return make_unique<transitions::mutex_unlock>(p, mut);
}

std::unique_ptr<model::transition> thread_create_callback(
    state::runner_id_t p, const volatile runner_mailbox& rmb,
    model_to_system_map& m) {
  pthread_t new_thread;
  memcpy_v(&new_thread, static_cast<const volatile void*>(&rmb.cnts),
           sizeof(pthread_t));
  runner_id_t new_thread_id = m.observe_remote_process_runner(
      (void*)new_thread, objects::thread::make(objects::thread::state::running),
      [](runner_id_t id) {
        return make_unique<transitions::thread_start>(id);
      });
  return make_unique<transitions::thread_create>(p, new_thread_id);
}

std::unique_ptr<model::transition> thread_exit_callback(
    state::runner_id_t p, const volatile runner_mailbox& rmb,
    model_to_system_map& m) {
  return make_unique<transitions::thread_exit>(p);
}

void do_model_checking(
    /* Pass arguments here or rearrange to configure the checker at
    runtime, e.g. to pick an algorithm, set a max depth, etc. */) {
  state_sequence state_of_program_at_main;
  pending_transitions initial_first_steps;
  transition_registry tr;

  state::runner_id_t main_thread_id = state_of_program_at_main.add_runner(
      objects::thread::make(objects::thread::state::running));
  initial_first_steps.displace_transition_for(
      0, new transitions::thread_start(main_thread_id));

  program model_for_program_starting_at_main(state_of_program_at_main,
                                             std::move(initial_first_steps));

  // For "vanilla" model checking where we start at the beginning of the
  // program, a `fork_process_source suffices` (fork() + exec() brings us to the
  // beginning).
  auto process_source = make_unique<fork_process_source>("hello-world");

  tr.register_transition(MUTEX_INIT_TYPE, &mutex_init_callback);
  tr.register_transition(MUTEX_LOCK_TYPE, &mutex_lock_callback);
  tr.register_transition(MUTEX_UNLOCK_TYPE, &mutex_unlock_callback);
  tr.register_transition(THREAD_CREATE_TYPE, &thread_create_callback);
  tr.register_transition(THREAD_EXIT_TYPE, &thread_exit_callback);

  coordinator coordinator(std::move(model_for_program_starting_at_main),
                          std::move(tr), std::move(process_source));

  std::unique_ptr<model_checking::algorithm> classic_dpor_checker =
      make_unique<model_checking::classic_dpor>();

  model_checking::algorithm::callbacks c;
  c.trace_completed = &finished_trace_classic_dpor;

  classic_dpor_checker->verify_using(coordinator, c);
  std::cout << "Model checking completed!" << std::endl;
}

void do_model_checking_from_dmtcp_ckpt_file(std::string file_name) {
  model::detached_state state_of_program_at_main;
  model::pending_transitions initial_first_steps;  // TODO: Create initializer
                                                   // or else add other methods

  // // TODO: Complete the initialization of the initial state here, i.e. a
  // // single thread "main" that is alive and then running the transition

  {
      // Read that information from the linked list __inside the restarted
      // image__
      // while (! not all information read yet) {}
      // read(...);

      // auto state_of_some_object_in_the_ckpt_image = new mutex();
      // state_of_program_at_main.add_state_for();
  }

  {
    // initial_first_steps
    // Figure out what thread `N` is doing. This probably involves coordination
    // between libmcmini.so, libdmtcp.so, and the `mcmini` process
  }

  model::program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  // TODO: With a checkpoint restart, a fork_process_source doesn't suffice.
  // We'll need to create a different process source that can provide the
  // functionality we need to spawn new processes from the checkpoint image.
  auto process_source =
      extensions::make_unique<real_world::fork_process_source>("ls");

  coordinator coordinator(std::move(model_for_program_starting_at_main),
                          model::transition_registry(),
                          std::move(process_source));

  std::unique_ptr<model_checking::algorithm> classic_dpor_checker =
      extensions::make_unique<model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);

  std::cerr << "Model checking completed!" << std::endl;
}

int main_cpp(int argc, const char** argv) {
  install_process_wide_signal_handlers();
  do_model_checking();
  return EXIT_SUCCESS;
}

int main(int argc, const char** argv) {
  try {
    return main_cpp(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "ERROR: Unknown error occurred" << std::endl;
    return EXIT_FAILURE;
  }
}
