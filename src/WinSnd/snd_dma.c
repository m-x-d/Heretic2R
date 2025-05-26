//
// snd_dma.c -- Main control for any streaming sound output device.
//
// Copyright 1998 Raven Software
//

#include "snd_dma.h"
#include "snd_mem.h"
#include "snd_mix.h"
#include "snd_win.h"
#include "client.h"
#include "q_clientserver.h"
#include "qcommon.h"
#include "Vector.h"

// Internal sound data & structures.
channel_t channels[MAX_CHANNELS];

static qboolean sound_started = false; //mxd. int in Q2.

dma_t dma;

static vec3_t listener_origin;
static vec3_t listener_forward;
static vec3_t listener_right;
static vec3_t listener_up;

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

// Q2 counterpart.
sfx_t* S_FindName(const char* name, const qboolean create)
{
	if (name == NULL || name[0] == 0) //mxd. Merged checks.
		Com_Error(ERR_FATAL, "S_FindName: empty name\n");

	if (strlen(name) >= MAX_QPATH)
		Com_Error(ERR_FATAL, "Sound name too long: %s", name);

	// See if already loaded.
	for (int i = 0; i < num_sfx; i++)
		if (strcmp(known_sfx[i].name, name) == 0)
			return &known_sfx[i];

	if (!create)
		return NULL;

	// Find a free sfx.
	int free_slot = 0;
	for (; free_slot < num_sfx; free_slot++)
		if (known_sfx[free_slot].name[0] == 0)
			break;

	if (free_slot == num_sfx)
	{
		if (num_sfx == MAX_SFX)
			Com_Error(ERR_FATAL, "S_FindName: out of sfx_t");

		num_sfx++;
	}

	sfx_t* sfx = &known_sfx[free_slot];
	memset(sfx, 0, sizeof(*sfx));
	strcpy_s(sfx->name, sizeof(sfx->name), name); //mxd. strcpy -> strcpy_s.
	sfx->registration_sequence = s_registration_sequence;

	return sfx;
}

// Q2 counterpart.
void S_BeginRegistration(void)
{
	s_registration_sequence++;
	s_registering = true;
}

// Q2 counterpart.
sfx_t* S_RegisterSound(const char* name)
{
	if (!sound_started)
		return NULL;

	sfx_t* sfx = S_FindName(name, true);
	sfx->registration_sequence = s_registration_sequence;

	if (!s_registering)
		S_LoadSound(sfx);

	return sfx;
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

static void S_Spatialize(channel_t* ch)
{
	NOT_IMPLEMENTED
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

static void S_AddLoopSounds(void)
{
	NOT_IMPLEMENTED
}

static void S_Update_(void) //TODO: rename to S_MixSound?
{
	NOT_IMPLEMENTED
}

// Called once each time through the main loop.
void S_Update(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up)
{
	if (!sound_started)
		return;

	// If the loading plaque is up, clear everything out to make sure we aren't looping a dirty dma buffer while loading.
	if (cls.disable_screen)
	{
		S_ClearBuffer();
		return;
	}

	// Rebuild scale tables if volume is modified.
	if (s_volume->modified)
		S_InitScaletable();

	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);

	// Update spatialization for dynamic sounds.
	channel_t* ch = &channels[0];
	for (int i = 0; i < MAX_CHANNELS; i++, ch++)
	{
		if (ch->sfx == NULL)
			continue;

		if (ch->autosound)
			memset(ch, 0, sizeof(*ch)); // Autosounds are regenerated fresh each frame.
		else
			S_Spatialize(ch); // Respatialize channel.

		// H2: missing 'clear channel when it can't be heard' logic. //TODO: re-add? Missing because pre-3.21 Q2 source was used?
	}

	// Add looping sounds.
	S_AddLoopSounds();

	// Debugging output.
	if ((int)s_show->value)
	{
		int total_sounds = 0;

		ch = &channels[0];
		for (int i = 0; i < MAX_CHANNELS; i++, ch++)
		{
			if (ch->sfx != NULL && (ch->leftvol > 0 || ch->rightvol > 0))
			{
				Com_Printf("%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name);
				total_sounds++;
			}
		}

		Com_Printf("----(%i)---- painted: %i\n", total_sounds, paintedtime);
	}

	// Mix some sound.
	S_Update_();
}