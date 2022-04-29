#ifndef GMAL_GMALDEFERRED_H
#define GMAL_GMALDEFERRED_H

// Solves static initialization fiasco in dynamic libraries
//template<typename T>
//struct GMALDeferred
//{
//    GMALDeferred(){}
//    ~GMALDeferred(){ value.~T(); }
//    template<typename...TArgs>
//    void Construct(TArgs&&...args)
//    {
//        new(&value) T(std::forward<TArgs>(args)...);
//    }
//
//    union
//    {
//        T value;
//    };
//
//public:
//    T get() { return value; }
//};

template<typename T>
struct GMALDeferred
{
    GMALDeferred(){}
    ~GMALDeferred(){ value->~T(); }
    template<typename...TArgs>
    void Construct(TArgs&&...args)
    {
        value = new T(std::forward<TArgs>(args)...);
    }

    union
    {
        T *value;
    };

public:
    T *get() { return value; }
};

#endif //GMAL_GMALDEFERRED_H
