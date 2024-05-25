#pragma once

#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

template <typename Source, typename Target>
struct copy_cv {
  using type = Target;
};

template <typename Source, typename Target>
struct copy_cv<const Source, Target> {
  using type = typename std::add_const<Target>::type;
};

template <typename Source, typename Target>
struct copy_cv<volatile Source, Target> {
  using type = typename std::add_volatile<Target>::type;
};

template <typename Source, typename Target>
struct copy_cv<const volatile Source, Target> {
  using type = typename std::add_cv<Target>::type;
};

template <typename InterfaceType, typename FunctionType>
struct double_dispatch_member_function_table;

template <typename InterfaceType, typename ReturnType, typename... Args>
struct double_dispatch_member_function_table<InterfaceType,
                                             ReturnType(Args...)> {
 private:
  using opaque_callback = ReturnType (InterfaceType::*)(InterfaceType*,
                                                        Args...);
  using stored_callback = ReturnType (*)(InterfaceType*, InterfaceType*,
                                         opaque_callback, Args...);

  std::unordered_map<
      std::type_index,
      std::unordered_map<std::type_index,
                         std::pair<stored_callback, opaque_callback>>>
      internal_table;

  std::unordered_map<std::type_index,
                     std::pair<stored_callback, opaque_callback>>
      interface_member_function_table;

  // In the intermediate
  // See "https://en.cppreference.com/w/cpp/language/reinterpret_cast"
  // """
  // 10) A pointer to member function can be converted to pointer to a
  // different member function of a different type. Conversion back to the
  // original type yields the original value, otherwise the resulting
  // pointer cannot be used safely.
  // """
  // The reinterpret_cast<> here is used to restore the specific callback
  // function specified at registration-time. Since function will only be
  // invoked if the RTTI runtime type-check lookup in the `call`, the cast is
  // safe. Furthermore, since the reinterpret_cast restores the original
  // pointer-to-member function pointer, subsequently invoking the callback
  // through `well_defined_handle` is defined.
  template <typename T1, typename T2>
  static ReturnType casting_function(InterfaceType* t1, InterfaceType* t2,
                                     opaque_callback callback, Args... args) {
    auto well_defined_handle =
        reinterpret_cast<member_function_callback<T1, T2>>(callback);
    return (static_cast<T1*>(t1)->*well_defined_handle)(static_cast<T2*>(t2),
                                                        std::forward(args)...);
  }

  template <typename T1, typename T2>
  static ReturnType casting_function_reverse(InterfaceType* t1,
                                             InterfaceType* t2,
                                             opaque_callback callback,
                                             Args... args) {
    auto well_defined_handle =
        reinterpret_cast<member_function_callback<T1, T2>>(callback);
    return (static_cast<T1*>(t2)->*well_defined_handle)(static_cast<T2*>(t1),
                                                        std::forward(args)...);
  }

  template <typename T>
  static ReturnType casting_function_interface_table(InterfaceType* t1,
                                                     InterfaceType* t2,
                                                     opaque_callback callback,
                                                     Args... args) {
    auto well_defined_handle =
        reinterpret_cast<interface_function_callback<T>>(callback);
    return (static_cast<T*>(t1)->*well_defined_handle)(t2,
                                                       std::forward(args)...);
  }

 public:
  static_assert(
      std::is_polymorphic<InterfaceType>::value,
      "InterfaceType must be a polymorphic type for double dispatch table "
      "to function properly. See the example and documentation for typeid() on "
      "cppreference.com (https://en.cppreference.com/w/cpp/language/typeid) "
      "for more details on why this is necessary");

  template <typename T1, typename T2>
  using member_function_callback = ReturnType (T1::*)(T2*, Args...) const;

  template <typename T1>
  using interface_function_callback = ReturnType (T1::*)(InterfaceType*,
                                                         Args...) const;
  template <typename T>
  void register_dd_entry(interface_function_callback<T> callback) {
    static_assert(std::is_base_of<InterfaceType, T>::value,
                  "T must be a subclass of InterfaceType");
    if (interface_member_function_table.count(std::type_index(typeid(T))) > 0) {
      throw std::runtime_error("A function is already registered for (" +
                               std::string(typeid(T).name()) + ")");
    }
    interface_member_function_table[std::type_index(typeid(T))] =
        std::make_pair(casting_function_interface_table<T>,
                       reinterpret_cast<opaque_callback>(callback));
  }

  template <typename T1, typename T2>
  void register_dd_entry(member_function_callback<T1, T2> callback) {
    static_assert(std::is_base_of<InterfaceType, T1>::value,
                  "T1 must be a subclass of InterfaceType");
    static_assert(std::is_base_of<InterfaceType, T2>::value,
                  "T2 must be a subclass of InterfaceType");
    // In the intermediate
    // See "https://en.cppreference.com/w/cpp/language/reinterpret_cast"
    // """
    // 10) A pointer to member function can be converted to pointer to a
    // different member function of a different type. Conversion back to the
    // original type yields the original value, otherwise the resulting pointer
    // cannot be used safely.
    // """
    // The reinterpret_cast<> here is used to store the variable-type callback
    // _callback_ registered for the particular combination
    if (internal_table.count(std::type_index(typeid(T1))) &&
        internal_table.at(std::type_index(typeid(T1)))
                .count(std::type_index(typeid(T2))) > 0) {
      throw std::runtime_error(
          "A function is already registered for the pair (" +
          std::string(typeid(T1).name()) + ", " +
          std::string(typeid(T2).name()) + ")");
    }
    const auto unspecified_callback_handle =
        reinterpret_cast<opaque_callback>(callback);
    internal_table[std::type_index(typeid(T1))][std::type_index(typeid(T2))] =
        std::make_pair(casting_function<T1, T2>, unspecified_callback_handle);

    internal_table[std::type_index(typeid(T2))][std::type_index(typeid(T1))] =
        std::make_pair(casting_function_reverse<T1, T2>,
                       unspecified_callback_handle);
  }

  ReturnType call_or(ReturnType fallback, InterfaceType* t1, InterfaceType* t2,
                     Args... args) const {
    const auto t1_type = std::type_index(typeid(*t1));
    const auto t2_type = std::type_index(typeid(*t2));
    if (internal_table.count(t1_type) > 0) {
      if (internal_table.at(t1_type).count(t2_type) > 0) {
        const auto& pair = internal_table.at(t1_type).at(t2_type);
        return pair.first(t1, t2, pair.second, std::forward<Args>(args)...);
      }
    } else if (interface_member_function_table.count(t1_type) > 0) {
      const auto& pair = interface_member_function_table.at(t1_type);
      return pair.first(t1, t2, pair.second, std::forward<Args>(args)...);
    } else if (interface_member_function_table.count(t2_type) > 0) {
      const auto& pair = interface_member_function_table.at(t2_type);
      // NOTE: t2 should come before t1 here since
      // `casting_function_interface_table` always casts its first argument
      return pair.first(t2, t1, pair.second, std::forward<Args>(args)...);
    }
    return fallback;
  }

  ReturnType call(InterfaceType* t1, InterfaceType* t2, Args... args) const {
    const auto t1_type = std::type_index(typeid(*t1));
    const auto t2_type = std::type_index(typeid(*t2));
    if (internal_table.count(t1_type) > 0) {
      if (internal_table.at(t1_type).count(t2_type) > 0) {
        const auto& pair = internal_table.at(t1_type).at(t2_type);
        return pair.first(t1, t2, pair.second, std::forward(args)...);
      }
    }
    throw std::runtime_error(
        "Attempted to invoke a method but missing runtime entry");
  }
};
