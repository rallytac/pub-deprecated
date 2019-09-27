//
//  Copyright (c) 2018 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef EngageAudioDevice_h
#define EngageAudioDevice_h


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(WIN32)
    #ifdef ENGAGE_EXPORTS
        // Windows needs dllexport to produce an import lib without a .DEF file
        #define ENGAGE_API  __declspec(dllexport) extern
    #else
        #define ENGAGE_API  extern
    #endif
#else
    #define ENGAGE_API
#endif

typedef enum {
    eadCreateInstance = 1,
    eadDestroyInstance,
    eadStart,
    eadStop,
    eadPause,
    eadResume,
    eadReset,
    eadRestart
} EngageAudioDeviceCtlOp_t;

static const int ENGAGE_AUDIO_DEVICE_RESULT_OK = 0;
static const int ENGAGE_AUDIO_DEVICE_GENERAL_ERROR = -1;
static const int ENGAGE_AUDIO_DEVICE_INVALID_CONFIGURATION = -2;
static const int ENGAGE_AUDIO_DEVICE_INVALID_DEVICE_ID = -3;
static const int ENGAGE_AUDIO_DEVICE_INVALID_INSTANCE_ID = -4;
static const int ENGAGE_AUDIO_DEVICE_INVALID_COMBINED_DEVICE_ID_AND_INSTANCE_ID = -5;
static const int ENGAGE_AUDIO_DEVICE_INVALID_OPERATION = -6;

typedef int (*PFN_ENGAGE_AUDIO_DEVICE_CTL)(int16_t deviceId, int16_t instanceId, EngageAudioDeviceCtlOp_t op, uintptr_t p1);

ENGAGE_API int16_t engageAudioDeviceRegister(const char *jsonConfiguration, PFN_ENGAGE_AUDIO_DEVICE_CTL pfnCtl);
ENGAGE_API int16_t engageAudioDeviceUnregister(int16_t deviceId);
ENGAGE_API int16_t engageAudioDeviceWriteBuffer(int16_t deviceId, int16_t instanceId, const int16_t *buffer, size_t samples);
ENGAGE_API int16_t engageAudioDeviceReadBuffer(int16_t deviceId, int16_t instanceId, int16_t *buffer, size_t samples);

#ifdef __cplusplus
}
#endif
#endif // EngageAudioDevice_h
