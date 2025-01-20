//
// fx_Maceball.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "ce_Dlight.h"
#include "q_Sprite.h"
#include "g_playstats.h"
#include "fx_debris.h"

#define BALL_RADIUS				0.15f
#define BALL_MAX_RADIUS			(BALL_RADIUS * 6.0f)
#define BALL_GROWTH				0.05f
#define BALL_BOUNCE_LIFETIME	500

#define NUM_RIPPER_PUFFS		12
#define RIPPER_PUFF_ANGLE		(ANGLE_360 / (float)NUM_RIPPER_PUFFS)
#define RIPPER_RING_VEL			96.0f

#define MACEBALL_RING_VEL		64.0f
#define MACEBALL_SPARK_VEL		128.0f
#define MACEBALL_EXPLOSION_VEL	128.0f

static struct model_s* mace_models[7];

void PreCacheMaceball(void)
{
	mace_models[0] = fxi.RegisterModel("sprites/spells/maceball.sp2");
	mace_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	mace_models[2] = fxi.RegisterModel("sprites/fx/neon.sp2");
	mace_models[3] = fxi.RegisterModel("models/spells/maceball/tris.fm");
	mace_models[4] = fxi.RegisterModel("sprites/fx/ballstreak.sp2");
	mace_models[5] = fxi.RegisterModel("sprites/spells/patball.sp2");
	mace_models[6] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
}

#pragma region ========================== MACE BALL ==========================

static qboolean FXMaceballThink(struct client_entity_s* self, centity_t* owner)
{
	self->dlight->intensity = 150.0f + cosf((float)fxi.cl->time * 0.01f) * 20.0f;
	self->r.angles[2] += ANGLE_30;

	if (self->r.scale >= BALL_MAX_RADIUS)
		self->d_scale = 0.0f;

	return true;
}

void FXMaceball(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	client_entity_t* ball = ClientEntity_new(type, flags, origin, NULL, 100);

	ball->r.model = &mace_models[0]; // Maceball sprite.
	ball->r.scale = BALL_RADIUS;
	ball->d_scale = BALL_GROWTH;
	ball->color.c = 0xff00ffff;
	ball->dlight = CE_DLight_new(ball->color, 150.0f, 0.0f);
	ball->Update = FXMaceballThink;

	AddEffect(owner, ball);
}

void FXMaceballBounce(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t normal;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MACEBALLBOUNCE].formatString, normal);

	// Take the normal and find two "axis" vectors that are in the plane the normal defines.
	vec3_t up;
	PerpendicularVector(up, normal);

	vec3_t right;
	CrossProduct(up, normal, right);

	client_entity_t* hit_fx = ClientEntity_new(type, flags, origin, NULL, BALL_BOUNCE_LIFETIME);
	hit_fx->r.flags = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	hit_fx->flags |= CEF_NO_DRAW | CEF_ADDITIVE_PARTS;
	hit_fx->radius = BALL_RADIUS;
	VectorScale(normal, MACEBALL_SPARK_VEL, hit_fx->velocity); // This velocity is used by the sparks.
	AddEffect(NULL, hit_fx);

	VectorScale(normal, 8.0f, normal);

	// Draw a circle of expanding lines.
	vec3_t last_vel;
	VectorScale(right, MACEBALL_RING_VEL, last_vel);
	const int ring_flags = CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS; //mxd
	float cur_yaw = 0.0f;

	for (int i = 0; i < NUM_RIPPER_PUFFS; i++)
	{
		cur_yaw += RIPPER_PUFF_ANGLE;

		client_entity_t* ring = ClientEntity_new(type, ring_flags, origin, NULL, 500);
		ring->r.model = &mace_models[2]; // Neon-green sprite.
		ring->r.frame = 1;
		ring->r.spriteType = SPRITE_LINE;
		ring->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		ring->radius = 64.0f;
		ring->r.scale = 0.5f;
		ring->d_scale = 16.0f;
		ring->alpha = 0.1f;
		ring->d_alpha = 4.0f;

		// The startpos and startvel comes from the last velocity.
		VectorCopy(last_vel, ring->velocity);
		VectorScale(ring->velocity, -1.0f, ring->acceleration);
		VectorMA(origin, 0.01f, ring->velocity, ring->r.startpos); // Move the line out a bit to avoid a zero-length line.

		VectorScale(up, RIPPER_RING_VEL * sinf(cur_yaw), ring->velocity2);
		VectorMA(ring->velocity2, MACEBALL_RING_VEL * cosf(cur_yaw), right, ring->velocity2);

		VectorScale(ring->velocity2, -1.0f, ring->acceleration2);
		VectorMA(origin, 0.01f, ring->velocity2, ring->r.endpos); // Move the line out a bit to avoid a zero-length line.

		// Finally, copy the last velocity we used.
		VectorCopy(ring->velocity2, last_vel);

		// NOW apply the extra directional velocity to force it slightly away from the surface.
		VectorAdd(ring->velocity, normal, ring->velocity);
		VectorAdd(ring->velocity2, normal, ring->velocity2);

		AddEffect(NULL, ring);

		// Now spawn a particle quick to save against the nasty joints (ugh).
		client_particle_t* p = ClientParticle_new(PART_16x16_SPARK_G, color_white, 500);
		VectorCopy(ring->r.startpos, p->origin);
		VectorCopy(ring->velocity, p->velocity);
		VectorCopy(ring->acceleration, p->acceleration);
		p->scale = 0.5f;
		p->d_scale = 16.0f;
		p->color.a = 1;
		p->d_alpha = 1024.0f;

		AddParticleToList(ring, p);
	}

	// Add a few sparks to the impact.
	for (int i = 0; i < 8; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_G, color_white, 500);

		VectorRandomSet(spark->velocity, MACEBALL_SPARK_VEL);
		spark->d_alpha = flrand(-768.0f, -512.0f);
		spark->scale = 4.0f;
		spark->d_scale = flrand(-10.0f, -8.0f);
		spark->acceleration[2] = -2.0f * MACEBALL_SPARK_VEL;

		AddParticleToList(hit_fx, spark);
	}
}

void FXMaceballExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MACEBALLEXPLODE].formatString, dir);

	// Create an expanding ball of gre.
	client_entity_t* explosion = ClientEntity_new(type, CEF_DONT_LINK | CEF_ADDITIVE_PARTS, origin, dir, 750);
	explosion->r.model = &mace_models[3]; // Maceball model.
	explosion->r.flags |= RF_TRANSLUCENT;
	explosion->r.scale = 0.17f;
	explosion->d_scale = -0.17f;
	explosion->d_alpha = -1.4f;
	explosion->radius = 64.0f;
	VectorScale(dir, 8.0f, explosion->velocity);
	explosion->color = color_white;
	AddEffect(NULL, explosion);

	for (int i = 0; i < 32; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_G, explosion->color, 1000);

		VectorSet(spark->velocity,
			flrand(-MACEBALL_EXPLOSION_VEL, MACEBALL_EXPLOSION_VEL),
			flrand(-MACEBALL_EXPLOSION_VEL, MACEBALL_EXPLOSION_VEL),
			flrand(0, MACEBALL_EXPLOSION_VEL));

		spark->d_alpha = flrand(-320.0f, -256.0f);
		spark->scale = 8.0f;
		spark->d_scale = flrand(-10.0f, -8.0f);
		spark->acceleration[2] = -2.0f * MACEBALL_SPARK_VEL;

		AddParticleToList(explosion, spark);
	}

	// Spawn some chunks too.
	const vec3_t mins = { BALL_RADIUS, BALL_RADIUS, BALL_RADIUS };
	FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6 | CEF_FLAG7 | CEF_FLAG8), origin, 5, MAT_METAL, dir, 80000.0f, mins, 1.5f, false);
}

#pragma endregion

#pragma region ========================== RIPPER BALL ==========================

static qboolean FXRipperExplodeBallThink(struct client_entity_s *self, centity_t *owner)
{
	client_entity_t *trail;
	vec3_t	diff, curpos;
	float	scale;
	int i;

	VectorScale(self->direction, -6, diff);
	VectorCopy(self->r.origin, curpos);
	scale = 0.8;
	for (i=0; i<4; i++)
	{
		trail = ClientEntity_new(FX_WEAPON_RIPPEREXPLODE, 0, curpos, NULL, 500);
		trail->r.model = mace_models + 6;
		trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		VectorCopy(self->velocity, trail->velocity);
		VectorScale(trail->velocity, -1.0, trail->acceleration);
		trail->r.scale = scale;
		trail->d_scale = -scale;
		trail->alpha = 0.3;
		trail->d_alpha = -0.6;
		trail->radius = 10.0;

		AddEffect(NULL, trail);

		VectorAdd(curpos, diff, curpos);
		scale -= 0.12;
	}

	return(true);
}


// Create Effect FX_WEAPON_RIPPEREXPLODE
void FXRipperExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t	*ripper;	
	paletteRGBA_t	color = {255, 255, 255, 255};
	short			ballarray[8];
	byte			byaw;
	float			curyaw;
	vec3_t			casterpos;
	int				i, num;
	float			length;
	vec3_t			lastvel, diff, curpos;
	client_entity_t	*flash, *ring;
	client_particle_t *spark;


	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_RIPPEREXPLODE].formatString, 
			casterpos, 
			&byaw, 
			&ballarray[0],
			&ballarray[1],
			&ballarray[2],
			&ballarray[3],
			&ballarray[4],
			&ballarray[5],
			&ballarray[6],
			&ballarray[7]);

	// Convert from a byte back to radians
	curyaw = ((float)byaw)*((2*M_PI)/256.0);

	//
	// Throw out a bunch o' balls
	//
	for (i=0; i<RIPPER_BALLS; i++)
	{
		// Create the ball
		ripper = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

		ripper->r.model = mace_models;
		ripper->r.flags = RF_TRANSLUCENT;		// Use the alpha channel

		// Set up the velocities
		VectorSet(ripper->velocity, cos(curyaw), sin(curyaw), 0.0);
		VectorCopy(ripper->velocity, ripper->direction);
		vectoangles(ripper->velocity, ripper->r.angles);
		Vec3ScaleAssign(RIPPER_EXPLODE_SPEED, ripper->velocity);

		// Set up the basic attributes
		ripper->r.scale = 0.25;
		ripper->r.color = color;
		ripper->radius = 10.0F;
		ripper->Update = FXRipperExplodeBallThink;

		// Add to the entity passed in, not the "owner".
		assert(ballarray[i]);
		AddEffect((centity_t *)(&fxi.server_entities[ballarray[i]]), ripper);

		curyaw += RIPPER_BALL_ANGLE;
	}

	//
	// Draw the impact graphic
	//
	flash = ClientEntity_new(type, 0, origin, NULL, 50);
	flash->r.model = mace_models + 1;
	flash->r.frame = 1;
	flash->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	flash->radius = 20.0;

	flash->r.scale = 0.75;
	flash->d_scale = -1.0;
	flash->d_alpha = -1.4;
	fxi.S_StartSound(flash->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/RipperImpact.wav"), 1, ATTN_NORM, 0);
	flash->dlight = CE_DLight_new(color, 150.0f, -100.0f);
	flash->lastThinkTime = fxi.cl->time + 750;
//	flash->Update = FXRipperExplodeLightThink;

	AddEffect(NULL, flash);

	// Draw a circle of expanding lines.
	curyaw = 0;
	VectorSet(lastvel, RIPPER_RING_VEL, 0.0, 0.0);
	for(i = 0; i < NUM_RIPPER_PUFFS; i++)
	{
		curyaw+=RIPPER_PUFF_ANGLE;

		ring = ClientEntity_new(type, CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS, 
									origin, NULL, 750);
		ring->r.model = mace_models + 2;
		ring->r.frame = 1;
		ring->r.spriteType = SPRITE_LINE;
		ring->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		ring->radius = 64.0;
		
		// The startpos and startvel comes from the last velocity.
		VectorCopy(lastvel, ring->velocity);
		VectorScale(ring->velocity, -1.0, ring->acceleration);
		VectorMA(origin, .01, ring->velocity, ring->r.startpos);	// Move the line out a bit to avoid a zero-length line.

		// The endpos is calculated from the current angle.
		VectorSet(ring->velocity2, RIPPER_RING_VEL*cos(curyaw), RIPPER_RING_VEL*sin(curyaw), 0.0);
		VectorScale(ring->velocity2, -1.0, ring->acceleration2);
		VectorMA(origin, .01, ring->velocity2, ring->r.endpos);	// Move the line out a bit to avoid a zero-length line.

		// Finally, copy the last velocity we used.
		VectorCopy(ring->velocity2, lastvel);

		// NOW apply the extra directional velocity.
		VectorAdd(ring->velocity, flash->velocity, ring->velocity);
		VectorAdd(ring->velocity2, flash->velocity, ring->velocity2);

		ring->r.scale = .5;
		ring->d_scale = 32.0;
		ring->alpha = 0.1;
		ring->d_alpha = 3.0;

		AddEffect(NULL, ring);

		// Now spawn a particle quick to save against the nasty joints (ugh).
		spark = ClientParticle_new(PART_16x16_SPARK_G, color, 750);
		VectorCopy(ring->r.startpos, spark->origin);
		VectorCopy(ring->velocity, spark->velocity);
		VectorCopy(ring->acceleration, spark->acceleration);
		spark->scale = 0.5;
		spark->d_scale = 32.0;
		spark->color.a = 1;
		spark->d_alpha = 768.0;

		AddParticleToList(ring, spark);
	}

	// Get the length for the firing streak.
	VectorSubtract(origin, casterpos, diff);
	length = VectorLength(diff);

	if (length > 8.0)
	{
		// Draw the streak from the caster to the impact point.
		flash = ClientEntity_new(FX_WEAPON_RIPPEREXPLODE, CEF_AUTO_ORIGIN, casterpos, NULL, 500);
		flash->r.model = mace_models + 4;
		flash->r.spriteType = SPRITE_LINE;
		flash->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorCopy(casterpos, flash->r.endpos);
		VectorCopy(origin, flash->r.startpos);
		flash->radius = length*0.5;

		flash->r.scale = 8.0;
		flash->d_scale = -8.0;
		flash->alpha = 0.5;
		flash->d_alpha = -1.0;

		AddEffect(NULL, flash);

		// Draw some flashy bits along the line for thickness
		num = (int)(length/32.0);
		VectorCopy(casterpos, curpos);
		VectorScale(diff, 1/(float)num, diff);
		if (num>40)
			num=40;
		for (i=0; i<num; i++)
		{
			flash = ClientEntity_new(FX_WEAPON_RIPPEREXPLODE, 0, curpos, NULL, 500);
			flash->r.model = mace_models + 5;
			flash->r.frame = 1;
			flash->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

			flash->r.scale = .16;
			flash->d_scale = -.16;
			flash->alpha = 0.5;
			flash->d_alpha = -1.0;

			AddEffect(NULL, flash);

			VectorAdd(curpos, diff, curpos);
		}
	}
	
}

#pragma endregion