//
// snd_main.c -- Main control for SDL3 streaming sound output device.
//
// Copyright 2025 mxd
//

#include "snd_main.h"
#include "snd_ogg.h"
#include "snd_sdl3.h"
#include "snd_wav.h"
#include "Vector.h"

#define SNDLIB_DECLSPEC __declspec(dllexport)

snd_import_t si;

// Internal sound data & structures.
channel_t channels[MAX_CHANNELS];

sound_t sound;

vec3_t listener_origin;
vec3_t listener_forward;
vec3_t listener_right;
vec3_t listener_up;

static int s_registration_sequence;
static qboolean s_registering;

int paintedtime;
int s_rawend;

float snd_attenuations[] = // H2
{
	0.0f,		// ATTN_NONE
	0.0008f,	// ATTN_NORM
	0.002f,		// ATTN_IDLE
	0.006f		// ATTN_STATIC
};

static qboolean sound_started;
static qboolean s_active;

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
cvar_t* s_show;
cvar_t* s_mixahead;
cvar_t* s_paused; //mxd

// H2: sound attenuation cvars.
static cvar_t* s_attn_norm;
static cvar_t* s_attn_idle;
static cvar_t* s_attn_static;

cvar_t* s_underwater_gain_hf; // YQ2
cvar_t* s_camera_under_surface; // H2

#pragma region ========================== Console commands ==========================

// Q2 counterpart.
static void S_Play_f(void) //mxd. Named 'S_Play' in original logic.
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static void S_SoundList_f(void) //mxd. Named 'S_SoundList' in original logic.
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static void S_SoundInfo_f(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

// Activate or deactivate sound backend.
static void S_Activate(const qboolean active)
{
	s_active = active;

	if (!active)
		S_StopAllSounds();
}

// Initializes the sound system and it's requested backend.
static void S_Init(void)
{
	si.Com_Printf("\n------- Sound initialization -------\n");

	const cvar_t* cv = si.Cvar_Get("s_initsound", "1", 0);
	if (cv->value == 0.0f)
	{
		si.Com_Printf("Not initializing.\n");
	}
	else
	{
		s_volume = si.Cvar_Get("s_volume", "0.5", CVAR_ARCHIVE);
		s_sounddir = si.Cvar_Get("s_sounddir", "sound", CVAR_ARCHIVE); // H2
		s_khz = si.Cvar_Get("s_khz", "44", CVAR_ARCHIVE);  // Q2: 11 //TODO: remove? Always run at 44 Khz?
		s_loadas8bit = si.Cvar_Get("s_loadas8bit", "0", CVAR_ARCHIVE); // Q2: 1

		s_mixahead = si.Cvar_Get("s_mixahead", "0.14", CVAR_ARCHIVE); // Q2: 0.2
		s_show = si.Cvar_Get("s_show", "0", 0);
		s_testsound = si.Cvar_Get("s_testsound", "0", 0);

		s_underwater_gain_hf = si.Cvar_Get("s_underwater_gain_hf", "0.25", CVAR_ARCHIVE); // YQ2
		s_camera_under_surface = si.Cvar_Get("cl_camera_under_surface", "0.0", 0); // H2
		//TODO: implement s_feedback_kind YQ2 logic?

		// H2: extra attenuation cvars.
		s_attn_norm = si.Cvar_Get("s_attn_norm", "0.0008", 0);
		s_attn_idle = si.Cvar_Get("s_attn_idle", "0.002", 0);
		s_attn_static = si.Cvar_Get("s_attn_static", "0.006", 0);

		s_paused = si.Cvar_Get("paused", "0", 0); //mxd

		si.Cmd_AddCommand("play", S_Play_f);
		si.Cmd_AddCommand("stopsound", S_StopAllSounds);
		si.Cmd_AddCommand("soundlist", S_SoundList_f);
		si.Cmd_AddCommand("soundinfo", S_SoundInfo_f);

		if (SDL_BackendInit())
		{
			sound_started = true;

			num_sfx = 0;
			paintedtime = 0;
			s_active = true;

			OGG_Init();

			si.Com_Printf("Sound sampling rate: %i\n", sound.speed);
			S_StopAllSounds();
		}
	}

	si.Com_Printf("------------------------------------\n");
}

static void S_Shutdown(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
static sfx_t* S_FindName(const char* name, const qboolean create)
{
	if (name == NULL || name[0] == 0) //mxd. Merged checks.
	{
		si.Com_Error(ERR_FATAL, "S_FindName: empty name\n");
		return NULL;
	}

	if (strlen(name) >= MAX_QPATH)
	{
		si.Com_Error(ERR_FATAL, "Sound name too long: %s", name);
		return NULL;
	}

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
		{
			si.Com_Error(ERR_FATAL, "S_FindName: out of sfx_t");
			return NULL;
		}

		num_sfx++;
	}

	sfx_t* sfx = &known_sfx[free_slot];
	memset(sfx, 0, sizeof(*sfx));
	strcpy_s(sfx->name, sizeof(sfx->name), name); //mxd. strcpy -> strcpy_s.
	sfx->registration_sequence = s_registration_sequence;

	return sfx;
}

// Q2 counterpart.
static void S_BeginRegistration(void)
{
	s_registration_sequence++;
	s_registering = true;
}

// Q2 counterpart.
static sfx_t* S_RegisterSound(const char* name)
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
static void S_EndRegistration(void)
{
	// Free any sounds not from this registration sequence.
	sfx_t* sfx = &known_sfx[0];
	for (int i = 0; i < num_sfx; i++, sfx++) //TODO: implement S_HasFreeSpace() YQ2 logic?
	{
		// Don't need this sound?
		if (sfx->name[0] != 0 && sfx->registration_sequence != s_registration_sequence)
		{
			// It is possible to have a leftover from a server that didn't finish loading.
			if (sfx->cache != NULL)
				si.Z_Free(sfx->cache);

			sfx->cache = NULL;
			sfx->name[0] = 0;
		}
		//mxd. Skip Com_PageInMemory() logic.
	}

	// Load everything in.
	sfx = &known_sfx[0];
	for (int i = 0; i < num_sfx; i++, sfx++)
		if (sfx->name[0] != 0)
			S_LoadSound(sfx);

	s_registering = false;
}

// Q2 counterpart.
channel_t* S_PickChannel(const int entnum, const int entchannel)
{
	if (entchannel < 0)
	{
		si.Com_Error(ERR_DROP, "S_PickChannel: entchannel<0");
		return NULL;
	}

	// Check for replacement sound, or find the best one to replace.
	int first_to_die = -1;
	int life_left = 0x7fffffff;

	for (int ch_idx = 0; ch_idx < MAX_CHANNELS; ch_idx++)
	{
		if (entchannel != 0 && channels[ch_idx].entnum == entnum && channels[ch_idx].entchannel == entchannel) // Channel 0 never overrides.
		{
			// Always override sound from same entity.
			first_to_die = ch_idx;
			break;
		}

		// Don't let monster sounds override player sounds.
		if (channels[ch_idx].entnum == si.cl->playernum + 1 && entnum != si.cl->playernum + 1 && channels[ch_idx].sfx != NULL)
			continue;

		if (channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
	}

	if (first_to_die == -1)
		return NULL;

	channel_t* ch = &channels[first_to_die];
	memset(ch, 0, sizeof(*ch));

	return ch;
}

// Q2 counterpart.
static playsound_t* S_AllocPlaysound(void)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Take the next playsound and begin it on the channel.
// This is never called directly by S_Play*, but only by the update loop.
void S_IssuePlaysound(playsound_t* ps)
{
	NOT_IMPLEMENTED
}

static sfx_t* S_RegisterSexedSound(const entity_state_t* ent, char* base)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Validates the params and queues the sound up.
// If origin is NULL, the sound will be dynamically sourced from the entity.
// Entchannel 0 will never override a playing sound.
static void S_StartSound(const vec3_t origin, const int ent_num, const int ent_channel, sfx_t* sfx, const float volume, const int attenuation, const float time_offset)
{
	static int s_beginofs = 0; //mxd. Made local static.

	// YQ2: extra !s_active check: a hack to prevent temporary entities generating sounds when the sound backend is not active and the game is not paused.
	if (!sound_started || !s_active || sfx == NULL || ent_num < 0 || ent_num >= MAX_EDICTS) //mxd. Add entnum sanity checks.
		return;

	if (sfx->name[0] == '*')
	{
		sfx = S_RegisterSexedSound(&si.cl_entities[ent_num].current, sfx->name);

		if (sfx == NULL) // YQ2: extra sanity check.
			return;
	}

	// Make sure the sound is loaded.
	if (S_LoadSound(sfx) == NULL)
		return;

	// Make the playsound_t.
	playsound_t* ps = S_AllocPlaysound();

	if (ps == NULL)
		return;

	if (origin != NULL)
	{
		//TODO: add YQ2 GetBSPEntitySoundOrigin() logic?
		VectorCopy(origin, ps->origin);
		ps->fixed_origin = true;
	}
	else
	{
		ps->fixed_origin = false;
	}

	ps->entnum = ent_num;
	ps->entchannel = ent_channel;
	ps->attenuation_index = attenuation;
	ps->volume = volume * 255.0f;
	ps->sfx = sfx;

	// Drift s_beginofs.
	int start = (int)((float)si.cl->frame.servertime * 0.001f * (float)sound.speed) + s_beginofs;

	if (start < paintedtime)
	{
		start = paintedtime;
		s_beginofs = start - (int)((float)si.cl->frame.servertime * 0.001f * (float)sound.speed);
	}
	else if (start > (int)((float)paintedtime + 0.3f * (float)sound.speed))
	{
		start = (int)((float)paintedtime + 0.1f * (float)sound.speed);
		s_beginofs = start - (int)((float)si.cl->frame.servertime * 0.001f * (float)sound.speed);
	}
	else
	{
		s_beginofs -= 10;
	}

	if (time_offset == 0.0f)
		ps->begin = paintedtime;
	else
		ps->begin = (int)((float)start + time_offset * (float)sound.speed);

	// Sort into the pending sound list.
	playsound_t* sort = s_pendingplays.next;
	while (sort != &s_pendingplays && sort->begin < ps->begin)
		sort = sort->next;

	ps->next = sort;
	ps->prev = sort->prev;

	ps->next->prev = ps;
	ps->prev->next = ps;
}

// Q2 counterpart.
static void S_StartLocalSound(const char* name)
{
	NOT_IMPLEMENTED
}

// Clears the playback buffer so that all playback stops.
void S_ClearBuffer(void)
{
	if (!sound_started)
		return;

	s_rawend = 0;

	if (sound.buffer != NULL)
	{
		const int clear = ((sound.samplebits == 8) ? 0x80 : 0);
		memset(sound.buffer, clear, sound.samples * sound.samplebits / 8);
	}
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

static void S_StopAllSounds_Sounding(void) // H2
{
	if (sound_started)
		S_ClearBuffer();
}

// Called once each time through the main loop.
static void S_Update(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up)
{
	if (!sound_started || !s_active)
		return;

	// If the loading plaque is up, clear everything out to make sure we aren't looping a dirty dma buffer while loading.
	if (si.cls->disable_screen)
	{
		S_ClearBuffer();
		return;
	}

	//H2:
	snd_attenuations[ATTN_NORM] = s_attn_norm->value;
	snd_attenuations[ATTN_IDLE] = s_attn_idle->value;
	snd_attenuations[ATTN_STATIC] = s_attn_static->value;

	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);

	SDL_Update();
}

SNDLIB_DECLSPEC snd_export_t GetSoundAPI(const snd_import_t snd_import)
{
	snd_export_t snd_export;

	si = snd_import;

	snd_export.api_version = SND_API_VERSION;
	snd_export.library_name = "SDL3";

	snd_export.Init = S_Init;
	snd_export.Shutdown = S_Shutdown;

	snd_export.StartSound = S_StartSound;
	snd_export.StartLocalSound = S_StartLocalSound;

	snd_export.StopAllSounds = S_StopAllSounds;
	snd_export.StopAllSounds_Sounding = S_StopAllSounds_Sounding;

	snd_export.Update = S_Update;
	snd_export.Activate = S_Activate;

	snd_export.BeginRegistration = S_BeginRegistration;
	snd_export.RegisterSound = S_RegisterSound;
	snd_export.EndRegistration = S_EndRegistration;

	snd_export.FindName = S_FindName;

	snd_export.SetEaxEnvironment = NULL;

	return snd_export;
}