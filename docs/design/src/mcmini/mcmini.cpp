#include "mcmini/mcmini.hpp"

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/detail/ddt.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model/transitions/mutex/mutex_init.hpp"
#include "mcmini/model/transitions/thread/thread_start.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor.hpp"
#include "mcmini/real_world/process/fork_process_source.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <utility>

void display_usage() {
  std::cout << "mcmini [options] <program>" << std::endl;
  std::exit(EXIT_FAILURE);
}

void do_model_checking(
    /* Pass arguments here or rearrange to configure the checker at
    runtime, e.g. to pick an algorithm, set a max depth, etc. */) {
  using namespace extensions;
  using namespace model;
  using namespace real_world;

  detached_state state_of_program_at_main;
  pending_transitions initial_first_steps;

  state::objid_t thread_id =
      state_of_program_at_main.add_object(model::objects::thread::make());
  initial_first_steps.displace_transition_for(
      0, make_unique<transitions::thread_start>(thread_id));

  state_sequence ss;
  ss.follow(*initial_first_steps.get_transition_for_runner(0));

  std::exit(EXIT_SUCCESS);

  /*
  TODO: Complete the initialization of the initial state here, i.e. a
  single thread "main" that is alive and then running the transition `t`
  */
  program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  // For "vanilla" model checking where we start at the beginning of the
  // program, a fork_process_source suffices (fork() + exec() brings us to the
  // beginning)
  auto process_source = make_unique<fork_process_source>("demo");

  coordinator coordinator(std::move(model_for_program_starting_at_main),
                          transition_registry(), std::move(process_source));

  std::unique_ptr<model_checking::algorithm> classic_dpor_checker =
      make_unique<model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);
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

int main(int argc, char **argv) { do_model_checking(); }
