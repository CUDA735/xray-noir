#ifndef SoundRender_CoreAH
#define SoundRender_CoreAH
#pragma once

#include "SoundRender_Core.h"
#define AL_ALEXT_PROTOTYPES 1
#include <openal/al.h>
#include <openal/alc.h>
#include <openal/efx.h>

#ifndef ALC_HRTF_SOFT
#define ALC_HRTF_SOFT 0x1992
#endif

#ifdef DEBUG
#define A_CHK(expr) { alGetError(); expr; ALenum error = alGetError(); VERIFY2(error == AL_NO_ERROR, (LPCSTR)alGetString(error)); }
#define AC_CHK(expr) { alcGetError(pDevice); expr; ALCenum error = alcGetError(pDevice); VERIFY2(error == ALC_NO_ERROR, (LPCSTR)alcGetString(pDevice, error)); }
#else
#define A_CHK(expr) { expr; }
#define AC_CHK(expr) { expr; }
#endif

class CSoundRender_CoreA : public CSoundRender_Core {
    typedef CSoundRender_Core inherited;
    ALCdevice* pDevice;
    ALCcontext* pContext;

    struct SListener {
        Fvector position;
        Fvector orientation[2];
    };
    SListener Listener;

public:
    // Змінні для слоту реверберації EFX
    ALuint effect_slot;
    ALuint reverb_effect;

    bool  bEFX_Initialized;
    float fTimeDelta;
    
    float env_density;
    float env_room;
    float env_room_hf;
    float env_decay_time;
    float env_decay_hf_ratio;
    float env_reflections_delay;
    float env_reverb_delay;
    float env_room_rolloff_factor;
    // ==============================================================

protected:
    virtual void update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt);
    virtual void update_environment(CSound_environment* E);

public:
    CSoundRender_CoreA();
    virtual ~CSoundRender_CoreA();

    virtual void _initialize(int stage);
    virtual void _clear();
    virtual void _restart();

    virtual void set_master_volume(float f);
    virtual const Fvector& listener_position() { return Listener.position; }
};

extern CSoundRender_CoreA* SoundRenderA;
#endif