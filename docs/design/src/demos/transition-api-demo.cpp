#include <dlfcn.h>

#include <functional>
#include <iostream>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
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
  // Here the RTTI needs to be preserved across the plugins and McMini. This
  // could pose a little bit of a problem... we'd need to look into this
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

class transitionSub1 : public transition {
 public:
  void specific5() {}
};

template <>
void serializeInto<transitionSub1>(transitionSub1* subtype, std::ostream& os) {
  // Specific serialization logic for transitionSub1
}

template <>
void serializeInto<transitionSub2>(transitionSub2* subtype, std::ostream& os) {
  // Specific serialization logic for transitionSub2
}

typedef void (*function_type)(transition*, transition*);

static std::unordered_map<std::type_index,
                          std::unordered_map<std::type_index, function_type>>
    double_dispatch_table;

template <typename T1, typename T2>
void is_dependent(T1* t1, T2* t2) {
  std::cout << "Hello world!!! " << typeid(T1).name() << " "
            << typeid(T2).name() << std::endl;
}

#include "type-id.hpp"

extern "C" bool my_shared_k(const std::type_info& ti);
extern "C" bool my_func(type_id_t q);

struct dd_table {
 public:
  using t_callback = void (*)(transition*, transition*);
  using stored_callback = void (*)(transition*, transition*, t_callback);

 private:
  std::unordered_map<std::type_index,
                     std::unordered_map<std::type_index,
                                        std::pair<stored_callback, t_callback>>>
      _internal_table;

 public:
  template <typename T1, typename T2>
  using function_callback = void (*)(T1*, T2*);

  template <typename T1, typename T2>
  static void casting_function(transition* t1, transition* t2,
                               t_callback callback) {
    auto well_defined_handle =
        reinterpret_cast<function_callback<T1, T2>>(callback);
    well_defined_handle(static_cast<T1*>(t1), static_cast<T2*>(t2));
  }

  template <typename T1, typename T2>
  static void casting_function_reverse(transition* t1, transition* t2,
                                       t_callback callback) {
    auto well_defined_handle =
        reinterpret_cast<function_callback<T1, T2>>(callback);
    well_defined_handle(static_cast<T1*>(t2), static_cast<T2*>(t1));
  }

  template <typename T1, typename T2>
  void register_dd_entry(function_callback<T1, T2> callback) {
    auto unspecified_function_handle_forward =
        reinterpret_cast<stored_callback>(casting_function<T1, T2>);
    auto unspecified_function_handle_reversed =
        reinterpret_cast<stored_callback>(casting_function_reverse<T1, T2>);
    auto unspecified_callback_handle = reinterpret_cast<t_callback>(callback);

    _internal_table[std::type_index(typeid(T1))][std::type_index(typeid(T2))] =
        std::make_pair(unspecified_function_handle_forward,
                       unspecified_callback_handle);

    _internal_table[std::type_index(typeid(T2))][std::type_index(typeid(T1))] =
        std::make_pair(unspecified_function_handle_reversed,
                       unspecified_callback_handle);
  }

  void call(transition* t1, transition* t2) {
    auto t1_type = std::type_index(typeid(*t1));
    auto t2_type = std::type_index(typeid(*t2));
    if (_internal_table.count(t1_type) > 0) {
      if (_internal_table[t1_type].count(t2_type) > 0) {
        // Only works if we cast it back...
        auto& pair = _internal_table[t1_type][t2_type];
        pair.first(t1, t2, pair.second);
      }
    }
  }
};

struct dd_table_member_functions {
 public:
  using t_callback = void (transition::*)(transition*);
  using stored_callback = void (*)(transition*, transition*, t_callback);

 private:
  std::unordered_map<std::type_index,
                     std::unordered_map<std::type_index,
                                        std::pair<stored_callback, t_callback>>>
      _internal_table;

 public:
  template <typename T1, typename T2>
  using member_function_callback = void (T1::*)(T2*);

  template <typename T1, typename T2>
  static void casting_function(transition* t1, transition* t2,
                               t_callback callback) {
    auto well_defined_handle =
        reinterpret_cast<member_function_callback<T1, T2>>(callback);
    (static_cast<T1&>(*t1).*well_defined_handle)(static_cast<T2*>(t2));
  }

  template <typename T1, typename T2>
  static void casting_function_reverse(transition* t1, transition* t2,
                                       t_callback callback) {
    auto well_defined_handle =
        reinterpret_cast<member_function_callback<T1, T2>>(callback);
    (static_cast<T1&>(*t2).*well_defined_handle)(static_cast<T2*>(t1));
  }

  template <typename T1, typename T2>
  void register_dd_entry(member_function_callback<T1, T2> callback) {
    auto unspecified_function_handle_forward =
        reinterpret_cast<stored_callback>(casting_function<T1, T2>);
    auto unspecified_function_handle_reversed =
        reinterpret_cast<stored_callback>(casting_function_reverse<T1, T2>);
    auto unspecified_callback_handle = reinterpret_cast<t_callback>(callback);

    _internal_table[std::type_index(typeid(T1))][std::type_index(typeid(T2))] =
        std::make_pair(unspecified_function_handle_forward,
                       unspecified_callback_handle);

    _internal_table[std::type_index(typeid(T2))][std::type_index(typeid(T1))] =
        std::make_pair(unspecified_function_handle_reversed,
                       unspecified_callback_handle);
  }

  void call(transition* t1, transition* t2) {
    auto t1_type = std::type_index(typeid(*t1));
    auto t2_type = std::type_index(typeid(*t2));
    if (_internal_table.count(t1_type) > 0) {
      if (_internal_table[t1_type].count(t2_type) > 0) {
        // Only works if we cast it back...
        auto& pair = _internal_table[t1_type][t2_type];
        pair.first(t1, t2, pair.second);
      }
    }
  }
};

class transitionSub3 : public transition {
 public:
  void specific(transitionSub1* sub1) { std::cout << "specific\n"; }
  void dependent2(transitionSub2* sub2) {
    std::cout << "specific2 \n" << sub2 << std::endl;
  }
};

static dd_table ddt;

template <typename T1, typename T2>
void add_example() {
  ddt.register_dd_entry<T1, T2>(is_dependent<T1, T2>);
}

int main() {
  add_example<transitionSub1, transitionSub2>();
  add_example<transitionSub1, transitionSub3>();
  add_example<transitionSub2, transitionSub3>();

  transition* sub1 = new transitionSub1();
  transition* sub2 = new transitionSub2();
  transition* sub3 = new transitionSub3();

  ddt.call(sub1, sub2);
  ddt.call(sub2, sub1);
  ddt.call(sub1, sub3);
  ddt.call(sub3, sub1);
  ddt.call(sub2, sub3);

  dd_table_member_functions members;

  members.register_dd_entry(&transitionSub3::specific);
  members.register_dd_entry(&transitionSub3::dependent2);

  members.call(sub1, sub3);

  std::cout << sub2 << std::endl;
  members.call(sub2, sub3);

  // double_dispatch_table[std::type_index(typeid(transitionSub1))]
  //                      [std::type_index(typeid(transitionSub2))] =
  //                          reinterpret_cast<void (*)(transition*,
  //                          transition*)>(
  //                              is_dependent<transitionSub1, transitionSub2>);

  // std::cout << typeid(*sub1).hash_code() << "\n"
  //           << typeid(transitionSub1).hash_code() << std::endl;

  // std::cout << double_dispatch_table.size() << std::endl;

  // auto h = double_dispatch_table[std::type_index(typeid(*sub1))]
  //                               [std::type_index(typeid(*sub2))];

  // std::cout << h << std::endl;

  // std::exit(0);

  // std::vector<void (*)(transition*, std::ostream&)> functions;

  // // Storing function pointers
  // functions.push_back(reinterpret_cast<void (*)(transition*, std::ostream&)>(
  //     serializeInto<transitionSub1>));
  // functions.push_back(reinterpret_cast<void (*)(transition*, std::ostream&)>(
  //     serializeInto<transitionSub2>));

  // // Example usage
  // transitionSub1 sub1;
  // transitionSub2 sub2;

  // std::unordered_map<type_id_t, int> uom;

  // uom[type_id<int>()] = 1;
  // uom[type_id<bool>()] = 2;

  // for (const auto& e : uom) {
  //   std::cout << " " << e.second << std::endl;
  // }

  // // std::vector<type_id_t> a;

  // mcmini_serialize_transition(&sub1, std::cout);

  // functions[0](&sub1,
  //              std::cout);  // Calls specialized function for transitionSub1
  // functions[1](&sub2,
  //              std::cout);  // Calls specialized function for transitionSub2

  // auto handle = dlopen("./libmcmini.so", RTLD_LAZY | RTLD_GLOBAL);

  // std::cerr << dlerror() << std::endl;

  // std::cout << handle << std::endl;

  // auto sym = dlsym(handle, "my_func");
  // auto shared_k_handle = dlsym(handle, "my_shared_k");

  // std::cerr << dlerror() << std::endl;

  // std::cout << "Sym: " << sym << std::endl;

  // auto actual = reinterpret_cast<bool (*)(type_id_t)>(sym);
  // auto actual_k =
  //     reinterpret_cast<bool (*)(const std::type_info&)>(shared_k_handle);

  // std::cout << "Equal template ?:" << actual(type_id<shared_k>()) <<
  // std::endl; std::cout << "Equal with --dynamic-list-cpp-typeinfo/RTTI?:"
  //           << actual_k(typeid(shared_k)) << std::endl;

  // dlclose(handle);

  // return 0;
}
