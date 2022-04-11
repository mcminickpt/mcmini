#ifndef GMAL_GMALREF_H
#define GMAL_GMALREF_H

template<typename T>
struct GMALRef {
public:
    inline virtual T *getRef() { return nullptr; }
    inline T *operator->() { return getRef(); }
};

template<typename T>
struct GMALConstRef : public GMALRef<T> {
private:
    T value;
public:
    inline explicit GMALConstRef(T *value) {
        GMAL_ASSERT(value != nullptr);
        this->value = *value;
    }
    inline T *getRef() override { return &this->value; }
};

template<typename T>
struct GMALPointerRef : public GMALRef<T> {
private:
    T *value;
public:
    inline explicit GMALPointerRef(T *value) : value(value) {}
    inline T *getRef() override { return this->value; }
};

template<typename T>
inline GMALRef<T> GMAL_PASS_DYNAMIC(T *value)
{
    return GMALPointerRef<T>(value);
}

template<typename T>
inline GMALRef<T> GMAL_PASS_STATIC(T *value)
{
    return GMALConstRef<T>(value);
}

#endif //GMAL_GMALREF_H
