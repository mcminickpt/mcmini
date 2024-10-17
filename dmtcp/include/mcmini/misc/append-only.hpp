#pragma once

#include <cstddef>
#include <vector>

template <typename T>
struct append_only {
 private:
  std::vector<T> contents;

 public:
  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;
  using size_type = typename std::vector<T>::size_type;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  append_only() = default;
  append_only(size_type count) : contents(count) {}
  append_only(std::vector<T> &&contents) : contents(std::move(contents)) {}
  append_only(append_only &&) = default;
  append_only(const append_only &) = default;
  append_only &operator=(append_only &&) = default;
  append_only &operator=(const append_only &) = default;
  template <typename Iter>
  append_only(Iter begin, Iter end) : contents(begin, end) {}

  void push_back(const T &element) { contents.push_back(element); }
  void push_back(T &&element) { contents.push_back(std::move(element)); }
  reference back() { return this->contents.back(); }
  const_reference back() const { return this->contents.back(); }
  size_t size() const { return contents.size(); }
  bool empty() const { return contents.empty(); }
  void clear() { contents.clear(); }
  reference at(size_t i) { return contents.at(i); }
  const_reference at(size_t i) const { return contents.at(i); }
  iterator begin() { return this->contents.begin(); }
  iterator end() { return this->contents.end(); }
  iterator erase(iterator first, iterator last) {
    return this->contents.erase(first, last);
  }
  iterator erase(const_iterator first, const_iterator last) {
    return this->contents.erase(first, last);
  }
  const_iterator begin() const { return this->contents.cbegin(); }
  const_iterator end() const { return this->contents.cend(); }
  const_iterator cbegin() const { return this->contents.cbegin(); }
  const_iterator cend() const { return this->contents.cend(); }
  reference operator[](size_type pos) { return this->contents[pos]; }
  const_reference operator[](size_type pos) const {
    return this->contents[pos];
  }
};
