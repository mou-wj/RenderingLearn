#pragma once

#include <array>
#include <cmath>

namespace Common {

// -------------------------------------------------------------------------------------------------
//  Int2: Represents a 2D integer vector or point
// -------------------------------------------------------------------------------------------------
class Int2
{
public:
    Int2() : Data{0, 0} {}
    Int2(int x, int y) : Data{x, y} {}

    int& operator[](size_t index) { return Data[index]; }
    const int& operator[](size_t index) const { return Data[index]; }

    int X() const { return Data[0]; }
    int Y() const { return Data[1]; }

    void SetX(int x) { Data[0] = x; }
    void SetY(int y) { Data[1] = y; }

    Int2 operator+(const Int2& other) const
    {
        return Int2(Data[0] + other.Data[0], Data[1] + other.Data[1]);
    }

    Int2 operator-(const Int2& other) const
    {
        return Int2(Data[0] - other.Data[0], Data[1] - other.Data[1]);
    }

    Int2 operator*(int scalar) const
    {
        return Int2(Data[0] * scalar, Data[1] * scalar);
    }

private:
    std::array<int, 2> Data;
};

// -------------------------------------------------------------------------------------------------
//  Int3: Represents a 3D integer vector or point
// -------------------------------------------------------------------------------------------------
class Int3
{
public:
    Int3() : Data{0, 0, 0} {}
    Int3(int x, int y, int z) : Data{x, y, z} {}

    int& operator[](size_t index) { return Data[index]; }
    const int& operator[](size_t index) const { return Data[index]; }

    int X() const { return Data[0]; }
    int Y() const { return Data[1]; }
    int Z() const { return Data[2]; }

    void SetX(int x) { Data[0] = x; }
    void SetY(int y) { Data[1] = y; }
    void SetZ(int z) { Data[2] = z; }

    Int3 operator+(const Int3& other) const
    {
        return Int3(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2]);
    }

    Int3 operator-(const Int3& other) const
    {
        return Int3(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2]);
    }

    Int3 operator*(int scalar) const
    {
        return Int3(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar);
    }

private:
    std::array<int, 3> Data;
};

// -------------------------------------------------------------------------------------------------
//  Int4: Represents a 4D integer vector or point
// -------------------------------------------------------------------------------------------------
class Int4
{
public:
    Int4() : Data{0, 0, 0, 0} {}
    Int4(int x, int y, int z, int w) : Data{x, y, z, w} {}

    int& operator[](size_t index) { return Data[index]; }
    const int& operator[](size_t index) const { return Data[index]; }

    int X() const { return Data[0]; }
    int Y() const { return Data[1]; }
    int Z() const { return Data[2]; }
    int W() const { return Data[3]; }

    void SetX(int x) { Data[0] = x; }
    void SetY(int y) { Data[1] = y; }
    void SetZ(int z) { Data[2] = z; }
    void SetW(int w) { Data[3] = w; }

    Int4 operator+(const Int4& other) const
    {
        return Int4(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2], Data[3] + other.Data[3]);
    }

    Int4 operator-(const Int4& other) const
    {
        return Int4(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2], Data[3] - other.Data[3]);
    }

    Int4 operator*(int scalar) const
    {
        return Int4(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar, Data[3] * scalar);
    }

private:
    std::array<int, 4> Data;
};


// -------------------------------------------------------------------------------------------------
//  Float2: Represents a 2D vector or point
// -------------------------------------------------------------------------------------------------
class Float2
{
public:
    Float2() : Data{0.0f, 0.0f} {}
    Float2(float x, float y) : Data{x, y} {}

    float& operator[](size_t index) { return Data[index]; }
    const float& operator[](size_t index) const { return Data[index]; }

    float X() const { return Data[0]; }
    float Y() const { return Data[1]; }

    void SetX(float x) { Data[0] = x; }
    void SetY(float y) { Data[1] = y; }

    Float2 operator+(const Float2& other) const
    {
        return Float2(Data[0] + other.Data[0], Data[1] + other.Data[1]);
    }

    Float2 operator-(const Float2& other) const
    {
        return Float2(Data[0] - other.Data[0], Data[1] - other.Data[1]);
    }

    Float2 operator*(float scalar) const
    {
        return Float2(Data[0] * scalar, Data[1] * scalar);
    }

    float Length() const
    {
        return std::sqrt(Data[0] * Data[0] + Data[1] * Data[1]);
    }

private:
    std::array<float, 2> Data;
};


// -------------------------------------------------------------------------------------------------
//  Float3: Represents a 3D vector or point
// -------------------------------------------------------------------------------------------------
class Float3
{
public:
    Float3() : Data{0.0f, 0.0f, 0.0f} {}
    Float3(float x, float y, float z) : Data{x, y, z} {}

    float& operator[](size_t index) { return Data[index]; }
    const float& operator[](size_t index) const { return Data[index]; }

    float X() const { return Data[0]; }
    float Y() const { return Data[1]; }
    float Z() const { return Data[2]; }

    void SetX(float x) { Data[0] = x; }
    void SetY(float y) { Data[1] = y; }
    void SetZ(float z) { Data[2] = z; }

    Float3 operator+(const Float3& other) const
    {
        return Float3(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2]);
    }

    Float3 operator-(const Float3& other) const
    {
        return Float3(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2]);
    }

    Float3 operator*(float scalar) const
    {
        return Float3(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar);
    }

    float Length() const
    {
        return std::sqrt(Data[0] * Data[0] + Data[1] * Data[1] + Data[2] * Data[2]);
    }

private:
    std::array<float, 3> Data;
};

// -------------------------------------------------------------------------------------------------
//  Float4: Represents a 4D vector or point
// -------------------------------------------------------------------------------------------------
class Float4
{
public:
    Float4() : Data{0.0f, 0.0f, 0.0f, 0.0f} {}
    Float4(float x, float y, float z, float w) : Data{x, y, z, w} {}

    float& operator[](size_t index) { return Data[index]; }
    const float& operator[](size_t index) const { return Data[index]; }

    float X() const { return Data[0]; }
    float Y() const { return Data[1]; }
    float Z() const { return Data[2]; }
    float W() const { return Data[3]; }

    void SetX(float x) { Data[0] = x; }
    void SetY(float y) { Data[1] = y; }
    void SetZ(float z) { Data[2] = z; }
    void SetW(float w) { Data[3] = w; }

    Float4 operator+(const Float4& other) const
    {
        return Float4(Data[0] + other.Data[0], Data[1] + other.Data[1], Data[2] + other.Data[2], Data[3] + other.Data[3]);
    }

    Float4 operator-(const Float4& other) const
    {
        return Float4(Data[0] - other.Data[0], Data[1] - other.Data[1], Data[2] - other.Data[2], Data[3] - other.Data[3]);
    }

    Float4 operator*(float scalar) const
    {
        return Float4(Data[0] * scalar, Data[1] * scalar, Data[2] * scalar, Data[3] * scalar);
    }

    float Length() const
    {
        return std::sqrt(Data[0] * Data[0] + Data[1] * Data[1] + Data[2] * Data[2] + Data[3] * Data[3]);
    }

private:
    std::array<float, 4> Data;
};


} // namespace WR::Common