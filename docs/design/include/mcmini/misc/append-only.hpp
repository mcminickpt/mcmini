#pragma once

#include <cstddef>
#include <vector>

namespace mcmini {
template <typename T>
struct append_only {
 private:
  std::vector<T> contents;

 public:
  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  append_only() = default;
  append_only(std::vector<T> &&contents) : contents(std::move(contents)) {}
  append_only(append_only &&) = default;
  append_only(const append_only &) = default;
  append_only &operator=(append_only &&) = default;
  append_only &operator=(const append_only &) = default;

  void push_back(const T &element) { contents.push_back(element); }
  size_t size() const { return contents.size(); }
  bool empty() const { return contents.empty(); }
  void clear() { contents.clear(); }
  reference at(size_t i) { return contents.at(i); }
  const_reference at(size_t i) const { return contents.at(i); }
  iterator begin() { return this->contents.begin(); }
  iterator end() { return this->contents.end(); }
  const_iterator cbegin() const { return this->contents.cbegin(); }
  const_iterator cend() const { return this->contents.cend(); }
};
}  // namespace mcmini