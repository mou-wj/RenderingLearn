#pragma once
#include "Matrix.hpp"
#include "Math.hpp"
namespace Core{

    // LookAt (right-handed)
    inline Mat4 LookAtRH(const Float3& eye, const Float3& center, const Float3& up) {
        Float3 f = { center.x - eye.x, center.y - eye.y, center.z - eye.z };
        float flen = std::sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
        f.x /= flen; f.y /= flen; f.z /= flen;
        Float3 s = { f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x };
        float slen = std::sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
        s.x /= slen; s.y /= slen; s.z /= slen;
        Float3 u = { s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x };

        Mat4 result = Mat4::Identity();
        result(0, 0) = s.x; result(0, 1) = s.y; result(0, 2) = s.z;
        result(1, 0) = u.x; result(1, 1) = u.y; result(1, 2) = u.z;
        result(2, 0) = -f.x; result(2, 1) = -f.y; result(2, 2) = -f.z;
        result(0, 3) = - (s.x * eye.x + s.y * eye.y + s.z * eye.z);
        result(1, 3) = - (u.x * eye.x + u.y * eye.y + u.z * eye.z);
        result(2, 3) =   (f.x * eye.x + f.y * eye.y + f.z * eye.z);
        return result;
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