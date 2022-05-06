#ifndef GMALOPTIONAL_H
#define GMALOPTIONAL_H

#include <cstdint>
#include <stdexcept>

template<typename Value>
class GMALOptional final {
    bool _hasValue;

    union {
        const Value value;


    };

    /* Construct the `nil` value */
    GMALOptional() : _hasValue(false) {}
    GMALOptional(Value value) : value(value), _hasValue(true) {}


public:

    static GMALOptional<Value>
    some(Value value)
    {
        return GMALOptional<Value>(value);
    }

    static GMALOptional<Value>
    nil()
    {
        return GMALOptional<Value>();
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
    Value
    unsafelyUnwrapped()
    {
        return value;
    }

    Value
    unwrapped()
    {
        if (!hasValue()) {
            throw std::runtime_error("Attempted to unwrap the `nil` optional value");
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