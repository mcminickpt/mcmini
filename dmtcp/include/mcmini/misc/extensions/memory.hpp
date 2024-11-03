#pragma once

#include <memory>

namespace extensions {

template <class ForwardIt>
void delete_all(ForwardIt first, ForwardIt last) {
  for (; first != last; ++first) delete *first;
}

}  // namespace extensions
