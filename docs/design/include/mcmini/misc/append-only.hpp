#pragma once

#include <vector>

namespace mcmini {
template <typename T>
struct append_only {
 private:
  std::vector<T> contents;

 public:
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
  typename std::vector<T>::reference at(size_t i) { return contents.at(i); }
  typename std::vector<T>::const_reference at(size_t i) const {
    return contents.at(i);
  }
  typename std::vector<T>::iterator begin() { return this->contents.begin(); }
  typename std::vector<T>::iterator end() { return this->contents.end(); }
};
}  // namespace mcmini