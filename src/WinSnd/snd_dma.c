//
// snd_dma.c -- Main control for any streaming sound output device.
//
// Copyright 1998 Raven Software
//

#include "snd_dma.h"
#include "snd_mem.h"
#include "snd_mix.h"
#include "snd_win.h"
#include "q_clientserver.h"
#include "qcommon.h"

// Internal sound data & structures.
channel_t channels[MAX_CHANNELS];

static qboolean sound_started = false; //mxd. int in Q2.

dma_t dma;

static int s_registration_sequence;
static qboolean s_registering;

static int soundtime; // Sample PAIRS.
int paintedtime; // Sample PAIRS.

// During registration it is possible to have more sounds than could actually be referenced during gameplay,
// because we don't want to free anything until we are sure we won't need it.
#define MAX_SFX	(MAX_SOUNDS * 2)
static sfx_t known_sfx[MAX_SFX];
static int num_sfx;

#define MAX_PLAYSOUNDS	128
static playsound_t s_playsounds[MAX_PLAYSOUNDS];
static playsound_t s_freeplays;
playsound_t s_pendingplays;

cvar_t* s_volume;
cvar_t* s_sounddir; // H2
cvar_t* s_testsound;
cvar_t* s_loadas8bit;
cvar_t* s_khz;
static cvar_t* s_show;
static cvar_t* s_mixahead;
cvar_t* s_primary;

#pragma region ========================== Console commands ==========================

static void S_Play(void) //TODO: rename to S_Play_f?
{
	NOT_IMPLEMENTED
}

static void S_SoundList(void) //TODO: rename to S_SoundList_f?
{
	NOT_IMPLEMENTED
}

static void S_SoundInfo_f(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

void S_Init(void)
{
	Com_Printf("\n------- sound initialization -------\n");

	const cvar_t* cv = Cvar_Get("s_initsound", "1", 0);
	if (cv->value == 0.0f)
	{
		Com_Printf("not initializing.\n");
	}
	else
	{
		s_volume = Cvar_Get("s_volume", "0.5", CVAR_ARCHIVE);
		s_sounddir = Cvar_Get("s_sounddir", "sound", CVAR_ARCHIVE); // H2
		s_khz = Cvar_Get("s_khz", "44", CVAR_ARCHIVE);  // Q2: 11
		s_loadas8bit = Cvar_Get("s_loadas8bit", "1", CVAR_ARCHIVE);

		s_mixahead = Cvar_Get("s_mixahead", "0.1", CVAR_ARCHIVE); // Q2: 0.2
		s_show = Cvar_Get("s_show", "0", 0);
		s_testsound = Cvar_Get("s_testsound", "0", 0);
		s_primary = Cvar_Get("s_primary", "0", CVAR_ARCHIVE);

		// H2: extra attenuation cvars. Not used anywhere, though... --mxd
		//s_attn_norm = Cvar_Get("s_attn_norm", "0.0008", 0);
		//s_attn_idle = Cvar_Get("s_attn_idle", "0.002", 0);
		//s_attn_static = Cvar_Get("s_attn_static", "0.006", 0);

		Cmd_AddCommand("play", S_Play);
		Cmd_AddCommand("stopsound", S_StopAllSounds);
		Cmd_AddCommand("soundlist", S_SoundList);
		Cmd_AddCommand("soundinfo", S_SoundInfo_f);

		if (SNDDMA_Init())
		{
			S_InitScaletable();

			sound_started = true;
			num_sfx = 0;
			soundtime = 0;
			paintedtime = 0;

			Com_Printf("sound sampling rate: %i\n", dma.speed);
			S_StopAllSounds();
		}
	}

	Com_Printf("------------------------------------\n");
}

// Q2 counterpart.
// Shutdown sound engine.
void S_Shutdown(void)
{
	if (!sound_started)
		return;

	SNDDMA_Shutdown();

	Cmd_RemoveCommand("play");
	Cmd_RemoveCommand("stopsound");
	Cmd_RemoveCommand("soundlist");
	Cmd_RemoveCommand("soundinfo");

	// Free all sounds.
	sfx_t* sfx = known_sfx;
	for (int i = 0; i < num_sfx; i++, sfx++)
	{
		if (sfx->name[0] == 0)
			continue;

		if (sfx->cache != NULL)
			Z_Free(sfx->cache);

		memset(sfx, 0, sizeof(*sfx));
	}

	num_sfx = 0;
	sound_started = false;
}

sfx_t* S_FindName(char* name, qboolean create)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart.
void S_BeginRegistration(void)
{
	s_registration_sequence++;
	s_registering = true;
}

sfx_t* S_RegisterSound(const char* name)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart.
void S_EndRegistration(void)
{
	// Free any sounds not from this registration sequence.
	sfx_t* sfx = known_sfx;
	for (int i = 0; i < num_sfx; i++, sfx++)
	{
		// Don't need this sound?
		if (sfx->name[0] != 0 && sfx->registration_sequence != s_registration_sequence)
		{
			// It is possible to have a leftover from a server that didn't finish loading.
			if (sfx->cache != NULL)	
				Z_Free(sfx->cache);

			memset(sfx, 0, sizeof(*sfx));
		}
		//mxd. Skip Com_PageInMemory() logic.
	}

	// Load everything in.
	sfx = known_sfx;
	for (int i = 0; i < num_sfx; i++, sfx++)
		if (sfx->name[0] != 0)
			S_LoadSound(sfx);

	s_registering = false;
}

void S_StartSound(const vec3_t origin, int entnum, int entchannel, sfx_t* sfx, float fvol, int attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

void S_StartLocalSound(const char* sound)
{
	NOT_IMPLEMENTED
}

static void S_ClearBuffer(void)
{
	if (!sound_started)
		return;

	//H2: no s_rawend reset.
	SNDDMA_BeginPainting();

	if (dma.buffer != NULL)
	{
		const int clear = ((dma.samplebits == 8) ? 0x80 : 0);
		memset(dma.buffer, clear, dma.samples * dma.samplebits / 8);
	}

	SNDDMA_Submit();
}

void S_StopAllSounds(void)
{
	if (!sound_started)
		return;

	// Clear all the playsounds.
	memset(s_playsounds, 0, sizeof(s_playsounds));

	s_freeplays.next = &s_freeplays;
	s_freeplays.prev = &s_freeplays;
	s_pendingplays.next = &s_pendingplays;
	s_pendingplays.prev = &s_pendingplays;

	for (int i = 0; i < MAX_PLAYSOUNDS; i++)
	{
		s_playsounds[i].prev = &s_freeplays;
		s_playsounds[i].next = s_freeplays.next;
		s_playsounds[i].prev->next = &s_playsounds[i];
		s_playsounds[i].next->prev = &s_playsounds[i];
	}

	// Clear all the channels.
	memset(channels, 0, sizeof(channels));
	// H2: no S_ClearBuffer() call.
}

void S_StopAllSounds_Sounding(void) // H2
{
	if (sound_started)
		S_ClearBuffer();
}

void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	NOT_IMPLEMENTED
}