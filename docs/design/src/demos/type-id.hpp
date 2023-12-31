#pragma once

#include <functional>

#define VISIBLE __attribute__((visibility("default")))

class VISIBLE shared_k {
 public:
};

class VISIBLE type_id_t {
 private:
  using sig = type_id_t();

  sig* id;
  VISIBLE explicit type_id_t(sig* id) : id{id} {}

 public:
  template <typename T>
  friend type_id_t type_id();
  friend std::hash<type_id_t>;

  VISIBLE bool operator==(type_id_t o) const {
    printf("YAY");
    return id == o.id;
  }
  VISIBLE bool operator!=(type_id_t o) const { return id != o.id; }
};

namespace std {
template <>
struct VISIBLE hash<type_id_t> {
  VISIBLE size_t operator()(const type_id_t& k) const {
    return std::hash<void*>()(
        reinterpret_cast<void*>(k.id));  // Example hash function
  }
};
}  // namespace std

template <typename T>
VISIBLE type_id_t type_id() {
  return type_id_t(&type_id<T>);
}