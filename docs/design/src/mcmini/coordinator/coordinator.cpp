#include "mcmini/coordinator/coordinator.hpp"

coordinator::coordinator(
    model::program &&initial_state,
    model::transition_registry &&runtime_transition_mapping,
    std::unique_ptr<real_world::process_source> &&process_source)
    : current_program_model(std::move(initial_state)),
      runtime_transition_mapping(std::move(runtime_transition_mapping)),
      process_source(std::move(process_source)) {
  // TODO: This may not be appropriate at construction time. Probably we only
  // need to do this when we actual ask the coordinator to do anything.
  this->current_process_handle = this->process_source->make_new_process();
}

void coordinator::execute_runner(runner::runner_id_t runner_id) {
  std::istream &wrapper_response_stream =
      this->current_process_handle->execute_runner(runner_id);
  model::transition_registry::runtime_type_id transition_registry_id;

  wrapper_response_stream >> transition_registry_id;

  // TODO: Handle the case where lookup fails (this indicates a failure on the
  // end of libmcmini.so... we should probably abort?)
  model_to_system_map remote_address_mapping = model_to_system_map(*this);
  model::transition_registry::transition_discovery_callback callback_function =
      runtime_transition_mapping.get_callback_for(transition_registry_id);
  std::unique_ptr<model::transition> transition_encountered_at_runtime =
      callback_function(wrapper_response_stream, remote_address_mapping);
  this->current_program_model.model_executing_runner(
      runner_id, std::move(transition_encountered_at_runtime));
}

void *model_to_system_map::get_remote_process_handle_for_object(
    model::state::objid_t id) const {
  // TODO:impl
  return nullptr;
}

optional<model::state::objid_t>
model_to_system_map::get_object_for_remote_process_handle(void *handle) const {
  if (_coordinator.system_address_mapping.count(handle) > 0) {
    return optional<model::state::objid_t>(
        _coordinator.system_address_mapping[handle]);
  }
  return optional<model::state::objid_t>();
}

model::state::objid_t model_to_system_map::record_new_object_association(
    void *remote_process_visible_object_handle,
    std::unique_ptr<model::visible_object_state> initial_state) {
  // TODO: Create a new object through the coordinator and then map handle
  // `remote_process_visible_object_handle` to the newly-created object.
  return 0;
}

model::state::objid_t model_to_system_map::observe_remote_process_handle(
    void *remote_process_visible_object_handle,
    std::unique_ptr<model::visible_object_state> fallback_initial_state) {
  return this
      ->get_object_for_remote_process_handle(
          remote_process_visible_object_handle)
      .value_or(this->record_new_object_association(
          remote_process_visible_object_handle,
          std::move(fallback_initial_state)));
}