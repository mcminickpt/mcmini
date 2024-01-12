#pragma once

namespace mcmini::detail {

template <typename Func>
struct function_traits;

template <typename RetType, typename... Args>
struct function_traits<RetType(Args...)> {
  using ReturnType = RetType;
  using Arguments = std::tuple<Args...>;
};

}  // namespace mcmini::detail
