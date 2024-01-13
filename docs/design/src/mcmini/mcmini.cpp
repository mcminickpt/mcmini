#include "mcmini/mcmini.hpp"

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/detail/ddt.hpp"
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
#include <utility>

void display_usage() {
  std::cout << "mcmini [options] <program>" << std::endl;
  std::exit(EXIT_FAILURE);
}

void do_model_checking(
    /* Pass arguments here or rearrange to configure the checker at
    runtime, e.g. to pick an algorithm, set a max depth, etc. */) {
  mcmini::model::detached_state state_of_program_at_main;
  mcmini::model::pending_transitions
      initial_first_steps;  // TODO: Create initializer or else add other
                            // methods
  /*
  TODO: Complete the initialization of the initial state here, i.e. a
  single thread "main" that is alive and then running the transition `t`
  */
  mcmini::model::program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  // For "vanilla" model checking where we start at the beginning of the
  // program, a fork_process_source suffices (fork() + exec() brings us to the
  // beginning)
  auto process_source =
      mcmini::extensions::make_unique<mcmini::real_world::fork_process_source>(
          "demo");

  mcmini::coordinator coordinator(std::move(model_for_program_starting_at_main),
                                  std::move(process_source));

  std::unique_ptr<mcmini::model_checking::algorithm> classic_dpor_checker =
      mcmini::extensions::make_unique<mcmini::model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);
  std::cout << "Model checking completed!" << std::endl;
}

void do_model_checking_from_dmtcp_ckpt_file(std::string file_name) {
  mcmini::model::detached_state state_of_program_at_main;
  mcmini::model::pending_transitions
      initial_first_steps;  // TODO: Create initializer or else add other
                            // methods

  // // TODO: Complete the initialization of the initial state here, i.e. a
  // // single thread "main" that is alive and then running the transition

  {
      // Read that information from the linked list __inside the restarted
      // image__
      // while (! not all information read yet) {}
      // read(...);

      // auto state_of_some_object_in_the_ckpt_image = new mutex_state();
      // state_of_program_at_main.record_new_state_for_visible_object();
  }

  {
    // initial_first_steps
    // Figure out what thread `N` is doing. This probably involves coordination
    // between libmcmini.so, libdmtcp.so, and the `mcmini` process
  }

  mcmini::model::program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  // TODO: With a checkpoint restart, a fork_process_source doesn't suffice.
  // We'll need to create a different process source that can provide the
  // functionality we need to spawn new processes from the checkpoint image.
  auto process_source =
      mcmini::extensions::make_unique<mcmini::real_world::fork_process_source>(
          "ls");

  mcmini::coordinator coordinator(std::move(model_for_program_starting_at_main),
                                  std::move(process_source));

  std::unique_ptr<mcmini::model_checking::algorithm> classic_dpor_checker =
      mcmini::extensions::make_unique<mcmini::model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);

  std::cerr << "Model checking completed!" << std::endl;
}

struct Base {
 public:
  virtual ~Base() = default;
};
struct Test : public Base {
  void greater_than(Test *) { std::cerr << "Yello" << std::endl; }
};

struct Test2 : public Test {
  void foo(Test *) { std::cerr << "Yello from test2" << std::endl; }
  int foobar(Test *) {
    std::cerr << "Yello from foobar" << std::endl;
    return 0;
  }

  int foobar2(Base *) {
    std::cerr << "Yello from foobar2" << std::endl;
    return 0;
  }
};

int main(int argc, char **argv) {
  do_model_checking();

  Base b;
  Test t1;
  Test t2;
  Test2 t22;

  // mcmini::detail::double_dispatch_member_function_table<Base, void(void)>
  // ddt;

  // ddt.register_dd_entry(&Test::greater_than);
  // ddt.register_dd_entry(&Test2::foo);

  // ddt.call(&t1, &t2);

  // ddt.call(&t22, &t2);=

  mcmini::detail::double_dispatch_member_function_table<Base, int(void)> ddt1;
  // ddt1;

  ddt1.register_dd_entry(&Test2::foobar);
  ddt1.register_dd_entry(&Test2::foobar2);

  int h = ddt1.call(&b, &t22).value();

  std::cerr << h << "\n";

  // auto y = std::make_index_sequence<10>{};
}
