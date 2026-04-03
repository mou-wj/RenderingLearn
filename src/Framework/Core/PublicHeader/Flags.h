#pragma
template<typename EnumType>
class TFlags
{
public:
    using StorageType = std::underlying_type_t<EnumType>;

    constexpr TFlags() = default;
    constexpr TFlags(EnumType value) : Bits(static_cast<StorageType>(value)) {}
    explicit constexpr TFlags(StorageType value) : Bits(value) {}

    constexpr StorageType GetBits() const { return Bits; }
    constexpr bool HasAny(TFlags rhs) const { return (Bits & rhs.Bits) != 0; }
    constexpr bool HasAll(TFlags rhs) const { return (Bits & rhs.Bits) == rhs.Bits; }
    constexpr bool IsEmpty() const { return Bits == 0; }
    constexpr EnumType ToEnum() const { return static_cast<EnumType>(Bits); }

    constexpr TFlags operator|(TFlags rhs) const { return TFlags(static_cast<StorageType>(Bits | rhs.Bits)); }
    constexpr TFlags operator&(TFlags rhs) const { return TFlags(static_cast<StorageType>(Bits & rhs.Bits)); }
    constexpr TFlags operator~() const { return TFlags(static_cast<StorageType>(~Bits)); }

    constexpr TFlags& operator|=(TFlags rhs)
    {
        Bits = static_cast<StorageType>(Bits | rhs.Bits);
        return *this;
    }

    constexpr TFlags& operator&=(TFlags rhs)
    {
        Bits = static_cast<StorageType>(Bits & rhs.Bits);
        return *this;
    }

private:
    StorageType Bits = 0;
};

template<typename EnumType>
inline constexpr bool EnumHasAnyFlags(TFlags<EnumType> Flags, TFlags<EnumType> Contains)
{
    return Flags.HasAny(Contains);
}

template<typename EnumType>
inline constexpr bool EnumHasAnyFlags(EnumType Flags, EnumType Contains)
{
    return EnumHasAnyFlags(TFlags<EnumType>(Flags), TFlags<EnumType>(Contains));
}

template<typename EnumType>
inline constexpr bool EnumHasAnyFlags(EnumType Flags, TFlags<EnumType> Contains)
{
    return EnumHasAnyFlags(TFlags<EnumType>(Flags), Contains);
}

template<typename EnumType>
inline constexpr bool EnumHasAnyFlags(TFlags<EnumType> Flags, EnumType Contains)
{
    return EnumHasAnyFlags(Flags, TFlags<EnumType>(Contains));
}

#define ENUM_CLASS_FLAGS(Enum, FlagsType) \
    using FlagsType = TFlags<Enum>; \
    inline constexpr FlagsType operator|(Enum a, Enum b) { return FlagsType(a) | FlagsType(b); } \
    inline constexpr FlagsType operator&(Enum a, Enum b) { return FlagsType(a) & FlagsType(b); } \
    inline constexpr FlagsType operator~(Enum a) { return ~FlagsType(a); } \
    inline constexpr FlagsType operator|(FlagsType a, Enum b) { return a | FlagsType(b); } \
    inline constexpr FlagsType operator&(FlagsType a, Enum b) { return a & FlagsType(b); } \
    inline constexpr FlagsType operator|(Enum a, FlagsType b) { return FlagsType(a) | b; } \
    inline constexpr FlagsType operator&(Enum a, FlagsType b) { return FlagsType(a) & b; }

