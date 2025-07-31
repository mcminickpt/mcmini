#include "mcmini/coordinator/coordinator.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "mcmini/coordinator/model_to_system_map.hpp"
#include "mcmini/model/program.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transition_registry.hpp"
#include "mcmini/model/visible_object_state.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process.hpp"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/remote_address.hpp"

using namespace real_world;

coordinator::coordinator(
    model::program &&initial_state,
    model::transition_registry runtime_transition_mapping,
    std::unique_ptr<real_world::process_source> &&process_source)
    : current_program_model(std::move(initial_state)),
      runtime_transition_mapping(std::move(runtime_transition_mapping)),
      process_source(std::move(process_source)) {
  this->assign_new_process_handle();
}

void coordinator::execute_runner(process::runner_id_t runner_id) {
  if (!current_process_handle) {
    throw real_world::process::execution_error(
        "Failed to execute runner with id \"" + std::to_string(runner_id) +
        "\": the process is not alive");
  }
  volatile runner_mailbox *mb =
      this->current_process_handle->execute_runner(runner_id);
  model::transition_registry::runtime_type_id const rttid = mb->type;

  model::transition_registry::transition_discovery_callback callback_function =
      runtime_transition_mapping.get_callback_for(rttid);
  if (!callback_function) {
    throw real_world::process::execution_error(
        "Execution resulted in a runner scheduled to execute the transition "
        "type with the RTTID '" +
        std::to_string(rttid) +
        "' but this identifier has not been registered before model checking "
        "began. Double check that the coordinator was properly configured "
        "before launch; otherwise, please report this as a bug in "
        "libmcmini.so with this message.");
  }
  model_to_system_map remote_address_mapping = model_to_system_map(*this);
  model::transition *pending_operation =
      callback_function(runner_id, *mb, remote_address_mapping);
  if (!pending_operation) {
    throw real_world::process::execution_error(
        "Failed to translate the data written into the mailbox of runner " +
        std::to_string(runner_id));
  }
  this->current_program_model.model_execution_of(runner_id, pending_operation);

  log_very_verbose(logger)
      << "\n\n**************** AFTER MODEL EXEC *********************\n\n\n"
      << current_program_model.get_trace().back()->debug_string()
      << " just executed\n\n"
      << pending_operation->debug_string() << " will execute next "
      << "\n\n"
      << this->current_program_model.get_state_sequence().back().debug_string()
      << "\n\n**************** AFTER MODEL EXEC *********************\n\n";
}

void coordinator::return_to_depth(uint32_t n) {
  this->assign_new_process_handle();
  this->current_program_model.restore_model_at_depth(n);

  // Now regenerate the process from scratch. The new process handle has a state
  // represented in the model as the state at depth `n = 0` in the state
  // sequence; hence to ensure the model and the real world correspond, we must
  // re-execute the threads in the order specified in the transition sequence.
  assert(this->current_program_model.get_trace().count() == n);
  for (const model::transition *t : this->current_program_model.get_trace()) {
    this->current_process_handle->execute_runner(t->get_executor());
  }
  std::cerr << "\n\n**************** AFTER RESTORATION *********************\n\n";
  get_current_program_model().dump_state(std::cerr);
  std::cerr << "\n\n**************** AFTER RESTORATION *********************\n\n";
  std::cerr.flush();
}

model::state::runner_id_t model_to_system_map::get_model_of_runner(
    remote_address<void> handle) const {
  model::state::objid_t objid = get_model_of_object(handle);
  if (objid != model::invalid_objid) {
    return _coordinator.get_current_program_model()
        .get_state_sequence()
        .get_runner_id_for_obj(objid);
  }
  return model::invalid_rid;
}

model::state::objid_t model_to_system_map::get_model_of_object(
    remote_address<void> handle) const {
  if (_coordinator.system_address_mapping.count(handle) > 0) {
    return _coordinator.system_address_mapping[handle];
  }
  return model::invalid_objid;
}

model::state::objid_t model_to_system_map::observe_object(
    real_world::remote_address<void> rp_vobj_handle,
    const model::visible_object_state *vobs) {
  if (contains(rp_vobj_handle)) {
    throw std::runtime_error(
        "Attempting to rebind a remote address to an object that already "
        "exists in the model. Did you check that the object doesn't already "
        "exist?");
  }
  model::state::objid_t const new_objid =
      _coordinator.current_program_model.get_current_state().add_object(vobs);
  _coordinator.system_address_mapping.insert({rp_vobj_handle, new_objid});
  return new_objid;
}

model::state::runner_id_t model_to_system_map::observe_runner(
    real_world::remote_address<void> rp_vobj_handle,
    const model::runner_state *vobs, const model::transition *t) {
  return observe_runner(std::move(rp_vobj_handle), vobs,
                        [t](runner_id_t id) { return t; });
}

model::state::runner_id_t model_to_system_map::observe_runner(
    real_world::remote_address<void> rp_vobj_handle,
    const model::runner_state *vobs, runner_generation_function f) {
  if (contains(rp_vobj_handle)) {
    throw std::runtime_error(
        "Attempting to rebind a remote address to an object that already "
        "exists in the model. Did you check that the object doesn't already "
        "exist?");
  }
  model::state::runner_id_t const new_runner_id =
      _coordinator.current_program_model.discover_runner(vobs, std::move(f));
  model::state::objid_t const new_objid =
      _coordinator.get_current_program_model()
          .get_state_sequence()
          .get_objid_for_runner(new_runner_id);
  _coordinator.system_address_mapping.insert({rp_vobj_handle, new_objid});
  return new_runner_id;
}

void model_to_system_map::observe_runner_transition(
    const model::transition *t) {
  _coordinator.current_program_model.get_pending_transitions().set_transition(
      t);
}

model::state::runner_id_t model_to_system_map::observe_runner(
    real_world::remote_address<void> rp_vobj_handle,
    const model::runner_state *rs) {
  // TODO: Remove code duplication here. Also there's a better way to do all of
  // this I think (instead of e.g. needing to add similar methods onto
  // `model::program`)
  model::state::runner_id_t const new_runner_id =
      _coordinator.current_program_model.get_current_state().add_runner(rs);
  model::state::objid_t const new_objid =
      _coordinator.get_current_program_model()
          .get_state_sequence()
          .get_objid_for_runner(new_runner_id);
  _coordinator.system_address_mapping.insert({rp_vobj_handle, new_objid});
  return new_runner_id;
}
