//
// fx_pespell.c
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

//mxd. Mirrored in m_plagueElf.h.
enum
{
	FX_PE_MAKE_SPELL,
	FX_PE_EXPLODE_SPELL,
	FX_PE_MAKE_SPELL2,
	FX_PE_EXPLODE_SPELL2,
	FX_PE_MAKE_SPELL3,
	FX_PE_EXPLODE_SPELL3,
};

static struct model_s* spell_models[5];

void PrecachePESpell(void)
{
	spell_models[0] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	spell_models[1] = fxi.RegisterModel("Sprites/Spells/spellhands_red.sp2");
	spell_models[2] = fxi.RegisterModel("Sprites/Spells/halo_ind.sp2");
	spell_models[3] = fxi.RegisterModel("Sprites/Spells/spark_ind.sp2");
	spell_models[4] = fxi.RegisterModel("Sprites/fx/core_b.sp2");
}

static qboolean FXPESpellTrailThink(struct client_entity_s* self, centity_t* owner)
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

static void FXPESpellGo(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t vel)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->flags |= CEF_NO_DRAW;
	missile->SpawnInfo = 32;

	VectorCopy(vel, missile->velocity);

	vec3_t dir;
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	if (r_detail->value > DETAIL_NORMAL)
	{
		const paletteRGBA_t light_color = { .c = 0xff20a0ff }; // Orange light.
		missile->dlight = CE_DLight_new(light_color, 120.0f, 0.0f);
	}

	missile->Update = FXPESpellTrailThink;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell.wav"), 1.0f, ATTN_NORM, 0);
}

static void FXPESpellExplode(const int type, const int flags, const vec3_t origin, vec3_t dir)
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
			fxi.S_StartSound(puff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spellhit.wav"), 1.0f, ATTN_NORM, 0);

			const paletteRGBA_t light_color = { .c = 0xff20a0ff }; // Orange light.
			puff->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
			VectorClear(puff->velocity);
		}

		AddEffect(NULL, puff);
	}
}

static qboolean FXPESpell2TrailThink(struct client_entity_s* self, centity_t* owner)
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

static void FXPESpell2Go(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t vel)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->flags |= CEF_NO_DRAW;
	missile->SpawnInfo = 32;

	VectorCopy(vel, missile->velocity);

	vec3_t dir;
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	if (r_detail->value > DETAIL_NORMAL)
	{
		const paletteRGBA_t light_color = { .c = 0xffff0077 }; // Purple.
		missile->dlight = CE_DLight_new(light_color, 120.0f, 0.0f);
	}

	missile->Update = FXPESpell2TrailThink;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell2.wav"), 1.0f, ATTN_NORM, 0);
}

static void FXPESpell2Explode(const int type, const int flags, const vec3_t origin, vec3_t dir)
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
			fxi.S_StartSound(puff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell2hit.wav"), 1.0f, ATTN_NORM, 0);

			const paletteRGBA_t light_color = { .c = 0xffff0077 }; // Purple light.
			puff->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
			VectorClear(puff->velocity);
		}

		AddEffect(NULL, puff);
	}
}

// ************************************************************************************************
// FXPESpell3Explode
// ************************************************************************************************

///////////////////////////////////////
// From CreateEffect FX_WEAPON_PESPELLEXPLODE
///////////////////////////////////////
void FXPESpell3Explode(centity_t *owner,int type,int flags,vec3_t origin, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	byte			powerup = 0;
	float			lightrad;
	
	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}

	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(8, 12), 0.8);
	LightColor.c = 0xffff6611;		// cyan
	lightrad = 150;

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = spell_models + 4;
		SmokePuff->r.scale=flrand(1.0, 1.8);
		SmokePuff->d_scale=-2.0;

		VectorRandomCopy(dir, SmokePuff->velocity, 64.0);
		SmokePuff->acceleration[0] = flrand(-200, 200);
		SmokePuff->acceleration[1] = flrand(-200, 200);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		/*
		SmokePuff->r.color.g = irand(40, 60);
		SmokePuff->r.color.r = irand(245, 255);
		SmokePuff->r.color.b = irand(95, 105);
		*/

		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{//precache this?
			fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/plagueelf/spell3hit.wav"), 
					1, ATTN_NORM, 0);
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}
}

//====================================================================
//	FX_PE_SPELL effect handler
//====================================================================

void FXPESpell(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte			whicheffect = 0;
	vec3_t			vel;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PE_SPELL].formatString, &whicheffect, vel);

	switch(whicheffect)
	{
		case FX_PE_MAKE_SPELL:
			FXPESpellGo(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL:
			FXPESpellExplode(type, flags, origin, vel);
			break;

		case FX_PE_MAKE_SPELL2:
			FXPESpell2Go(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL2:
			FXPESpell2Explode(type, flags, origin, vel);
			break;
		
		case FX_PE_MAKE_SPELL3:
			FXCWStars(owner, type, origin);
			//FXPESpell3Go(owner, type, flags, origin, vel);
			break;

		case FX_PE_EXPLODE_SPELL3:
			FXPESpell3Explode(owner, type, flags, origin, vel);
			break;

		default:
			Com_Printf("Unknown effect type (%d) for FXSsithraArrow\n", whicheffect);
			break;
	}
}