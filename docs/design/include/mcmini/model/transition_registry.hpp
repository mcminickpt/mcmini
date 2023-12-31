#pragma once

#include <memory>

#include "mcmini/model/transition.hpp"

namespace mcmini::model {

/**
 * @brief A central repository where transitions are dynamically registered with
 * McMini to be included during model checking.
 */
class transition_registry final {
 private:
  using runtime_type_id = uint32_t;
  // TODO: Mapping between types and the serialization/deserialization function
  // pointers. For plugins loaded by McMini, each will have the chance
  // to register the transitions it defines. Here the RTTI needs to be
  // preserved-list- across the plugins and McMini. There are some challenges
  // here. See the `ld` man page and specifically the two linker flags
  // `--dynamic-list-cpp-typeinfo` and `-E` for details.
  template <typename T>
  runtime_type_id register_transition_type(
      transition_serializer<T> serializer_for_wrapper_return,
      transition_deserializer<T> deserializer_for_wrapper_hit);
};

}  // namespace mcmini::model