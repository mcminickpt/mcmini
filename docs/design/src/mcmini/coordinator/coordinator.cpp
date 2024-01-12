#include "mcmini/coordinator/coordinator.hpp"

using namespace mcmini;

coordinator::coordinator(
    model::program &&initial_state,
    std::unique_ptr<real_world::process_source> &&process_source)
    : current_program_model(std::move(initial_state)),
      process_source(std::move(process_source)) {
  // TODO: This may not be appropriate at construction time. Probably we only
  // need to do this when we actual ask the coordinator to do anything.
  this->current_process_handle = this->process_source->make_new_process();
}

void coordinator::execute_runner(runner::runner_id_t runner_id) {
  // this->current_process_handle->execute_runner(runner_id);
  // this->current_program_model.execute_runner(runner_id,);
  /* What's the new transition? */
}