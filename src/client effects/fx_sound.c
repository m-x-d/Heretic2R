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

enum FXSoundID_e //mxd
{
	SND_SEAGULLS1,
	SND_SEAGULLS2,
	SND_SEAGULLS3,

	SND_BIRDS1,
	SND_BIRDS2,
	SND_BIRDS3,
	SND_BIRDS4,
	SND_BIRDS5,
	SND_BIRDS6,
	SND_BIRDS7,
	SND_BIRDS8,
	SND_BIRDS9,
	SND_BIRDS10,

	SND_CRICKETS1,
	SND_CRICKETS2,
	SND_CRICKETS3,

	SND_FROGS1,
	SND_FROGS2,

	SND_CRYING1,
	SND_CRYING2,
	SND_CRYING3,
	SND_CRYING4,

	SND_MOSQUITOES1,
	SND_MOSQUITOES2,

	SND_BUBBLES,
	SND_BELL,

	SND_FOOTSTEPS1,
	SND_FOOTSTEPS2,
	SND_FOOTSTEPS3,

	SND_MOANS1,
	SND_MOANS2,
	SND_MOANS3,
	SND_MOANS4,
	SND_MOANS5,

	SND_SEWERDRIPS1,
	SND_SEWERDRIPS2,
	SND_SEWERDRIPS3,

	SND_WATERDRIPS1,
	SND_WATERDRIPS2,
	SND_WATERDRIPS3,

	SND_HEAVYDRIPS1,
	SND_HEAVYDRIPS2,
	SND_HEAVYDRIPS3,

	SND_WINDCHIME,

	SND_BIRD1,
	SND_BIRD2,

	SND_GONG,

	SND_ROCKS1,
	SND_ROCKS2,
	SND_ROCKS3,

	SND_CAVECREAK1,
	SND_CAVECREAK2,
	SND_CAVECREAK3,

	NUM_SOUNDS
};

static struct sfx_s* fx_sounds[NUM_SOUNDS]; //mxd

void PreCacheFXSoundSFX(void) //mxd
{
	fx_sounds[SND_SEAGULLS1] = fxi.S_RegisterSound("ambient/gull1.wav");
	fx_sounds[SND_SEAGULLS2] = fxi.S_RegisterSound("ambient/gull2.wav");
	fx_sounds[SND_SEAGULLS3] = fxi.S_RegisterSound("ambient/gull3.wav");

	fx_sounds[SND_BIRDS1] = fxi.S_RegisterSound("ambient/bird1.wav");
	fx_sounds[SND_BIRDS2] = fxi.S_RegisterSound("ambient/bird2.wav");
	fx_sounds[SND_BIRDS3] = fxi.S_RegisterSound("ambient/bird3.wav");
	fx_sounds[SND_BIRDS4] = fxi.S_RegisterSound("ambient/bird4.wav");
	fx_sounds[SND_BIRDS5] = fxi.S_RegisterSound("ambient/bird5.wav");
	fx_sounds[SND_BIRDS6] = fxi.S_RegisterSound("ambient/bird6.wav");
	fx_sounds[SND_BIRDS7] = fxi.S_RegisterSound("ambient/bird7.wav");
	fx_sounds[SND_BIRDS8] = fxi.S_RegisterSound("ambient/bird8.wav");
	fx_sounds[SND_BIRDS9] = fxi.S_RegisterSound("ambient/bird9.wav");
	fx_sounds[SND_BIRDS10] = fxi.S_RegisterSound("ambient/bird10.wav");

	fx_sounds[SND_CRICKETS1] = fxi.S_RegisterSound("ambient/cricket1.wav");
	fx_sounds[SND_CRICKETS2] = fxi.S_RegisterSound("ambient/cricket2.wav");
	fx_sounds[SND_CRICKETS3] = fxi.S_RegisterSound("ambient/cricket3.wav");

	fx_sounds[SND_FROGS1] = fxi.S_RegisterSound("ambient/frog.wav");
	fx_sounds[SND_FROGS2] = fxi.S_RegisterSound("ambient/frog2.wav");

	fx_sounds[SND_CRYING1] = fxi.S_RegisterSound("ambient/femcry1.wav");
	fx_sounds[SND_CRYING2] = fxi.S_RegisterSound("ambient/femcry2.wav");
	fx_sounds[SND_CRYING3] = fxi.S_RegisterSound("ambient/kidcry1.wav");
	fx_sounds[SND_CRYING4] = fxi.S_RegisterSound("ambient/kidcry2.wav");

	fx_sounds[SND_MOSQUITOES1] = fxi.S_RegisterSound("ambient/insects1.wav");
	fx_sounds[SND_MOSQUITOES2] = fxi.S_RegisterSound("ambient/insects2.wav");

	fx_sounds[SND_BUBBLES] = fxi.S_RegisterSound("ambient/bubbles.wav");
	fx_sounds[SND_BELL] = fxi.S_RegisterSound("ambient/bell.wav");

	fx_sounds[SND_FOOTSTEPS1] = fxi.S_RegisterSound("ambient/runaway1.wav");
	fx_sounds[SND_FOOTSTEPS2] = fxi.S_RegisterSound("ambient/runaway2.wav");
	fx_sounds[SND_FOOTSTEPS3] = fxi.S_RegisterSound("ambient/sewerrun.wav");

	fx_sounds[SND_MOANS1] = fxi.S_RegisterSound("ambient/moan1.wav");
	fx_sounds[SND_MOANS2] = fxi.S_RegisterSound("ambient/moan2.wav");
	fx_sounds[SND_MOANS3] = fxi.S_RegisterSound("ambient/scream1.wav");
	fx_sounds[SND_MOANS4] = fxi.S_RegisterSound("ambient/scream2.wav");
	fx_sounds[SND_MOANS5] = fxi.S_RegisterSound("ambient/coughing.wav");

	fx_sounds[SND_SEWERDRIPS1] = fxi.S_RegisterSound("ambient/sewerdrop1.wav");
	fx_sounds[SND_SEWERDRIPS2] = fxi.S_RegisterSound("ambient/sewerdrop2.wav");
	fx_sounds[SND_SEWERDRIPS3] = fxi.S_RegisterSound("ambient/sewerdrop3.wav");

	fx_sounds[SND_WATERDRIPS1] = fxi.S_RegisterSound("ambient/waterdrop1.wav");
	fx_sounds[SND_WATERDRIPS2] = fxi.S_RegisterSound("ambient/waterdrop2.wav");
	fx_sounds[SND_WATERDRIPS3] = fxi.S_RegisterSound("ambient/waterdrop3.wav");

	fx_sounds[SND_HEAVYDRIPS1] = fxi.S_RegisterSound("ambient/soliddrop1.wav");
	fx_sounds[SND_HEAVYDRIPS2] = fxi.S_RegisterSound("ambient/soliddrop2.wav");
	fx_sounds[SND_HEAVYDRIPS3] = fxi.S_RegisterSound("ambient/soliddrop3.wav");

	fx_sounds[SND_WINDCHIME] = fxi.S_RegisterSound("ambient/windchimes.wav");

	fx_sounds[SND_BIRD1] = fxi.S_RegisterSound("ambient/bird5.wav");
	fx_sounds[SND_BIRD2] = fxi.S_RegisterSound("ambient/bird8.wav");

	fx_sounds[SND_GONG] = fxi.S_RegisterSound("ambient/gong.wav");

	fx_sounds[SND_ROCKS1] = fxi.S_RegisterSound("ambient/rocks1.wav");
	fx_sounds[SND_ROCKS2] = fxi.S_RegisterSound("ambient/rocks4.wav");
	fx_sounds[SND_ROCKS3] = fxi.S_RegisterSound("ambient/rocks5.wav");

	fx_sounds[SND_CAVECREAK1] = fxi.S_RegisterSound("ambient/cavecreak1.wav");
	fx_sounds[SND_CAVECREAK2] = fxi.S_RegisterSound("ambient/cavecreak2.wav");
	fx_sounds[SND_CAVECREAK3] = fxi.S_RegisterSound("ambient/cavecreak3.wav");
}

static qboolean SoundThink(struct client_entity_s* self, centity_t* owner)
{
	static cvar_t* cinematicfreeze = NULL; //mxd

	const sound_think_info_t* soundinfo = (sound_think_info_t*)self->extra;

	// Flag if we are not doing a sound.
	struct sfx_s* sfx = NULL;

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

			case AS_SEAGULLS:	sfx = fx_sounds[irand(SND_SEAGULLS1, SND_SEAGULLS3)]; break;
			case AS_BIRDS:		sfx = fx_sounds[irand(SND_BIRDS1, SND_BIRDS10)]; break;
			case AS_CRICKETS:	sfx = fx_sounds[irand(SND_CRICKETS1, SND_CRICKETS3)]; break;
			case AS_FROGS:		sfx = fx_sounds[irand(SND_FROGS1, SND_FROGS2)]; break;
			case AS_CRYING:		sfx = fx_sounds[irand(SND_CRYING1, SND_CRYING4)]; break;
			case AS_MOSQUITOES:	sfx = fx_sounds[irand(SND_MOSQUITOES1, SND_MOSQUITOES2)]; break;
			case AS_BUBBLES:	sfx = fx_sounds[SND_BUBBLES]; break;
			case AS_BELL:		sfx = fx_sounds[SND_BELL]; break;
			case AS_FOOTSTEPS:	sfx = fx_sounds[irand(SND_FOOTSTEPS1, SND_FOOTSTEPS3)]; break;
			case AS_MOANS:		sfx = fx_sounds[irand(SND_MOANS1, SND_MOANS5)]; break;
			case AS_SEWERDRIPS:	sfx = fx_sounds[irand(SND_SEWERDRIPS1, SND_SEWERDRIPS3)]; break;
			case AS_WATERDRIPS:	sfx = fx_sounds[irand(SND_WATERDRIPS1, SND_WATERDRIPS3)]; break;
			case AS_HEAVYDRIPS:	sfx = fx_sounds[irand(SND_HEAVYDRIPS1, SND_HEAVYDRIPS3)]; break;
			case AS_WINDCHIME:	sfx = fx_sounds[SND_WINDCHIME]; break;
			case AS_BIRD1:		sfx = fx_sounds[SND_BIRD1]; break;
			case AS_BIRD2:		sfx = fx_sounds[SND_BIRD2]; break;
			case AS_GONG:		sfx = fx_sounds[SND_GONG]; break;
			case AS_ROCKS:		sfx = fx_sounds[irand(SND_ROCKS1, SND_ROCKS3)]; break;
			case AS_CAVECREAK:	sfx = fx_sounds[irand(SND_CAVECREAK1, SND_CAVECREAK3)]; break; //BUGFIX: mxd. Original logic uses "cavecreak1" in all 3 cases.

			default:
				Com_DPrintf("ERROR: invalid ambient sound type %d at %s\n", soundinfo->style, pv(self->origin));
				break;
		}
	}

	// If we have a sound to do, lets do it.
	if (sfx != NULL)
	{
		const int attenuation = (Vec3IsZero(self->origin) ? ATTN_NONE : soundinfo->attenuation); //mxd
		fxi.S_StartSound(self->origin, owner->current.number, CHAN_WEAPON, sfx, soundinfo->volume, attenuation, 0.0f);
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