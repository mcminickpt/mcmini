#pragma once

#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace mcmini::detail {

template <typename InterfaceType, typename ReturnType = void>
struct double_dispatch_member_function_table {
 private:
  using t_callback = ReturnType (InterfaceType::*)(InterfaceType*);
  using stored_callback = ReturnType (*)(InterfaceType*, InterfaceType*,
                                         t_callback);

  std::unordered_map<std::type_index,
                     std::unordered_map<std::type_index,
                                        std::pair<stored_callback, t_callback>>>
      _internal_table;

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
  static void casting_function(InterfaceType* t1, InterfaceType* t2,
                               t_callback callback) {
    auto well_defined_handle =
        reinterpret_cast<member_function_callback<T1, T2>>(callback);
    (static_cast<T1&>(*t1).*well_defined_handle)(static_cast<T2*>(t2));
  }

  template <typename T1, typename T2>
  static void casting_function_reverse(InterfaceType* t1, InterfaceType* t2,
                                       t_callback callback) {
    auto well_defined_handle =
        reinterpret_cast<member_function_callback<T1, T2>>(callback);
    (static_cast<T1&>(*t2).*well_defined_handle)(static_cast<T2*>(t1));
  }

 public:
  static_assert(
      std::is_polymorphic<InterfaceType>::value,
      "InterfaceType must be a polymorphic type for double dispatch table "
      "to function properly. See the example and documentation for typeid() on "
      "cppreference.com (https://en.cppreference.com/w/cpp/language/typeid) "
      "for more details on why this is necessary");

  template <typename T1, typename T2>
  using member_function_callback = void (T1::*)(T2*);

  template <typename T1, typename T2>
  void register_dd_entry(member_function_callback<T1, T2> callback) {
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
    auto unspecified_callback_handle = reinterpret_cast<t_callback>(callback);

    // TODO: Check if the callback has been registered; if so, an error should
    // be returned indicated that this is the case.
    _internal_table[std::type_index(typeid(T1))][std::type_index(typeid(T2))] =
        std::make_pair(casting_function<T1, T2>, unspecified_callback_handle);

    _internal_table[std::type_index(typeid(T2))][std::type_index(typeid(T1))] =
        std::make_pair(casting_function_reverse<T1, T2>,
                       unspecified_callback_handle);
  }

  void call(InterfaceType* t1, InterfaceType* t2) {
    auto t1_type = std::type_index(typeid(*t1));
    auto t2_type = std::type_index(typeid(*t2));
    if (_internal_table.count(t1_type) > 0) {
      if (_internal_table[t1_type].count(t2_type) > 0) {
        auto& pair = _internal_table[t1_type][t2_type];
        pair.first(t1, t2, pair.second);
      }
    }
  }
};

}  // namespace mcmini::detail