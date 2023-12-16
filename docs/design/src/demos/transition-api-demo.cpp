#include <iostream>
#include <memory>
#include <vector>

// -----------------------------------------------------------------------------
// -----McMini Public API (inside McMini executable that runs the checker)------
// -----------------------------------------------------------------------------

// Plugins subclass from this
class transition {
 public:
  virtual ~transition() = default;
};

// Each subclass specializes the following two templates. The templates provide
// functionality for serializing an deserializing a __model-side__ transition!
// Important! I'll write it again: a transition that is used _in the McMini
// model_. That is, we are attempting to represent these transitions to model
// checking algorithms. The serialization is necessary to translate from the
// "real world" to the model.
//
// Note that the function isn't actually defined. Subclasses must override it
// for `mcmini_serialize_transition`
template <typename T>
void serializeInto(T* subtype, std::ostream& os);

template <typename T>
std::unique_ptr<T> deserialize_from(std::istream& is) {
  // By default, simply attempt to deserialize the value with the constructor.
  return std::unique_ptr<T>(new T(is));
}

// A central repository where transitions are dynamically registered with
// McMini to be included during model checking.
class transition_registry final {
 private:
  // Mapping between types and the serialization/deserialization function
  // pointers. The plugin, when loaded by McMini, will have the chance
  // to register the transitions it defines. And it will give us a pointer
  // to the specialized templates!!! This is awesome! Things will work out as
  // long as the plugin providers give us the correct type....
  //
  // Here the RTTI needs to be preserved across the plugins and McMini. This could
  // pose a little bit of a problem... we'd need to look into this
};

// -----------------------------------------------------------------------------
// -----McMini Private API (inside McMini executable that runs the checker)-----
// -----------------------------------------------------------------------------

// Some metaprogramming here gives us a nice way to avoid
// Primary template (can be undefined or have a default implementation)
template <typename T, typename Enable = void>
class serializer;

// Specialization for base classes
template <typename T>
class serializer<
    T, typename std::enable_if<std::is_base_of<transition, T>::value &&
                               std::is_class<T>::value>::type> {
 public:
  static void serialize(T* subtype, std::ostream& os) {
    // Look up identifier for the specific transition
    int specific_id_for_subtype = 10;
    os << specific_id_for_subtype;
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
void mcmini_serialize_transition(T* subtype, std::ostream& os) {
  serializer<T>::serialize(subtype, os);
}

// -----------------------------------------------------------------------------
// -----Transition Programming Interface ----
// -----------------------------------------------------------------------------

class transitionSub2 : public transition {};

class transitionSub1 : public transition {};

template <>
void serializeInto<transitionSub1>(transitionSub1* subtype, std::ostream& os) {
  // Specific serialization logic for transitionSub1
}

template <>
void serializeInto<transitionSub2>(transitionSub2* subtype, std::ostream& os) {
  // Specific serialization logic for transitionSub2
}

int main() {
  std::vector<void (*)(transition*, std::ostream&)> functions;

  // Storing function pointers
  functions.push_back(reinterpret_cast<void (*)(transition*, std::ostream&)>(
      serializeInto<transitionSub1>));
  functions.push_back(reinterpret_cast<void (*)(transition*, std::ostream&)>(
      serializeInto<transitionSub2>));

  // Example usage
  transitionSub1 sub1;
  transitionSub2 sub2;

  int b = 0;

  mcmini_serialize_transition(&sub1, std::cout);

  functions[0](&sub1,
               std::cout);  // Calls specialized function for transitionSub1
  functions[1](&sub2,
               std::cout);  // Calls specialized function for transitionSub2

  return 0;
}
