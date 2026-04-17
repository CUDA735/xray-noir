#include "stdafx.h"
#pragma once

#ifndef _EDITOR
#include "render.h"
#endif
#include "Thunderbolt.h"
#include "igame_persistent.h"
#include "LightAnimLibrary.h"

#ifdef _EDITOR
#include "ui_toolscustom.h"
#else
#include "igame_level.h"
#include "../xrcdb/xr_area.h"
#include "xr_object.h"
#endif

SThunderboltDesc::SThunderboltDesc() : m_GradientTop(0), m_GradientCenter(0) {}

SThunderboltDesc::~SThunderboltDesc() {
    m_pRender->DestroyModel();
    m_GradientTop->m_pFlare->DestroyShader();
    m_GradientCenter->m_pFlare->DestroyShader();
    snd.destroy();

    xr_delete(m_GradientTop);
    xr_delete(m_GradientCenter);
}

void SThunderboltDesc::create_top_gradient(CInifile& pIni, shared_str const& sect) {
    m_GradientTop = xr_new<SFlare>();
    m_GradientTop->shader = pIni.r_string(sect, "gradient_top_shader");
    m_GradientTop->texture = pIni.r_string(sect, "gradient_top_texture");
    m_GradientTop->fRadius = pIni.r_fvector2(sect, "gradient_top_radius");
    m_GradientTop->fOpacity = pIni.r_float(sect, "gradient_top_opacity");
    m_GradientTop->m_pFlare->CreateShader(*m_GradientTop->shader, m_GradientTop->texture.c_str());
}

void SThunderboltDesc::create_center_gradient(CInifile& pIni, shared_str const& sect) {
    m_GradientCenter = xr_new<SFlare>();
    m_GradientCenter->shader = pIni.r_string(sect, "gradient_center_shader");
    m_GradientCenter->texture = pIni.r_string(sect, "gradient_center_texture");
    m_GradientCenter->fRadius = pIni.r_fvector2(sect, "gradient_center_radius");
    m_GradientCenter->fOpacity = pIni.r_float(sect, "gradient_center_opacity");
    m_GradientCenter->m_pFlare->CreateShader(*m_GradientCenter->shader, m_GradientCenter->texture.c_str());
}

void SThunderboltDesc::load(CInifile& pIni, shared_str const& sect) {
    create_top_gradient(pIni, sect);
    create_center_gradient(pIni, sect);

    name = sect;
    color_anim = LALib.FindItem(pIni.r_string(sect, "color_anim"));
    VERIFY(color_anim);
    color_anim->fFPS = (float)color_anim->iFrameCount;

    // models
    LPCSTR m_name;
    m_name = pIni.r_string(sect, "lightning_model");
    m_pRender->CreateModel(m_name);

    // sound
    m_name = pIni.r_string(sect, "sound");
    if (m_name && m_name[0])
        snd.create(m_name, st_Effect, sg_Undefined);
}

//----------------------------------------------------------------------------------------------
// collection
//----------------------------------------------------------------------------------------------
SThunderboltCollection::SThunderboltCollection() {}

void SThunderboltCollection::load(CInifile* pIni, CInifile* thunderbolts, LPCSTR sect) {
    section = sect;
    const int tb_count = pIni->line_count(sect);
    for (int tb_idx = 0; tb_idx < tb_count; tb_idx++) {
        LPCSTR N, V;
        if (pIni->r_line(sect, tb_idx, &N, &V))
            palette.push_back(
                g_pGamePersistent->Environment().thunderbolt_description(*thunderbolts, N));
    }
}
SThunderboltCollection::~SThunderboltCollection() {
    for (auto d_it = palette.begin(); d_it != palette.end(); d_it++)
        xr_delete(*d_it);

    palette.clear();
}

//----------------------------------------------------------------------------------------------
// thunderbolt effect
//----------------------------------------------------------------------------------------------
CEffect_Thunderbolt::CEffect_Thunderbolt() {
    current = 0;
    life_time = 0.f;
    state = stIdle;
    next_lightning_time = 0.f;
    bEnabled = FALSE;

    m_bEnableCustomLightning = false; // Ваніль за замовчуванням
    
    m_fHeightFactor = 0.8f;
    m_fSizeMultiplier = 2.5f;
    m_iProbNear = 20; 
    m_iProbMedium = 40;
    m_fDistNearMin = 0.f; 
    m_fDistNearMax = 5.f;
    m_fDistMediumMin = 10.f; 
    m_fDistMediumMax = 40.f;
    m_fDistFarMin = 40.f; 
    m_fDistFarMax = 150.f;

    string_path ext_path;
    if (FS.exist(ext_path, "$game_config$", "noirEngineExtention.ltx")) {
        CInifile ext_ini(ext_path);
        if (ext_ini.section_exist("environment") && ext_ini.line_exist("environment", "enable_custom_lightning")) {
            m_bEnableCustomLightning = ext_ini.r_bool("environment", "enable_custom_lightning");
        }
    }

    if (m_bEnableCustomLightning) {
        string_path wex_path;
        if (FS.exist(wex_path, "$game_config$", "environment\\noirWeatherEffect.ltx")) {
            CInifile wex_ini(wex_path);
            if (wex_ini.section_exist("custom_lightning")) {
                m_fHeightFactor = wex_ini.r_float("custom_lightning", "height_factor");
                m_fSizeMultiplier = wex_ini.r_float("custom_lightning", "size_multiplier");
                m_iProbNear = wex_ini.r_s32("custom_lightning", "prob_near");
                m_iProbMedium = wex_ini.r_s32("custom_lightning", "prob_medium");
                m_fDistNearMin = wex_ini.r_float("custom_lightning", "dist_near_min");
                m_fDistNearMax = wex_ini.r_float("custom_lightning", "dist_near_max");
                m_fDistMediumMin = wex_ini.r_float("custom_lightning", "dist_medium_min");
                m_fDistMediumMax = wex_ini.r_float("custom_lightning", "dist_medium_max");
                m_fDistFarMin = wex_ini.r_float("custom_lightning", "dist_far_min");
                m_fDistFarMax = wex_ini.r_float("custom_lightning", "dist_far_max");
            } else {
                m_bEnableCustomLightning = false;
            }
        } else {
            m_bEnableCustomLightning = false;
        }
    }
}

CEffect_Thunderbolt::~CEffect_Thunderbolt() {
    for (auto d_it = collection.begin(); d_it != collection.end(); d_it++)
        xr_delete(*d_it);
    collection.clear();
}

std::string CEffect_Thunderbolt::AppendDef(CEnvironment& environment, CInifile* pIni,
                                           CInifile* thunderbolts, LPCSTR sect) {
    if (!sect || (0 == sect[0]))
        return "";
    for (const auto* item : collection) {
        if (item->section == sect)
            return item->section;
    }
    collection.push_back(environment.thunderbolt_collection(pIni, thunderbolts, sect));
    return collection.back()->section;
}

BOOL CEffect_Thunderbolt::RayPick(const Fvector& s, const Fvector& d, float& dist) {
    BOOL bRes = TRUE;
#ifdef _EDITOR
    bRes = Tools->RayPick(s, d, dist, 0, 0);
#else
    collide::rq_result RQ;
    CObject* E = g_pGameLevel->CurrentViewEntity();
    bRes = g_pGameLevel->ObjectSpace.RayPick(s, d, dist, collide::rqtBoth, RQ, E);
    if (bRes)
        dist = RQ.range;
    else {
        Fvector N = { 0.f, -1.f, 0.f };
        Fvector P = { 0.f, 0.f, 0.f };
        Fplane PL;
        PL.build(P, N);
        float dst = dist;
        if (PL.intersectRayDist(s, d, dst) && (dst <= dist)) {
            dist = dst;
            return true;
        } else
            return false;
    }
#endif
    return bRes;
}
#define FAR_DIST g_pGamePersistent->Environment().CurrentEnv->far_plane

void CEffect_Thunderbolt::Bolt(const std::string& id, const float period, const float lt) {
    VERIFY(!id.empty());
    state = stWorking;
    life_time = lt + Random.randF(-lt * 0.5f, lt * 0.5f);
    current_time = 0.f;

    current = g_pGamePersistent->Environment().thunderbolt_collection(collection, id.c_str())->GetRandomDesc();
    VERIFY(current);

    Fmatrix XF, S;
    Fvector pos, dev;
    float sun_h, sun_p;
    CEnvironment& environment = g_pGamePersistent->Environment();
    environment.CurrentEnv->sun_dir.getHP(sun_h, sun_p);

    if (m_bEnableCustomLightning) {
        // ========================================================
        // NOIR ENGINE CUSTOM LIGHTNING
        // ========================================================
        float height = FAR_DIST * m_fHeightFactor; 
        float R = 0.f;
        int strike_type = Random.randI(0, 100);

        if (strike_type < m_iProbNear) {
            R = Random.randF(m_fDistNearMin, m_fDistNearMax);
        } else if (strike_type < (m_iProbNear + m_iProbMedium)) {
            R = Random.randF(m_fDistMediumMin, m_fDistMediumMax);
        } else {
            R = Random.randF(m_fDistFarMin, m_fDistFarMax);
        }

        float alt = atan2(height, R); 
        float dist = sqrt(height * height + R * R);
        float lng = Random.randF(0.f, PI_MUL_2); 

        current_direction.setHP(lng, alt);
        pos.mad(Device.vCameraPosition, current_direction, dist);
        dev.x = Random.randF(-environment.p_tilt, environment.p_tilt);
        dev.y = Random.randF(0, PI_MUL_2);
        dev.z = Random.randF(-environment.p_tilt, environment.p_tilt);
        XF.setXYZi(dev);

        Fvector light_dir = { 0.f, -1.f, 0.f };
        XF.transform_dir(light_dir);
        
        lightning_size = dist * m_fSizeMultiplier; 
        if (lightning_size < 50.f) lightning_size = 50.f; 

        RayPick(pos, light_dir, lightning_size);

        lightning_center.mad(pos, light_dir, lightning_size * 0.5f);

        S.scale(lightning_size, lightning_size, lightning_size);
        XF.translate_over(pos);
        current_xform.mul_43(XF, S);

        float next_v = Random.randF();

        if (next_v < environment.p_second_prop) {
            next_lightning_time = Device.fTimeGlobal + lt + EPS_L;
        } else {
            next_lightning_time = Device.fTimeGlobal + period + Random.randF(-period * 0.3f, period * 0.3f);
            current->snd.play_no_feedback(0, 0, dist / 300.f, &pos, 0, 0, &Fvector2().set(dist / 2, dist * 2.f));
        }

        current_direction.invert(); 
    } else {
        // ========================================================
        // VANILLA
        // ========================================================
        float alt = environment.p_var_alt; 
        float lng = Random.randF(sun_h - environment.p_var_long + PI, sun_h + environment.p_var_long + PI);
        float dist = Random.randF(FAR_DIST * environment.p_min_dist, FAR_DIST * .95f);
        current_direction.setHP(lng, alt);
        pos.mad(Device.vCameraPosition, current_direction, dist);
        dev.x = Random.randF(-environment.p_tilt, environment.p_tilt);
        dev.y = Random.randF(0, PI_MUL_2);
        dev.z = Random.randF(-environment.p_tilt, environment.p_tilt);
        XF.setXYZi(dev);

        Fvector light_dir = { 0.f, -1.f, 0.f };
        XF.transform_dir(light_dir);
        lightning_size = FAR_DIST * 2.f;
        RayPick(pos, light_dir, lightning_size);

        lightning_center.mad(pos, light_dir, lightning_size * 0.5f);

        S.scale(lightning_size, lightning_size, lightning_size);
        XF.translate_over(pos);
        current_xform.mul_43(XF, S);

        float next_v = Random.randF();

        if (next_v < environment.p_second_prop) {
            next_lightning_time = Device.fTimeGlobal + lt + EPS_L;
        } else {
            next_lightning_time = Device.fTimeGlobal + period + Random.randF(-period * 0.3f, period * 0.3f);
            current->snd.play_no_feedback(0, 0, dist / 300.f, &pos, 0, 0, &Fvector2().set(dist / 2, dist * 2.f));
        }

        current_direction.invert(); 
    }
}

void CEffect_Thunderbolt::OnFrame(const std::string& id, const float period, const float duration) {
    const bool enabled = !id.empty();
    if (bEnabled != enabled) {
        bEnabled = enabled;
        next_lightning_time =
            Device.fTimeGlobal + period + Random.randF(-period * 0.5f, period * 0.5f);
    } else if (bEnabled && (Device.fTimeGlobal > next_lightning_time)) {
        if (state == stIdle && !!(id.size()))
            Bolt(id, period, duration);
    }
    if (state == stWorking) {
        if (current_time > life_time)
            state = stIdle;
        current_time += Device.fTimeDelta;
        Fvector fClr;
        int frame;
        const u32 uClr = current->color_anim->CalculateRGB(current_time / life_time, frame);
        fClr.set(clampr(float(color_get_R(uClr) / 255.f), 0.f, 1.f),
                 clampr(float(color_get_G(uClr) / 255.f), 0.f, 1.f),
                 clampr(float(color_get_B(uClr) / 255.f), 0.f, 1.f));

        lightning_phase = 1.5f * (current_time / life_time);
        clamp(lightning_phase, 0.f, 1.f);

        CEnvironment& environment = g_pGamePersistent->Environment();

        Fvector& sky_color = environment.CurrentEnv->sky_color;
        sky_color.mad(fClr, environment.p_sky_color);
        clamp(sky_color.x, 0.f, 1.f);
        clamp(sky_color.y, 0.f, 1.f);
        clamp(sky_color.z, 0.f, 1.f);

        environment.CurrentEnv->sun_color.mad(fClr, environment.p_sun_color);
        environment.CurrentEnv->fog_color.mad(fClr, environment.p_fog_color);

        if (::Render->get_generation() == IRender_interface::GENERATION_R2) {
            R_ASSERT(xr::valid(current_direction));
            g_pGamePersistent->Environment().CurrentEnv->sun_dir = current_direction;
            VERIFY2(g_pGamePersistent->Environment().CurrentEnv->sun_dir.y < 0,
                    "Invalid sun direction settings while CEffect_Thunderbolt");
        }
    }
}

void CEffect_Thunderbolt::Render() {
    if (state == stWorking) {
        m_pRender->Render(*this);
	}
}

// --- SCRIPT‑DRIVEN LIGHTNING STRIKE ---
void CEffect_Thunderbolt::ForceStrike(LPCSTR id, const Fvector& target_pos) {
    if (!m_bEnableCustomLightning || !id || !id[0]) return;
    
    state = stWorking;
    life_time = 2.0f + Random.randF(-0.5f, 0.5f); 
    current_time = 0.f;

    current = g_pGamePersistent->Environment().thunderbolt_collection(collection, id)->GetRandomDesc();
    if (!current) return;

    Fmatrix XF, S;
    float height = FAR_DIST * m_fHeightFactor;
    Fvector pos = target_pos;
    pos.y += height; 

    Fvector light_dir = { 0.f, -1.f, 0.f };
    lightning_size = height + 10.f; 
    RayPick(pos, light_dir, lightning_size);

    lightning_center.mad(pos, light_dir, lightning_size * 0.5f);

    S.scale(lightning_size, lightning_size, lightning_size);
    XF.setXYZi(Random.randF(-0.05f, 0.05f), Random.randF(0, PI_MUL_2), Random.randF(-0.05f, 0.05f));
    XF.translate_over(pos);
    current_xform.mul_43(XF, S);

    Fvector snd_pos = target_pos; 
    current->snd.play_no_feedback(0, 0, Device.vCameraPosition.distance_to(target_pos) / 300.f, &snd_pos, 0, 0, &Fvector2().set(10.f, 200.f));

    current_direction.sub(target_pos, Device.vCameraPosition).normalize_safe();
}