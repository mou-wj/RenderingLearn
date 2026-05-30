#pragma once
#include "Matrix.hpp"
#include "Math.hpp"
namespace Core{

    // LookAt (right-handed)
    inline Mat4 LookAtRH(const Float3& eye, const Float3& center, const Float3& up) {
        Float3 f =
            Normalize(center - eye);

        Float3 s =
            Normalize(Cross(f, up));

        Float3 u =
            Cross(s, f);

        Mat4 m =
            Mat4::Identity();

        m(0, 0) = s.x;
        m(0, 1) = s.y;
        m(0, 2) = s.z;
        m(0, 3) = -Dot(s, eye);

        m(1, 0) = u.x;
        m(1, 1) = u.y;
        m(1, 2) = u.z;
        m(1, 3) = -Dot(u, eye);

        m(2, 0) = -f.x;
        m(2, 1) = -f.y;
        m(2, 2) = -f.z;
        m(2, 3) = Dot(f, eye);

        return m;
    }

    // Perspective projection (right-handed)
    inline Mat4 PerspectiveRH(float fovy, float aspect, float zNear, float zFar) {
        float tanHalfFovy = std::tan(fovy / 2.0f);
        Mat4 result{};
        result(0, 0) = 1.0f / (aspect * tanHalfFovy);
        result(1, 1) = 1.0f / tanHalfFovy;
        result(2, 2) = -(zFar + zNear) / (zFar - zNear);
        result(2, 3) = -2.0f * zFar * zNear / (zFar - zNear);
        result(3, 2) = -1.0f;
        return result;
    }

    // Orthographic projection (right-handed)
    inline Mat4 OrthoRH(float left, float right, float bottom, float top, float zNear, float zFar) {
        Mat4 result = Mat4::Identity();
        result(0, 0) = 2.0f / (right - left);
        result(1, 1) = 2.0f / (top - bottom);
        result(2, 2) = -2.0f / (zFar - zNear);
        result(0, 3) = -(right + left) / (right - left);
        result(1, 3) = -(top + bottom) / (top - bottom);
        result(2, 3) = -(zFar + zNear) / (zFar - zNear);
        return result;
    }




}