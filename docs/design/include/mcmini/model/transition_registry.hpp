#pragma once

#include <memory>

#include "mcmini/model/transition.hpp"

namespace mcmini::model {

/**
 * @brief A central repository where transitions are dynamically registered with
 * McMini to be included during model checking.
 */
///
class transition_registry final {
 private:
 public:
  transition_registry() = default;
  // Mapping between types and the serialization/deserialization function
  // pointers. The plugin, when loaded by McMini, will have the chance
  // to register the transitions it defines. And it will give us a pointer
  // to the specialized templates!!! This is awesome! Things will work out as
  // long as the plugin providers give us the correct type....
  //
  // Here the RTTI needs to be preserved across the plugins and McMini. This
  // could pose a little bit of a problem... we'd need to look into this

  // NOTE: We would wrap  the given serializers and deserializers
  // into `transition_serializer<transition>` and `static_cast` to T as
  // appropriate to be able to store them in the mapping. This is OK though so
  // long as T is always a subclass of transition (TODO: enforce this)
  template <typename T>
  transition::category register_transition_type(
      transition_serializer<T> serializer_for_wrapper_return,
      transition_deserializer<T> deserializer_for_wrapper_hit);

  template <typename T>
  bool is_transition_type_registered() const;

  bool is_category_registered(transition::category) const;
};

}  // namespace mcmini::model