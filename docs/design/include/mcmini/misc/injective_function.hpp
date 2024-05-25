#pragma once

#include <exception>
#include <stdexcept>
#include <unordered_map>

template <typename DomainType, typename RangeType>
struct injective_function {
 private:
  struct invalid_insertion : public std::runtime_error {
    invalid_insertion(const char* msg) : std::runtime_error(msg) {}
  };

  // INVARIANT: `forward` and `backward` are mirrors of one another. The keys of
  // one are the values of the other.
  std::unordered_map<DomainType, RangeType> forward;
  std::unordered_map<RangeType, DomainType> backward;

 public:
  using value_type = typename decltype(forward)::value_type;
  using iterator = typename decltype(forward)::iterator;
  using size_type = typename decltype(forward)::size_type;

  auto begin() -> decltype(forward.begin()) { return forward.begin(); }
  auto end() -> decltype(forward.end()) { return forward.end(); }
  auto begin() const -> decltype(forward.cbegin()) { return forward.cbegin(); }
  auto end() const -> decltype(forward.cend()) { return forward.cend(); }
  auto cbegin() -> decltype(forward.cbegin()) const { return forward.cbegin(); }
  auto cend() -> decltype(forward.cend()) const { return forward.cend(); }

  bool empty() const { return this->forward.empty(); }
  size_type size() const { return this->forward.size(); }
  bool is_mapped(const RangeType& rt) const { return this->backward.count(rt); }

  RangeType& at(const DomainType& key) { return this->forward.at(key); }
  const RangeType& at(const DomainType& key) const {
    return this->forward.at(key);
  }
  DomainType& range_at(const RangeType& key) { return this->backward.at(key); }
  const DomainType& range_at(const RangeType& key) const {
    return this->backward.at(key);
  }
  size_type count_domain(const DomainType& key) const {
    return this->forward.count(key);
  }
  size_type count_range(const RangeType& key) const {
    return this->backward.count(key);
  }

  std::pair<iterator, bool> insert(const value_type& vt) {
    std::pair<iterator, bool> result = this->forward.insert(vt);
    if (result.second && this->count_range(vt.second))
      throw invalid_insertion(
          "A value in the domain already maps to this value. This would break "
          "the one-to-one invariance of this mapping");
    if (result.second) {
      this->backward.insert({vt.second, vt.first});
    }
    return result;
  }
};
