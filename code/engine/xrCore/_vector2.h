#pragma once

#include <cmath>
#include <algorithm>

template <class T>
struct _vector2 {
    typedef T TYPE;
    typedef _vector2<T> Self;
    typedef Self& SelfRef;
    typedef const Self& SelfCRef;

    T x, y;

    // C++17: constexpr дозволяє обчислювати вектори на етапі компіляції.
    // noexcept гарантує оптимізатору, що тут не буде винятків, що прискорює створення об'єктів.
    
    constexpr SelfRef set(float _u, float _v) noexcept {
        x = T(_u);
        y = T(_v);
        return *this;
    }
    
    constexpr SelfRef set(double _u, double _v) noexcept {
        x = T(_u);
        y = T(_v);
        return *this;
    }
    
    constexpr SelfRef set(int _u, int _v) noexcept {
        x = T(_u);
        y = T(_v);
        return *this;
    }
    
    constexpr SelfRef set(SelfCRef p) noexcept {
        x = p.x;
        y = p.y;
        return *this;
    }

    constexpr SelfRef abs(SelfCRef p) noexcept {
        x = std::abs(p.x); // C++17: std::abs перевантажений для всіх типів
        y = std::abs(p.y);
        return *this;
    }

    constexpr SelfRef min(SelfCRef p) noexcept {
        x = std::min(x, p.x);
        y = std::min(y, p.y);
        return *this;
    }
    
    constexpr SelfRef min(T _x, T _y) noexcept {
        x = std::min(x, _x);
        y = std::min(y, _y);
        return *this;
    }

    constexpr SelfRef max(SelfCRef p) noexcept {
        x = std::max(x, p.x);
        y = std::max(y, p.y);
        return *this;
    }
    
    constexpr SelfRef max(T _x, T _y) noexcept {
        x = std::max(x, _x);
        y = std::max(y, _y);
        return *this;
    }

    constexpr SelfRef sub(const T p) noexcept {
        x -= p;
        y -= p;
        return *this;
    }
    
    constexpr SelfRef sub(SelfCRef p) noexcept {
        x -= p.x;
        y -= p.y;
        return *this;
    }
    
    constexpr SelfRef sub(SelfCRef p1, SelfCRef p2) noexcept {
        x = p1.x - p2.x;
        y = p1.y - p2.y;
        return *this;
    }
    
    constexpr SelfRef sub(SelfCRef p, float d) noexcept {
        x = p.x - T(d);
        y = p.y - T(d);
        return *this;
    }

    constexpr SelfRef add(const T p) noexcept {
        x += p;
        y += p;
        return *this;
    }
    
    constexpr SelfRef add(SelfCRef p) noexcept {
        x += p.x;
        y += p.y;
        return *this;
    }
    
    constexpr SelfRef add(SelfCRef p1, SelfCRef p2) noexcept {
        x = p1.x + p2.x;
        y = p1.y + p2.y;
        return *this;
    }
    
    constexpr SelfRef add(SelfCRef p, float d) noexcept {
        x = p.x + T(d);
        y = p.y + T(d);
        return *this;
    }

    constexpr SelfRef mul(const T s) noexcept {
        x *= s;
        y *= s;
        return *this;
    }
    
    constexpr SelfRef mul(SelfCRef p) noexcept {
        x *= p.x;
        y *= p.y;
        return *this;
    }

    constexpr SelfRef div(const T s) noexcept {
        x /= s;
        y /= s;
        return *this;
    }
    
    constexpr SelfRef div(SelfCRef p) noexcept {
        x /= p.x;
        y /= p.y;
        return *this;
    }

    constexpr SelfRef rot90() noexcept {
        T t = -x;
        x = y;
        y = t;
        return *this;
    }

    constexpr SelfRef cross(SelfCRef D) noexcept {
        x = D.y;
        y = -D.x;
        return *this;
    }

    // C++17 [[nodiscard]] попереджає, якщо результат функції не використовується.
    [[nodiscard]] constexpr T dot(SelfCRef p) const noexcept { 
        return x * p.x + y * p.y; 
    }
    [[nodiscard]] constexpr T dot(SelfRef p) const noexcept { 
        return x * p.x + y * p.y; 
    }

    SelfRef norm() noexcept {
        float m = std::sqrt(float(x * x + y * y));
        x = T(x / m);
        y = T(y / m);
        return *this;
    }

    SelfRef norm_safe() noexcept {
        float m = std::sqrt(float(x * x + y * y));
        if (m > std::numeric_limits<float>::epsilon()) { // Безпечніша перевірка замість if (m)
            x = T(x / m);
            y = T(y / m);
        }
        return *this;
    }

    [[nodiscard]] T distance_to(SelfCRef p) const noexcept {
        return std::sqrt(float((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y)));
    }

    [[nodiscard]] constexpr T square_magnitude() const noexcept { 
        return x * x + y * y; 
    }
    
    [[nodiscard]] T magnitude() const noexcept { 
        return std::sqrt(float(square_magnitude())); 
    }

    constexpr SelfRef mad(SelfCRef p, SelfCRef d, T r) noexcept {
        x = p.x + d.x * r;
        y = p.y + d.y * r;
        return *this;
    }

    [[nodiscard]] constexpr Self Cross() const noexcept {
        return Self{ y, -x }; // C++17 ініціалізація, усуває зайве копіювання
    }

    [[nodiscard]] constexpr bool similar(SelfCRef p, T eu, T ev) const noexcept { 
        return std::abs(x - p.x) < eu && std::abs(y - p.y) < ev; 
    }

    [[nodiscard]] constexpr bool similar(SelfCRef p, float E = EPS_L) const noexcept {
        return std::abs(x - p.x) < E && std::abs(y - p.y) < E;
    }

    constexpr SelfRef averageA(SelfCRef p1, SelfCRef p2) noexcept {
        x = T((p1.x + p2.x) * 0.5f);
        y = T((p1.y + p2.y) * 0.5f);
        return *this;
    }

    SelfRef averageG(SelfCRef p1, SelfCRef p2) noexcept {
        x = T(std::sqrt(float(p1.x * p2.x)));
        y = T(std::sqrt(float(p1.y * p2.y)));
        return *this;
    }

    // Оператор доступу залишаємо оригінальним, але додаємо const correctness
    [[nodiscard]] constexpr T& operator[](int i) noexcept {
        return *(&x + i);
    }
    [[nodiscard]] constexpr const T& operator[](int i) const noexcept {
        return *(&x + i);
    }

    SelfRef normalize() noexcept { return norm(); }
    SelfRef normalize_safe() noexcept { return norm_safe(); }
    
    SelfRef normalize(SelfCRef v) noexcept {
        const float m = std::sqrt(float(v.x * v.x + v.y * v.y));
        x = T(v.x / m);
        y = T(v.y / m);
        return *this;
    }
    
    SelfRef normalize_safe(SelfCRef v) noexcept {
        const float m = std::sqrt(float(v.x * v.x + v.y * v.y));
        if (m > std::numeric_limits<float>::epsilon()) {
            x = T(v.x / m);
            y = T(v.y / m);
        }
        return *this;
    }

    [[nodiscard]] constexpr float dotproduct(SelfCRef p) const noexcept { return dot(p); }
    [[nodiscard]] constexpr float crossproduct(SelfCRef p) const noexcept { return y * p.x - x * p.y; }
    
    [[nodiscard]] float getH() const noexcept {
        if (fis_zero(y)) {
            if (fis_zero(x))
                return 0.f;
            else
                return ((x > 0.0f) ? -PI_DIV_2 : PI_DIV_2);
        } else if (y < 0.f) {
            return float(-(std::atan(x / y) - PI));
        } else {
            return float(-std::atan(x / y));
        }
    }
};

typedef _vector2<float> Fvector2;
typedef _vector2<double> Dvector2;
typedef _vector2<int> Ivector2;

namespace xr {
    template <class T>
    [[nodiscard]] constexpr bool valid(const _vector2<T>& v) noexcept {
        return valid(v.x) && valid(v.y);
    }
} // namespace xr