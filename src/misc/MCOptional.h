#ifndef MCOPTIONAL_H
#define MCOPTIONAL_H

#include <stdexcept>
#include <memory>

template<typename Value>
class MCOptional final {
    bool _hasValue;
    union { 
        Value value;
    };
public:
    MCOptional() : _hasValue(false) {}
    MCOptional(Value value) : value(value), _hasValue(true) {}

    static MCOptional<Value>
    some(Value value)
    {
        return MCOptional<Value>(value);
    }

    static MCOptional<Value>
    of(Value value)
    {
        return MCOptional<Value>(value);
    }

    static MCOptional<Value>
    withValue(Value value)
    {
        return MCOptional<Value>(value);
    }

    static MCOptional<Value>
    nil()
    {
        return MCOptional<Value>();
    }

    Value
    unwrapped() const
    {
        if (!_hasValue) { 
            throw std::runtime_error("Attempting to unwrap a `nil` optional value");
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

    bool 
    operator==(const Value &value) const 
    {  
        if (!hasValue()) return false;
        return unwrapped() == value;
    }

    bool 
    operator==(const MCOptional<Value> &optional) const 
    {  
        if (!hasValue() && !optional.hasValue()) 
            return true;
        else if (hasValue() && optional.hasValue())
            return this == optional.unwrapped();
        else 
            return false;
    }
};

#endif