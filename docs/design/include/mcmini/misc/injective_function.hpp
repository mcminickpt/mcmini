#pragma once

#include <unordered_map>

template <typename DomainType, typename RangeType>
struct injective_function {
 private:
  // INVARIANT: `forward` and `backward` are mirrors of one another. The keys of
  // one are the values of the other.
  std::unordered_map<DomainType, RangeType> forward;
  std::unordered_map<RangeType, DomainType> backward;

 public:
  using size_type =
      typename std::unordered_map<DomainType, RangeType>::size_type;

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
  RangeType& operator[](const DomainType& key) {
    RangeType& rt = this->forward[key];
    this->backward[rt] = key;
    return rt;
  }
};
