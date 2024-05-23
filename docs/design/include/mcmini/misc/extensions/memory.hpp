#pragma once

#include <memory>

namespace extensions {

template <typename T>
constexpr void destroy_at(T *p) {
  if (p) p->~T();
}

template <class ForwardIt>
constexpr void destroy(ForwardIt first, ForwardIt last) {
  for (; first != last; ++first) destroy_at(std::addressof(*first));
}

}  // namespace extensions