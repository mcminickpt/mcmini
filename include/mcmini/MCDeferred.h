#ifndef MC_MCDEFERRED_H
#define MC_MCDEFERRED_H

#include <utility>

// Solves static initialization fiasco in dynamic libraries
// C++ seems to invoke the constructor of a globally-defined
// object AFTER exiting a dynamic library's constructor. This is
// a problem as we only expect a global object to be constructed once
// (once in the constructor). This clever workaround helps resolve
// this issue by wrapping the underlying object in a union
template<typename T>
struct MCDeferred {
  MCDeferred() {}
  ~MCDeferred()
  {
    if (value) { value->~T(); }
  }
  template<typename... TArgs>
  void
  Construct(TArgs &&...args)
  {
    value = new T(std::forward<TArgs>(args)...);
  }

  union {
    T *value;
  };

public:

  T *
  get()
  {
    return value;
  }

  /* Conveniently access the underlying value */
  T *
  operator->()
  {
    return value;
  }
};

#endif // MC_MCDEFERRED_H
