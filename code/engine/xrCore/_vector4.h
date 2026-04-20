#pragma once

#include <cmath>
#include <algorithm>

#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

template <class T>
struct _vector4 {
    typedef T TYPE;
    typedef _vector4<T> Self;
    typedef Self& SelfRef;
    typedef const Self& SelfCRef;

    T x, y, z, w;

    // C++17: constexpr та nodiscard
    [[nodiscard]] constexpr T& operator[](int i) noexcept { return *((T*)this + i); }
    [[nodiscard]] constexpr const T& operator[](int i) const noexcept { return *((T*)this + i); }

    constexpr SelfRef set(T _x, T _y, T _z, T _w = 1) noexcept {
        x = _x; y = _y; z = _z; w = _w;
        return *this;
    }
    constexpr SelfRef set(SelfCRef v) noexcept {
        x = v.x; y = v.y; z = v.z; w = v.w;
        return *this;
    }

    // Для базової арифметики покладаємося на автовекторизацію компілятора (вона ефективніша за ручні інтринсики)
    constexpr SelfRef add(SelfCRef v) noexcept {
        x += v.x; y += v.y; z += v.z; w += v.w;
        return *this;
    }
    constexpr SelfRef add(T s) noexcept {
        x += s; y += s; z += s; w += s;
        return *this;
    }
    constexpr SelfRef add(SelfCRef a, SelfCRef v) noexcept {
        x = a.x + v.x; y = a.y + v.y; z = a.z + v.z; w = a.w + v.w;
        return *this;
    }
    constexpr SelfRef add(SelfCRef a, T s) noexcept {
        x = a.x + s; y = a.y + s; z = a.z + s; w = a.w + s;
        return *this;
    }

    constexpr SelfRef sub(T _x, T _y, T _z, T _w = 1) noexcept {
        x -= _x; y -= _y; z -= _z; w -= _w;
        return *this;
    }
    constexpr SelfRef sub(SelfCRef v) noexcept {
        x -= v.x; y -= v.y; z -= v.z; w -= v.w;
        return *this;
    }
    constexpr SelfRef sub(T s) noexcept {
        x -= s; y -= s; z -= s; w -= s;
        return *this;
    }
    constexpr SelfRef sub(SelfCRef a, SelfCRef v) noexcept {
        x = a.x - v.x; y = a.y - v.y; z = a.z - v.z; w = a.w - v.w;
        return *this;
    }
    constexpr SelfRef sub(SelfCRef a, T s) noexcept {
        x = a.x - s; y = a.y - s; z = a.z - s; w = a.w - s;
        return *this;
    }

    constexpr SelfRef mul(T _x, T _y, T _z, T _w = 1) noexcept {
        x *= _x; y *= _y; z *= _z; w *= _w;
        return *this;
    }
    constexpr SelfRef mul(SelfCRef v) noexcept {
        x *= v.x; y *= v.y; z *= v.z; w *= v.w;
        return *this;
    }
    constexpr SelfRef mul(T s) noexcept {
        x *= s; y *= s; z *= s; w *= s;
        return *this;
    }
    constexpr SelfRef mul(SelfCRef a, SelfCRef v) noexcept {
        x = a.x * v.x; y = a.y * v.y; z = a.z * v.z; w = a.w * v.w;
        return *this;
    }
    constexpr SelfRef mul(SelfCRef a, T s) noexcept {
        x = a.x * s; y = a.y * s; z = a.z * s; w = a.w * s;
        return *this;
    }

    constexpr SelfRef div(SelfCRef v) noexcept {
        x /= v.x; y /= v.y; z /= v.z; w /= v.w;
        return *this;
    }
    constexpr SelfRef div(T s) noexcept {
        x /= s; y /= s; z /= s; w /= s;
        return *this;
    }
    constexpr SelfRef div(SelfCRef a, SelfCRef v) noexcept {
        x = a.x / v.x; y = a.y / v.y; z = a.z / v.z; w = a.w / v.w;
        return *this;
    }
    constexpr SelfRef div(SelfCRef a, T s) noexcept {
        x = a.x / s; y = a.y / s; z = a.z / s; w = a.w / s;
        return *this;
    }

    // Замінено BOOL на bool для відповідності стандартам C++17
    [[nodiscard]] constexpr bool similar(SelfCRef v, T E = EPS_L) const noexcept {
        return std::abs(x - v.x) < E && std::abs(y - v.y) < E && std::abs(z - v.z) < E && std::abs(w - v.w) < E;
    }

    // C++17 Динамічна векторизація
    [[nodiscard]] inline T magnitude_sqr() const noexcept { 
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
            // Безпечне не-вирівняне завантаження 16 байт прямо з пам'яті
            __m128 vec = _mm_loadu_ps(&x);
            // 0xFF означає: перемножити всі 4 і записати суму у всі 4 слоти
            return _mm_cvtss_f32(_mm_dp_ps(vec, vec, 0xFF));
#else
            return x * x + y * y + z * z + w * w;
#endif
        } else {
            return x * x + y * y + z * z + w * w; 
        }
    }

    [[nodiscard]] inline T magnitude() const noexcept { 
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
            __m128 vec = _mm_loadu_ps(&x);
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(vec, vec, 0xFF)));
#else
            return std::sqrt(magnitude_sqr());
#endif
        } else {
            return std::sqrt(magnitude_sqr()); 
        }
    }

    SelfRef normalize() noexcept { 
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
            __m128 vec = _mm_loadu_ps(&x);
            __m128 dp = _mm_dp_ps(vec, vec, 0xFF);
            __m128 rsqrt = _mm_rsqrt_ps(dp); // Апаратний обернений корінь
            _mm_storeu_ps(&x, _mm_mul_ps(vec, rsqrt));
            return *this;
#endif
        }
        return mul(T(1) / magnitude()); 
    }

    // Оптимізація для побудови фрустума і кліпінгу
    SelfRef normalize_as_plane() noexcept { 
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
            __m128 vec = _mm_loadu_ps(&x);
            // 0x7F означає: взяти квадрат суми ТІЛЬКИ x, y, z (0111), 
            // але записати результат в усі 4 слоти регістра (1111)
            __m128 dp = _mm_dp_ps(vec, vec, 0x7F);
            __m128 rsqrt = _mm_rsqrt_ps(dp);
            
            // За одну операцію масштабуємо x, y, z ТА w (!)
            _mm_storeu_ps(&x, _mm_mul_ps(vec, rsqrt));
            return *this;
#endif
        }
        return mul(T(1) / std::sqrt(x * x + y * y + z * z)); 
    }

    constexpr SelfRef lerp(SelfCRef p1, SelfCRef p2, T t) noexcept {
        T invt = T(1) - t;
        x = p1.x * invt + p2.x * t;
        y = p1.y * invt + p2.y * t;
        z = p1.z * invt + p2.z * t;
        w = p1.w * invt + p2.w * t;
        return *this;
    }
};

typedef _vector4<float> Fvector4;
typedef _vector4<double> Dvector4;
typedef _vector4<s32> Ivector4;

typedef __declspec(align(16)) _vector4<float> Fvector4a;
typedef __declspec(align(16)) _vector4<double> Dvector4a;
typedef __declspec(align(16)) _vector4<s32> Ivector4a;

namespace xr {
    template <class T>
    [[nodiscard]] constexpr bool valid(const _vector4<T>& v) noexcept {
        return valid(v.x) && valid(v.y) && valid(v.z) && valid(v.w);
    }
} // namespace xr