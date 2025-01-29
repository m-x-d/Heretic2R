//
// fx_ssithra.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "fx_debris.h"

#define SSARROW_TRAIL_SCALE		0.25f

//mxd. Mirrored in m_plagueSsithra.c.
enum
{
	FX_SS_MAKE_ARROW,
	FX_SS_MAKE_ARROW2,
	FX_SS_EXPLODE_ARROW,
	FX_SS_EXPLODE_ARROW2
};

static struct model_s* arrow_models[3];

void PrecacheSsithraArrow(void)
{
	arrow_models[0] = fxi.RegisterModel("sprites/fx/steam.sp2"); // Unpowered trail.
	arrow_models[1] = fxi.RegisterModel("sprites/fx/fire.sp2"); // Powered trail.
	arrow_models[2] = fxi.RegisterModel("models/objects/projectiles/sitharrow/tris.fm"); // Projectile model.
}

static qboolean FXSsithraArrowTrailThink(struct client_entity_s* self, centity_t* owner)
{
	self->updateTime = 20;
	self->r.angles[ROLL] += 10;

	if (self->SpawnInfo > 9)
		self->SpawnInfo--;

	vec3_t trail_vel;
	VectorCopy(self->velocity, trail_vel);
	VectorNormalize(trail_vel);

	const int count = GetScaledCount(irand(self->SpawnInfo >> 3, self->SpawnInfo >> 2), 0.8f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SSITHRA_ARROW, 0, self->r.origin, NULL, 1000);

		if (self->flags & CEF_FLAG7)
		{
			// Powered.
			trail->r.model = &arrow_models[1]; // fire sprite.
			trail->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			trail->r.scale = SSARROW_TRAIL_SCALE + flrand(0.0f, 0.05f);

			VectorRandomCopy(self->r.origin, trail->r.origin, flrand(-8.0f, 8.0f));
			VectorScale(trail_vel, flrand(-400.0f, -100.0f), trail->velocity);
		}
		else
		{
			// Make this use tinting instead of darken?
			trail->r.model = &arrow_models[0]; // steam sprite.
			trail->r.flags = RF_TRANSLUCENT; // Darken.
			trail->r.scale = SSARROW_TRAIL_SCALE + flrand(-0.2f, 0.2f);
			COLOUR_SETA(trail->r.color, 75, 50, 100, 100); //mxd. Use macro.

			VectorRandomCopy(self->r.origin, trail->r.origin, flrand(-5.0f, 5.0f));
			VectorScale(trail_vel, flrand(-400.0f, -50.0f), trail->velocity);
		}

		trail->d_alpha = flrand(-1.5f, -2.0f);
		trail->d_scale = flrand(-1.0f, -1.25f);
		trail->updateTime = (int)(trail->alpha * 1000.0f / -trail->d_scale);
		trail->radius = 20.0f;

		AddEffect(NULL, trail);
	}

	return true;
}

static void FXDoSsithraArrow(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t velocity)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->r.model = &arrow_models[2]; // sitharrow model.
	missile->r.flags = RF_GLOW;

	VectorCopy(velocity, missile->velocity);

	vec3_t dir;
	VectorNormalize2(velocity, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->dlight = CE_DLight_new(color_orange, 120.0f, 0.0f);
	missile->SpawnInfo = 32;
	missile->Update = FXSsithraArrowTrailThink;

	AddEffect(owner, missile);
}

static void FXDoSsithraArrow2(centity_t* owner, const int type, const int flags, const vec3_t origin, const vec3_t velocity)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 128.0f;
	missile->flags |= CEF_FLAG7;
	missile->r.model = &arrow_models[2]; // sitharrow model.
	missile->r.flags = RF_GLOW;
	missile->r.scale = 1.5f;

	VectorCopy(velocity, missile->velocity);

	vec3_t dir;
	VectorNormalize2(velocity, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->dlight = CE_DLight_new(color_red, 160.0f, 0.0f);
	missile->SpawnInfo = 32;
	missile->Update = FXSsithraArrowTrailThink;

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