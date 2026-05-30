#pragma once

inline void HashCombine(size_t& seed, size_t value)
{
    seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
}
template<typename T>
inline uint64_t GetHash(const T& v)
{
    return std::hash<T>{}(v);
}
