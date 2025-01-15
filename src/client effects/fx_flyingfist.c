//
// fx_flyingfist.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "Random.h"
#include "Utilities.h"
#include "fx_debris.h"
#include "g_playstats.h"

#define	FIST_SCALE				0.25f
#define	FIST_WIMPY_SCALE		0.15f
#define FIST_BLAST_VEL			64.0f
#define FIST_POWER_BLAST_VEL	200.0f

static struct model_s* fist_models[3];

void PreCacheFist(void)
{
	fist_models[0] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	fist_models[1] = fxi.RegisterModel("Sprites/Spells/spellhands_red.sp2");
	fist_models[2] = fxi.RegisterModel("models/spells/meteorbarrier/tris.fm");
}

static qboolean FXFlyingFistTrailThink(struct client_entity_s* self, centity_t* owner)
{
	self->updateTime = 20;

	if (self->SpawnInfo > 9)
		self->SpawnInfo--;

	qboolean is_wimpy = false;
	float trailscale = FIST_SCALE;
	int count = GetScaledCount(irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8f);

	if (self->flags & CEF_FLAG8)
	{
		is_wimpy = true;
		trailscale = FIST_WIMPY_SCALE;
		count /= 2;
	}

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail_ent = ClientEntity_new(FX_WEAPON_FLYINGFIST, 0, self->r.origin, NULL, 1000);
		trail_ent->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		vec3_t accel_dir;
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);

		if (self->flags & CEF_FLAG7)
		{
			trail_ent->r.model = &fist_models[1];
			trail_ent->r.scale = 3.0f * (trailscale + flrand(0.0f, 0.05f));
			VectorRandomCopy(self->r.origin, trail_ent->r.origin, flrand(-8.0f, 8.0f));
			VectorScale(accel_dir, flrand(-100.0f, -400.0f), trail_ent->velocity);
		}
		else
		{
			trail_ent->r.model = &fist_models[0];
			trail_ent->r.scale = trailscale + flrand(0.0f, 0.05f);
			VectorRandomCopy(self->r.origin, trail_ent->r.origin, flrand(-5.0f, 5.0f));
			VectorScale(accel_dir, flrand(-50.0f, -400.0f), trail_ent->velocity);
		}

		if (is_wimpy) // Wimpy shot, because no mana.
			VectorScale(trail_ent->velocity, 0.5f, trail_ent->velocity);

		trail_ent->d_alpha = flrand(-1.5f, -2.0f);
		trail_ent->d_scale = flrand(-1.0f, -1.25f);
		trail_ent->updateTime = (int)(trail_ent->alpha * 1000.0f / -trail_ent->d_scale);
		trail_ent->radius = 20.0f;

		AddEffect(NULL, trail_ent);
	}

	return true;
}

// ************************************************************************************************
// FXFlyingFist
// ************************************************************************************************

////////////////////////////////////
// From CreateEffect FX_WEAPON_FLYINGFIST
////////////////////////////////////
void FXFlyingFist(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			vel, dir;
	client_entity_t	*missile;	
	paletteRGBA_t	LightColor;
	float			lightsize;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FLYINGFIST].formatString, vel);
	if (flags & CEF_FLAG6)
		Vec3ScaleAssign(FLYING_FIST_SPEED/2,vel);
	else
		Vec3ScaleAssign(FLYING_FIST_SPEED,vel);

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	if (flags & CEF_FLAG7)
	{	// Powered up fireball.  Use a meteor model.
		missile->r.model = fist_models + 2;
		missile->r.skinnum = 1;
		if (flags & CEF_FLAG8)		// Wimpy shot, because didn't have mana.
			missile->r.scale = 1.0;
		else
			missile->r.scale = 1.5;

		LightColor.c = 0xff0000ff;		// Red light
		lightsize = 160.0;
	}
	else
	{	// Just a normal fireball.
		missile->flags |= CEF_NO_DRAW;
		LightColor.c = 0xff2040ff;		// Orange light
		lightsize = 120.0;
	}
	
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 128;
	missile->dlight = CE_DLight_new(LightColor, lightsize, 0.0f);
	missile->Update = FXFlyingFistTrailThink;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);
}



// ************************************************************************************************
// FXFlyingFistExplode
// ************************************************************************************************

///////////////////////////////////////
// From CreateEffect FX_WEAPON_FLYINGFISTEXPLODE
///////////////////////////////////////
void FXFlyingFistExplode(centity_t *owner,int type,int flags,vec3_t origin)
{
	vec3_t			dir, mins;
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	byte			powerup = 0, wimpy=0;
	float			lightrad;
	float			blastvel;
	float			volume=1.0;
	
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FLYINGFISTEXPLODE].formatString, dir);

	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}

	if (flags&CEF_FLAG7)
	{
		powerup=1;
	}

	if (flags&CEF_FLAG8)
	{
		wimpy=1;
		volume=0.75;
	}

//	fxi.GetEffect(owner, flags, "xb", dir, &powerup);
	Vec3ScaleAssign(32.0, dir);

	if(powerup)
	{
		i = GetScaledCount(irand(12, 16), 0.8);
		LightColor.c = 0xff0000ff;
		if (wimpy)
			lightrad = 160;
		else
			lightrad = 200;
	}
	else
	{
		i = GetScaledCount(irand(8, 12), 0.8);
		LightColor.c = 0xff2040ff;
		if (wimpy)
			lightrad = 120;
		else
			lightrad = 150;
	}

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = fist_models + 1;
		if (powerup)
		{	// Meteor impact!
			SmokePuff->d_scale=-2.0;
			blastvel = FIST_POWER_BLAST_VEL;
			if (wimpy)
			{
				blastvel*=0.3;
				SmokePuff->r.scale=flrand(0.8,1.4);
			}
			else
			{
				SmokePuff->r.scale=flrand(1.2,2.0);
			}

			VectorRandomCopy(dir, SmokePuff->velocity, blastvel);
			SmokePuff->velocity[2] += 100.0;
			SmokePuff->acceleration[2] = -400.0;
		}
		else
		{	// Non-powered up.
			SmokePuff->d_scale=-2.0;
			blastvel = FIST_BLAST_VEL;
			if (wimpy)
			{
				blastvel*=0.5;
				SmokePuff->r.scale=flrand(0.5,1.0);
			}
			else
			{
				SmokePuff->r.scale=flrand(0.8,1.6);
			}

			VectorRandomCopy(dir, SmokePuff->velocity, blastvel);
			SmokePuff->acceleration[0] = flrand(-200, 200);
			SmokePuff->acceleration[1] = flrand(-200, 200);
			SmokePuff->acceleration[2] = flrand(-40, -60);
		}

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{
			if (powerup)
			{
				fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/FireballPowerImpact.wav"), 
						volume, ATTN_NORM, 0);
			}
			else
			{
				fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/FlyingFistImpact.wav"), 
						volume, ATTN_NORM, 0);
			}
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad, -50.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}

	if (powerup)
	{	// Meteor throws out chunks.
		VectorSet(dir, 0.0, 0.0, 1.0);
		VectorSet(mins, 2.0, 2.0, 2.0);	// because SpawnChunks needs a value for bounding box

		if (wimpy)	// No mana meteors are wimpy! - clear out cef_flag# stuff, means different stuff to debris
			FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6|CEF_FLAG7|CEF_FLAG8), origin, 5, MAT_GREYSTONE, dir, 80000.0f, mins, 0.5, false);
		else
			FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6|CEF_FLAG7|CEF_FLAG8), origin, 5, MAT_GREYSTONE, dir, 80000.0f, mins, 1.0, false);
	}
}