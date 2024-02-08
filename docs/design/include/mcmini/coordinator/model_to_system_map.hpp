#pragma once

#include "mcmini/forwards.hpp"
#include "mcmini/misc/optional.hpp"
#include "mcmini/model/state.hpp"

/**
 * @brief A mapping between the remote addresses pointing to the C/C++ structs
 * the objects in McMini's model are emulating.
 *
 * As McMini explores different paths of execution of the target program at
 * runtime, it may discover new visible objects. However, visible objects are
 * only a _representation in the McMini model_ of the actual underlying
 * structs containing the information used to implement the primitive. The
 * underlying process, however, refers to these objects as pointers to the
 * multi-threaded primitives (e.g. a `pthread_mutex_t*` in
 * `pthread_mutex_lock()`). McMini therefore needs to maintain a
 * correspondence between these addresses and the identifiers in McMini's
 * model used to represent those objects to the model checker.
 */
struct model_to_system_map final {
 private:
  coordinator &_coordinator;

  /*
   * Prevent external construction (only the coordinator can construct
   * instances of this class)
   */
  model_to_system_map(coordinator &coordinator) : _coordinator(coordinator) {}
  friend coordinator;

 public:
  /* Prevent external construction */
  model_to_system_map() = delete;

  /**
   * @brief Retrieve the remote address of the object with id `id`.
   *
   * TODO: See the TODOs below
   */
  void *get_remote_process_handle_for_object(model::state::objid_t id) const;

  /**
   * @brief Retrieve the object that corresponds to the given remote address, or
   * the empty optional if no such address exists
   */
  optional<model::state::objid_t> get_object_for_remote_process_handle(
      void *) const;

  /**
   * @brief Record the presence of a new visible object that is
   * represented with the system id `system_handle`.
   *
   * @param remote_process_visible_object_handle the address containing
   * the data for the new visible object across process handles of the
   *
   * TODO: Handles are assumed to remain valid _across process source
   * invocations_. In the future we could support the ability to _remap_
   * process handles dynamically during each new re-execution scheduled by
   * the coordinator to handle aliasing etc.
   *
   * TODO: The handle could be _any_ value that is used in the
   * multi-threaded program. For now, we restrict it to addresses for the
   * mutex case etc.
   *
   * TODO: This should probably have a `result` as a return type. If the
   * handle is already mapped, how we should deal with this situation is a
   * bit unclear.
   */
  model::state::objid_t record_new_object_association(
      void *remote_process_visible_object_handle,
      std::unique_ptr<model::visible_object_state> initial_state);

  model::state::objid_t observe_remote_process_handle(
      void *remote_process_visible_object_handle,
      std::unique_ptr<model::visible_object_state> fallback_initial_state);
};
