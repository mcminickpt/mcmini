#include "mcmini/mcmini.hpp"

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor.hpp"
#include "mcmini/real_world/process/fork_process_source.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

void display_usage() {
  std::cout << "mcmini [options] <program>" << std::endl;
  std::exit(EXIT_FAILURE);
}

void do_model_checking(
    /* Pass arguments here or rearrange to configure the checker at
    runtime*/) {
  mcmini::model::detached_state state_of_program_at_main;
  mcmini::model::pending_transitions
      initial_first_steps;  // TODO: Create initializer or else add other
                            // methods

  // TODO: Complete the initialization of the initial state here, i.e. a
  // single thread "main" that is alive and then running the transition `t`

  mcmini::model::program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  mcmini::coordinator coordinator(
      std::move(model_for_program_starting_at_main),
      mcmini::extensions::make_unique<
          mcmini::real_world::fork_process_source>());

  std::unique_ptr<mcmini::model_checking::algorithm> classic_dpor_checker =
      mcmini::extensions::make_unique<mcmini::model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);

  std::cout << "Model checking completed!" << std::endl;
}

// int do_model_checking_from_dmtcp_ckpt_file(std::string file_name) {
//   mcmini::model::detached_state state_of_program_at_main;
//   mcmini::model::pending_transitions
//       initial_first_steps;  // TODO: Create initializer or else add other
//                             // methods

//   // // TODO: Complete the initialization of the initial state here, i.e. a
//   // // single thread "main" that is alive and then running the transition
//   `t`

//   // {
//   //   // Read that information from the linked list __inside the restarted
//   //   image__
//   //   // while (! not all information read yet) {}
//   //   // read(...);

//   //   auto state_of_some_object_in_the_ckpt_image = new mutex_state();

//   //   state_of_program_at_main.record_new_state_for_visible_object();
//   // }

//   // {
//   //   // initial_first_steps
//   //   // Figure out what thread 10 is doing or all this stuff
//   //   // this probably involves coordination between libmcmini.so,
//   libdmtcp.so,
//   //   // etc.
//   //   //
//   // }

//   mcmini::model::program model_for_program_starting_at_main(
//       std::move(state_of_program_at_main), std::move(initial_first_steps));

//   mcmini::coordinator coordinator(
//       std::move(model_for_program_starting_at_main),
//       mcmini::extensions::make_unique<
//           mcmini::real_world::fork_process_source>());

//   std::unique_ptr<mcmini::model_checking::algorithm> classic_dpor_checker =
//       mcmini::extensions::make_unique<mcmini::model_checking::classic_dpor>();

//   classic_dpor_checker->verify_using(coordinator);

//   std::cout << "Model checking completed!" << std::endl;
// }

int main(int argc, char **argv) {
  do_model_checking();
  // if (argc > 1) {
  //   char **cur_arg = &argv[1];
  //   char buf[1000];
  //   buf[sizeof(buf) - 1] = '\0';
  //   snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
  //            (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
  //            dirname(argv[0]));
  //   setenv("LD_PRELOAD", buf, 1);

  //   if (fork() == 0) {
  //     execvp(cur_arg[0], cur_arg);
  //   } else {
  //   }
  // } else {
  //   std::cout << "Droid is ready!" << std::endl;
  // }
}

///////////////////////////////////////////////////////

// int dmtcp_restart(void *cntx) {
//   // Check for different things
// }