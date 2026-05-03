#include "stdafx.h"
#pragma hdrstop

#include "soundrender_coreA.h"
#include "soundrender_targetA.h"

CSoundRender_CoreA* SoundRenderA = 0;

CSoundRender_CoreA::CSoundRender_CoreA() : CSoundRender_Core() {
    pDevice = 0;
    pContext = 0;
    effect_slot = 0;
    reverb_effect = 0;

    // === ДОДАНО: Ініціалізація EFX буферів ===
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
    // =========================================
}

CSoundRender_CoreA::~CSoundRender_CoreA() {}

void CSoundRender_CoreA::_restart() {
    inherited::_restart();
}

void CSoundRender_CoreA::_initialize(int stage) {
    if (stage == 0) return;

    pDevice = alcOpenDevice(nullptr); // Дефолтний пристрій ОС
    if (pDevice == NULL) {
        CHECK_OR_EXIT(0, "SOUND: OpenAL: Failed to create device.");
        bPresent = FALSE;
        return;
    }

    // Запит на активацію HRTF
    ALCint contextAttr[] = { ALC_HRTF_SOFT, ALC_TRUE, 0 };
    pContext = alcCreateContext(pDevice, contextAttr);
    if (0 == pContext) {
        pContext = alcCreateContext(pDevice, nullptr); // Фолбек
        if (0 == pContext) {
            CHECK_OR_EXIT(0, "SOUND: OpenAL: Failed to create context.");
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
    Msg("* OpenAL Soft: HRTF is %s", hrtf_state ? "ENABLED" : "DISABLED");
	
	alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
	
    A_CHK(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
    A_CHK(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
    Fvector orient[2] = { { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f } };
    A_CHK(alListenerfv(AL_ORIENTATION, &orient[0].x));
    A_CHK(alListenerf(AL_GAIN, 1.f));

    // Ініціалізація EFX
    bEAX = false;
    bEFX_Initialized = false; // Скидаємо прапорець при рестарті
    
    if (alcIsExtensionPresent(pDevice, "ALC_EXT_EFX")) {
        Msg("* OpenAL: EFX extension found. Setting up Reverb.");
        bEAX = true;
        
        alGenAuxiliaryEffectSlots(1, &effect_slot);
        alGenEffects(1, &reverb_effect);
        alEffecti(reverb_effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    }

    inherited::_initialize(stage);

    if (stage == 1) {
        CSoundRender_Target* T = 0;
        for (u32 tit = 0; tit < u32(psSoundTargets); tit++) {
            T = xr_new<CSoundRender_TargetA>();
            if (T->_initialize()) {
                s_targets.push_back(T);
            } else {
                Log("! SOUND: OpenAL: Max targets - ", tit);
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

    // Видаляємо слоти EFX при виході
    if (bEAX) {
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
    if (!bEAX) return;
    CSoundRender_Environment* E = static_cast<CSoundRender_Environment*>(_E);
    
    // === ДОДАНО: Логіка лінійної інтерполяції (Lerp) ===
    if (!bEFX_Initialized) {
        // Якщо це перший кадр гри, миттєво записуємо значення, щоб уникнути багів
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

        // Плавно підтягуємо поточні значення до цільових (E->...)
        env_density             += (E->EnvironmentDiffusion - env_density) * lerp_speed;
        env_room                += (E->Room                - env_room) * lerp_speed;
        env_room_hf             += (E->RoomHF              - env_room_hf) * lerp_speed;
        env_decay_time          += (E->DecayTime           - env_decay_time) * lerp_speed;
        env_decay_hf_ratio      += (E->DecayHFRatio        - env_decay_hf_ratio) * lerp_speed;
        env_reflections_delay   += (E->ReflectionsDelay    - env_reflections_delay) * lerp_speed;
        env_reverb_delay        += (E->ReverbDelay         - env_reverb_delay) * lerp_speed;
        env_room_rolloff_factor += (E->RoomRolloffFactor   - env_room_rolloff_factor) * lerp_speed;
    }
    // ===================================================

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

    // Завантажуємо налаштований ефект у слот
    alAuxiliaryEffectSloti(effect_slot, AL_EFFECTSLOT_EFFECT, reverb_effect);
}

void CSoundRender_CoreA::update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) {
    // Зберігаємо час кадру для нашого Lerp
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