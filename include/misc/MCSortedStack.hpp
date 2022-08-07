#ifndef MC_MCSORTEDSTACK_HPP
#define MC_MCSORTEDSTACK_HPP

#include <stack>
#include <stdexcept>

template<typename Index>
struct MCSortedStack final {
private:

  std::stack<Index> contents;

public:

  inline void
  push(Index index)
  {
    if (!this->contents.empty() && this->contents.top() > index)
      throw std::invalid_argument(
        "A sorted stack must always insert an"
        " element larger than the current top");
    this->contents.push(index);
  }

  inline Index
  top() const
  {
    return this->contents.top();
  }

  inline void
  pop()
  {
    if (!this->contents.empty()) this->contents.pop();
  }

  inline void
  popGreaterThan(Index index)
  {
    while (!this->empty() && this->top() > index) this->pop();
  }

  inline bool
  empty() const
  {
    return this->contents.empty();
  }

  size_t
  size() const
  {
    return this->size();
  }
};

#endif /* MC_MCSORTEDSTACK_HPP */