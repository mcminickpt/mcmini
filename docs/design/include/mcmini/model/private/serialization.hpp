#pragma once

#include <ostream>

#include "mcmini/model/transition.hpp"

namespace mcmini::model {

// -----------------------------------------------------------------------------
// -----McMini Private API (inside McMini executable that runs the checker)-----
// -----------------------------------------------------------------------------

// Some metaprogramming here gives us a nice way to avoid creating serializers
// for types that do not subclass `transition`.
template <typename T, typename Enable = void>
class serializer;

template <typename T>
class serializer<
    T, typename std::enable_if<std::is_base_of<transition, T>::value &&
                               std::is_class<T>::value>::type> {
 public:
  static void serialize(T* subtype, std::ostream& os) {
    // TODO: Look up identifier for the specific type --> look at the global
    // registry!
    // TODO: Decide if the registry is "global" or passed in. Probably as an
    // argument but we need to think about it (i.e. whether the singleton
    // pattern is appropriate here)
    int specific_id_for_subtype = 10;
    os << typeid(*T).hash_code();
    serializeInto(subtype, os);

    // Do other work here

    // Flush the stream e.g. (ensure the write actually is carried out)
    os.flush();
  }
};

// This is a private template that McMini invokes to serialize a transition
// that it's sending to libmcmini.so. This ensures that the specific type is
// used.
template <typename T>
void serialize_transition(T* subtype, std::ostream& os) {
  serializer<T>::serialize(subtype, os);
}

}  // namespace mcmini::model