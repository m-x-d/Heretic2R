//
// fx_Morph.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define MORPH_PARTICLE_VEL		8
#define MORPH_COLOR				0xff00ff00
#define MORPH_ARROW_SPEED		400.0f
#define MORPH_GLOW_INTENSITY	255
#define MORPH_ANGLE_INC			(ANGLE_360 / NUM_OF_OVUMS)

#define FEATHER_VEL				50.0f
#define FEATHER_FLOAT_TIME		15
#define FEATHER_FLOAT_SLOW		(ANGLE_180 / (float)FEATHER_FLOAT_TIME)

static struct model_s* morph_models[4];

void PreCacheMorph(void)
{
	morph_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	morph_models[1] = fxi.RegisterModel("models/objects/eggs/chickenegg/tris.fm");
	morph_models[2] = fxi.RegisterModel("models/objects/feathers/feather1/tris.fm");
	morph_models[3] = fxi.RegisterModel("models/objects/feathers/feather2/tris.fm");
}

static qboolean FXMorphMissileThink(client_entity_t* missile, centity_t* owner)
{
	// Create a new entity for these particles to attach to.
	const int flags = (int)(missile->flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS); //mxd
	client_entity_t* trail = ClientEntity_new(FX_SPELL_MORPHMISSILE, flags, missile->r.origin, NULL, 1000);

	// And give it no owner, so its not deleted when the missile is.
	AddEffect(NULL, trail);

	// Create small particles.
	const int count = GetScaledCount(7, 0.5f);

	vec3_t diff;
	VectorSubtract(missile->r.origin, missile->origin, diff);
	Vec3ScaleAssign(1.0f / (float)count, diff);

	vec3_t cur_pos;
	VectorClear(cur_pos);

	int duration;
	if ((int)r_detail->value >= DETAIL_HIGH)
		duration = 700;
	else if ((int)r_detail->value == DETAIL_NORMAL)
		duration = 600;
	else
		duration = 500;

	for (int i = 0; i < count; i++)
	{
		client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_G, color_white, duration);
		ce->acceleration[2] = 0.0f;

		// Figure out our random velocity.
		VectorRandomSet(ce->origin, MORPH_PARTICLE_VEL);

		// Scale it and make it the origin.
		VectorScale(ce->origin, -1.0f, ce->velocity);
		VectorAdd(cur_pos, ce->origin, ce->origin);

		// Add a fraction of the missile velocity to this particle velocity.
		vec3_t scaled_vel;
		VectorScale(missile->velocity, 0.1f, scaled_vel);
		VectorAdd(scaled_vel, ce->velocity, ce->velocity);

		ce->scale = flrand(3.0f, 6.0f);
		AddParticleToList(trail, ce);

		Vec3SubtractAssign(diff, cur_pos);
	}

	// Remember for even spread of particles.
	VectorCopy(missile->r.origin, missile->origin);

	// Rotate the missile.
	missile->r.angles[0] -= 0.7f;

	return true;
}

// We reflected, create a new missile.
void FXMorphMissile(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	byte yaw;
	byte pitch;

	// Get the initial yaw.
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_MORPHMISSILE].formatString, &yaw, &pitch);

	// Create the client effect with the light on it.
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->radius = 32.0f;
	missile->r.angles[YAW] = (float)yaw * BYTEANGLE_TO_RAD; //mxd. Use macro.
	missile->r.angles[PITCH] = (float)pitch * BYTEANGLE_TO_RAD; //mxd. Use macro.

	// Figure out where we are going.
	DirFromAngles(missile->r.angles, missile->velocity);

	const float arrow_speed = MORPH_ARROW_SPEED * ((flags & CEF_FLAG6) ? 0.5f : 1.0f); //mxd
	Vec3ScaleAssign(arrow_speed, missile->velocity);

	missile->r.model = &morph_models[1]; // Egg model.
	missile->r.scale = 3.0f;
	missile->r.angles[PITCH] = -ANGLE_90; // Set the pitch AGAIN.
	missile->color.c = MORPH_COLOR;
	missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);
	missile->Update = FXMorphMissileThink;

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
		Vec3ScaleAssign(MORPH_ARROW_SPEED, missile->velocity);

		missile->r.model = morph_models + 1;
		missile->r.scale = 3.0;
		missile->r.angles[0] = -1.57;
		missile->Update = FXMorphMissileThink;
		missile->radius = 32.0F;
		missile->color.c = MORPH_COLOR;
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

		glow->color.c = MORPH_COLOR;
		glow->r.color.c = MORPH_COLOR;
		glow->dlight = CE_DLight_new(glow->color, MORPH_GLOW_INTENSITY, -MORPH_GLOW_INTENSITY);
 		glow->d_scale = 1.8;
 		glow->r.scale = 0.5;
		glow->d_alpha = -1.0;
		glow->SpawnInfo = MORPH_GLOW_INTENSITY;
		
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
	color.c = MORPH_COLOR;
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
				self->LifeTime = FEATHER_FLOAT_TIME;
				self->xscale = -self->xscale;
				self->yscale = -self->yscale;
			}
			// add in the feather swing to the origin
			scale = sin(self->LifeTime * FEATHER_FLOAT_SLOW);
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
			self->LifeTime = FEATHER_FLOAT_TIME;
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
			VectorSet(feather->velocity, flrand(-FEATHER_VEL, FEATHER_VEL), flrand(-FEATHER_VEL, FEATHER_VEL), flrand(80,140));
			feather->r.scale = flrand(0.5, 2.5);
			feather->SpawnInfo = 170;
			feather->yscale = flrand(0.1,0.3);
			feather->xscale = flrand(-0.3,0.3);
			AddEffect(NULL, feather);
		}
	}
}


// end
