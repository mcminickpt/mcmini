#pragma once

#include <functional>

class shared_k {
 public:
  virtual void compute() = 0;
};

class type_id_t {
 private:
  using sig = type_id_t();

  sig* id;
  explicit type_id_t(sig* id) : id{id} {}

 public:
  template <typename T>
  friend type_id_t type_id();
  friend std::hash<type_id_t>;

  bool operator==(type_id_t o) const {
    printf("this is a test!!!\n");
    return id == o.id;
  }
  bool operator!=(type_id_t o) const { return id != o.id; }
};

namespace std {
template <>
struct hash<type_id_t> {
  size_t operator()(const type_id_t& k) const {
    return std::hash<void*>()(
        reinterpret_cast<void*>(k.id));  // Example hash function
  }
};
}  // namespace std

template <typename T>
__attribute__((weak)) type_id_t type_id() {
  return type_id_t(&type_id<T>);
}