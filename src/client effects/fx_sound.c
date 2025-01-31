//
// fx_sound.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "Random.h"

#define TAG_LEVEL	766 // Tags get cleared at each new level.

typedef struct sound_think_info_s
{
	int style;
	int attenuation; //mxd. float in original version.
	float volume;
	float wait;
} sound_think_info_t;

static qboolean SoundThink(struct client_entity_s* self, centity_t* owner)
{
	static cvar_t* cinematicfreeze = NULL; //mxd

	const sound_think_info_t* soundinfo = (sound_think_info_t*)self->extra;

	// Flag if we are not doing a sound.
	char* soundname = NULL;

	// If we are in a cinematic, stop us making noises.
	if (cinematicfreeze == NULL) //mxd. Resolve once, instead of on every call.
		cinematicfreeze = Cvar_Get("cl_cinematicfreeze", "0", 0);

	if ((int)cinematicfreeze->value == 0)
	{
		// These are peripheral ambient noises.
		switch (soundinfo->style)
		{
			//mxd. In original logic, in all cases when multiple sounds are used, the last sound has 2x chance to be picked.
			// Original programmer assumed that irand() never rolls max?

			case AS_SEAGULLS:
				soundname = va("ambient/gull%i.wav", irand(1, 3));
				break;

			case AS_BIRDS:
				soundname = va("ambient/bird%i.wav", irand(1, 10));
				break;

			case AS_CRICKETS:
				soundname = va("ambient/cricket%i.wav", irand(1, 3));
				break;

			case AS_FROGS:
			{
				static char* la_frogs[] = { "frog", "frog2" }; //mxd
				soundname = va("ambient/%s.wav", la_frogs[irand(0, 1)]);
			} break;

			case AS_CRYING:
			{
				static char* cries[] = { "femcry1", "femcry2", "kidcry1", "kidcry2" }; //mxd
				soundname = va("ambient/%s.wav", cries[irand(0, 3)]);
			} break;

			case AS_MOSQUITOES:
				soundname = va("ambient/insects%i.wav", irand(1, 2));
				break;

			case AS_BUBBLES:
				soundname = "ambient/bubbles.wav";
				break;

			case AS_BELL:
				soundname = "ambient/bell.wav";
				break;

			case AS_FOOTSTEPS:
			{
				static char* footsteps[] = { "runaway1", "runaway2", "sewerrun" }; //mxd
				soundname = va("ambient/%s.wav", footsteps[irand(0, 2)]);
			} break;

			case AS_MOANS:
			{
				static char* moans[] = { "moan1", "moan2", "scream1", "scream2", "coughing" }; //mxd
				soundname = va("ambient/%s.wav", moans[irand(0, 4)]);
			} break;

			case AS_SEWERDRIPS:
				soundname = va("ambient/sewerdrop%i.wav", irand(1, 3)); //mxd
				break;

			case AS_WATERDRIPS:
				soundname = va("ambient/waterdrop%i.wav", irand(1, 3)); //mxd
				break;

			case AS_HEAVYDRIPS:
				soundname = va("ambient/soliddrop%i.wav", irand(1, 3)); //mxd
				break;

			case AS_WINDCHIME:
				soundname = "ambient/windchimes.wav";
				break;

			case AS_BIRD1:
				soundname = "ambient/bird5.wav";
				break;

			case AS_BIRD2:
				soundname = "ambient/bird8.wav";
				break;

			case AS_GONG:
				soundname = "ambient/gong.wav";
				break;

			case AS_ROCKS:
			{
				static char* rocks[] = { "rocks1", "rocks4", "rocks5" }; //mxd
				soundname = va("ambient/%s.wav", rocks[irand(0, 2)]);
			} break;

			case AS_CAVECREAK:
				soundname = va("ambient/cavecreak%i.wav", irand(1, 3)); //BUGFIX: mxd. Original logic uses "cavecreak1" in all 3 cases.
				break;

			default:
				Com_DPrintf("ERROR: invalid ambient sound type %d at %f %f %f\n", soundinfo->style, self->origin[0], self->origin[1], self->origin[2]);
				break;
		}
	}

	// If we have a sound to do, lets do it.
	if (soundname != NULL)
	{
		const int attenuation = (Vec3IsZero(self->origin) ? 0 : soundinfo->attenuation); //mxd
		fxi.S_StartSound(self->origin, owner->current.number, CHAN_WEAPON, fxi.S_RegisterSound(soundname), soundinfo->volume, attenuation, 0.0f);
	}

	self->updateTime = (int)(soundinfo->wait * flrand(0.5f, 1.5f));
	self->updateTime = max(MIN_UPDATE_TIME, self->updateTime);

	return true; // Keep everything around so we can shut them down when needed.
}

void FXSound(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte style;
	byte attenuation;
	byte volume;
	byte wait;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SOUND].formatString, &style, &attenuation, &volume, &wait);

	client_entity_t* self = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_NOMOVE), origin, NULL, 20);

	self->flags &= ~CEF_OWNERS_ORIGIN;
	self->extra = fxi.TagMalloc(sizeof(sound_think_info_t), TAG_LEVEL);
	self->Update = SoundThink;

	sound_think_info_t* info = self->extra;
	info->style = style;
	info->attenuation = attenuation;
	info->volume = (float)volume / 255.0f;
	info->wait = (float)wait * 1000.0f;

	AddEffect(owner, self);
}