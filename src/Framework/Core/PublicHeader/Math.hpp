// Vector.hpp
#pragma once
#include <array>
#include <cmath>
#include "Matrix.hpp"

namespace Core {

    struct Int2 {
        union {
            std::array<int, 2> Data;
            struct { int x, y; };
        };
        Int2() : Data{ 0,0 } {}
        Int2(int ix, int iy) : x(ix), y(iy) {}
        int& operator[](size_t i) { return Data[i]; }
        const int& operator[](size_t i) const { return Data[i]; }
    };

    struct Int3 {
        union {
            std::array<int, 3> Data;
            struct { int x, y, z; };
        };
        Int3() : Data{ 0,0,0 } {}
        Int3(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {}
        int& operator[](size_t i) { return Data[i]; }
        const int& operator[](size_t i) const { return Data[i]; }
    };

    struct Int4 {
        union {
            std::array<int, 4> Data;
            struct { int x, y, z, w; };
        };
        Int4() : Data{ 0,0,0,0 } {}
        Int4(int ix, int iy, int iz, int iw) : x(ix), y(iy), z(iz), w(iw) {}
        int& operator[](size_t i) { return Data[i]; }
        const int& operator[](size_t i) const { return Data[i]; }
    };

    struct Float2 {
        union {
            std::array<float, 2> Data;
            struct { float x, y; };
        };
        Float2() : Data{ 0.0f, 0.0f } {}
        Float2(float ix, float iy) : x(ix), y(iy) {}
        float& operator[](size_t i) { return Data[i]; }
        const float& operator[](size_t i) const { return Data[i]; }
        float Length() const { return std::sqrt(x * x + y * y); }
    };

    struct Float3 {
        union {
            std::array<float, 3> Data;
            struct { float x, y, z; };
        };
        Float3() : Data{ 0.0f, 0.0f, 0.0f } {}
        Float3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
        bool operator<(const Float3& Other) const { return x < Other.x && y < Other.y && z < Other.z; }
        bool operator>(const Float3& Other) const { return x > Other.x && y > Other.y && z > Other.z; }
        float& operator[](size_t i) { return Data[i]; }
        const float& operator[](size_t i) const { return Data[i]; }
        float Length() const { return std::sqrt(x * x + y * y + z * z); }
    };

    // ǿ������ 16 �ֽ�Ӳ�����룬����֧�� SIMD �ü��� Uniform Buffer �ô�����
    struct Float4 {
        union {
            alignas(16) std::array<float, 4> Data;
            struct { float x, y, z, w; };
        };
        Float4() : Data{ 0.0f, 0.0f, 0.0f, 0.0f } {}
        Float4(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}
        float& operator[](size_t i) { return Data[i]; }
        const int& operator[](size_t i) const { return Data[i]; }
        float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    };
    using Float4x4 = Mat4;


    template<typename T>
    constexpr T Pi()
    {
        static_assert(
            std::is_floating_point_v<T>,
            "Pi only supports floating point types");

        return static_cast<T>(
            3.1415926535897932384626433832795);
    }

    template<typename T>
    constexpr T DegToRad(T degree)
    {
        static_assert(
            std::is_floating_point_v<T>,
            "DegToRad only supports floating point types");

        return degree *
            (Pi<T>() / static_cast<T>(180.0));
    }

    template<typename T>
    constexpr T RadToDeg(T radian)
    {
        static_assert(
            std::is_floating_point_v<T>,
            "RadToDeg only supports floating point types");

        return radian *
            (static_cast<T>(180.0) / Pi<T>());
    }

} // namespace Core