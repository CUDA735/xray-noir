#include "stdafx.h"
#include "ISpatial.h"
#pragma warning(push)
#pragma warning(disable : 4995)
#include <xmmintrin.h>
#pragma warning(pop)

// can you say "barebone"?
#ifndef _MM_ALIGN16
#define _MM_ALIGN16 __declspec(align(16))
#endif // _MM_ALIGN16

struct _MM_ALIGN16 vec_t : public Fvector3 {
    float pad;
};

struct _MM_ALIGN16 aabb_t {
    vec_t min;
    vec_t max;
};

struct _MM_ALIGN16 ray_t {
    vec_t pos;
    vec_t inv_dir;
    vec_t fwd_dir;
};

struct ray_segment_t {
    float t_near, t_far;
};

// C++17 FIX: Додано перевантаження для const float& для обходу помилок компіляції
ICF u32& uf(float& x) { return (u32&)x; }
ICF const u32& uf(const float& x) { return (const u32&)x; }

ICF BOOL isect_fpu(const Fvector& min, const Fvector& max, const ray_t& ray, Fvector& coord) {
    Fvector MaxT;
    MaxT.x = MaxT.y = MaxT.z = -1.0f;
    BOOL Inside = TRUE;

    // Find candidate planes.
    if (ray.pos[0] < min[0]) {
        coord[0] = min[0]; Inside = FALSE;
        if (uf(ray.inv_dir[0])) MaxT[0] = (min[0] - ray.pos[0]) * ray.inv_dir[0];
    } else if (ray.pos[0] > max[0]) {
        coord[0] = max[0]; Inside = FALSE;
        if (uf(ray.inv_dir[0])) MaxT[0] = (max[0] - ray.pos[0]) * ray.inv_dir[0];
    }
    
    if (ray.pos[1] < min[1]) {
        coord[1] = min[1]; Inside = FALSE;
        if (uf(ray.inv_dir[1])) MaxT[1] = (min[1] - ray.pos[1]) * ray.inv_dir[1];
    } else if (ray.pos[1] > max[1]) {
        coord[1] = max[1]; Inside = FALSE;
        if (uf(ray.inv_dir[1])) MaxT[1] = (max[1] - ray.pos[1]) * ray.inv_dir[1];
    }
    
    if (ray.pos[2] < min[2]) {
        coord[2] = min[2]; Inside = FALSE;
        if (uf(ray.inv_dir[2])) MaxT[2] = (min[2] - ray.pos[2]) * ray.inv_dir[2];
    } else if (ray.pos[2] > max[2]) {
        coord[2] = max[2]; Inside = FALSE;
        if (uf(ray.inv_dir[2])) MaxT[2] = (max[2] - ray.pos[2]) * ray.inv_dir[2];
    }

    // Ray pos inside bounding box
    if (Inside) {
        coord = ray.pos;
        return true;
    }

    // Get largest of the maxT's for final choice of intersection
    u32 WhichPlane = 0;
    if (MaxT[1] > MaxT[0]) WhichPlane = 1;
    if (MaxT[2] > MaxT[WhichPlane]) WhichPlane = 2;

    // Check final candidate actually inside box (if max < 0)
    if (uf(MaxT[WhichPlane]) & 0x80000000)
        return false;

    if (0 == WhichPlane) { // 1 & 2
        coord[1] = ray.pos[1] + MaxT[0] * ray.fwd_dir[1];
        if ((coord[1] < min[1]) || (coord[1] > max[1])) return false;
        coord[2] = ray.pos[2] + MaxT[0] * ray.fwd_dir[2];
        if ((coord[2] < min[2]) || (coord[2] > max[2])) return false;
        return true;
    }
    if (1 == WhichPlane) { // 0 & 2
        coord[0] = ray.pos[0] + MaxT[1] * ray.fwd_dir[0];
        if ((coord[0] < min[0]) || (coord[0] > max[0])) return false;
        coord[2] = ray.pos[2] + MaxT[1] * ray.fwd_dir[2];
        if ((coord[2] < min[2]) || (coord[2] > max[2])) return false;
        return true;
    }
    if (2 == WhichPlane) { // 0 & 1
        coord[0] = ray.pos[0] + MaxT[2] * ray.fwd_dir[0];
        if ((coord[0] < min[0]) || (coord[0] > max[0])) return false;
        coord[1] = ray.pos[1] + MaxT[2] * ray.fwd_dir[1];
        if ((coord[1] < min[1]) || (coord[1] > max[1])) return false;
        return true;
    }
    return false;
}

// turn those verbose intrinsics into something readable.
#define loadps(mem) _mm_load_ps((const float* const)(mem))
#define storess(ss, mem) _mm_store_ss((float* const)(mem), (ss))
#define minss _mm_min_ss
#define maxss _mm_max_ss
#define minps _mm_min_ps
#define maxps _mm_max_ps
#define mulps _mm_mul_ps
#define subps _mm_sub_ps
#define rotatelps(ps) _mm_shuffle_ps((ps), (ps), 0x39) // a,b,c,d -> b,c,d,a
#define muxhps(low, high) _mm_movehl_ps((low), (high)) // low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}

static const float flt_plus_inf = -logf(0); // let's keep C and C++ compilers happy.
static const float _MM_ALIGN16 ps_cst_plus_inf[4] = { flt_plus_inf, flt_plus_inf, flt_plus_inf, flt_plus_inf },
                               ps_cst_minus_inf[4] = { -flt_plus_inf, -flt_plus_inf, -flt_plus_inf, -flt_plus_inf };

ICF BOOL isect_sse(const aabb_t& box, const ray_t& ray, float& dist) {
    const __m128 plus_inf = loadps(ps_cst_plus_inf), minus_inf = loadps(ps_cst_minus_inf);
    
    const __m128 box_min = loadps(&box.min), box_max = loadps(&box.max), pos = loadps(&ray.pos), inv_dir = loadps(&ray.inv_dir);

    const __m128 l1 = mulps(subps(box_min, pos), inv_dir);
    const __m128 l2 = mulps(subps(box_max, pos), inv_dir);

    const __m128 filtered_l1a = minps(l1, plus_inf);
    const __m128 filtered_l2a = minps(l2, plus_inf);

    const __m128 filtered_l1b = maxps(l1, minus_inf);
    const __m128 filtered_l2b = maxps(l2, minus_inf);

    __m128 lmax = maxps(filtered_l1a, filtered_l2a);
    __m128 lmin = minps(filtered_l1b, filtered_l2b);

    const __m128 lmax0 = rotatelps(lmax);
    const __m128 lmin0 = rotatelps(lmin);
    lmax = minss(lmax, lmax0);
    lmin = maxss(lmin, lmin0);

    const __m128 lmax1 = muxhps(lmax, lmax);
    const __m128 lmin1 = muxhps(lmin, lmin);
    lmax = minss(lmax, lmax1);
    lmin = maxss(lmin, lmin1);

    const BOOL ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax, lmin);

    storess(lmin, &dist);

    return ret;
}

extern Fvector c_spatial_offset[8];

template <bool b_use_sse, bool b_first, bool b_nearest>
class _MM_ALIGN16 walker {
public:
    ray_t ray;
    u32 mask;
    float range;
    float range2;
    ISpatial_DB* space;

public:
    walker(ISpatial_DB* _space, u32 _mask, const Fvector& _start, const Fvector& _dir, float _range) {
        mask = _mask;
        ray.pos.set(_start);
        ray.inv_dir.set(1.f, 1.f, 1.f).div(_dir);
        ray.fwd_dir.set(_dir);
        if (!b_use_sse) {
            if (xr::abs(_dir.x) <= flt_eps) ray.inv_dir.x = 0;
            if (xr::abs(_dir.y) <= flt_eps) ray.inv_dir.y = 0;
            if (xr::abs(_dir.z) <= flt_eps) ray.inv_dir.z = 0;
        }
        range = _range;
        range2 = _range * _range;
        space = _space;
    }
    
    // fpu
    ICF BOOL _box_fpu(const Fvector& n_C, const float n_R, Fvector& coord) {
        float n_vR = 2 * n_R;
        Fbox BB;
        BB.set(n_C.x - n_vR, n_C.y - n_vR, n_C.z - n_vR, n_C.x + n_vR, n_C.y + n_vR, n_C.z + n_vR);
        return isect_fpu(BB.min, BB.max, ray, coord);
    }
    
    // sse
    ICF BOOL _box_sse(const Fvector& n_C, const float n_R, float& dist) {
        aabb_t box;
        __m128 NR = _mm_load_ss((float*)&n_R);
        __m128 NC = _mm_unpacklo_ps(_mm_load_ss((float*)&n_C.x), _mm_load_ss((float*)&n_C.y));
        NR = _mm_add_ss(NR, NR);
        NC = _mm_movelh_ps(NC, _mm_load_ss((float*)&n_C.z));
        NR = _mm_shuffle_ps(NR, NR, _MM_SHUFFLE(1, 0, 0, 0));

        _mm_store_ps((float*)&box.min, _mm_sub_ps(NC, NR));
        _mm_store_ps((float*)&box.max, _mm_add_ps(NC, NR));

        return isect_sse(box, ray, dist);
    }

    void walk(ISpatial_NODE* N, Fvector& n_C, float n_R) {
        // Actual ray/aabb test
        if (b_use_sse) {
            float d;
            if (!_box_sse(n_C, n_R, d)) return;
            if (d > range) return;
        } else {
            Fvector P;
            if (!_box_fpu(n_C, n_R, P)) return;
            if (P.distance_to_sqr(ray.pos) > range2) return;
        }

        // test items
        xr_vector<ISpatial*>::iterator _it = N->items.begin();
        xr_vector<ISpatial*>::iterator _end = N->items.end();
        for (; _it != _end; _it++) {
            ISpatial* S = *_it;
            if (mask != (S->spatial.type & mask)) continue;
            
            Fsphere& sS = S->spatial.sphere;
            int quantity;
            float afT[2];
            Fsphere::ERP_Result result = sS.intersect(ray.pos, ray.fwd_dir, range, quantity, afT);

            if (result == Fsphere::rpOriginInside || ((result == Fsphere::rpOriginOutside) && (afT[0] < range))) {
                if (b_nearest) {
                    switch (result) {
                    case Fsphere::rpOriginInside: range = afT[0] < range ? afT[0] : range; break;
                    case Fsphere::rpOriginOutside: range = afT[0]; break;
                    }
                    range2 = range * range;
                }
                space->q_result->push_back(S);
                if (b_first) return;
            }
        }

        // recurse
        float c_R = n_R / 2;
        for (u32 octant = 0; octant < 8; octant++) {
            if (0 == N->children[octant]) continue;
            Fvector c_C;
            c_C.mad(n_C, c_spatial_offset[octant], c_R);
            walk(N->children[octant], c_C, c_R);
            if (b_first && !space->q_result->empty()) return;
        }
    }
};

void ISpatial_DB::q_ray(xr_vector<ISpatial*>& R, u32 _o, u32 _mask_and, const Fvector& _start, const Fvector& _dir, float _range) {
    std::lock_guard<decltype(cs)> lock(cs);
    q_result = &R;
    q_result->clear();
    
    if (CPU::ID.hasFeature(CpuFeature::Sse)) {
        if (_o & O_ONLYFIRST) {
            if (_o & O_ONLYNEAREST) { walker<true, true, true> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); } 
            else { walker<true, true, false> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); }
        } else {
            if (_o & O_ONLYNEAREST) { walker<true, false, true> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); } 
            else { walker<true, false, false> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); }
        }
    } else {
        if (_o & O_ONLYFIRST) {
            if (_o & O_ONLYNEAREST) { walker<false, true, true> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); } 
            else { walker<false, true, false> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); }
        } else {
            if (_o & O_ONLYNEAREST) { walker<false, false, true> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); } 
            else { walker<false, false, false> W(this, _mask_and, _start, _dir, _range); W.walk(m_root, m_center, m_bounds); }
        }
    }
}