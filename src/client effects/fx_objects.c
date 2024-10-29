//
// fx_objects.c
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
#include "fx_debris.h"

#define BARREL_EXPLODE_SPEED	80.0
#define BARREL_EXPLODE_BALLS	3
#define BARREL_EXPLODE_BITS		16
#define BARREL_EXPLODE_GRAVITY	(-320.0)
#define BARREL_EXPLODE_SCALE	14.0
#define BARREL_RADIUS			24

#define	NUM_OBJECT_MODELS		2

static struct model_s *obj_models[NUM_OBJECT_MODELS];

void PreCacheObjects()
{
	obj_models[0] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
	obj_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

extern qboolean FXPhoenixExplosionBallThink(client_entity_t *explosion, centity_t *owner);
extern qboolean FXPhoenixExplosionSmallBallThink(client_entity_t *explosion, centity_t *owner);
extern client_entity_t *CreatePhoenixSmallExplosion(vec3_t ballorigin);




// ************************************************************************************************
// FXBarrelExplode
// ************************************************************************************************
// Create Effect FX_BARREL_EXPLODE
void FXBarrelExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*explosion, *subexplosion;
	paletteRGBA_t		color;
	vec3_t				dir={0, 0, 1};
	client_particle_t	*spark;
	int					i;
	float				ballnum;
	vec3_t				mins={BARREL_RADIUS, BARREL_RADIUS, BARREL_RADIUS};

	// Create three smaller explosion spheres.
	for (i=0; i < BARREL_EXPLODE_BALLS; i++)
	{
		ballnum = i;
		subexplosion = CreatePhoenixSmallExplosion(origin);
		VectorSet(subexplosion->velocity, 
						flrand(-BARREL_EXPLODE_SPEED, BARREL_EXPLODE_SPEED),
						flrand(-BARREL_EXPLODE_SPEED, BARREL_EXPLODE_SPEED),
						flrand(-BARREL_EXPLODE_SPEED, BARREL_EXPLODE_SPEED));
		subexplosion->r.scale = 0.1;
		subexplosion->d_scale = 3.0 + ballnum;
		subexplosion->d_alpha = -1.5 - 0.5*ballnum;

		AddEffect(NULL, subexplosion);
	}

	// Create the main big explosion sphere.
	explosion = ClientEntity_new(type, flags, origin, NULL, 17);
	explosion->r.model = obj_models;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->flags |= CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA;
	explosion->alpha = 0.1;
	explosion->r.scale= 0.1;
	explosion->d_alpha = 3.0;
	explosion->d_scale=5.0;
	explosion->radius=128;
	explosion->startTime = fxi.cl->time;
	explosion->lastThinkTime = fxi.cl->time;
	explosion->velocity2[YAW] = flrand(-M_PI, M_PI);
	explosion->velocity2[PITCH] = flrand(-M_PI, M_PI);

	color.c = 0xff00ffff;
	explosion->dlight = CE_DLight_new(color, 150.0F, 0.0F);
	explosion->Update = FXPhoenixExplosionBallThink;
	AddEffect(NULL, explosion);
	
	// Add some glowing blast particles.
	VectorScale(dir,BARREL_EXPLODE_SPEED,dir);
	for(i = 0; i < BARREL_EXPLODE_BITS; i++)
	{
		spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 2000);
		VectorSet(spark->velocity,	flrand(-BARREL_EXPLODE_SPEED, BARREL_EXPLODE_SPEED), 
									flrand(-BARREL_EXPLODE_SPEED, BARREL_EXPLODE_SPEED), 
									flrand(-BARREL_EXPLODE_SPEED, BARREL_EXPLODE_SPEED));
		VectorAdd(spark->velocity, dir, spark->velocity);
		spark->acceleration[2] = BARREL_EXPLODE_GRAVITY;
		spark->scale = BARREL_EXPLODE_SCALE;
		spark->d_scale = flrand(-20.0, -10.0);
		spark->d_alpha = flrand(-400.0, -320.0);
		spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(explosion, spark);
	}

	// ...and a big-ass flash
	explosion = ClientEntity_new(-1, flags, origin, NULL, 250);
	explosion->r.model = obj_models + 1;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 1;
	explosion->radius=128;
	explosion->r.scale=2.0;
	explosion->d_alpha=-4.0;
	explosion->d_scale=-4.0;
	AddEffect(NULL, explosion);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/PhoenixHit.wav"), 1, ATTN_NORM, 0);
}