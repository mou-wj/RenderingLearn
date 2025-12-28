#pragma once

#include <array>
#include <cmath>

namespace Core {

// -------------------------------------------------------------------------------------------------
//  Int2: Represents a 2D integer vector or point
// -------------------------------------------------------------------------------------------------
struct CORE_API Int2 {
    std::array<int, 2> Data;
    int& x; int& y; // convenient aliases to underlying storage

    Int2() : Data{0,0}, x(Data[0]), y(Data[1]) {}
    Int2(int ix, int iy) : Data{ix, iy}, x(Data[0]), y(Data[1]) {}

    int& operator[](size_t index) { return Data[index]; }
    const int& operator[](size_t index) const { return Data[index]; }

    Int2 operator+(const Int2& other) const { return Int2(Data[0] + other.Data[0], Data[1] + other.Data[1]); }
    Int2 operator-(const Int2& other) const { return Int2(Data[0] - other.Data[0], Data[1] - other.Data[1]); }
    Int2 operator*(int scalar) const { return Int2(Data[0] * scalar, Data[1] * scalar); }
    // Copy assignment: copy underlying storage
    Int2& operator=(const Int2& other) { Data = other.Data; return *this; }
};

// -------------------------------------------------------------------------------------------------
//  Int3: Represents a 3D integer vector or point
// -------------------------------------------------------------------------------------------------
struct CORE_API Int3 {
    std::array<int, 3> Data;
    int& x; int& y; int& z;

    Int3() : Data{0,0,0}, x(Data[0]), y(Data[1]), z(Data[2]) {}
    Int3(int ix, int iy, int iz) : Data{ix,iy,iz}, x(Data[0]), y(Data[1]), z(Data[2]) {}

    int& operator[](size_t index) { return Data[index]; }
    const int& operator[](size_t index) const { return Data[index]; }

    Int3 operator+(const Int3& other) const { return Int3(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2]); }
    Int3 operator-(const Int3& other) const { return Int3(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2]); }
    Int3 operator*(int scalar) const { return Int3(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar); }
    // Copy assignment
    Int3& operator=(const Int3& other) { Data = other.Data; return *this; }
};

// -------------------------------------------------------------------------------------------------
//  Int4: Represents a 4D integer vector or point
// -------------------------------------------------------------------------------------------------
struct CORE_API Int4 {
    std::array<int, 4> Data;
    int& x; int& y; int& z; int& w;

    Int4() : Data{0,0,0,0}, x(Data[0]), y(Data[1]), z(Data[2]), w(Data[3]) {}
    Int4(int ix, int iy, int iz, int iw) : Data{ix,iy,iz,iw}, x(Data[0]), y(Data[1]), z(Data[2]), w(Data[3]) {}

    int& operator[](size_t index) { return Data[index]; }
    const int& operator[](size_t index) const { return Data[index]; }

    Int4 operator+(const Int4& other) const { return Int4(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2], Data[3] + other.Data[3]); }
    Int4 operator-(const Int4& other) const { return Int4(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2], Data[3] - other.Data[3]); }
    Int4 operator*(int scalar) const { return Int4(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar, Data[3] * scalar); }
    // Copy assignment
    Int4& operator=(const Int4& other) { Data = other.Data; return *this; }
};


// -------------------------------------------------------------------------------------------------
//  Float2: Represents a 2D vector or point
// -------------------------------------------------------------------------------------------------
struct CORE_API Float2 {
    std::array<float, 2> Data;
    float& x; float& y;

    Float2() : Data{0.0f,0.0f}, x(Data[0]), y(Data[1]) {}
    Float2(float ix, float iy) : Data{ix,iy}, x(Data[0]), y(Data[1]) {}

    float& operator[](size_t index) { return Data[index]; }
    const float& operator[](size_t index) const { return Data[index]; }

    Float2 operator+(const Float2& other) const { return Float2(Data[0] + other.Data[0], Data[1] + other.Data[1]); }
    Float2 operator-(const Float2& other) const { return Float2(Data[0] - other.Data[0], Data[1] - other.Data[1]); }
    Float2 operator*(float scalar) const { return Float2(Data[0] * scalar, Data[1] * scalar); }

    float Length() const { return std::sqrt(Data[0] * Data[0] + Data[1] * Data[1]); }
    // Copy assignment
    Float2& operator=(const Float2& other) { Data = other.Data; return *this; }
};


// -------------------------------------------------------------------------------------------------
//  Float3: Represents a 3D vector or point
// -------------------------------------------------------------------------------------------------
struct CORE_API Float3 {
    std::array<float, 3> Data;
    float& x; float& y; float& z;

    Float3() : Data{0.0f,0.0f,0.0f}, x(Data[0]), y(Data[1]), z(Data[2]) {}
    Float3(float ix, float iy, float iz) : Data{ix,iy,iz}, x(Data[0]), y(Data[1]), z(Data[2]) {}

    float& operator[](size_t index) { return Data[index]; }
    const float& operator[](size_t index) const { return Data[index]; }

    Float3 operator+(const Float3& other) const { return Float3(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2]); }
    Float3 operator-(const Float3& other) const { return Float3(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2]); }
    Float3 operator*(float scalar) const { return Float3(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar); }

    float Length() const { return std::sqrt(Data[0] * Data[0] + Data[1] * Data[1] + Data[2] * Data[2]); }
    // Copy assignment
    Float3& operator=(const Float3& other) { Data = other.Data; return *this; }
};

// -------------------------------------------------------------------------------------------------
//  Float4: Represents a 4D vector or point
// -------------------------------------------------------------------------------------------------
struct CORE_API Float4 {
    std::array<float, 4> Data;
    float& x; float& y; float& z; float& w;

    Float4() : Data{0.0f,0.0f,0.0f,0.0f}, x(Data[0]), y(Data[1]), z(Data[2]), w(Data[3]) {}
    Float4(float ix, float iy, float iz, float iw) : Data{ix,iy,iz,iw}, x(Data[0]), y(Data[1]), z(Data[2]), w(Data[3]) {}

    float& operator[](size_t index) { return Data[index]; }
    const float& operator[](size_t index) const { return Data[index]; }

    Float4 operator+(const Float4& other) const { return Float4(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2], Data[3] + other.Data[3]); }
    Float4 operator-(const Float4& other) const { return Float4(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2], Data[3] - other.Data[3]); }
    Float4 operator*(float scalar) const { return Float4(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar, Data[3] * scalar); }

    float Length() const { return std::sqrt(Data[0] * Data[0] + Data[1] * Data[1] + Data[2] * Data[2] + Data[3] * Data[3]); }
    // Copy assignment
    Float4& operator=(const Float4& other) { Data = other.Data; return *this; }
};


} // namespace WR::Common