//
// fx_PlagueElfSpells.c -- mxd. Named fx_pespell.c in original version.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "fx_mork.h" //mxd
#include "g_playstats.h"

#define SPELL_SCALE		0.25f

static struct model_s* spell_models[5];

enum PESpellSoundID_e //mxd
{
	SND_SPELL1,
	SND_SPELL1_HIT,
	SND_SPELL2,
	SND_SPELL2_HIT,
	SND_SPELL3_HIT,

	NUM_SOUNDS
};

static struct sfx_s* spell_sounds[NUM_SOUNDS];

void PreCachePESpell(void) //mxd. Named 'PrecachePESpell' in original logic.
{
	spell_models[0] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	spell_models[1] = fxi.RegisterModel("Sprites/Spells/spellhands_red.sp2");
	spell_models[2] = fxi.RegisterModel("Sprites/Spells/halo_ind.sp2");
	spell_models[3] = fxi.RegisterModel("Sprites/Spells/spark_ind.sp2");
	spell_models[4] = fxi.RegisterModel("Sprites/fx/core_b.sp2");
}

void PreCachePESpellSFX(void) //mxd
{
	spell_sounds[SND_SPELL1] =		fxi.S_RegisterSound("monsters/plagueelf/spell.wav");
	spell_sounds[SND_SPELL1_HIT] =	fxi.S_RegisterSound("monsters/plagueelf/spellhit.wav");
	spell_sounds[SND_SPELL2] =		fxi.S_RegisterSound("monsters/plagueelf/spell2.wav");
	spell_sounds[SND_SPELL2_HIT] =	fxi.S_RegisterSound("monsters/plagueelf/spell2hit.wav");
	spell_sounds[SND_SPELL3_HIT] =	fxi.S_RegisterSound("monsters/plagueelf/spell3hit.wav");
}

static qboolean PESpellTrailThink(struct client_entity_s* self, centity_t* owner)
{
	self->updateTime = 20;

	if (self->SpawnInfo > 9)
		self->SpawnInfo--;

	vec3_t accel_dir;
	VectorCopy(self->velocity, accel_dir);
	VectorNormalize(accel_dir);

	const int count = GetScaledCount(irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_PE_SPELL, 0, self->r.origin, NULL, 1000);

		trail->radius = 20.0f;
		trail->r.model = &spell_models[0]; // Flyingfist sprite.
		trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.scale = SPELL_SCALE + flrand(0.0f, 0.05f);

		COLOUR_SET(trail->r.color, irand(40, 60), irand(245, 255), irand(95, 105)); //mxd. Use macro.
		VectorRandomCopy(self->r.origin, trail->r.origin, flrand(-5.0f, 5.0f));
		VectorScale(accel_dir, flrand(-400.0f, -50.0f), trail->velocity);

		trail->d_alpha = flrand(-2.0f, -1.5f);
		trail->d_scale = flrand(-1.25f, -1.0f);
		trail->updateTime = (int)((trail->alpha * 1000.0f) / -trail->d_scale);

		AddEffect(NULL, trail);
	}

	return true;
}

static void PESpellGo(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t vel)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->flags |= CEF_NO_DRAW;
	missile->SpawnInfo = 32;

	VectorCopy(vel, missile->velocity);

	vec3_t dir;
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	if (R_DETAIL > DETAIL_NORMAL)
	{
		const paletteRGBA_t light_color = { .c = 0xff20a0ff }; // Orange light.
		missile->dlight = CE_DLight_new(light_color, 120.0f, 0.0f);
	}

	missile->Update = PESpellTrailThink;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, spell_sounds[SND_SPELL1], 1.0f, ATTN_NORM, 0.0f);
}

static void PESpellExplode(const int type, const int flags, vec3_t origin, vec3_t dir)
{
	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	Vec3ScaleAssign(32.0f, dir);

	const int count = GetScaledCount(irand(8, 12), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		const int next_think_time = (is_last_puff ? 500 : 1000); //mxd

		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, next_think_time);

		puff->radius = 20.0f;
		puff->r.model = &spell_models[1]; // Spellhands_red sprite.
		puff->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		puff->r.scale = flrand(0.8f, 1.6f);
		puff->d_scale = -2.0f;
		puff->d_alpha = -0.4f;

		VectorRandomCopy(dir, puff->velocity, 64.0f);
		VectorSet(puff->acceleration, flrand(-200.0f, 200.0f), flrand(-200.0f, 200.0f), flrand(-60.0f, -40.0f));
		COLOUR_SET(puff->r.color, irand(40, 60), irand(245, 255), irand(95, 105)); //mxd. Use macro.

		if (is_last_puff)
		{
			fxi.S_StartSound(puff->r.origin, -1, CHAN_WEAPON, spell_sounds[SND_SPELL1_HIT], 1.0f, ATTN_NORM, 0.0f);

			const paletteRGBA_t light_color = { .c = 0xff20a0ff }; // Orange light.
			puff->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
			VectorClear(puff->velocity);
		}

		AddEffect(NULL, puff);
	}
}

static qboolean PESpell2TrailThink(struct client_entity_s* self, centity_t* owner)
{
	self->updateTime = 20;

	if (self->SpawnInfo > 9)
		self->SpawnInfo--;

	const int count = GetScaledCount(irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_PE_SPELL, 0, self->r.origin, NULL, 1000);

		trail->radius = 20.0f;
		trail->r.model = &spell_models[2]; // Indigo halo sprite.
		trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		trail->r.frame = irand(0, 1);
		trail->r.scale = SPELL_SCALE + flrand(0.0f, 0.05f);

		vec3_t accel_dir;
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);
		VectorRandomCopy(self->r.origin, trail->r.origin, flrand(-5.0f, 5.0f));
		VectorScale(accel_dir, flrand(-400.0f, -50.0f), trail->velocity);

		trail->d_alpha = flrand(-2.0f, -1.5f);
		trail->d_scale = flrand(-1.25f, -1.0f);
		trail->updateTime = (int)(trail->alpha * 1000.0f / -trail->d_scale);

		AddEffect(NULL, trail);
	}

	return true;
}

static void PESpell2Go(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t vel)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->flags |= CEF_NO_DRAW;
	missile->SpawnInfo = 32;

	VectorCopy(vel, missile->velocity);

	vec3_t dir;
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	if (R_DETAIL > DETAIL_NORMAL)
	{
		const paletteRGBA_t light_color = { .c = 0xffff0077 }; // Purple.
		missile->dlight = CE_DLight_new(light_color, 120.0f, 0.0f);
	}

	missile->Update = PESpell2TrailThink;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, spell_sounds[SND_SPELL2], 1.0f, ATTN_NORM, 0.0f);
}

static void PESpell2Explode(const int type, const int flags, vec3_t origin, vec3_t dir)
{
	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	Vec3ScaleAssign(32.0f, dir);

	const int count = GetScaledCount(irand(8, 12), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		const int next_think_time = (is_last_puff ? 500 : 1000); //mxd

		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, next_think_time);

		puff->radius = 20.0f;
		puff->r.model = &spell_models[3]; // Indigo spark sprite.
		puff->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		puff->r.scale = flrand(0.8f, 1.6f);
		puff->d_scale = -2.0f;
		puff->d_alpha = -0.4f;

		VectorRandomCopy(dir, puff->velocity, 64.0f);
		VectorSet(puff->acceleration, flrand(-200.0f, 200.0f), flrand(-200.0f, 200.0f), flrand(-60.0f, -40.0f));

		if (is_last_puff)
		{
			fxi.S_StartSound(puff->r.origin, -1, CHAN_WEAPON, spell_sounds[SND_SPELL2_HIT], 1.0f, ATTN_NORM, 0.0f);

			const paletteRGBA_t light_color = { .c = 0xffff0077 }; // Purple light.
			puff->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
			VectorClear(puff->velocity);
		}

		AddEffect(NULL, puff);
	}
}

static void PESpell3Explode(const int type, const int flags, vec3_t origin, vec3_t dir)
{
	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	Vec3ScaleAssign(32.0f, dir);

	const int count = GetScaledCount(irand(8, 12), 0.8f);

	for (int i = 0; i < count; i++)
	{
		const qboolean is_last_puff = (i == count - 1); //mxd
		const int next_think_time = (is_last_puff ? 500 : 1000); //mxd

		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, next_think_time);

		puff->radius = 20.0f;
		puff->r.model = &spell_models[4]; // Blue core sprite.
		puff->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		puff->r.scale = flrand(1.0f, 1.8f);
		puff->d_scale = -2.0f;
		puff->d_alpha = -0.4f;

		VectorRandomCopy(dir, puff->velocity, 64.0f);
		VectorSet(puff->acceleration, flrand(-200.0f, 200.0f), flrand(-200.0f, 200.0f), flrand(-60.0f, -40.0f));

		if (is_last_puff)
		{
			fxi.S_StartSound(puff->r.origin, -1, CHAN_WEAPON, spell_sounds[SND_SPELL3_HIT], 1.0f, ATTN_NORM, 0.0f);

			const paletteRGBA_t light_color = { .c = 0xffff6611 }; // Cyan light.
			puff->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
			VectorClear(puff->velocity);
		}

		AddEffect(NULL, puff);
	}
}

void FXPESpell(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte fx_type = 0;
	vec3_t vel;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PE_SPELL].formatString, &fx_type, vel);

	switch (fx_type)
	{
		case FX_PE_MAKE_SPELL:
			PESpellGo(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL:
			PESpellExplode(type, flags, origin, vel);
			break;

		case FX_PE_MAKE_SPELL2:
			PESpell2Go(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL2:
			PESpell2Explode(type, flags, origin, vel);
			break;

		case FX_PE_MAKE_SPELL3:
			FXCWStars(owner, type, origin);
			break;

		case FX_PE_EXPLODE_SPELL3:
			PESpell3Explode(type, flags, origin, vel);
			break;

		default:
			Com_Printf("Unknown effect type (%d) for FXPESpell\n", fx_type);
			break;
	}
}