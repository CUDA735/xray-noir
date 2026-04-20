#pragma once

#include "_vector3d.h"
#include <algorithm>

[[nodiscard]] constexpr inline Fvector cr_fvector3(float f) noexcept {
    return { f, f, f };
}

[[nodiscard]] constexpr inline Fvector cr_fvector3(float x, float y, float z) noexcept {
    return { x, y, z };
}

[[nodiscard]] inline Fvector cr_fvector3_hp(float h, float p) noexcept {
    Fvector res;
    res.setHP(h, p);
    return res;
}

[[nodiscard]] constexpr inline Fvector operator+(const Fvector& v1, const Fvector& v2) noexcept {
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

[[nodiscard]] constexpr inline Fvector operator-(const Fvector& v1, const Fvector& v2) noexcept {
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

[[nodiscard]] constexpr inline Fvector operator-(const Fvector& v) noexcept { 
    return { -v.x, -v.y, -v.z }; 
}

[[nodiscard]] constexpr inline Fvector operator*(const Fvector& v, float f) noexcept {
    return { v.x * f, v.y * f, v.z * f };
}

[[nodiscard]] constexpr inline Fvector operator*(float f, const Fvector& v) noexcept {
    return { v.x * f, v.y * f, v.z * f };
}

[[nodiscard]] constexpr inline Fvector operator/(const Fvector& v, float f) noexcept {
    const float repr_f = 1.f / f;
    return { v.x * repr_f, v.y * repr_f, v.z * repr_f };
}

namespace xr {

[[nodiscard]] constexpr inline Fvector min(const Fvector& v1, const Fvector& v2) noexcept {
    return { std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z) };
}

[[nodiscard]] constexpr inline Fvector max(const Fvector& v1, const Fvector& v2) noexcept {
    return { std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z) };
}

// C++17 FIX: Прибрано constexpr через використання std::abs
[[nodiscard]] inline Fvector abs(const Fvector& v) noexcept {
    return { std::abs(v.x), std::abs(v.y), std::abs(v.z) };
}

} // xr namespace

[[nodiscard]] inline Fvector normalize(const Fvector& v) noexcept {
    Fvector r(v);
    r.normalize();
    return r;
}

[[nodiscard]] inline float magnitude(const Fvector& v) noexcept { 
    return v.magnitude(); 
}

[[nodiscard]] constexpr inline float sqaure_magnitude(const Fvector& v) noexcept { 
    return v.square_magnitude(); 
}

[[nodiscard]] inline float dotproduct(const Fvector& v1, const Fvector& v2) noexcept {
    return v1.dotproduct(v2);
}

[[nodiscard]] constexpr inline Fvector crossproduct(const Fvector& v1, const Fvector& v2) noexcept {
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

[[nodiscard]] inline Fvector cr_vectorHP(float h, float p) noexcept {
    const float ch = std::cos(h), cp = std::cos(p), sh = std::sin(h), sp = std::sin(p);
    return { -cp * sh, sp, cp * ch };
}

[[nodiscard]] inline float angle_between_vectors(const Fvector& v1, const Fvector& v2) noexcept {
    const float mag1 = v1.magnitude();
    const float mag2 = v2.magnitude();
    const float epsilon = 1e-6f;
    
    if (mag1 < epsilon || mag2 < epsilon) {
        return 0.f;
    }

    float angle_cos = v1.dotproduct(v2) / (mag1 * mag2);
    return std::acos(std::clamp(angle_cos, -1.f, 1.f));
}

[[nodiscard]] inline Fvector rotate_point(const Fvector& point, const float angle) noexcept {
    const float cos_alpha = std::cos(angle);
    const float sin_alpha = std::sin(angle);

    return {
        point.x * cos_alpha - point.z * sin_alpha, 
        0.0f,
        point.x * sin_alpha + point.z * cos_alpha
    };
}