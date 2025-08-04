#pragma once

#include <istream>
#include <memory>
#include <ostream>

namespace real_world {

template <typename T>
struct remote_address final {
 private:
  T* remote_addr = nullptr;

  T*& get_as_ref() { return remote_addr; }

  template <typename U>
  friend std::ostream& operator<<(std::ostream& os, remote_address<U>& ra) {
    return (os << static_cast<void*>(ra.get_as_ref()));
  }
  template <typename U>
  friend std::istream& operator>>(std::istream& is, remote_address<U>& ra) {
    return (is >> ra.get_as_ref());
  }

 public:
  T* get() const { return remote_addr; }
  remote_address() : remote_addr(nullptr) {}
  remote_address(T* p) : remote_addr(p) {}

  bool operator==(const remote_address<T>& o) const {
    return o.remote_addr == remote_addr;
  }
};

template <typename T>
using remote_object = remote_address<T>;

}  //  namespace real_world

template <typename T>
struct std::hash<real_world::remote_address<T>> {
  std::size_t operator()(const real_world::remote_address<T>& o) const {
    return std::hash<T*>{}(o.get());
  }
};
