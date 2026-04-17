#include "stdafx.h"
#pragma once

#include "Rain.h"
#include "igame_persistent.h"
#include "environment.h"

#ifdef _EDITOR
#include "ui_toolscustom.h"
#else
#include "render.h"
#include "igame_level.h"
#include "../xrcdb/xr_area.h"
#include "xr_object.h"
#endif

// Warning: duplicated in dxRainRender
static const int max_desired_items = 2500;
static const float source_radius = 12.5f;
static const float source_offset = 40.f;
static const float max_distance = source_offset * 1.25f;
static const float sink_offset = -(max_distance - source_offset);
static const float drop_length = 5.f;
static const float drop_width = 0.30f;
static const float drop_angle = 3.0f;
static const float drop_max_wind_vel = 20.0f;
static const float drop_speed_min = 40.f;
static const float drop_speed_max = 80.f;

const int max_particles = 1000;
const int particles_cache = 400;
const float particles_time = .3f;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEffect_Rain::CEffect_Rain() {
    state = stIdle;

    snd_Ambient.create("ambient\\rain", st_Effect, sg_Undefined);
    
    // === NOIR ENGINE MODULE INITIALIZATION ===
    m_bEnableMaterialSounds = false; 
    m_bEnableDynamicWind = false;
    m_fWindMaxAngle = 10.0f;         
    m_fWindSpeedMultiplier = 0.0f;   

    string_path ext_path;
    if (FS.exist(ext_path, "$game_config$", "noirEngineExtention.ltx")) {
        CInifile ext_ini(ext_path);
        if (ext_ini.section_exist("environment")) {
            if (ext_ini.line_exist("environment", "enable_rain_material_sounds"))
                m_bEnableMaterialSounds = ext_ini.r_bool("environment", "enable_rain_material_sounds");
            
            if (ext_ini.line_exist("environment", "enable_dynamic_rain_wind"))
                m_bEnableDynamicWind = ext_ini.r_bool("environment", "enable_dynamic_rain_wind");
        }
    }

    if (m_bEnableMaterialSounds || m_bEnableDynamicWind) {
        string_path wex_path;
        if (FS.exist(wex_path, "$game_config$", "environment\\noirWeatherEffect.ltx")) {
            CInifile wex_ini(wex_path);
            
            if (m_bEnableMaterialSounds && wex_ini.section_exist("rainmaterial")) {
                u32 line_count = wex_ini.line_count("rainmaterial");
                for (u32 i = 0; i < line_count; ++i) {
                    LPCSTR key, value;
                    wex_ini.r_line("rainmaterial", i, &key, &value);
                    
                    if (key && value && xr_strlen(value) > 0) {
                        RainHitSound rhs;
                        rhs.keyword = key;
                        rhs.snd.create(value, st_Effect, sg_Undefined);
                        float snd_length = rhs.snd.get_length_sec();
                        if (snd_length > 0.5f) { 
                            Msg("! [Noir Engine] ERROR: Rain sound '%s' is too long (%.2f sec). Max allowed is 0.5s! Sound disabled.", value, snd_length);
                            rhs.snd.destroy(); 
                        } else {
                            rhs.last_play_time = 0; 
                            m_HitSounds.push_back(rhs);
                        }
                    }
                }
            } else if (m_bEnableMaterialSounds) {
                m_bEnableMaterialSounds = false; 
            }

            if (m_bEnableDynamicWind && wex_ini.section_exist("dynamic_wind")) {
                m_fWindMaxAngle = wex_ini.r_float("dynamic_wind", "max_angle");
                m_fWindSpeedMultiplier = wex_ini.r_float("dynamic_wind", "speed_wind_multiplier");
            } else if (m_bEnableDynamicWind) {
                Msg("! [Rain] Section [dynamic_wind] not found, using vanilla wind physics.");
                m_bEnableDynamicWind = false;
            }
        } else {
            Msg("! [Rain] File noirWeatherEffect.ltx not found, disabling rain extensions.");
            m_bEnableMaterialSounds = false;
            m_bEnableDynamicWind = false;
        }
    }
    // ==========================================

    p_create();
}

CEffect_Rain::~CEffect_Rain() {
    snd_Ambient.destroy();
    for (auto& rhs : m_HitSounds) {
        rhs.snd.destroy();
    }
    m_HitSounds.clear();
    p_destroy();
}

void CEffect_Rain::Born(Item& dest, float radius) {
    Fvector axis;
    axis.set(0, -1, 0);
    float gust = g_pGamePersistent->Environment().wind_strength_factor / 10.f;
    float k = g_pGamePersistent->Environment().CurrentEnv->wind_velocity * gust / drop_max_wind_vel;
    clamp(k, 0.f, 1.f);
    
    float pitch = deg2rad(m_fWindMaxAngle) * k - PI_DIV_2;
    axis.setHP(g_pGamePersistent->Environment().CurrentEnv->wind_direction, pitch);

    Fvector& view = Device.vCameraPosition;
    float angle = ::Random.randF(0, PI_MUL_2);
    float dist = ::Random.randF();
    dist = std::sqrt(dist) * radius;
    float x = dist * std::cos(angle);
    float z = dist * std::sin(angle);
    dest.D.random_dir(axis, deg2rad(drop_angle));
    dest.P.set(x + view.x - dest.D.x * source_offset, source_offset + view.y,
               z + view.z - dest.D.z * source_offset);
               
    float base_speed = ::Random.randF(drop_speed_min, drop_speed_max);
    dest.fSpeed = base_speed + (base_speed * k * m_fWindSpeedMultiplier);

    float height = max_distance;
    u16 mat_idx = GAMEMTL_NONE_IDX; 
    
    BOOL bHit = RayPick(dest.P, dest.D, height, collide::rqtBoth, mat_idx);
    RenewItem(dest, height, bHit, mat_idx);
}

BOOL CEffect_Rain::RayPick(const Fvector& s, const Fvector& d, float& range, collide::rq_target tgt, u16& material_idx) {
    BOOL bRes = TRUE;
    material_idx = GAMEMTL_NONE_IDX;
#ifdef _EDITOR
    Tools->RayPick(s, d, range);
#else
    collide::rq_result RQ;
    CObject* E = g_pGameLevel->CurrentViewEntity();
    bRes = g_pGameLevel->ObjectSpace.RayPick(s, d, range, tgt, RQ, E);
    if (bRes) {
        range = RQ.range;
        if (!RQ.O) {
            CDB::TRI* T = g_pGameLevel->ObjectSpace.GetStaticTris() + RQ.element;
            material_idx = T->material;
        }
    }
#endif
    return bRes;
}

void CEffect_Rain::RenewItem(Item& dest, float height, BOOL bHit, u16 material_idx) {
    dest.uv_set = Random.randI(2);
    dest.material_idx = material_idx; 
    if (bHit) {
        dest.dwTime_Life = Device.dwTimeGlobal + iFloor(1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.dwTime_Hit = Device.dwTimeGlobal + iFloor(1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.Phit.mad(dest.P, dest.D, height);
    } else {
        dest.dwTime_Life = Device.dwTimeGlobal + iFloor(1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.dwTime_Hit = Device.dwTimeGlobal + iFloor(2 * 1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.Phit.set(dest.P);
    }
}

void CEffect_Rain::OnFrame() {
#ifndef _EDITOR
    if (!g_pGameLevel) return;
#endif

    float factor = g_pGamePersistent->Environment().CurrentEnv->rain_density;
    static float hemi_factor = 0.f;
#ifndef _EDITOR
    CObject* E = g_pGameLevel->CurrentViewEntity();
    if (E && E->renderable_ROS()) {
        float* hemi_cube = E->renderable_ROS()->get_luminocity_hemi_cube();
        float hemi_val = std::max(hemi_cube[0], hemi_cube[1]);
        hemi_val = std::max(hemi_val, hemi_cube[2]);
        hemi_val = std::max(hemi_val, hemi_cube[3]);
        hemi_val = std::max(hemi_val, hemi_cube[5]);

        float f = hemi_val;
        float t = Device.fTimeDelta;
        clamp(t, 0.001f, 1.0f);
        hemi_factor = hemi_factor * (1.0f - t) + f * t;
    }
#endif

    switch (state) {
    case stIdle:
        if (factor < EPS_L) return;
        state = stWorking;
        snd_Ambient.play(0, sm_Looped);
        snd_Ambient.set_position(Fvector().set(0, 0, 0));
        snd_Ambient.set_range(source_offset, source_offset * 2.f);
        break;
    case stWorking:
        if (factor < EPS_L) {
            state = stIdle;
            snd_Ambient.stop();
            return;
        }
        break;
    }

    if (snd_Ambient._feedback()) {
        snd_Ambient.set_volume(std::max(0.1f, factor) * hemi_factor);
    }
}

void CEffect_Rain::Render() {
#ifndef _EDITOR
    if (!g_pGameLevel) return;
#endif
    m_pRender->Render(*this);
}

void CEffect_Rain::Hit(Fvector& pos, u16 material_idx) {
    if (m_bEnableMaterialSounds) {
        float dist_sq = Device.vCameraPosition.distance_to_sqr(pos);
        if (dist_sq < 625.f) { 
            if (material_idx != GAMEMTL_NONE_IDX) {
                SGameMtl* mtl = GMLib.GetMaterialByIdx(material_idx);
                if (mtl) {
                    LPCSTR mtl_name = mtl->m_Name.c_str(); 
                    
                    LPCSTR short_name = strrchr(mtl_name, '\\');
                    if (short_name) {
                        short_name++; 
                    } else {
                        short_name = strrchr(mtl_name, '/');
                        if (short_name) short_name++;
                        else short_name = mtl_name; 
                    }

                    for (auto& rhs : m_HitSounds) { 
                        if (strcmp(short_name, rhs.keyword.c_str()) == 0) { 
                            if (Device.dwTimeGlobal > rhs.last_play_time + 50) {
                                rhs.snd.play_no_feedback(0, 0, 0, &pos, 0, 0, &Fvector2().set(5.f, 25.f));
                                rhs.last_play_time = Device.dwTimeGlobal; 
                            }
                            break; 
                        }
                    }
                }
            }
        }
    }

    if (0 != ::Random.randI(2)) return;
    Particle* P = p_allocate();
    if (0 == P) return;

    const Fsphere& bv_sphere = m_pRender->GetDropBounds();

    P->time = particles_time;
    P->mXForm.rotateY(::Random.randF(PI_MUL_2));
    P->mXForm.translate_over(pos);
    P->mXForm.transform_tiny(P->bounds.P, bv_sphere.P);
    P->bounds.R = bv_sphere.R;
}

void CEffect_Rain::p_create() {
    particle_pool.resize(max_particles);
    for (u32 it = 0; it < particle_pool.size(); it++) {
        Particle& P = particle_pool[it];
        P.prev = it ? (&particle_pool[it - 1]) : 0;
        P.next = (it < (particle_pool.size() - 1)) ? (&particle_pool[it + 1]) : 0;
    }
    particle_active = 0;
    particle_idle = &particle_pool.front();
}

void CEffect_Rain::p_destroy() {
    particle_active = 0;
    particle_idle = 0;
    particle_pool.clear();
}

void CEffect_Rain::p_remove(Particle* P, Particle*& LST) {
    VERIFY(P);
    Particle* prev = P->prev;
    P->prev = NULL;
    Particle* next = P->next;
    P->next = NULL;
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    if (LST == P) LST = next;
}

void CEffect_Rain::p_insert(Particle* P, Particle*& LST) {
    VERIFY(P);
    P->prev = 0;
    P->next = LST;
    if (LST) LST->prev = P;
    LST = P;
}

int CEffect_Rain::p_size(Particle* P) {
    if (0 == P) return 0;
    int cnt = 0;
    while (P) { P = P->next; cnt += 1; }
    return cnt;
}

CEffect_Rain::Particle* CEffect_Rain::p_allocate() {
    Particle* P = particle_idle;
    if (0 == P) return NULL;
    p_remove(P, particle_idle);
    p_insert(P, particle_active);
    return P;
}

void CEffect_Rain::p_free(Particle* P) {
    p_remove(P, particle_active);
    p_insert(P, particle_idle);
}