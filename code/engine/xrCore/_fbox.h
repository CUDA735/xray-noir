#ifndef __FBOX
#define __FBOX

template <class T>
class _box3 {
public:
    typedef T TYPE;
    typedef _box3<T> Self;
    typedef Self& SelfRef;
    typedef const Self& SelfCRef;
    typedef _vector3<T> Tvector;
    typedef _matrix<T> Tmatrix;

public:
    union {
        struct {
            Tvector min;
            Tvector max;
        };
        struct {
            T x1, y1, z1;
            T x2, y2, z2;
        };
    };

    [[nodiscard]] constexpr inline bool is_valid() const noexcept { return (x2 >= x1) && (y2 >= y1) && (z2 >= z1); }

    [[nodiscard]] constexpr inline const T* data() const noexcept { return &min.x; }

    constexpr inline SelfRef set(const Tvector& _min, const Tvector& _max) noexcept {
        min.set(_min);
        max.set(_max);
        return *this;
    }
    constexpr inline SelfRef set(T x1, T y1, T z1, T x2, T y2, T z2) noexcept {
        min.set(x1, y1, z1);
        max.set(x2, y2, z2);
        return *this;
    }
    constexpr inline SelfRef set(SelfCRef b) noexcept {
        min.set(b.min);
        max.set(b.max);
        return *this;
    }
    constexpr inline SelfRef setb(const Tvector& center, const Tvector& dim) noexcept {
        min.sub(center, dim);
        max.add(center, dim);
        return *this;
    }

    constexpr inline SelfRef null() noexcept {
        min.set(0, 0, 0);
        max.set(0, 0, 0);
        return *this;
    }
    constexpr inline SelfRef identity() noexcept {
        min.set(-0.5, -0.5, -0.5);
        max.set(0.5, 0.5, 0.5);
        return *this;
    }
    constexpr inline SelfRef invalidate() noexcept {
        min.set(type_max<T>, type_max<T>, type_max<T>);
        max.set(type_min<T>, type_min<T>, type_min<T>);
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
    constexpr inline SelfRef sub(const Tvector& p) noexcept {
        min.sub(p);
        max.sub(p);
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

    [[nodiscard]] constexpr inline bool contains(T x, T y, T z) const noexcept {
        return (x >= x1) && (x <= x2) && (y >= y1) && (y <= y2) && (z >= z1) && (z <= z2);
    }
    [[nodiscard]] constexpr inline bool contains(const Tvector& p) const noexcept { return contains(p.x, p.y, p.z); }
    [[nodiscard]] constexpr inline bool contains(SelfCRef b) const noexcept { return contains(b.min) && contains(b.max); }

    [[nodiscard]] constexpr inline bool similar(SelfCRef b) const noexcept { return min.similar(b.min) && max.similar(b.max); }

    constexpr inline SelfRef modify(const Tvector& p) noexcept {
        min.min(p);
        max.max(p);
        return *this;
    }
    constexpr inline SelfRef modify(T x, T y, T z) noexcept {
        _vector3<T> tmp = { x, y, z };
        return modify(tmp);
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
    
    inline SelfRef xform(SelfCRef B, const Tmatrix& m) noexcept {
        Tvector vx, vy, vz;
        vx.mul(m.i, B.max.x - B.min.x);
        vy.mul(m.j, B.max.y - B.min.y);
        vz.mul(m.k, B.max.z - B.min.z);

        m.transform_tiny(min, B.min);
        max.set(min);

        if (negative(vx.x)) min.x += vx.x; else max.x += vx.x;
        if (negative(vx.y)) min.y += vx.y; else max.y += vx.y;
        if (negative(vx.z)) min.z += vx.z; else max.z += vx.z;
        if (negative(vy.x)) min.x += vy.x; else max.x += vy.x;
        if (negative(vy.y)) min.y += vy.y; else max.y += vy.y;
        if (negative(vy.z)) min.z += vy.z; else max.z += vy.z;
        if (negative(vz.x)) min.x += vz.x; else max.x += vz.x;
        if (negative(vz.y)) min.y += vz.y; else max.y += vz.y;
        if (negative(vz.z)) min.z += vz.z; else max.z += vz.z;
        return *this;
    }
    inline SelfRef xform(const Tmatrix& m) noexcept {
        Self b;
        b.set(*this);
        return xform(b, m);
    }

    constexpr inline void getsize(Tvector& R) const noexcept { R.sub(max, min); }
    inline void getradius(Tvector& R) const noexcept {
        getsize(R);
        R.mul(0.5f);
    }
    inline T getradius() const noexcept {
        Tvector R;
        getradius(R);
        return R.magnitude();
    }
    inline T getvolume() const noexcept {
        Tvector sz;
        getsize(sz);
        return sz.x * sz.y * sz.z;
    }
    constexpr inline SelfCRef getcenter(Tvector& C) const noexcept {
        C.x = (min.x + max.x) * 0.5f;
        C.y = (min.y + max.y) * 0.5f;
        C.z = (min.z + max.z) * 0.5f;
        return *this;
    }
    constexpr inline SelfCRef get_CD(Tvector& bc, Tvector& bd) const noexcept {
        bd.sub(max, min).mul(.5f);
        bc.add(min, bd);
        return *this;
    }
    inline SelfRef scale(float s) noexcept {
        Fvector bd;
        bd.sub(max, min).mul(s);
        grow(bd);
        return *this;
    }
    inline SelfCRef getsphere(Tvector& C, T& R) const noexcept {
        getcenter(C);
        R = C.distance_to(max);
        return *this;
    }

    [[nodiscard]] constexpr inline bool intersect(SelfCRef box) const noexcept {
        if (max.x < box.min.x) return false;
        if (max.y < box.min.y) return false;
        if (max.z < box.min.z) return false;
        if (min.x > box.max.x) return false;
        if (min.y > box.max.y) return false;
        if (min.z > box.max.z) return false;
        return true;
    }

    inline BOOL Pick(const Tvector& start, const Tvector& dir) const noexcept {
        T alpha, xt, yt, zt;
        Tvector rvmin, rvmax;

        rvmin.sub(min, start);
        rvmax.sub(max, start);

        if (!fis_zero(dir.x)) {
            alpha = rvmin.x / dir.x;
            yt = alpha * dir.y;
            if (yt >= rvmin.y && yt <= rvmax.y) {
                zt = alpha * dir.z;
                if (zt >= rvmin.z && zt <= rvmax.z) return true;
            }
            alpha = rvmax.x / dir.x;
            yt = alpha * dir.y;
            if (yt >= rvmin.y && yt <= rvmax.y) {
                zt = alpha * dir.z;
                if (zt >= rvmin.z && zt <= rvmax.z) return true;
            }
        }

        if (!fis_zero(dir.y)) {
            alpha = rvmin.y / dir.y;
            xt = alpha * dir.x;
            if (xt >= rvmin.x && xt <= rvmax.x) {
                zt = alpha * dir.z;
                if (zt >= rvmin.z && zt <= rvmax.z) return true;
            }
            alpha = rvmax.y / dir.y;
            xt = alpha * dir.x;
            if (xt >= rvmin.x && xt <= rvmax.x) {
                zt = alpha * dir.z;
                if (zt >= rvmin.z && zt <= rvmax.z) return true;
            }
        }

        if (!fis_zero(dir.z)) {
            alpha = rvmin.z / dir.z;
            xt = alpha * dir.x;
            if (xt >= rvmin.x && xt <= rvmax.x) {
                yt = alpha * dir.y;
                if (yt >= rvmin.y && yt <= rvmax.y) return true;
            }
            alpha = rvmax.z / dir.z;
            xt = alpha * dir.x;
            if (xt >= rvmin.x && xt <= rvmax.x) {
                yt = alpha * dir.y;
                if (yt >= rvmin.y && yt <= rvmax.y) return true;
            }
        }
        return false;
    }

    // C++17 FIX: Додано const перевантаження для обходу помилки конвертації
    IC u32& IR(T& x) const noexcept { return (u32&)x; }
    IC const u32& IR(const T& x) const noexcept { return (const u32&)x; }

    enum ERP_Result {
        rpNone = 0,
        rpOriginInside = 1,
        rpOriginOutside = 2,
        fcv_forcedword = u32(-1)
    };
    
    inline ERP_Result Pick2(const Tvector& origin, const Tvector& dir, Tvector& coord) const noexcept {
        BOOL Inside = TRUE;
        Tvector MaxT;
        MaxT.x = MaxT.y = MaxT.z = -1.0f;

        {
            if (origin[0] < min[0]) {
                coord[0] = min[0]; Inside = FALSE;
                if (IR(dir[0])) MaxT[0] = (min[0] - origin[0]) / dir[0];
            } else if (origin[0] > max[0]) {
                coord[0] = max[0]; Inside = FALSE;
                if (IR(dir[0])) MaxT[0] = (max[0] - origin[0]) / dir[0];
            }
        }
        {
            if (origin[1] < min[1]) {
                coord[1] = min[1]; Inside = FALSE;
                if (IR(dir[1])) MaxT[1] = (min[1] - origin[1]) / dir[1];
            } else if (origin[1] > max[1]) {
                coord[1] = max[1]; Inside = FALSE;
                if (IR(dir[1])) MaxT[1] = (max[1] - origin[1]) / dir[1];
            }
        }
        {
            if (origin[2] < min[2]) {
                coord[2] = min[2]; Inside = FALSE;
                if (IR(dir[2])) MaxT[2] = (min[2] - origin[2]) / dir[2];
            } else if (origin[2] > max[2]) {
                coord[2] = max[2]; Inside = FALSE;
                if (IR(dir[2])) MaxT[2] = (max[2] - origin[2]) / dir[2];
            }
        }

        if (Inside) {
            coord = origin;
            return rpOriginInside;
        }

        u32 WhichPlane = 0;
        if (MaxT[1] > MaxT[0]) WhichPlane = 1;
        if (MaxT[2] > MaxT[WhichPlane]) WhichPlane = 2;

        if (IR(MaxT[WhichPlane]) & 0x80000000)
            return rpNone;

        if (0 == WhichPlane) {
            coord[1] = origin[1] + MaxT[0] * dir[1];
            if ((coord[1] < min[1]) || (coord[1] > max[1])) return rpNone;
            coord[2] = origin[2] + MaxT[0] * dir[2];
            if ((coord[2] < min[2]) || (coord[2] > max[2])) return rpNone;
            return rpOriginOutside;
        }
        if (1 == WhichPlane) {
            coord[0] = origin[0] + MaxT[1] * dir[0];
            if ((coord[0] < min[0]) || (coord[0] > max[0])) return rpNone;
            coord[2] = origin[2] + MaxT[1] * dir[2];
            if ((coord[2] < min[2]) || (coord[2] > max[2])) return rpNone;
            return rpOriginOutside;
        }
        if (2 == WhichPlane) {
            coord[0] = origin[0] + MaxT[2] * dir[0];
            if ((coord[0] < min[0]) || (coord[0] > max[0])) return rpNone;
            coord[1] = origin[1] + MaxT[2] * dir[1];
            if ((coord[1] < min[1]) || (coord[1] > max[1])) return rpNone;
            return rpOriginOutside;
        }
        return rpNone;
    }

    constexpr inline void getpoint(int index, Tvector& result) const noexcept {
        switch (index) {
        case 0: result.set(min.x, min.y, min.z); break;
        case 1: result.set(min.x, min.y, max.z); break;
        case 2: result.set(max.x, min.y, max.z); break;
        case 3: result.set(max.x, min.y, min.z); break;
        case 4: result.set(min.x, max.y, min.z); break;
        case 5: result.set(min.x, max.y, max.z); break;
        case 6: result.set(max.x, max.y, max.z); break;
        case 7: result.set(max.x, max.y, min.z); break;
        default: result.set(0, 0, 0); break;
        }
    }
    constexpr inline void getpoints(Tvector* result) const noexcept {
        result[0].set(min.x, min.y, min.z);
        result[1].set(min.x, min.y, max.z);
        result[2].set(max.x, min.y, max.z);
        result[3].set(max.x, min.y, min.z);
        result[4].set(min.x, max.y, min.z);
        result[5].set(min.x, max.y, max.z);
        result[6].set(max.x, max.y, max.z);
        result[7].set(max.x, max.y, min.z);
    }

    inline SelfRef modify(SelfCRef src, const Tmatrix& M) noexcept {
        Tvector pt;
        for (int i = 0; i < 8; i++) {
            src.getpoint(i, pt);
            M.transform_tiny(pt);
            modify(pt);
        }
        return *this;
    }
};

typedef _box3<float> Fbox;
typedef _box3<float> Fbox3;
typedef _box3<double> Dbox;
typedef _box3<double> Dbox3;

template <class T>
[[nodiscard]] constexpr inline bool _valid(const _box3<T>& c) noexcept {
    return _valid(c.min) && _valid(c.max);
}

#endif