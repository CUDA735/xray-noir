// Rain.h: interface for the CRain class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ThunderboltH
#define ThunderboltH
#pragma once

// refs
class ENGINE_API IRender_DetailModel;
class ENGINE_API CLAItem;

#include "xrRender/FactoryPtr.h"
#include "xrRender/LensFlareRender.h"
#include "xrRender/ThunderboltDescRender.h"
#include "xrRender/ThunderboltRender.h"

#ifdef INGAME_EDITOR
#define INGAME_EDITOR_VIRTUAL virtual
#else 
#define INGAME_EDITOR_VIRTUAL
#endif 

class CEnvironment;

struct SThunderboltDesc {
    FactoryPtr<IThunderboltDescRender> m_pRender;
    ref_sound snd;
    struct SFlare {
        float fOpacity;
        Fvector2 fRadius;
        std::string texture;
        shared_str shader;
        FactoryPtr<IFlareRender> m_pFlare;
        SFlare() {
            fOpacity = 0;
            fRadius.set(0.f, 0.f);
        }
    };
    SFlare* m_GradientTop;
    SFlare* m_GradientCenter;
    shared_str name;
    CLAItem* color_anim;

public:
    SThunderboltDesc();
    INGAME_EDITOR_VIRTUAL ~SThunderboltDesc();
    void load(CInifile& pIni, shared_str const& sect);
    INGAME_EDITOR_VIRTUAL void create_top_gradient(CInifile& pIni, shared_str const& sect);
    INGAME_EDITOR_VIRTUAL void create_center_gradient(CInifile& pIni, shared_str const& sect);
};

#undef INGAME_EDITOR_VIRTUAL

struct SThunderboltCollection {
    using DescVec = xr_vector<SThunderboltDesc*>;
    DescVec palette;
    std::string section;

public:
    SThunderboltCollection();
    ~SThunderboltCollection();
    void load(CInifile* pIni, CInifile* thunderbolts, LPCSTR sect);
    SThunderboltDesc* GetRandomDesc() {
        VERIFY(!palette.empty());
        return palette[Random.randI(palette.size())];
    }
};

#define THUNDERBOLT_CACHE_SIZE 8
//
class ENGINE_API CEffect_Thunderbolt {
    friend class dxThunderboltRender;

protected:
    using CollectionVec = xr_vector<SThunderboltCollection*>;
    CollectionVec collection;
    SThunderboltDesc* current;

private:
    Fmatrix current_xform;
    Fvector3 current_direction;

    FactoryPtr<IThunderboltRender> m_pRender;
    
    enum EState { stIdle, stWorking };
    EState state;

    Fvector lightning_center;
    float lightning_size;
    float lightning_phase;

    float life_time;
    float current_time;
    float next_lightning_time;
    BOOL bEnabled;

    // === ПАРАМЕТРИ NOIR ENGINE ===
    bool m_bEnableCustomLightning;
    float m_fHeightFactor;
    float m_fSizeMultiplier;
    int m_iProbNear;
    int m_iProbMedium;
    float m_fDistNearMin, m_fDistNearMax;
    float m_fDistMediumMin, m_fDistMediumMax;
    float m_fDistFarMin, m_fDistFarMax;
    // =============================

private:
    static BOOL RayPick(const Fvector& s, const Fvector& d, float& range);
    void Bolt(const std::string& id, const float period, const float life_time);

public:
    CEffect_Thunderbolt();
    ~CEffect_Thunderbolt();

    void OnFrame(const std::string& id, const float period, const float duration);
    void Render();

    std::string AppendDef(CEnvironment& environment, CInifile* pIni, CInifile* thunderbolts, LPCSTR sect);
    void ForceStrike(LPCSTR id, const Fvector& target_pos);
};

#endif // ThunderboltH