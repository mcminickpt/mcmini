#ifndef MCOPTIONAL_H
#define MCOPTIONAL_H

#include <cstdint>
#include <stdexcept>

template<typename Value>
class MCOptional final {
  bool _hasValue;

  union {
    Value value;
  };

  /* Construct the `nil` value */
  MCOptional() : _hasValue(false) {}
  MCOptional(Value value) : _hasValue(true), value(value) {}

public:

  Value
  value_or(Value defaultValue) const
  {
    return _hasValue ? value : defaultValue;
  }

  static MCOptional<Value>
  some(Value value)
  {
    return MCOptional<Value>(value);
  }

  static MCOptional<Value>
  nil()
  {
    return MCOptional<Value>();
  }

  /**
   * Returns the value stored in the optional if
   * such a value exists
   *
   * If the optional does not contain any underlying
   * value, the results of accessing and using the
   * value are undefined
   *
   */
  Value &
  unsafelyUnwrapped()
  {
    return value;
  }

  Value &
  unwrapped()
  {
    if (!hasValue()) {
      throw std::runtime_error(
        "Attempted to unwrap the `nil` optional value");
    }
    return value;
  }

  bool
  hasValue() const
  {
    return _hasValue;
  }

  bool
  isNil() const
  {
    return !_hasValue;
  }
};

#endif