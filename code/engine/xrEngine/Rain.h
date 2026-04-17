// Rain.h: interface for the CRain class.
//
//////////////////////////////////////////////////////////////////////

#ifndef RainH
#define RainH
#pragma once

#include "../xrcdb/xr_collide_defs.h"
#include "GameMtlLib.h" // Бібліотека матеріалів

// refs
class ENGINE_API IRender_DetailModel;

#include "xrRender/FactoryPtr.h"
#include "xrRender/RainRender.h"
//
class ENGINE_API CEffect_Rain {
    friend class dxRainRender;

private:
    struct Item {
        Fvector P;
        Fvector Phit;
        Fvector D;
        float fSpeed;
        u32 dwTime_Life;
        u32 dwTime_Hit;
        u32 uv_set;
        u16 material_idx;
        void invalidate() { dwTime_Life = 0; }
    };
    struct Particle {
        Particle *next, *prev;
        Fmatrix mXForm;
        Fsphere bounds;
        float time;
    };
    enum States { stIdle = 0, stWorking };

    // === NOIR ENGINE DYNAMIC RAIN SYSTEM ===
	// Material-based raindrop impact sound descriptor
    struct RainHitSound {
        shared_str keyword; 
        ref_sound snd;      
		u32 last_play_time;
    };
    xr_vector<RainHitSound> m_HitSounds; 
    
    // Module feature flags
    bool m_bEnableMaterialSounds;        
    bool m_bEnableDynamicWind;

    // Wind simulation parameters
    float m_fWindMaxAngle;
    float m_fWindSpeedMultiplier;
    // ==============================================

private:
    FactoryPtr<IRainRender> m_pRender;

    xr_vector<Item> items;
    States state;

    xr_vector<Particle> particle_pool;
    Particle* particle_active;
    Particle* particle_idle;

    ref_sound snd_Ambient;

    void p_create();
    void p_destroy();

    void p_remove(Particle* P, Particle*& LST);
    void p_insert(Particle* P, Particle*& LST);
    int p_size(Particle* LST);
    Particle* p_allocate();
    void p_free(Particle* P);

    void Born(Item& dest, float radius);
    void Hit(Fvector& pos, u16 material_idx); 
    BOOL RayPick(const Fvector& s, const Fvector& d, float& range, collide::rq_target tgt, u16& material_idx); 
    void RenewItem(Item& dest, float height, BOOL bHit, u16 material_idx); 

public:
    CEffect_Rain();
    ~CEffect_Rain();

    void Render();
    void OnFrame();
};

#endif // RainH