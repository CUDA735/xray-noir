#include "stdafx.h"
#pragma hdrstop

#include "soundrender_coreA.h"
#include "soundrender_targetA.h"

CSoundRender_CoreA* SoundRenderA = 0;

__declspec(dllexport) u32 snd_device_id = 0;
__declspec(dllexport) xr_token* snd_devices_token = NULL;
__declspec(dllexport) int snd_hrtf = 1;

CSoundRender_CoreA::CSoundRender_CoreA() : CSoundRender_Core() {
    pDevice = 0;
    pContext = 0;
    effect_slot = 0;
    reverb_effect = 0;

    bEFX_Initialized = false;
    fTimeDelta = 0.033f;
    env_density = 0.0f;
    env_room = 0.0f;
    env_room_hf = 0.0f;
    env_decay_time = 0.0f;
    env_decay_hf_ratio = 0.0f;
    env_reflections_delay = 0.0f;
    env_reverb_delay = 0.0f;
    env_room_rolloff_factor = 0.0f;
}

CSoundRender_CoreA::~CSoundRender_CoreA() {}

void CSoundRender_CoreA::_restart() {
    inherited::_restart();
}

void CSoundRender_CoreA::_initialize(int stage) {
    if (stage == 0) {
        if (!snd_devices_token) {
            xr_vector<xr_token> tokens;
            tokens.push_back({ xr_strdup("Default"), 0 });
            
            if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT")) {
                const ALCchar* devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
                int id = 1;
                while (devices && *devices != '\0') {
                    tokens.push_back({ xr_strdup(devices), id++ });
                    devices += xr_strlen(devices) + 1; 
                }
            }
            tokens.push_back({ NULL, -1 }); 
            
            snd_devices_token = xr_alloc<xr_token>(tokens.size());
            for (size_t i = 0; i < tokens.size(); ++i) {
                snd_devices_token[i] = tokens[i];
            }
        }
        return;
    }

    LPCSTR device_to_open = nullptr;
    if (snd_devices_token && snd_device_id != 0) {
        xr_token* tok = snd_devices_token;
        while (tok->name) {
            if (tok->id == (int)snd_device_id) {
                device_to_open = tok->name;
                break;
            }
            tok++;
        }
    }

    pDevice = alcOpenDevice(device_to_open); 
    if (pDevice == NULL) {
        Msg("! [Noir Engine] OpenAL: Failed to open device '%s'. Falling back to default.", device_to_open ? device_to_open : "Default");
        pDevice = alcOpenDevice(nullptr); 
        if (pDevice == NULL) {
            CHECK_OR_EXIT(0, "! [Noir Engine] OpenAL: Failed to create device.");
            bPresent = FALSE;
            return;
        }
    }

    ALCint contextAttr[] = { ALC_HRTF_SOFT, (snd_hrtf ? ALC_TRUE : ALC_FALSE), 0 };
    pContext = alcCreateContext(pDevice, contextAttr);
    if (0 == pContext) {
        pContext = alcCreateContext(pDevice, nullptr); 
        if (0 == pContext) {
            CHECK_OR_EXIT(0, "! [Noir Engine] OpenAL: Failed to create context.");
            bPresent = FALSE;
            alcCloseDevice(pDevice);
            pDevice = 0;
            return;
        }
    }

    alGetError();
    alcGetError(pDevice);
    AC_CHK(alcMakeContextCurrent(pContext));

    ALCint hrtf_state;
    alcGetIntegerv(pDevice, ALC_HRTF_SOFT, 1, &hrtf_state);
    Msg("* [Noir Engine] OpenAL Soft: HRTF is %s", hrtf_state ? "ENABLED" : "DISABLED");
    
    alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
    
    A_CHK(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
    A_CHK(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
    Fvector orient[2] = { { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f } };
    A_CHK(alListenerfv(AL_ORIENTATION, &orient[0].x));
    A_CHK(alListenerf(AL_GAIN, 1.f));

    // ПЕРЕМИКАЧ EFX (Апаратний рівень)
    bEFX = false;
    bEFX_Initialized = false; 
    
    if (alcIsExtensionPresent(pDevice, "ALC_EXT_EFX")) {
        Msg("* [Noir Engine] OpenAL: Hardware supports EFX. Ready for dynamic toggling.");
        bEFX = true;
        
        alGenAuxiliaryEffectSlots(1, &effect_slot);
        alGenEffects(1, &reverb_effect);
        alEffecti(reverb_effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    } else {
        Msg("* [Noir Engine] OpenAL: EFX is NOT supported by hardware.");
    }

    inherited::_initialize(stage);

    if (stage == 1) {
        CSoundRender_Target* T = 0;
        for (u32 tit = 0; tit < u32(psSoundTargets); tit++) {
            T = xr_new<CSoundRender_TargetA>();
            if (T->_initialize()) {
                s_targets.push_back(T);
            } else {
                Log("! [Noir Engine] OpenAL: Max targets - ", tit);
                T->_destroy();
                xr_delete(T);
                break;
            }
        }
    }
}

void CSoundRender_CoreA::set_master_volume(float f) {
    if (bPresent) A_CHK(alListenerf(AL_GAIN, f));
}

void CSoundRender_CoreA::_clear() {
    inherited::_clear();
    CSoundRender_Target* T = 0;
    for (u32 tit = 0; tit < s_targets.size(); tit++) {
        T = s_targets[tit];
        T->_destroy();
        xr_delete(T);
    }

    if (bEFX) {
        alDeleteEffects(1, &reverb_effect);
        alDeleteAuxiliaryEffectSlots(1, &effect_slot);
    }

    alcMakeContextCurrent(NULL);
    if (pContext) alcDestroyContext(pContext);
    pContext = 0;
    if (pDevice) alcCloseDevice(pDevice);
    pDevice = 0;
}

void CSoundRender_CoreA::update_environment(CSound_environment* _E) {
    if (!bEFX) return;
    
    // ДИНАМІЧНЕ ВИМКНЕННЯ ЛУНИ 
    // Якщо гравець вимкнув ефект у меню (ss_EFX дорівнює false)
    if (!psSoundFlags.test(ss_EFX)) {
        alEffectf(reverb_effect, AL_REVERB_GAIN, 0.0f);
        alAuxiliaryEffectSloti(effect_slot, AL_EFFECTSLOT_EFFECT, reverb_effect);
        
        bEFX_Initialized = false; 
        return;
    }

    CSoundRender_Environment* E = static_cast<CSoundRender_Environment*>(_E);
    // Логіка лінійної інтерполяції (Lerp) 
    if (!bEFX_Initialized) {
        env_density             = E->EnvironmentDiffusion;
        env_room                = E->Room;
        env_room_hf             = E->RoomHF;
        env_decay_time          = E->DecayTime;
        env_decay_hf_ratio      = E->DecayHFRatio;
        env_reflections_delay   = E->ReflectionsDelay;
        env_reverb_delay        = E->ReverbDelay;
        env_room_rolloff_factor = E->RoomRolloffFactor;
        bEFX_Initialized = true;
    } else {
        // Швидкість переходу
        float lerp_speed = 1.5f * fTimeDelta; 
        clamp(lerp_speed, 0.0f, 1.0f);

        env_density             += (E->EnvironmentDiffusion - env_density) * lerp_speed;
        env_room                += (E->Room                - env_room) * lerp_speed;
        env_room_hf             += (E->RoomHF              - env_room_hf) * lerp_speed;
        env_decay_time          += (E->DecayTime           - env_decay_time) * lerp_speed;
        env_decay_hf_ratio      += (E->DecayHFRatio        - env_decay_hf_ratio) * lerp_speed;
        env_reflections_delay   += (E->ReflectionsDelay    - env_reflections_delay) * lerp_speed;
        env_reverb_delay        += (E->ReverbDelay         - env_reverb_delay) * lerp_speed;
        env_room_rolloff_factor += (E->RoomRolloffFactor   - env_room_rolloff_factor) * lerp_speed;
    }
    
    // Передаємо ІНТЕРПОЛЬОВАНІ параметри в OpenAL EFX
    float gain   = powf(10.0f, env_room / 2000.0f);
    float gainHF = powf(10.0f, env_room_hf / 2000.0f);
    
    alEffectf(reverb_effect, AL_REVERB_DENSITY,               env_density);
    alEffectf(reverb_effect, AL_REVERB_GAIN,                  gain);
    alEffectf(reverb_effect, AL_REVERB_GAINHF,                gainHF);
    alEffectf(reverb_effect, AL_REVERB_DECAY_TIME,            env_decay_time);
    alEffectf(reverb_effect, AL_REVERB_DECAY_HFRATIO,         env_decay_hf_ratio);
    alEffectf(reverb_effect, AL_REVERB_REFLECTIONS_DELAY,     env_reflections_delay);
    alEffectf(reverb_effect, AL_REVERB_LATE_REVERB_DELAY,     env_reverb_delay);
    alEffectf(reverb_effect, AL_REVERB_ROOM_ROLLOFF_FACTOR,   env_room_rolloff_factor);

    alAuxiliaryEffectSloti(effect_slot, AL_EFFECTSLOT_EFFECT, reverb_effect);
}

void CSoundRender_CoreA::update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) {
    fTimeDelta = dt; 

    inherited::update_listener(P, D, N, dt);

    if (!Listener.position.similar(P)) {
        Listener.position.set(P);
        bListenerMoved = TRUE;
    }
    Listener.orientation[0].set(D.x, D.y, -D.z);
    Listener.orientation[1].set(N.x, N.y, -N.z);

    A_CHK(alListener3f(AL_POSITION, Listener.position.x, Listener.position.y, -Listener.position.z));
    A_CHK(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
    A_CHK(alListenerfv(AL_ORIENTATION, &Listener.orientation[0].x));
}