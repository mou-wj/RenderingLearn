#pragma once
//============================================================
   // Compile-Time FNV-1a 32-bit Hash
   //============================================================

constexpr uint32_t HashString(
    const char* str)
{
    uint32_t hash =
        2166136261u;

    while (*str)
    {
        hash ^=
            static_cast<uint32_t>(
                *str++);

        hash *=
            16777619u;
    }

    return hash;
}


//============================================================
// Root Type
//============================================================

#define DECLARE_TYPE_ID_BASE_TYPE(ClassName) \
public: \
    using Super = void; \
\
    static constexpr uint32_t \
    StaticTypeID() \
    { \
        return HashString( \
            #ClassName); \
    } \
\
    virtual uint32_t \
    GetTypeID() const \
    { \
        return StaticTypeID(); \
    } \
\
    virtual bool IsA( \
        uint32_t typeId) const \
    { \
        return \
            typeId == \
            StaticTypeID(); \
    } \
\
    template<typename T> \
    bool IsA() const \
    { \
        return IsA( \
            T::StaticTypeID()); \
    } \
\
    template<typename T> \
    T* Cast() \
    { \
        return IsA<T>() \
            ? static_cast<T*>( \
                this) \
            : nullptr; \
    } \
\
    template<typename T> \
    const T* Cast() const \
    { \
        return IsA<T>() \
            ? static_cast< \
                const T*>( \
                    this) \
            : nullptr; \
    }


//============================================================
// Derived Type
//============================================================

#define DECLARE_TYPE_ID_DERIVED_TYPE( \
    ClassName, \
    ParentClass) \
public: \
    using Super = ParentClass; \
\
    static constexpr uint32_t \
    StaticTypeID() \
    { \
        return HashString( \
            #ClassName); \
    } \
\
    virtual uint32_t \
    GetTypeID() const override \
    { \
        return StaticTypeID(); \
    } \
\
    virtual bool IsA( \
        uint32_t typeId) const override \
    { \
        return \
            typeId == \
                StaticTypeID() || \
            ParentClass::IsA( \
                typeId); \
    }

