//
// fx_ssithra.c
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

#define ARROW_DELTA_FORWARD	8.0
#define ARROW_DELTA_THETA	0.12
#define	ARROW_SPIRAL_RAD		0.75
#define	ARROW_SCALE			0.25

#define	NUM_ARROW_MODELS		3

static struct model_s *arrow_models[NUM_ARROW_MODELS];

void PrecacheSsithraArrow()
{
	arrow_models[0] = fxi.RegisterModel("sprites/fx/steam.sp2");//unpowered trail
	arrow_models[1] = fxi.RegisterModel("sprites/fx/fire.sp2");//powered trail
	arrow_models[2] = fxi.RegisterModel("models/objects/projectiles/sitharrow/tris.fm");//projectile model
}

enum 
{
	FX_SS_MAKE_ARROW,
	FX_SS_MAKE_ARROW2,
	FX_SS_EXPLODE_ARROW,
	FX_SS_EXPLODE_ARROW2
};
// ************************************************************************************************
// FXSsithraArrowTrailThink
// ************************************************************************************************

static qboolean FXSsithraArrowTrailThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t	*TrailEnt;
	vec3_t			accel_dir;
	int				i;

	self->updateTime = 20;

	self->r.angles[ROLL] += 10;

	if(self->SpawnInfo > 9)
		self->SpawnInfo--;

	i = GetScaledCount( irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8 );
	while(i--)
	{
		TrailEnt = ClientEntity_new(FX_SSITHRA_ARROW, 0, self->r.origin, NULL, 1000);
	
		VectorCopy(self->velocity, accel_dir);
		VectorNormalize(accel_dir);

		if (self->flags & CEF_FLAG7)
		{//powered
			TrailEnt->r.flags |= (RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
			TrailEnt->r.model = arrow_models + 1;
			TrailEnt->r.scale = (ARROW_SCALE + flrand(0.0, 0.05));
			VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-8.0, 8.0));
			VectorScale(accel_dir, flrand(-100.0, -400.0), TrailEnt->velocity);
		}
		else
		{
			//make this use tinting instead of darken?
			TrailEnt->r.flags |= RF_TRANSLUCENT;//darken
			TrailEnt->r.color.r = 75;
			TrailEnt->r.color.g = 50;
			TrailEnt->r.color.b = 100;
			TrailEnt->r.color.a = 100;
			TrailEnt->r.model = arrow_models;
			TrailEnt->r.scale = ARROW_SCALE + flrand(-0.2, 0.2);
			VectorRandomCopy(self->r.origin, TrailEnt->r.origin, flrand(-5.0, 5.0));
			VectorScale(accel_dir, flrand(-50.0, -400.0), TrailEnt->velocity);
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
// FXSsithraArrow
// ************************************************************************************************

////////////////////////////////////
// From CreateEffect FX_WEAPON_SSITHRAARROW
////////////////////////////////////
void FXDoSsithraArrow(centity_t *owner, int type, int flags, vec3_t origin, vec3_t vel)
{
	vec3_t			dir;
	client_entity_t	*missile;	
	paletteRGBA_t	LightColor;
	float			lightsize;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->r.flags |= RF_GLOW;
	missile->r.model = arrow_models + 2;
	missile->r.skinnum = 0;
	missile->r.scale = 1.0;
	LightColor.c = 0xff2040ff;		// Orange light
	lightsize = 120.0;
	
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 128;
	missile->dlight = CE_DLight_new(LightColor, lightsize, 0.0f);
	missile->Update = FXSsithraArrowTrailThink;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);
}

void FXDoSsithraArrow2(centity_t *owner, int type, int flags, vec3_t origin, vec3_t vel)
{
	vec3_t			dir;
	client_entity_t	*missile;	
	paletteRGBA_t	LightColor;
	float			lightsize;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->r.flags |= RF_GLOW;
	missile->flags |= CEF_FLAG7;
	missile->r.model = arrow_models + 2;
	missile->r.skinnum = 0;
	missile->r.scale = 1.5;
	LightColor.c = 0xff0000ff;		// Red light
	lightsize = 160.0;
	
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 128;
	missile->dlight = CE_DLight_new(LightColor, lightsize, 0.0f);
	missile->Update = FXSsithraArrowTrailThink;

	missile->SpawnInfo = 32;

	AddEffect(owner, missile);
}



// ************************************************************************************************
// FXSsithraArrowExplode
// ************************************************************************************************

///////////////////////////////////////
// From CreateEffect FX_WEAPON_SSITHRAARROWEXPLODE
///////////////////////////////////////
void FXSsithraArrowBoom(centity_t *owner,int type,int flags,vec3_t origin, vec3_t dir)
{
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	float			lightrad;
	
	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(8, 12), 0.8);
	LightColor.c = 0xff2040ff;
	lightrad = 150;

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = arrow_models;
		SmokePuff->r.scale=flrand(0.8,1.6);
		SmokePuff->d_scale=-2.0;

		VectorRandomCopy(dir, SmokePuff->velocity, 64.0);
		SmokePuff->acceleration[0] = flrand(-200, 200);
		SmokePuff->acceleration[1] = flrand(-200, 200);
		SmokePuff->acceleration[2] = flrand(-40, -60);

		//make this use tinting instead of darken?
		SmokePuff->r.flags |= RF_TRANSLUCENT;
		SmokePuff->r.color.r = 75;
		SmokePuff->r.color.g = 50;
		SmokePuff->r.color.b = 100;
		SmokePuff->r.color.a = 100;

		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{
			fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/SsithraArrowImpact.wav"), 
					1, ATTN_NORM, 0);
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}
}

void FXSsithraArrow2Boom(centity_t *owner,int type,int flags,vec3_t origin, vec3_t dir)
{
	vec3_t			mins;
	client_entity_t	*SmokePuff;
	int				i;
	paletteRGBA_t	LightColor;
	float			lightrad;
	
	Vec3ScaleAssign(32.0, dir);

	i = GetScaledCount(irand(12, 16), 0.8);
	LightColor.c = 0xff0000ff;
	lightrad = 200;

	while(i--)
	{
		if (!i)
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,500);
		else
			SmokePuff=ClientEntity_new(type,flags,origin,NULL,1000);

		SmokePuff->r.model = arrow_models + 1;
		SmokePuff->r.scale=flrand(1.2,2.0);
		SmokePuff->d_scale=-2.0;

		VectorRandomCopy(dir, SmokePuff->velocity, 200);
		SmokePuff->velocity[2] += 100.0;
		SmokePuff->acceleration[2] = -400.0;

		SmokePuff->r.flags |=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		SmokePuff->r.frame=0;

		SmokePuff->d_alpha= -0.4;
			
		SmokePuff->radius=20.0;

		if(!i)
		{
			fxi.S_StartSound(SmokePuff->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/FireballPowerImpact.wav"), 
					1, ATTN_NORM, 0);
			SmokePuff->dlight=CE_DLight_new(LightColor,lightrad,0.0f);
			VectorClear(SmokePuff->velocity);
		}	

		AddEffect(NULL,SmokePuff);
	}

	VectorSet(dir, 0.0, 0.0, 1.0);
	VectorSet(mins, 2.0, 2.0, 2.0);	// because SpawnChunks needs a value for bounding box
	//clear out cef_flag# stuff, means different stuff to debris
	FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6|CEF_FLAG7|CEF_FLAG8), origin, 5, MAT_GREYSTONE, dir, 80000.0f, mins, 1.0, false);
}

void FXSsithraArrow(centity_t *owner, int type, int flags, vec3_t origin)
{
	byte			whicheffect = 0;
	vec3_t			vel;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SSITHRA_ARROW].formatString, &whicheffect, vel);

	switch(whicheffect)
	{
		case FX_SS_MAKE_ARROW:
			FXDoSsithraArrow(owner, type, flags, origin, vel);
			break;

		case FX_SS_MAKE_ARROW2:
			FXDoSsithraArrow2(owner, type, flags, origin, vel);
			break;

		case FX_SS_EXPLODE_ARROW:
			FXSsithraArrowBoom(owner, type, flags, origin, vel);
			break;

		case FX_SS_EXPLODE_ARROW2:
			FXSsithraArrow2Boom(owner, type, flags, origin, vel);
			break;

		default:
			Com_Printf("Unknown effect type (%d) for FXSsithraArrow\n", whicheffect);
			break;
	}
}