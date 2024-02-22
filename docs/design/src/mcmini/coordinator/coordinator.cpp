#include "mcmini/coordinator/coordinator.hpp"

#include <iostream>

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

  // Check if the transition_registry_id read successfully
  if (!wrapper_response_stream) {
    // Log the error and abort the execution
    std::cerr << "Failed to read the transition_registry_id from the wrapper "
                 "response stream."
              << std::endl;
    // Handle the failure appropriately, potentially aborting the execution
    abort();
  }

  model_to_system_map remote_address_mapping = model_to_system_map(*this);
  auto callback_function =
      runtime_transition_mapping.get_callback_for(transition_registry_id);

  // Check if the callback function is valid
  if (!callback_function) {
    // Log the error and abort the execution
    std::cerr << "Failed to retrieve a valid callback for the "
                 "transition_registry_id: "
              << transition_registry_id << std::endl;
    // Handle the failure appropriately, potentially aborting the execution
    abort();
  }

  std::unique_ptr<model::transition> transition_encountered_at_runtime =
      callback_function(wrapper_response_stream, remote_address_mapping);
  this->current_program_model.model_executing_runner(
      runner_id, std::move(transition_encountered_at_runtime));
}

void *model_to_system_map::get_remote_process_handle_for_object(
    model::state::objid_t id) const {
  for (const auto &pair : _coordinator.system_address_mapping) {
    if (pair.second == id) {
      return pair
          .first;  // Found the system handle corresponding to the object ID
    }
  }

  // Optional: Log an error if the object ID is not found
  std::cerr << "Object ID " << id << " not found in the system address mapping."
            << std::endl;

  return nullptr;  // Return nullptr if the object ID is not found
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
  // Generate a new unique object ID. This could be based on a counter, UUID, or
  // any other unique identifier strategy
  static model::state::objid_t nextObjId =
      0;  // Example using a simple counter, should be replaced with a
          // thread-safe and more robust approach
  model::state::objid_t newObjId = nextObjId++;

  // Associate the new object ID with the provided system handle in the
  // coordinator's mapping Direct access is used here because
  // model_to_system_map is a friend of coordinator
  _coordinator.system_address_mapping[remote_process_visible_object_handle] =
      newObjId;

  // You might also need to store the initial state of the object somewhere in
  // the coordinator or another appropriate place

  // Return the new object ID
  return newObjId;
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