#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>

#include "mcmini/model/transition.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

namespace model {

/**
 * @brief A central repository where transitions are dynamically registered with
 * McMini to be included during model checking.
 *
 * At runtime, McMini assigns a unique integer identifier to each type of
 * transition that could exist during verification. When exploring the state
 * space of the executable undergoing verification, McMini will be notified of
 * threads running the registered transitions as `libmcmini.so` intercepts
 * library calls. `libmcmini.so` will transfer the value registered at runtime
 * along with a transition-specific payload. With the runtime id, a lookup is
 * performed and the appropriate function pointer is invoked that knows how to
 * represent the intercepted library call as a transition in McMini's model.
 */
class transition_registry final {
 public:
  using runtime_type_id = uint32_t;
  using rttid = runtime_type_id;
  using transition_discovery_callback =
      transition *(*)(state::runner_id_t, const volatile runner_mailbox &,
                      model_to_system_map &);
  static transition_registry default_registry();

  /**
   * @brief Marks the specified transition subclass as possible to encounter at
   * runtime
   *
   * @param transition_subclass the concrete type of transition that McMini will
   * associate with the returned id
   * @returns a positive integer which conceptually represents the transition.
   */
  void register_transition(rttid rttid,
                           transition_discovery_callback callback) {
    // TODO: Mapping between types and the serialization
    // function pointers. For plugins loaded by McMini, each will have the
    // chance to register the transitions it defines. Here the RTTI needs to
    // be preserved across the plugins and McMini. There are some challenges
    // here. See the `ld` man page and specifically the two linker flags
    // `--dynamic-list-cpp-typeinfo` and `-E` for details. `-E` is definitely
    // sufficient it seems in my small testing
    runtime_callbacks.insert({rttid, callback});
  }

  /**
   * @brief Marks the specified transition subclass as possible to encounter at
   * runtime, choosing the default `deserialize_from_wrapper_contents` static
   * function defined on the transition subclass if its available.
   *
   * @param transition_subclass the concrete type of transition that McMini will
   * associate with the returned id
   * @returns a positive integer which conceptually represents the transition.
   */
  template <typename transition_subclass>
  runtime_type_id register_transition() {
    return register_transition<transition_subclass>(
        &transition_subclass::from_wrapper_contents);
  }

  /**
   * @brief Retireve the function pointer registered for the given runtime type
   * id this registry assigned.
   *
   * @returns a function pointer which can produce a new transition of the type
   * assigned id `rttid`, or `nullptr` if no such `rttid` has been registered.
   */
  transition_discovery_callback get_callback_for(runtime_type_id rttid) {
    if (this->runtime_callbacks.count(rttid) == 0) return nullptr;
    return this->runtime_callbacks.at(rttid);
  }

 private:
  std::unordered_map<rttid, transition_discovery_callback> runtime_callbacks;
};

}  // namespace model
