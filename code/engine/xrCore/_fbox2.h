#pragma once

#include <cmath>
#include <algorithm>

template <class T>
class _box2 {
public:
    typedef T TYPE;
    typedef _box2<T> Self;
    typedef Self& SelfRef;
    typedef const Self& SelfCRef;
    typedef _vector2<T> Tvector;

    union {
        struct {
            Tvector min;
            Tvector max;
        };
        struct {
            T x1, y1;
            T x2, y2;
        };
    };

    constexpr inline SelfRef set(const Tvector& _min, const Tvector& _max) noexcept {
        min.set(_min);
        max.set(_max);
        return *this;
    }
    constexpr inline SelfRef set(T _x1, T _y1, T _x2, T _y2) noexcept {
        min.set(_x1, _y1);
        max.set(_x2, _y2);
        return *this;
    }
    constexpr inline SelfRef set(SelfCRef b) noexcept {
        min.set(b.min);
        max.set(b.max);
        return *this;
    }

    constexpr inline SelfRef null() noexcept {
        min.set(0.f, 0.f);
        max.set(0.f, 0.f);
        return *this;
    }
    constexpr inline SelfRef identity() noexcept {
        min.set(-0.5f, -0.5f);
        max.set(0.5f, 0.5f);
        return *this;
    }
    constexpr inline SelfRef invalidate() noexcept {
        min.set(type_max<T>, type_max<T>);
        max.set(type_min<T>, type_min<T>);
        return *this;
    }

    constexpr inline SelfRef shrink(T s) noexcept {
        min.add(s);
        max.sub(s);
        return *this;
    }
    constexpr inline SelfRef shrink(const Tvector& s) noexcept {
        min.add(s);
        max.sub(s);
        return *this;
    }
    constexpr inline SelfRef grow(T s) noexcept {
        min.sub(s);
        max.add(s);
        return *this;
    }
    constexpr inline SelfRef grow(const Tvector& s) noexcept {
        min.sub(s);
        max.add(s);
        return *this;
    }

    constexpr inline SelfRef add(const Tvector& p) noexcept {
        min.add(p);
        max.add(p);
        return *this;
    }
    constexpr inline SelfRef offset(const Tvector& p) noexcept {
        min.add(p);
        max.add(p);
        return *this;
    }
    constexpr inline SelfRef add(SelfCRef b, const Tvector& p) noexcept {
        min.add(b.min, p);
        max.add(b.max, p);
        return *this;
    }

    [[nodiscard]] constexpr inline bool contains(T x, T y) const noexcept { 
        return (x >= x1) && (x <= x2) && (y >= y1) && (y <= y2); 
    }
    [[nodiscard]] constexpr inline bool contains(const Tvector& p) const noexcept { 
        return contains(p.x, p.y); 
    }
    [[nodiscard]] constexpr inline bool contains(SelfCRef b) const noexcept { 
        return contains(b.min) && contains(b.max); 
    }

    // C++17 FIX: inline замість constexpr, бо similar використовує std::abs()
    [[nodiscard]] inline bool similar(SelfCRef b) const noexcept { 
        return min.similar(b.min) && max.similar(b.max); 
    }

    constexpr inline SelfRef modify(const Tvector& p) noexcept {
        min.min(p);
        max.max(p);
        return *this;
    }
    constexpr inline SelfRef merge(SelfCRef b) noexcept {
        modify(b.min);
        modify(b.max);
        return *this;
    }
    constexpr inline SelfRef merge(SelfCRef b1, SelfCRef b2) noexcept {
        invalidate();
        merge(b1);
        merge(b2);
        return *this;
    }

    constexpr inline void getsize(Tvector& R) const noexcept { R.sub(max, min); }
    
    inline void getradius(Tvector& R) const noexcept {
        getsize(R);
        R.mul(0.5f);
    }
    inline T getradius() const noexcept {
        Tvector R;
        getsize(R);
        R.mul(0.5f);
        return R.magnitude();
    }

    constexpr inline void getcenter(Tvector& C) const noexcept {
        C.x = (min.x + max.x) * 0.5f;
        C.y = (min.y + max.y) * 0.5f;
    }
    inline void getsphere(Tvector& C, T& R) const noexcept {
        getcenter(C);
        R = C.distance_to(max);
    }

    [[nodiscard]] constexpr inline bool intersect(SelfCRef box) const noexcept {
        if (max.x < box.min.x) return false;
        if (max.y < box.min.y) return false;
        if (min.x > box.max.x) return false;
        if (min.y > box.max.y) return false;
        return true;
    }

    constexpr inline SelfRef sort() noexcept {
        T tmp;
        if (min.x > max.x) {
            tmp = min.x; min.x = max.x; max.x = tmp;
        }
        if (min.y > max.y) {
            tmp = min.y; min.y = max.y; max.y = tmp;
        }
        return *this;
    }

    // Переведено на bool, а також зроблено константними методами
    inline bool Pick(const Tvector& start, const Tvector& dir) const noexcept {
        T alpha, xt, yt;
        Tvector rvmin, rvmax;

        rvmin.sub(min, start);
        rvmax.sub(max, start);

        if (!fis_zero(dir.x)) {
            alpha = rvmin.x / dir.x;
            yt = alpha * dir.y;
            if (yt >= rvmin.y && yt <= rvmax.y) return true;
            alpha = rvmax.x / dir.x;
            yt = alpha * dir.y;
            if (yt >= rvmin.y && yt <= rvmax.y) return true;
        }

        if (!fis_zero(dir.y)) {
            alpha = rvmin.y / dir.y;
            xt = alpha * dir.x;
            if (xt >= rvmin.x && xt <= rvmax.x) return true;
            alpha = rvmax.y / dir.y;
            xt = alpha * dir.x;
            if (xt >= rvmin.x && xt <= rvmax.x) return true;
        }
        return false;
    }

    inline bool pick_exact(const Tvector& start, const Tvector& dir) const noexcept {
        T alpha, xt, yt;
        Tvector rvmin, rvmax;

        rvmin.sub(min, start);
        rvmax.sub(max, start);

        if (std::abs(dir.x) != 0.f) {
            alpha = rvmin.x / dir.x;
            yt = alpha * dir.y;
            if (yt >= rvmin.y - EPS && yt <= rvmax.y + EPS) return true;
            alpha = rvmax.x / dir.x;
            yt = alpha * dir.y;
            if (yt >= rvmin.y - EPS && yt <= rvmax.y + EPS) return true;
        }
        if (std::abs(dir.y) != 0.f) {
            alpha = rvmin.y / dir.y;
            xt = alpha * dir.x;
            if (xt >= rvmin.x - EPS && xt <= rvmax.x + EPS) return true;
            alpha = rvmax.y / dir.y;
            xt = alpha * dir.x;
            if (xt >= rvmin.x - EPS && xt <= rvmax.x + EPS) return true;
        }
        return false;
    }

    // C++17 FIX: Додано перевантаження для const, щоб не ламалася компіляція у Pick2
    inline u32& IR(T& x) const noexcept { return (u32&)x; }
    inline const u32& IR(const T& x) const noexcept { return (const u32&)x; }

    inline bool Pick2(const Tvector& origin, const Tvector& dir, Tvector& coord) const noexcept {
        bool Inside = true;
        Tvector MaxT;
        MaxT.x = MaxT.y = -1.0f;

        if (origin[0] < min[0]) {
            coord[0] = min[0]; Inside = false;
            if (IR(dir[0])) MaxT[0] = (min[0] - origin[0]) / dir[0];
        } else if (origin[0] > max[0]) {
            coord[0] = max[0]; Inside = false;
            if (IR(dir[0])) MaxT[0] = (max[0] - origin[0]) / dir[0];
        }

        if (origin[1] < min[1]) {
            coord[1] = min[1]; Inside = false;
            if (IR(dir[1])) MaxT[1] = (min[1] - origin[1]) / dir[1];
        } else if (origin[1] > max[1]) {
            coord[1] = max[1]; Inside = false;
            if (IR(dir[1])) MaxT[1] = (max[1] - origin[1]) / dir[1];
        }

        if (Inside) {
            coord = origin;
            return true;
        }

        u32 WhichPlane = 0;
        if (MaxT[1] > MaxT[0]) WhichPlane = 1;

        if (IR(MaxT[WhichPlane]) & 0x80000000)
            return false;

        if (0 == WhichPlane) {
            coord[1] = origin[1] + MaxT[0] * dir[1];
            if ((coord[1] < min[1]) || (coord[1] > max[1])) return false;
            return true;
        } else {
            coord[0] = origin[0] + MaxT[1] * dir[0];
            if ((coord[0] < min[0]) || (coord[0] > max[0])) return false;
            return true;
        }
    }

    constexpr inline void getpoint(int index, Tvector& result) const noexcept {
        switch (index) {
        case 0: result.set(min.x, min.y); break;
        case 1: result.set(min.x, min.y); break;
        case 2: result.set(max.x, min.y); break;
        case 3: result.set(max.x, min.y); break;
        default: result.set(0.f, 0.f); break;
        }
    }
    constexpr inline void getpoints(Tvector* result) const noexcept {
        result[0].set(min.x, min.y);
        result[1].set(min.x, min.y);
        result[2].set(max.x, min.y);
        result[3].set(max.x, min.y);
    }
};

typedef _box2<float> Fbox2;
typedef _box2<double> Dbox2;

namespace xr {
    template <class T>
    [[nodiscard]] inline bool valid(const _box2<T>& c) noexcept {
        return valid(c.min) && valid(c.max);
    }
} // xr namespace