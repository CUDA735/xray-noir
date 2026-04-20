#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

#ifndef IC
#define IC inline
#endif

template <class T>
struct _vector3 {
    typedef T TYPE;
    typedef _vector3<T> Self;
    typedef Self& SelfRef;
    typedef const Self& SelfCRef;

    T x, y, z;

    [[nodiscard]] constexpr T& operator[](int i) noexcept { return *((T*)this + i); }
    [[nodiscard]] constexpr const T& operator[](int i) const noexcept { return *((T*)this + i); }

    constexpr SelfRef set(T _x, T _y, T _z) noexcept {
        x = _x; y = _y; z = _z;
        return *this;
    }
    constexpr SelfRef set(const _vector3<float>& v) noexcept {
        x = T(v.x); y = T(v.y); z = T(v.z);
        return *this;
    }
    constexpr SelfRef set(const _vector3<double>& v) noexcept {
        x = T(v.x); y = T(v.y); z = T(v.z);
        return *this;
    }
    constexpr SelfRef set(const float* p) noexcept {
        x = T(p[0]); y = T(p[1]); z = T(p[2]);
        return *this;
    }
    constexpr SelfRef set(const double* p) noexcept {
        x = T(p[0]); y = T(p[1]); z = T(p[2]);
        return *this;
    }

    constexpr SelfRef add(SelfCRef v) noexcept {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    constexpr SelfRef add(T s) noexcept {
        x += s; y += s; z += s;
        return *this;
    }
    constexpr SelfRef add(SelfCRef a, SelfCRef v) noexcept {
        x = a.x + v.x; y = a.y + v.y; z = a.z + v.z;
        return *this;
    }
    constexpr SelfRef add(SelfCRef a, T s) noexcept {
        x = a.x + s; y = a.y + s; z = a.z + s;
        return *this;
    }

    constexpr SelfRef sub(SelfCRef v) noexcept {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    constexpr SelfRef sub(T s) noexcept {
        x -= s; y -= s; z -= s;
        return *this;
    }
    constexpr SelfRef sub(SelfCRef a, SelfCRef v) noexcept {
        x = a.x - v.x; y = a.y - v.y; z = a.z - v.z;
        return *this;
    }
    constexpr SelfRef sub(SelfCRef a, T s) noexcept {
        x = a.x - s; y = a.y - s; z = a.z - s;
        return *this;
    }

    constexpr SelfRef mul(SelfCRef v) noexcept {
        x *= v.x; y *= v.y; z *= v.z;
        return *this;
    }
    constexpr SelfRef mul(T s) noexcept {
        x *= s; y *= s; z *= s;
        return *this;
    }
    constexpr SelfRef mul(SelfCRef a, SelfCRef v) noexcept {
        x = a.x * v.x; y = a.y * v.y; z = a.z * v.z;
        return *this;
    }
    constexpr SelfRef mul(SelfCRef a, T s) noexcept {
        x = a.x * s; y = a.y * s; z = a.z * s;
        return *this;
    }

    constexpr SelfRef div(SelfCRef v) noexcept {
        x /= v.x; y /= v.y; z /= v.z;
        return *this;
    }
    constexpr SelfRef div(T s) noexcept {
        x /= s; y /= s; z /= s;
        return *this;
    }
    constexpr SelfRef div(SelfCRef a, SelfCRef v) noexcept {
        x = a.x / v.x; y = a.y / v.y; z = a.z / v.z;
        return *this;
    }
    constexpr SelfRef div(SelfCRef a, T s) noexcept {
        x = a.x / s; y = a.y / s; z = a.z / s;
        return *this;
    }

    constexpr SelfRef invert() noexcept {
        x = -x; y = -y; z = -z;
        return *this;
    }
    constexpr SelfRef invert(SelfCRef a) noexcept {
        x = -a.x; y = -a.y; z = -a.z;
        return *this;
    }

    constexpr SelfRef min(SelfCRef v1, SelfCRef v2) noexcept {
        x = std::min(v1.x, v2.x); y = std::min(v1.y, v2.y); z = std::min(v1.z, v2.z);
        return *this;
    }
    constexpr SelfRef min(SelfCRef v) noexcept {
        x = std::min(x, v.x); y = std::min(y, v.y); z = std::min(z, v.z);
        return *this;
    }
    constexpr SelfRef max(SelfCRef v1, SelfCRef v2) noexcept {
        x = std::max(v1.x, v2.x); y = std::max(v1.y, v2.y); z = std::max(v1.z, v2.z);
        return *this;
    }
    constexpr SelfRef max(SelfCRef v) noexcept {
        x = std::max(x, v.x); y = std::max(y, v.y); z = std::max(z, v.z);
        return *this;
    }

    SelfRef abs(SelfCRef v) noexcept {
        x = std::abs(v.x); y = std::abs(v.y); z = std::abs(v.z);
        return *this;
    }
    
    [[nodiscard]] inline bool similar(SelfCRef v, T E = EPS_L) const noexcept {
        return std::abs(x - v.x) < E && std::abs(y - v.y) < E && std::abs(z - v.z) < E;
    }

    SelfRef set_length(T l) noexcept {
        mul(l / magnitude());
        return *this;
    }

    SelfRef align() noexcept {
        y = 0;
        if (std::abs(z) >= std::abs(x)) {
            z /= std::abs(z ? z : 1);
            x = 0;
        } else {
            x /= std::abs(x);
            z = 0;
        }
        return *this;
    }

    SelfRef squeeze(T Epsilon) noexcept {
        if (std::abs(x) < Epsilon) x = 0;
        if (std::abs(y) < Epsilon) y = 0;
        if (std::abs(z) < Epsilon) z = 0;
        return *this;
    }

    SelfRef clamp(SelfCRef min, SelfCRef max) noexcept {
        x = std::clamp(x, min.x, max.x); 
        y = std::clamp(y, min.y, max.y);
        z = std::clamp(z, min.z, max.z);
        return *this;
    }

    SelfRef clamp(SelfCRef _v) noexcept {
        const T vx = std::abs(_v.x), vy = std::abs(_v.y), vz = std::abs(_v.z);
        x = std::clamp(x, -vx, vx);
        y = std::clamp(y, -vy, vy);
        z = std::clamp(z, -vz, vz);
        return *this;
    }

    constexpr SelfRef inertion(SelfCRef p, T v) noexcept {
        T inv = T(1) - v;
        x = v * x + inv * p.x;
        y = v * y + inv * p.y;
        z = v * z + inv * p.z;
        return *this;
    }
    constexpr SelfRef average(SelfCRef p) noexcept {
        x = (x + p.x) * 0.5f; y = (y + p.y) * 0.5f; z = (z + p.z) * 0.5f;
        return *this;
    }
    constexpr SelfRef average(SelfCRef p1, SelfCRef p2) noexcept {
        x = (p1.x + p2.x) * 0.5f; y = (p1.y + p2.y) * 0.5f; z = (p1.z + p2.z) * 0.5f;
        return *this;
    }
    constexpr SelfRef lerp(SelfCRef p1, SelfCRef p2, T t) noexcept {
        T invt = T(1) - t;
        x = p1.x * invt + p2.x * t;
        y = p1.y * invt + p2.y * t;
        z = p1.z * invt + p2.z * t;
        return *this;
    }

    constexpr SelfRef mad(SelfCRef d, T m) noexcept {
        x += d.x * m; y += d.y * m; z += d.z * m;
        return *this;
    }
    constexpr SelfRef mad(SelfCRef p, SelfCRef d, T m) noexcept {
        x = p.x + d.x * m; y = p.y + d.y * m; z = p.z + d.z * m;
        return *this;
    }
    constexpr SelfRef mad(SelfCRef d, SelfCRef s) noexcept {
        x += d.x * s.x; y += d.y * s.y; z += d.z * s.z;
        return *this;
    }
    constexpr SelfRef mad(SelfCRef p, SelfCRef d, SelfCRef s) noexcept {
        x = p.x + d.x * s.x; y = p.y + d.y * s.y; z = p.z + d.z * s.z;
        return *this;
    }

    [[nodiscard]] constexpr T square_magnitude() const noexcept { 
        return x * x + y * y + z * z; 
    }

    [[nodiscard]] inline T magnitude() const noexcept { 
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
            __m128 v = _mm_set_ps(0.0f, z, y, x);
            __m128 dp = _mm_dp_ps(v, v, 0x71); 
            return _mm_cvtss_f32(_mm_sqrt_ss(dp));
#else
            return std::sqrt(x * x + y * y + z * z);
#endif
        } else {
            return std::sqrt(x * x + y * y + z * z); 
        }
    }

    T normalize_magn() noexcept {
        T sq_mag = square_magnitude();
        if (sq_mag > std::numeric_limits<T>::min()) {
            T len = std::sqrt(sq_mag);
            T inv_len = T(1) / len;
            x *= inv_len; y *= inv_len; z *= inv_len;
            return len;
        }
        return T(0);
    }

    SelfRef normalize() noexcept {
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
            __m128 v = _mm_set_ps(0.0f, z, y, x);
            __m128 dp = _mm_dp_ps(v, v, 0x7F); 
            __m128 rsqrt = _mm_rsqrt_ps(dp); 
            __m128 res = _mm_mul_ps(v, rsqrt);
            alignas(16) float tmp[4];
            _mm_store_ps(tmp, res);
            x = tmp[0]; y = tmp[1]; z = tmp[2];
            return *this;
#endif
        }
        T magsq = x * x + y * y + z * z;
        if (magsq > std::numeric_limits<T>::min()) {
            T mag = std::sqrt(T(1) / magsq);
            x *= mag; y *= mag; z *= mag;
        }
        return *this;
    }

    SelfRef normalize_safe() noexcept {
        T magsq = square_magnitude();
        if (magsq > std::numeric_limits<T>::min()) {
            T mag = std::sqrt(T(1) / magsq);
            x *= mag; y *= mag; z *= mag;
        }
        return *this;
    }

    SelfRef normalize(SelfCRef v) noexcept {
        T magsq = v.x * v.x + v.y * v.y + v.z * v.z;
        if (magsq > std::numeric_limits<T>::min()) {
            T mag = std::sqrt(T(1) / magsq);
            x = v.x * mag; y = v.y * mag; z = v.z * mag;
        }
        return *this;
    }

    SelfRef normalize_safe(SelfCRef v) noexcept {
        T magsq = v.x * v.x + v.y * v.y + v.z * v.z;
        if (magsq > std::numeric_limits<T>::min()) {
            T mag = std::sqrt(T(1) / magsq);
            x = v.x * mag; y = v.y * mag; z = v.z * mag;
        }
        return *this;
    }

    // C++17 FIX: Повертаємо класичні сигнатури з CRandom& R = ::Random
    SelfRef random_dir(CRandom& R = ::Random) {
        z = std::cos(R.randF(PI));
        T a = R.randF(PI_MUL_2);
        T r = std::sqrt(T(1) - z * z);
        T sa = std::sin(a);
        T ca = std::cos(a);
        x = r * ca;
        y = r * sa;
        return *this;
    }
    
    SelfRef random_dir(SelfCRef ConeAxis, float ConeAngle, CRandom& R = ::Random) {
        Self rnd;
        rnd.random_dir(R);
        mad(ConeAxis, rnd, R.randF(std::tan(ConeAngle)));
        normalize();
        return *this;
    }
    
    SelfRef random_point(SelfCRef BoxSize, CRandom& R = ::Random) {
        x = R.randFs(BoxSize.x);
        y = R.randFs(BoxSize.y);
        z = R.randFs(BoxSize.z);
        return *this;
    }
    
    SelfRef random_point(T r, CRandom& R = ::Random) {
        random_dir(R);
        mul(R.randF(r));
        return *this;
    }

    [[nodiscard]] inline T dotproduct(SelfCRef v) const noexcept {
        if constexpr (std::is_same_v<T, float>) {
#if defined(XR_AVX2) || defined(XR_AVX) || defined(XR_SSE4) || defined(__SSE4_1__)
            __m128 m1 = _mm_set_ps(0.0f, z, y, x);
            __m128 m2 = _mm_set_ps(0.0f, v.z, v.y, v.x);
            return _mm_cvtss_f32(_mm_dp_ps(m1, m2, 0x71));
#else
            return x * v.x + y * v.y + z * v.z;
#endif
        } else {
            return x * v.x + y * v.y + z * v.z;
        }
    }

    constexpr SelfRef crossproduct(SelfCRef v1, SelfCRef v2) noexcept {
        x = v1.y * v2.z - v1.z * v2.y;
        y = v1.z * v2.x - v1.x * v2.z;
        z = v1.x * v2.y - v1.y * v2.x;
        return *this;
    }

    [[nodiscard]] inline T distance_to_xz(SelfCRef v) const noexcept {
        return std::sqrt((x - v.x) * (x - v.x) + (z - v.z) * (z - v.z));
    }
    [[nodiscard]] constexpr T distance_to_xz_sqr(SelfCRef v) const noexcept {
        return (x - v.x) * (x - v.x) + (z - v.z) * (z - v.z);
    }
    [[nodiscard]] constexpr T distance_to_sqr(SelfCRef v) const noexcept {
        return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z);
    }
    [[nodiscard]] inline T distance_to(SelfCRef v) const noexcept { 
        return std::sqrt(distance_to_sqr(v)); 
    }

    constexpr SelfRef from_bary(SelfCRef V1, SelfCRef V2, SelfCRef V3, T u, T v, T w) noexcept {
        x = V1.x * u + V2.x * v + V3.x * w;
        y = V1.y * u + V2.y * v + V3.y * w;
        z = V1.z * u + V2.z * v + V3.z * w;
        return *this;
    }
    constexpr SelfRef from_bary(SelfCRef V1, SelfCRef V2, SelfCRef V3, SelfCRef B) noexcept {
        return from_bary(V1, V2, V3, B.x, B.y, B.z);
    }
    constexpr SelfRef from_bary4(SelfCRef V1, SelfCRef V2, SelfCRef V3, SelfCRef V4, T u, T v, T w, T t) noexcept {
        x = V1.x * u + V2.x * v + V3.x * w + V4.x * t;
        y = V1.y * u + V2.y * v + V3.y * w + V4.y * t;
        z = V1.z * u + V2.z * v + V3.z * w + V4.z * t;
        return *this;
    }

    SelfRef mknormal_non_normalized(SelfCRef p0, SelfCRef p1, SelfCRef p2) noexcept {
        _vector3<T> v01, v12;
        v01.sub(p1, p0);
        v12.sub(p2, p1);
        crossproduct(v01, v12);
        return *this;
    }
    SelfRef mknormal(SelfCRef p0, SelfCRef p1, SelfCRef p2) noexcept {
        mknormal_non_normalized(p0, p1, p2);
        normalize_safe();
        return *this;
    }

    SelfRef setHP(T h, T p) noexcept {
        T _ch = std::cos(h), _cp = std::cos(p), _sh = std::sin(h), _sp = std::sin(p);
        x = -_cp * _sh;
        y = _sp;
        z = _cp * _ch;
        return *this;
    }
    
    void getHP(T& h, T& p) const noexcept {
        if (std::abs(x) < EPS_L && std::abs(z) < EPS_L) {
            h = 0.0f;
            if (std::abs(float(y)) >= EPS_L)
                p = (y > 0.0f) ? PI_DIV_2 : -PI_DIV_2;
            else
                p = 0.0f;
        } else {
            if (std::abs(z) < EPS_L)
                h = (x > 0.0f) ? -PI_DIV_2 : PI_DIV_2;
            else if (z < 0.0f)
                h = -(std::atan(x / z) - PI);
            else
                h = -std::atan(x / z);
            
            const float hyp = std::sqrt(x * x + z * z);
            if (std::abs(float(hyp)) < EPS_L)
                p = (y > 0.0f) ? PI_DIV_2 : -PI_DIV_2;
            else
                p = std::atan(y / hyp);
        }
    }
    
    [[nodiscard]] inline float getH() const noexcept {
        if (std::abs(x) < EPS_L && std::abs(z) < EPS_L) {
            return 0.0f;
        } else {
            if (std::abs(z) < EPS_L)
                return (x > 0.0f) ? -PI_DIV_2 : PI_DIV_2;
            else if (z < 0.0f)
                return -(std::atan(x / z) - PI);
            else
                return -std::atan(x / z);
        }
    }
    
    [[nodiscard]] inline float getP() const noexcept {
        if (std::abs(x) < EPS_L && std::abs(z) < EPS_L) {
            if (std::abs(float(y)) >= EPS_L)
                return (y > 0.0f) ? PI_DIV_2 : -PI_DIV_2;
            else
                return 0.0f;
        } else {
            const float hyp = std::sqrt(x * x + z * z);
            if (std::abs(float(hyp)) < EPS_L)
                return (y > 0.0f) ? PI_DIV_2 : -PI_DIV_2;
            else
                return std::atan(y / hyp);
        }
    }

    inline SelfRef reflect(SelfCRef dir, SelfCRef norm) noexcept {
        return mad(dir, norm, -2 * dir.dotproduct(norm));
    }
    inline SelfRef slide(SelfCRef dir, SelfCRef norm) noexcept {
        return mad(dir, norm, -dir.dotproduct(norm));
    }

    static void generate_orthonormal_basis(SelfCRef dir, _vector3<T>& up, _vector3<T>& right) noexcept {
        T fInvLength;

        if (std::abs(dir.x) >= std::abs(dir.y)) {
            fInvLength = T(1) / std::sqrt(dir.x * dir.x + dir.z * dir.z);
            up.x = -dir.z * fInvLength;
            up.y = 0.0f;
            up.z = +dir.x * fInvLength;
        } else {
            fInvLength = T(1) / std::sqrt(dir.y * dir.y + dir.z * dir.z);
            up.x = 0.0f;
            up.y = +dir.z * fInvLength;
            up.z = -dir.y * fInvLength;
        }
        right.crossproduct(up, dir);
    }

    static void generate_orthonormal_basis_normalized(_vector3<T>& dir, _vector3<T>& up, _vector3<T>& right) noexcept {
        T fInvLength;
        dir.normalize();
        
        if (std::abs(dir.y - 1.f) < EPS) {
            up.set(0.f, 0.f, 1.f);
            fInvLength = T(1) / std::sqrt(dir.x * dir.x + dir.y * dir.y);
            right.x = -dir.y * fInvLength;
            right.y = dir.x * fInvLength;
            right.z = 0.f;
            
            up.x = -dir.z * right.y;
            up.y = dir.z * right.x;
            up.z = dir.x * right.y - dir.y * right.x;
        } else {
            up.set(0.f, 1.f, 0.f);
            fInvLength = T(1) / std::sqrt(dir.x * dir.x + dir.z * dir.z);
            right.x = dir.z * fInvLength;
            right.y = 0.f;
            right.z = -dir.x * fInvLength;
            
            up.x = dir.y * right.z;
            up.y = dir.z * right.x - dir.x * right.z;
            up.z = -dir.y * right.x;
        }
    }
};

typedef _vector3<float> Fvector;
typedef _vector3<float> Fvector3;
typedef _vector3<double> Dvector;
typedef _vector3<double> Dvector3;
typedef _vector3<int> Ivector;
typedef _vector3<int> Ivector3;

namespace xr {
    template <class T>
    [[nodiscard]] inline bool valid(const _vector3<T>& v) noexcept {
        return valid(v.x) && valid(v.y) && valid(v.z);
    }
}

//////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4244)

inline double rsqrt(const double v) noexcept { return 1.0 / std::sqrt(v); }

IC BOOL exact_normalize(float* a) noexcept {
#if defined(XR_AVX) || defined(XR_AVX2) || defined(XR_SSE4) || defined(__SSE4_1__)
    __m128 v = _mm_set_ps(0.0f, a[2], a[1], a[0]);
    __m128 dp = _mm_dp_ps(v, v, 0x7F);
    float sqr_magnitude = _mm_cvtss_f32(dp);
    
    if (sqr_magnitude > 1.192092896e-05F) {
        __m128 rsqrt_val = _mm_rsqrt_ps(dp);
        __m128 res = _mm_mul_ps(v, rsqrt_val);
        alignas(16) float tmp[4];
        _mm_store_ps(tmp, res);
        a[0] = tmp[0]; a[1] = tmp[1]; a[2] = tmp[2];
        return TRUE;
    }
#else
    const double sqr_magnitude = double(a[0])*a[0] + double(a[1])*a[1] + double(a[2])*a[2];
    const double epsilon = 1.192092896e-05;
    if (sqr_magnitude > epsilon) {
        double l = rsqrt(sqr_magnitude);
        a[0] = float(a[0] * l);
        a[1] = float(a[1] * l);
        a[2] = float(a[2] * l);
        return TRUE;
    }
#endif

    double a0 = a[0], a1 = a[1], a2 = a[2];
    double aa0 = std::abs(a0), aa1 = std::abs(a1), aa2 = std::abs(a2);
    double l;
    
    if (aa1 > aa0) {
        if (aa2 > aa1) {
            goto aa2_largest;
        } else {
            a0 /= aa1; a2 /= aa1;
            l = rsqrt(a0 * a0 + a2 * a2 + 1.0);
            a[0] = float(a0 * l); a[1] = float(std::copysign(l, a1)); a[2] = float(a2 * l);
        }
    } else {
        if (aa2 > aa0) {
        aa2_largest:
            a0 /= aa2; a1 /= aa2;
            l = rsqrt(a0 * a0 + a1 * a1 + 1.0);
            a[0] = float(a0 * l); a[1] = float(a1 * l); a[2] = float(std::copysign(l, a2));
        } else {
            if (aa0 <= 0) {
                a[0] = 0.f; a[1] = 1.f; a[2] = 0.f;
                return FALSE;
            }
            a1 /= aa0; a2 /= aa0;
            l = rsqrt(a1 * a1 + a2 * a2 + 1.0);
            a[0] = float(std::copysign(l, a0)); a[1] = float(a1 * l); a[2] = float(a2 * l);
        }
    }
    return TRUE;
}

IC BOOL exact_normalize(Fvector3& a) noexcept { return exact_normalize(&a.x); }

#pragma warning(pop)