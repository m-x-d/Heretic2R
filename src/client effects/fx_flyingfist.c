//
// fx_flyingfist.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "random.h"
#include "Utilities.h"
#include "fx_debris.h"
#include "g_playstats.h"

#define FIST_DELTA_FORWARD	8.0
#define FIST_DELTA_THETA	0.12
#define	FIST_SPIRAL_RAD		0.75
#define	FIST_SCALE			0.25
#define	FIST_WIMPY_SCALE	0.15
#define FIST_BLAST_VEL		64.0
#define FIST_POWER_BLAST_VEL	200.0

#define	NUM_FIST_MODELS		3

static struct model_s *fist_models[NUM_FIST_MODELS];

void PreCacheFist()
{
	fist_models[0] = fxi.RegisterModel("Sprites/Spells/flyingfist.sp2");
	fist_models[1] = fxi.RegisterModel("Sprites/Spells/spellhands_red.sp2");
	fist_models[2] = fxi.RegisterModel("models/spells/meteorbarrier/tris.fm");
}

// ************************************************************************************************
// FXFlyingFistTrailThink
// ************************************************************************************************

static qboolean FXFlyingFistTrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	vec3_t			accel_dir;
	int				i;
	qboolean		wimpy=false;
	float			trailscale=FIST_SCALE;

	self->updateTime = 20;

	if(self->SpawnInfo > 9)
		self->SpawnInfo--;

	i = GetScaledCount( irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8 );
	if (self->flags & CEF_FLAG8)
	{
		wimpy=true;
		i /= 2.0;
		trailscale = FIST_WIMPY_SCALE;
	}

	while(i--)
	{
		TrailEnt = ClientEntity_new(FX_WEAPON_FLYINGFIST, 0, self->r.origin, NULL, 1000);
		TrailEnt->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);

		if (self->flags & CEF_FLAG7)
		{
			TrailEnt->r.model = fist_models + 1;
			TrailEnt->r.scale = 3.0 * (trailscale + flrand(0.0, 0.05));
			VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-8.0, 8.0));
			VectorScale(accel_dir, flrand(-100.0, -400.0), TrailEnt->velocity);
		}
		else
		{
			TrailEnt->r.model = fist_models;
			TrailEnt->r.scale = trailscale + flrand(0.0, 0.05);
			VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-5.0, 5.0));
			VectorScale(accel_dir, flrand(-50.0, -400.0), TrailEnt->velocity);
		}

		if (wimpy)
		{	// Wimpy shot, because no mana.
			VectorScale(TrailEnt->velocity, 0.5, TrailEnt->velocity);
		}

		TrailEnt->d_alpha = flrand(-1.5, -2.0);
		TrailEnt->d_scale = flrand(-1.0, -1.25);
		TrailEnt->updateTime = (TrailEnt->alpha * 1000.0) / -TrailEnt->d_scale;
		TrailEnt->radius = 20.0;
		
		AddEffect(NULL,TrailEnt);
	}

	return(true);
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