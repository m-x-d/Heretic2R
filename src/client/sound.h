//
// sound.h -- Sound library interface.
//
// Copyright 1998 Raven Software
//

#pragma once

#ifndef SNDLIB_DECLSPEC //mxd. So sound library could use this header...
extern cvar_t* snd_dll;

// Sound library function pointers.
extern void (*S_Init)(void);
extern void (*S_Shutdown)(void);

// If origin is NULL, the sound will be dynamically sourced from the entity
extern void (*S_StartSound)(const vec3_t origin, int entnum, int entchannel, struct sfx_s* sfx, float fvol, int attenuation, float timeofs);
extern void (*S_StartLocalSound)(const char* sound);

extern void (*S_StopAllSounds)(void);
extern void (*S_StopAllSounds_Sounding)(void);

extern void (*S_Update)(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up);
extern void (*S_Activate)(qboolean active);

extern void (*S_BeginRegistration)(void);
extern struct sfx_s* (*S_RegisterSound)(const char* name);
extern void (*S_EndRegistration)(void);

extern struct sfx_s* (*S_FindName)(const char* name, qboolean create);

#ifdef __A3D_GEOM
extern void (*S_A3D_ExportRenderGeom)(refexport_t* re);
#endif

extern void (*SNDEAX_SetEnvironment)(int env_index);
extern void (*SNDEAX_SetEnvironmentLevel)(int level);

// Sound module logic.
extern void SndDll_Init(void);
extern void SndDll_FreeLibrary(void);
#endif

// The sound code makes callbacks to the client for entitiy position information, so entities can be dynamically re-spatialized.
GAME_DECLSPEC extern void CL_GetEntitySoundOrigin(int ent, vec3_t org);