//
// fx_waterwake.c
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
#include "angles.h"
#include "g_playstats.h"
#include "random.h"

#define	NUM_WAKE_MODELS	2
static struct model_s *wake_models[NUM_WAKE_MODELS];
void PreCacheWake()
{
	wake_models[0] = fxi.RegisterModel("sprites/fx/wake_add.sp2");
	wake_models[1] = fxi.RegisterModel("sprites/fx/wfall.sp2");
}

// -----------------------------------------------------------------



void FXWaterWake(centity_t *owner, int type, int flags, vec3_t origin)
{
	short			CreatorEntNum;
	centity_t		*CreatorClientEntity;
	float			YawAngle;
	vec3_t			dir = {0.0, 0.0, 1.0}, Velocity, fwd, right;
	client_entity_t	*self;
	float			len, vel;
	byte			blah;
	int				i, parts;
	client_particle_t *part;
	paletteRGBA_t	color={0xff, 0xff, 0xff, 0xff};

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WATER_WAKE].formatString, &CreatorEntNum, &blah, Velocity);

	YawAngle = (blah /255.0) * 6.283185;

	CreatorClientEntity=&fxi.server_entities[CreatorEntNum];

	// Create a water wake.

	self=ClientEntity_new(FX_WATER_WAKE,flags,origin,dir,5000);

	self->r.model = wake_models;
	self->r.origin[1]=CreatorClientEntity->origin[1];
	self->r.origin[0]=CreatorClientEntity->origin[0];
	self->alpha=0.7;
	self->r.scale=0.3;
	self->r.flags |= RF_FIXED|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	self->r.angles[YAW]=YawAngle;

	vel = VectorNormalize(Velocity);

	len = vel-30.0;
	if(len > 50.0)
	{
		len = 50.0;
	}

	VectorScale(Velocity, len, self->velocity);

	self->velocity[2] = 0.0;

	self->d_scale=1.1;
	self->d_alpha=-0.8;
		
	AddEffect(NULL, self);

	if (vel > 320.0) 
		vel=320.0;
	// splashies too
	if (r_detail->value >= DETAIL_HIGH)
		parts = (int)(vel/32.0);
	else if (r_detail->value >= DETAIL_NORMAL)
		parts = (int)(vel/64.0);
	else
		parts = 0;

	if (parts<4)
		return;

	VectorSet(fwd, -cos(YawAngle), -sin(YawAngle), 0);
	VectorSet(right, -sin(YawAngle), cos(YawAngle), 0);

	for (i=0; i<parts; i++)
	{
		part = ClientParticle_new(PART_32x32_WFALL, color, 500);
		VectorScale(right, flrand(-12.0,12.0), part->origin);
		VectorScale(part->origin, 0.01*vel, part->velocity);
		VectorMA(part->velocity, flrand(0.2*vel, 0.5*vel), fwd, part->velocity);
		part->velocity[2] += flrand(30.0, 40.0);
		part->acceleration[2] = -320;

		part->scale = 4.0;
		part->d_scale = vel/5.0;

		AddParticleToList(self, part);

		part = ClientParticle_new(PART_4x4_WHITE, color, 750);
		VectorScale(right, flrand(-0.2*vel, 0.2*vel), part->velocity);
		VectorMA(part->velocity, flrand(0, 0.5*vel), fwd, part->velocity);
		part->velocity[2] += flrand(.1*vel, .2*vel);
		part->acceleration[2] = -320;

		part->scale = 1.0;
		part->d_scale = -1.0;
		part->color.a = 128;
		part->d_alpha = flrand(-200, -160);

		AddParticleToList(self, part);
	}

}
