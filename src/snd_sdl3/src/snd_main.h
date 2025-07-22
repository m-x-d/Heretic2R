//
// snd_main.h
//
// Copyright 2025 mxd
//

#pragma once

#include "snd_local.h"

extern sound_t sound; // Q2: dma_t dma.

extern vec3_t listener_origin;
extern vec3_t listener_forward;
extern vec3_t listener_right;
extern vec3_t listener_up;

extern int paintedtime;
extern int s_rawend;
extern float snd_attenuations[]; //mxd

extern channel_t channels[MAX_CHANNELS];
extern playsound_t s_pendingplays;

extern cvar_t* s_volume;
extern cvar_t* s_sounddir; // H2
extern cvar_t* s_testsound;
extern cvar_t* s_loadas8bit;
extern cvar_t* s_khz;
extern cvar_t* s_show; //mxd
extern cvar_t* s_mixahead; //mxd
extern cvar_t* s_paused; //mxd

extern cvar_t* s_underwater_gain_hf; // YQ2
extern cvar_t* s_camera_under_surface; // H2

extern channel_t* S_PickChannel(int entnum, int entchannel);
extern void S_IssuePlaysound(playsound_t* ps);
extern void S_StopAllSounds(void);
extern void S_RawSamples(int num_samples, uint rate, int width, int num_channels, const byte* data, float volume);

// Local forward declarations for snd_main.c
static sfx_t* S_RegisterSound(const char* name);
static void S_StartSound(const vec3_t origin, int ent_num, int ent_channel, sfx_t* sfx, float volume, int attenuation, float time_offset);