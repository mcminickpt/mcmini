#ifndef MC_MCTYPES_HPP
#define MC_MCTYPES_HPP
#include <typeinfo>
#include <functional>

using TypeInfoRef = std::reference_wrapper<const std::type_info>;
using MCType = const std::type_info&;

struct TypeHasher {
    std::size_t operator()(TypeInfoRef code) const
    {
        return code.get().hash_code();
    }
};

struct TypesEqual {
    bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const
    {
        return lhs.get() == rhs.get();
    }
};
#endif /* MC_MCTYPES_HPP */