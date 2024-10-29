//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "angles.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "motion.h"
#include "Reference.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define	SMALL_RAD	8
#define	MORPH_GLOW_DUR	10
#define MORPH_COL	0xff00ff00
#define GLOW_INTENSITY 255
#define FX_SPHERE_BASE_RADIUS	89.0
#define FEATH_RAD 50
#define FLOAT_TIME 15.0
#define FLOAT_SLOW 3.14156/FLOAT_TIME
#define MORPH_ANGLE_INC 6.283185 / NUM_OF_OVUMS

#define	NUM_MORPH_MODELS	4
static struct model_s *morph_models[NUM_MORPH_MODELS];
void PreCacheMorph()
{
	morph_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	morph_models[1] = fxi.RegisterModel("models/objects/eggs/chickenegg/tris.fm");
	morph_models[2] = fxi.RegisterModel("models/objects/feathers/feather1/tris.fm");
	morph_models[3] = fxi.RegisterModel("models/objects/feathers/feather2/tris.fm");
}

// -----------------------------------------------------------------------------------------

#define	PART_OFF			5.0
#define NUM_TRAIL_PARTS		6

static qboolean FXMorphMissileThink(client_entity_t *missile, centity_t *owner)
{
	int					i;
	client_particle_t	*ce;
	vec3_t				scaled_vel;
	client_entity_t	*trail;
	int					count;
	paletteRGBA_t	  	color;
	vec3_t				diff, curpos;
	int					dur;

	// create a new entity for these particles to attach to
	trail = ClientEntity_new(FX_SPELL_MORPHMISSILE, missile->flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS, missile->r.origin, NULL, 1000);
	// and give it no onwer, so its not deleted if the missile is
	AddEffect(NULL, trail);

	// ensure we can see this stuff
	color.c = 0xffffffff;

	// create small particles
	count = GetScaledCount(7, 0.5);
	VectorSubtract(missile->r.origin, missile->origin, diff);
	Vec3ScaleAssign((1.0 / count), diff);
	VectorClear(curpos);

	if (r_detail->value >= DETAIL_HIGH)
	{
		dur = 700;
	}
	else
	if (r_detail->value >= DETAIL_NORMAL)
	{
		dur= 600;
	}
	else
	{
		dur= 500;
	}


	for (i=0; i<count; i++)
	{
		ce = ClientParticle_new(PART_16x16_SPARK_G, color, dur);
		ce->acceleration[2] = 0.0; 
		// figure out our random velocity
		VectorSet(ce->origin, flrand(-SMALL_RAD,SMALL_RAD), flrand(-SMALL_RAD,SMALL_RAD), flrand(-SMALL_RAD,SMALL_RAD) );
		// scale it and make it the origin
		VectorScale(ce->origin, -1.0, ce->velocity);
		VectorAdd(curpos, ce->origin, ce->origin);
		// add a fraction of the missile velocity to this particle velocity
		VectorScale(missile->velocity, 0.1, scaled_vel);
		VectorAdd(scaled_vel, ce->velocity, ce->velocity);
		ce->scale = flrand(3, 6);
		AddParticleToList(trail, ce);
		Vec3SubtractAssign(diff, curpos);
	}

	// Remember for even spread of particles
	VectorCopy(missile->r.origin, missile->origin);
	// rotate the missile
	missile->r.angles[0] -= 0.7;
	return(true);
}

#define ARROW_SPEED			400.0F

// we reflected, create a new missile
void FXMorphMissile(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*missile;
	byte				blah,pitch;

	// get the initial Yaw
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_MORPHMISSILE].formatString, &blah,&pitch);

	// create the client effect with the light on it
	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);
	missile->r.angles[YAW] = (blah /255.0) * 6.283185;
	missile->r.angles[PITCH] = (pitch /255.0) * 6.283185;

	// figure out where we are going
	DirFromAngles(missile->r.angles, missile->velocity);
	if (flags & CEF_FLAG6)
		Vec3ScaleAssign(ARROW_SPEED/2, missile->velocity);
	else
		Vec3ScaleAssign(ARROW_SPEED, missile->velocity);

	missile->r.model = morph_models + 1;
	missile->r.scale = 3.0;
	missile->r.angles[0] = -1.57;
	missile->Update = FXMorphMissileThink;
	missile->radius = 32.0F;
	missile->color.c = MORPH_COL;
	missile->dlight = CE_DLight_new(missile->color, 150.0F, 00.0F);
	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/OvumFire.wav"), 1, ATTN_NORM, 0);
}

// initial entry from server - create first object - this has the light on it - but no trail yet
void FXMorphMissile_initial(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*missile;
	client_entity_t		*glow;
	byte				yaw;
	float				yawf;
	short				morpharray[NUM_OF_OVUMS];
	int					i;

	// get the initial Yaw
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_MORPHMISSILE_INITIAL].formatString, 
			&yaw, 
			&morpharray[0],
			&morpharray[1],
			&morpharray[2],
			&morpharray[3],
			&morpharray[4],
			&morpharray[5]);

	yawf = (yaw /255.0) * 6.283185;
	for (i = 0; i<NUM_OF_OVUMS;i++)
	{
		// create the client effect with the light on it
		missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);
		missile->r.angles[YAW] = yawf;

		// figure out where we are going
		DirFromAngles(missile->r.angles, missile->velocity);
		Vec3ScaleAssign(ARROW_SPEED, missile->velocity);

		missile->r.model = morph_models + 1;
		missile->r.scale = 3.0;
		missile->r.angles[0] = -1.57;
		missile->Update = FXMorphMissileThink;
		missile->radius = 32.0F;
		missile->color.c = MORPH_COL;
		missile->dlight = CE_DLight_new(missile->color, 110.0F, 00.0F);
		AddEffect((centity_t *)(&fxi.server_entities[morpharray[i]]), missile);
		yawf += MORPH_ANGLE_INC;
	}

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/OvumFire.wav"), 1, ATTN_NORM, 0);

	if (r_detail->value >= DETAIL_HIGH)
	{
		glow = ClientEntity_new(type, flags, origin, 0, 800);
		glow->r.model = morph_models;

		glow->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		glow->color.c = MORPH_COL;
		glow->r.color.c = MORPH_COL;
		glow->dlight = CE_DLight_new(glow->color, GLOW_INTENSITY, -GLOW_INTENSITY);
 		glow->d_scale = 1.8;
 		glow->r.scale = 0.5;
		glow->d_alpha = -1.0;
		glow->SpawnInfo = MORPH_GLOW_DUR;
		
		AddEffect(NULL, glow);
	}
}


// -----------------------------------------------------------------------------------------

// we hit a wall or an object
#define SMOKE_SPEED 160
void FXMorphExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*dlight;
	paletteRGBA_t		color;
	vec3_t				Dir;
	client_particle_t	*ce;
	int					i, count, dur;
	float				max_rand;

	fxi.GetEffect(owner,flags,clientEffectSpawners[FX_SPELL_MORPHEXPLODE].formatString,Dir);

	// make a bunch of small particles explode out the wall / object
	VectorScale(Dir,SMOKE_SPEED,Dir);
	count = GetScaledCount(40, 0.3);

	if (r_detail->value >= DETAIL_HIGH)
	{
		max_rand = 10.0;
		dur = 600;
	}
	else
	if (r_detail->value >= DETAIL_NORMAL)
	{
		max_rand = 8.0;
		dur= 500;
	}
	else
	{
		max_rand = 7.0;
		dur= 400;
	}

	// create a light at the point of explosion
	dlight = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE | CEF_ADDITIVE_PARTS, origin, NULL, dur);
	color.c = MORPH_COL;
	dlight->dlight = CE_DLight_new(color, 110.0F, 100.0F);
	AddEffect(NULL, dlight);
	
	for(i=0;i<count;i++)
	{
		color.g=irand(200, 255);
		color.r=irand(25, 50);
		color.b=color.r;

		ce = ClientParticle_new(PART_16x16_SPARK_G, color, dur);

		VectorCopy(Dir,ce->velocity);
		
		ce->scale=flrand(3.0, max_rand);

		ce->velocity[0]+=flrand(-SMOKE_SPEED,SMOKE_SPEED);
		ce->velocity[1]+=flrand(-SMOKE_SPEED,SMOKE_SPEED);
		ce->velocity[2]+=flrand(-SMOKE_SPEED,SMOKE_SPEED);

	 	AddParticleToList(dlight, ce);
	}
}

// make the feather float down
static qboolean FXFeatherThink(client_entity_t *self, centity_t *owner)
{
	float scale;

	if (!(--self->SpawnInfo))
		return(false);

	if (self->SpawnInfo < 10)
		self->alpha -= 0.1;

	// is the feather on the way down ?
	if (self->velocity[2] < 2)
	{
		// yes, set gravity much lower.
		self->acceleration[2] = -1.0;
		self->velocity[2] = -13;
		// make the x and z motion much less each time
		self->velocity[0] -= self->velocity[0] * 0.1;
		self->velocity[1] -= self->velocity[1] * 0.1;
		// has the feather already hit the horizontal ?
		if (self->r.angles[PITCH] == 1.2f)
		{
			// if its time, reverse the direction of the swing
			if (!(--self->LifeTime))
			{
				self->LifeTime = FLOAT_TIME;
				self->xscale = -self->xscale;
				self->yscale = -self->yscale;
			}
			// add in the feather swing to the origin
			scale = sin(self->LifeTime * FLOAT_SLOW);
			self->r.origin[0] += (self->xscale * scale);
			self->r.origin[1] += (self->yscale * scale);
			self->r.origin[2] += (1.4 * scale) - 0.7;
		}
		// wait till the feather hits the horizontal by itself
		else
		if ((self->r.angles[PITCH] < 1.3) && (self->r.angles[PITCH] > 1.1) )
		{
			self->r.angles[PITCH] = 1.2;
			self->yscale = flrand(-2.5,2.5);
			self->xscale = flrand(-2.5,2.5);
			self->LifeTime = FLOAT_TIME;
		}
		else
		{
			// not hit the horizontal yet, so keep it spinning
			self->r.angles[YAW] += self->xscale;
			self->r.angles[PITCH] += self->yscale;
			// this is bogus, but has to be done if the above pitch check is going to work
			if (self->r.angles[PITCH] < 0)
				self->r.angles[PITCH] += 6.28;
			else
			if (self->r.angles[PITCH] > 6.28)
				self->r.angles[PITCH] -= 6.28;
		}
		return(true);
	}
	else
	{
		// still on the way up, make the feather turn
		self->r.angles[PITCH] += self->yscale;
		self->r.angles[YAW] += self->xscale;
		// this is bogus, but has to be done if the above pitch check is going to work
		if (self->r.angles[PITCH] < 0)
			self->r.angles[PITCH] += 6.28;
		else
		if (self->r.angles[PITCH] > 6.28)
			self->r.angles[PITCH] -= 6.28;
	}
	return(true);
}

// make the feathers zip out of the carcess and float down
void FXChickenExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*feather;
	int i;

	//NOTENOTE: CEF_FLAG6 denotes we just want to spawn a couple feathers
	if (flags & CEF_FLAG6)
	{
		i = irand(1,2);
		
		while (i--)
		{
			feather = ClientEntity_new(type, flags & ~CEF_OWNERS_ORIGIN , origin, NULL, 40);
			feather->radius = 5.0F;
			feather->r.model = morph_models + irand(2,3);
			feather->r.flags= RF_TRANSLUCENT;
			feather->Update = FXFeatherThink;
			feather->acceleration[2] = irand(-85, -120);
			VectorSet(feather->velocity, flrand(-100,100), flrand(-100,100), flrand(50,150));
			feather->r.scale = flrand(0.5, 1.5);
			feather->SpawnInfo = 170;
			feather->yscale = flrand(0.05,0.2);
			feather->xscale = flrand(-0.2,0.2);
			
			feather->origin[0] += flrand(-8.0F, 8.0F);
			feather->origin[1] += flrand(-8.0F, 8.0F);
			feather->origin[2] += flrand(-8.0F, 8.0F);

			AddEffect(NULL, feather);
		}
	}
	else
	{
		for (i=0; i<20; i++)
		{
			feather = ClientEntity_new(type, flags & ~CEF_OWNERS_ORIGIN , origin, NULL, 40);
			feather->radius = 5.0F;
			feather->r.model = morph_models + irand(2,3);
			feather->r.flags= RF_TRANSLUCENT;
			feather->Update = FXFeatherThink;
			feather->acceleration[2] = -85;
			VectorSet(feather->velocity, flrand(-FEATH_RAD,FEATH_RAD), flrand(-FEATH_RAD,FEATH_RAD), flrand(80,140));
			feather->r.scale = flrand(0.5, 2.5);
			feather->SpawnInfo = 170;
			feather->yscale = flrand(0.1,0.3);
			feather->xscale = flrand(-0.3,0.3);
			AddEffect(NULL, feather);
		}
	}
}


// end
