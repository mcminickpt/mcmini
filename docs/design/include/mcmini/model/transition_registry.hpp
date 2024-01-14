#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::model {

/**
 * @brief A central repository where transitions are dynamically registered with
 * McMini to be included during model checking.
 */
class transition_registry final {
 private:
  using runtime_type_id = uint32_t;
  using transition_discovery_callback =
      void (transition::*)(std::istream&, coordinator::model_to_system_map&);
  std::vector<transition_discovery_callback> runtime_callbacks;

 public:
  // TODO: Mapping between types and the serialization/deserialization
  // function pointers. For plugins loaded by McMini, each will have the
  // chance to register the transitions it defines. Here the RTTI needs to
  // be preserved across the plugins and McMini. There are some challenges
  // here. See the `ld` man page and specifically the two linker flags
  // `--dynamic-list-cpp-typeinfo` and `-E` for details.
  template <typename transition_subclass>
  runtime_type_id register_transition() {
    static_assert(std::is_base_of<transition_subclass, transition>::value,
                  "Must be a subclass of `mcmini::model::transition`");
    runtime_callbacks.push_back(
        &transition_subclass::deserialize_from_wrapper_contents);
    return runtime_callbacks.size() - 1;
  }
};

}  // namespace mcmini::model