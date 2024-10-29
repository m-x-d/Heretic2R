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
#include "q_Sprite.h"
#include "g_playstats.h"
#include "fx_debris.h"

#define BALL_RADIUS		0.15
#define NUM_RIPPER_PUFFS	12
#define RIPPER_PUFF_ANGLE	((360.0*ANGLE_TO_RAD)/(float)NUM_RIPPER_PUFFS)
#define RIPPER_RING_VEL		96.0
#define MACEBALL_RING_VEL	64.0
#define MACEBALL_SPARK_VEL	128.0

#define	NUM_MACE_MODELS		7
static struct model_s *mace_models[NUM_MACE_MODELS];
void PreCacheMaceball()
{
	mace_models[0] = fxi.RegisterModel("sprites/spells/maceball.sp2");
	mace_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
	mace_models[2] = fxi.RegisterModel("sprites/fx/neon.sp2");
	mace_models[3] = fxi.RegisterModel("models/spells/maceball/tris.fm");
	mace_models[4] = fxi.RegisterModel("sprites/fx/ballstreak.sp2");
	mace_models[5] = fxi.RegisterModel("sprites/spells/patball.sp2");
	mace_models[6] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
}

// --------------------------------------------------------------
// Need to create some pretty effect here

#define BALL_GROWTH		0.05
#define BALL_MAX		(6.0 * BALL_RADIUS)

static qboolean FXMaceballThink(struct client_entity_s *self, centity_t *owner)
{
	self->dlight->intensity = 150.0 + (cos(fxi.cl->time * 0.01) * 20.0);
	self->r.angles[2] += ANGLE_30;
	if(self->r.scale >= BALL_MAX)
	{
		self->d_scale = 0.0;
	}
	return(true);
}

void FXMaceball(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;

	glow = ClientEntity_new(type, flags, origin, 0, 100);

	glow->r.model = mace_models;
	glow->r.scale = BALL_RADIUS;
	glow->d_scale = BALL_GROWTH;
	glow->color.c = 0xff00ffff;
	glow->dlight = CE_DLight_new(glow->color, 150.0F, 0.0F);
	glow->Update = FXMaceballThink;
	
	AddEffect(owner, glow);
}

// -----------------------------------------------------------------------------------------

#define NUM_BOUNCE_PARTS	8
#define BOUNCE_LIFETIME		500
#define BOUNCE_PART_SPEED	40.0

void FXMaceballBounce(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ring, *hitfx;
	client_particle_t	*spark;
	paletteRGBA_t		color;
	int					i;
	vec3_t				norm, up, right, lastvel;
	float				curyaw;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MACEBALLBOUNCE].formatString, norm);

	color.c = 0xffffffff;

	// Take the normal and find two "axis" vectors that are in the plane the normal defines
	PerpendicularVector(up, norm);
	CrossProduct(up, norm, right);

	hitfx = ClientEntity_new(type, flags, origin, NULL, BOUNCE_LIFETIME);
	hitfx->r.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA;
	hitfx->flags |= CEF_NO_DRAW | CEF_ADDITIVE_PARTS;
	hitfx->radius = BALL_RADIUS;
	VectorScale(norm, MACEBALL_SPARK_VEL, hitfx->velocity);		// This velocity is used by the sparks.
	AddEffect(NULL, hitfx);

	VectorScale(norm, 8.0, norm);
	color.c = 0xffffffff;

	// Draw a circle of expanding lines.
	curyaw = 0;
	VectorScale(right, MACEBALL_RING_VEL, lastvel);
	for(i = 0; i < NUM_RIPPER_PUFFS; i++)
	{
		curyaw+=RIPPER_PUFF_ANGLE;

		ring = ClientEntity_new(type, CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS, 
									origin, NULL, 500);
		ring->r.model = mace_models + 2;
		ring->r.frame = 1;
		ring->r.spriteType = SPRITE_LINE;
		ring->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		ring->radius = 64.0;
		
		// The startpos and startvel comes from the last velocity.
		VectorCopy(lastvel, ring->velocity);
		VectorScale(ring->velocity, -1.0, ring->acceleration);
		VectorMA(origin, .01, ring->velocity, ring->r.startpos);	// Move the line out a bit to avoid a zero-length line.

		VectorScale(up, RIPPER_RING_VEL*sin(curyaw), ring->velocity2);
		VectorMA(ring->velocity2, MACEBALL_RING_VEL*cos(curyaw), right, ring->velocity2);

		VectorScale(ring->velocity2, -1.0, ring->acceleration2);
		VectorMA(origin, .01, ring->velocity2, ring->r.endpos);	// Move the line out a bit to avoid a zero-length line.

		// Finally, copy the last velocity we used.
		VectorCopy(ring->velocity2, lastvel);

		// NOW apply the extra directional velocity to force it slightly away from the surface.
		VectorAdd(ring->velocity, norm, ring->velocity);
		VectorAdd(ring->velocity2, norm, ring->velocity2);

		ring->r.scale = .5;
		ring->d_scale = 16.0;
		ring->alpha = 0.1;
		ring->d_alpha = 4.0;

		AddEffect(NULL, ring);

		// Now spawn a particle quick to save against the nasty joints (ugh).
		spark = ClientParticle_new(PART_16x16_SPARK_G, color, 500);
		VectorCopy(ring->r.startpos, spark->origin);
		VectorCopy(ring->velocity, spark->velocity);
		VectorCopy(ring->acceleration, spark->acceleration);
		spark->scale = 0.5;
		spark->d_scale = 16.0;
		spark->color.a = 1;
		spark->d_alpha = 1024.0;

		AddParticleToList(ring, spark);
	}

	// Add a few sparks to the imapct
	for (i=0; i<8; i++)
	{
		spark = ClientParticle_new(PART_16x16_SPARK_G, color, 500);
		VectorSet(spark->velocity, 
					flrand(-MACEBALL_SPARK_VEL, MACEBALL_SPARK_VEL),
					flrand(-MACEBALL_SPARK_VEL, MACEBALL_SPARK_VEL),
					flrand(-MACEBALL_SPARK_VEL, MACEBALL_SPARK_VEL));
		spark->d_alpha = flrand(-768.0, -512.0);
		spark->scale = 4.0;
		spark->d_scale = flrand(-10.0, -8.0);
		spark->acceleration[2] = -2.0*MACEBALL_SPARK_VEL;

		AddParticleToList(hitfx, spark);
	}
}

// ****************************************************************************
// FXMaceballExplode -
// ****************************************************************************

#define MACEBALL_EXP_VEL	128.0
void FXMaceballExplode(centity_t *owner,int type,int flags,vec3_t origin)
{
	vec3_t			dir;
	client_entity_t	*explosion;
	client_particle_t *spark;
	int				i;
	vec3_t			mins={BALL_RADIUS, BALL_RADIUS, BALL_RADIUS};

	fxi.GetEffect(owner,flags,clientEffectSpawners[FX_WEAPON_MACEBALLEXPLODE].formatString,dir);

	// Create an expanding ball of gre.

	explosion=ClientEntity_new(type,CEF_DONT_LINK|CEF_ADDITIVE_PARTS,origin,dir,750);
	explosion->r.model = mace_models+3;
	explosion->r.flags |= RF_TRANSLUCENT;
	explosion->r.scale = 0.17;
	explosion->d_scale = -0.17;
	explosion->d_alpha = -1.4;
	explosion->radius = 64.0;
	VectorScale(dir, 8.0, explosion->velocity);
	explosion->color.c=0xffffffff;
	AddEffect(NULL,explosion);

	for (i=0; i<32; i++)
	{
		spark = ClientParticle_new(PART_16x16_SPARK_G, explosion->color, 1000);
		VectorSet(spark->velocity, 
					flrand(-MACEBALL_EXP_VEL, MACEBALL_EXP_VEL),
					flrand(-MACEBALL_EXP_VEL, MACEBALL_EXP_VEL),
					flrand(0, MACEBALL_EXP_VEL));
		spark->d_alpha = flrand(-320.0, -256.0);
		spark->scale = 8.0;
		spark->d_scale = flrand(-10.0, -8.0);
		spark->acceleration[2] = -2.0*MACEBALL_SPARK_VEL;

		AddParticleToList(explosion, spark);
	}		

	// Spawn some chunks too
	FXDebris_SpawnChunks(type, flags & ~(CEF_FLAG6|CEF_FLAG7|CEF_FLAG8), origin, 5, MAT_METAL, dir, 80000.0f, mins, 1.5, false);
}



// end






// ************************************************************************************************
// FXRipperBall
// ************************************************************************************************


// ************************************************************************************************
// FXRipperExplode
// ---------------------
// ************************************************************************************************

static qboolean FXRipperExplodeLightThink(struct client_entity_s *self, centity_t *owner)
{
	if (fxi.cl->time > self->lastThinkTime)
		return(false);
	
	if(self->dlight->intensity > 0.0F)
		self->dlight->intensity -= 10.0F;

	return(true);
}



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


// end