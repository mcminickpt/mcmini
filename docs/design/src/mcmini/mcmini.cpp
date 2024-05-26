#include <cstdint>
#include <memory>
#include <string>

#include "mcmini/common/transitions.h"
#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/defines.h"
#include "mcmini/mem.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/exception.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model/pending_transitions.hpp"
#include "mcmini/model/program.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transition_registry.hpp"
#include "mcmini/model/transitions/mutex/callbacks.hpp"
#include "mcmini/model/transitions/thread/callbacks.hpp"
#include "mcmini/model_checking/algorithm.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/fork_process_source.hpp"
#include "mcmini/signal.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>

using namespace extensions;
using namespace model;
using namespace model_checking;
using namespace objects;
using namespace real_world;

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

void do_model_checking(
    /* Pass arguments here or rearrange to configure the checker at
    runtime, e.g. to pick an algorithm, set a max depth, etc. */) {
  // For "vanilla" model checking where we start at the beginning of the
  // program, a `fork_process_source suffices` (fork() + exec() brings us to the
  // beginning).

  using namespace transitions;

  algorithm::callbacks c;
  transition_registry tr;
  detached_state state_of_program_at_main;
  pending_transitions initial_first_steps;
  classic_dpor::dependency_relation_type dr;
  classic_dpor::coenabled_relation_type cr;

  const state::runner_id_t main_thread_id = state_of_program_at_main.add_runner(
      new objects::thread(objects::thread::state::running));
  initial_first_steps.set_transition(new thread_start(main_thread_id));

  program model_for_program_starting_at_main(state_of_program_at_main,
                                             std::move(initial_first_steps));

  tr.register_transition(MUTEX_INIT_TYPE, &mutex_init_callback);
  tr.register_transition(MUTEX_LOCK_TYPE, &mutex_lock_callback);
  tr.register_transition(MUTEX_UNLOCK_TYPE, &mutex_unlock_callback);
  tr.register_transition(THREAD_CREATE_TYPE, &thread_create_callback);
  tr.register_transition(THREAD_EXIT_TYPE, &thread_exit_callback);
  tr.register_transition(THREAD_JOIN_TYPE, &thread_join_callback);

  coordinator coordinator(std::move(model_for_program_starting_at_main),
                          std::move(tr),
                          make_unique<fork_process_source>("hello-world"));

  dr.register_dd_entry<const thread_create>(&thread_create::depends);
  dr.register_dd_entry<const thread_join>(&thread_join::depends);

  dr.register_dd_entry<const mutex_lock, const mutex_init>(
      &mutex_lock::depends);
  dr.register_dd_entry<const mutex_lock, const mutex_lock>(
      &mutex_lock::depends);
  cr.register_dd_entry<const thread_create>(&thread_create::coenabled_with);
  cr.register_dd_entry<const thread_join>(&thread_join::coenabled_with);
  cr.register_dd_entry<const mutex_lock, const mutex_unlock>(
      &mutex_lock::coenabled_with);

  model_checking::classic_dpor classic_dpor_checker(std::move(dr),
                                                    std::move(cr));

  c.trace_completed = &finished_trace_classic_dpor;
  classic_dpor_checker.verify_using(coordinator, c);

  std::cout << "Model checking completed!" << std::endl;
}

void do_model_checking_from_dmtcp_ckpt_file(std::string file_name) {
  model::detached_state const state_of_program_at_main;
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
