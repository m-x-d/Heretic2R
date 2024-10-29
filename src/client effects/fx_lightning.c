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

#define LIGHTNING_WIDTH		6.0
#define LIGHTNING_WIDTH_MULT	1.4
#define LIGHTNING_POWER_WIDTH_MULT	2.0
#define LIGHTNING_JOINT_SCALE	(1.0/12.0)

#define LIGHTNING_TYPE_BLUE		0
#define LIGHTNING_TYPE_RED		1
#define LIGHTNING_TYPE_GREEN	2
#define LIGHTNING_JOINT_OFFSET	3

#define MIN_LIGHTNING_PARTS		5
#define MAX_LIGHTNING_SEGMENT	64.0

#define LIGHTNING_INTERVAL	100
#define NUM_LIGHTNING_RINGBITS	12
#define LIGHTNING_RING_VEL		320
#define LIGHTNING_RING_UPVEL	32

#define	NUM_LIGHTNING_MODELS	7
static struct model_s *lightning_models[NUM_LIGHTNING_MODELS];

void PreCacheLightning()
{
	lightning_models[LIGHTNING_TYPE_BLUE] = fxi.RegisterModel("sprites/fx/lightning.sp2");
	lightning_models[LIGHTNING_TYPE_RED] = fxi.RegisterModel("sprites/fx/rlightning.sp2");
	lightning_models[LIGHTNING_TYPE_GREEN] = fxi.RegisterModel("sprites/fx/plightning.sp2");
	lightning_models[LIGHTNING_TYPE_BLUE + LIGHTNING_JOINT_OFFSET] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");
	lightning_models[LIGHTNING_TYPE_RED + LIGHTNING_JOINT_OFFSET] = fxi.RegisterModel("sprites/spells/spark_red.sp2");
	lightning_models[LIGHTNING_TYPE_GREEN + LIGHTNING_JOINT_OFFSET] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
	lightning_models[6] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

// --------------------------------------------------------------

client_entity_t *MakeLightningPiece(int type, float width, vec3_t start, vec3_t end, float radius)
{
	client_entity_t *lightning;

	lightning = ClientEntity_new(FX_LIGHTNING, CEF_DONT_LINK, start, NULL, 250);
	lightning->r.model = lightning_models + type;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = width;
	lightning->radius = radius;
	lightning->alpha = 0.95;
	lightning->d_alpha = -4.0;
	VectorCopy(start, lightning->r.startpos);
	VectorCopy(end, lightning->r.endpos);
 	lightning->r.spriteType = SPRITE_LINE;
	AddEffect(NULL, lightning); 

	lightning = ClientEntity_new(FX_LIGHTNING, CEF_DONT_LINK, start, NULL, 400);
	lightning->r.model = lightning_models + type;
	lightning->r.frame = 1;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = width * LIGHTNING_WIDTH_MULT;
	lightning->radius = radius;
	lightning->alpha = 0.5;
	lightning->d_alpha = -1.250;
	VectorCopy(start, lightning->r.startpos);
	VectorCopy(end, lightning->r.endpos);
	lightning->r.spriteType = SPRITE_LINE;
	AddEffect(NULL, lightning); 

	// Add a little ball at the joint (end)

	lightning = ClientEntity_new(FX_LIGHTNING, CEF_DONT_LINK, start, NULL, 250);
	lightning->r.model = lightning_models + type + LIGHTNING_JOINT_OFFSET;
	lightning->r.frame = 0;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = width * LIGHTNING_JOINT_SCALE;
	lightning->radius = radius;
	lightning->alpha = 0.95;
	lightning->d_alpha = -2.0;
	lightning->d_scale = -2.0;
	VectorCopy(start, lightning->r.startpos);
	VectorCopy(end, lightning->r.endpos);
	AddEffect(NULL, lightning); 

	return(lightning);
}

// Occasional lightning bolt strikes inside
void DoLightning(vec3_t groundpos, vec3_t airpos)
{
	vec3_t curpos, lastpos, top, bottom, refpoint, diffpos, rand;
	float scale;
	int i;

	VectorSet(top, flrand(-RED_RAIN_RADIUS, RED_RAIN_RADIUS), flrand(-RED_RAIN_RADIUS, RED_RAIN_RADIUS), 0);
	VectorAdd(airpos, top, top);
	VectorSet(bottom, flrand(-RED_RAIN_RADIUS, RED_RAIN_RADIUS), flrand(-RED_RAIN_RADIUS, RED_RAIN_RADIUS), 0);
	VectorAdd(groundpos, bottom, bottom);

	VectorSubtract(top, bottom, diffpos);
	VectorScale(diffpos, 0.2, diffpos);
	scale = (airpos[2] - groundpos[2]) / 10.0;

	VectorCopy(bottom, lastpos);
	VectorCopy(bottom, refpoint);
	for (i=0; i<5; i++)
	{
		VectorAdd(refpoint, diffpos, refpoint);
		VectorSet(rand, flrand(-scale, scale), flrand(-scale, scale), flrand(-scale, scale));
		VectorAdd(refpoint, rand, curpos);
		MakeLightningPiece(LIGHTNING_TYPE_RED, LIGHTNING_WIDTH, curpos, lastpos, scale);
		VectorCopy(curpos, lastpos);
	}
}


// Directed lightning bolt
void LightningBolt(int model, float width, vec3_t startpos, vec3_t endpos)
{
	vec3_t curpos, lastpos, refpoint, diffpos, rand;
	vec3_t fwd, right, up;
	float segmult, length, variance;
	int i, segments;

	VectorSubtract(endpos, startpos, diffpos);
	length = VectorLength(diffpos);
	if (length < MIN_LIGHTNING_PARTS*MAX_LIGHTNING_SEGMENT)
	{
		segments = MIN_LIGHTNING_PARTS;
		segmult = 1.0/(float)MIN_LIGHTNING_PARTS;
	}
	else
	{
		segments = (int)(length / (float)MAX_LIGHTNING_SEGMENT);
		segmult = 1.0/(float)segments;
	}
	VectorNormalize2(diffpos, fwd);
	PerpendicularVector(up, fwd);
	CrossProduct(up, fwd, right);

	VectorScale(diffpos, segmult, diffpos);
	variance = 0.4 * length*segmult;

	VectorCopy(startpos, lastpos);
	VectorCopy(startpos, refpoint);
	for (i=0; i<segments-1; i++)
	{
		VectorAdd(refpoint, diffpos, refpoint);
		VectorScale(up, flrand(-variance, variance), rand);
		VectorMA(rand, flrand(-variance, variance), right, rand);
		VectorAdd(refpoint, rand, curpos);
		MakeLightningPiece(model, width, curpos, lastpos, variance);
		VectorCopy(curpos, lastpos);
	}

	// Draw the last point to the destination, no variance
	MakeLightningPiece(model, width, endpos, lastpos, variance);
}


static qboolean FXLightningThink(client_entity_t *thinker, centity_t *owner)
{
	if (fxi.cl->time - thinker->lastThinkTime < thinker->LifeTime)
	{
		LightningBolt(thinker->SpawnInfo, thinker->xscale, thinker->r.startpos, thinker->r.endpos);
		return true;
	}
	else
	{
		return false;
	}
}


// This is from creating the effect FX_LIGHTNING
void FXLightning(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	vec3_t				target, diffpos;
	byte				width, duration;
	client_entity_t		*lightning;
		
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_LIGHTNING].formatString, target, &width, &duration);

	if (duration > 1)	// duration is in 1/10 of a second
	{	// Create a client effect to zap over time.
		lightning = ClientEntity_new(FX_LIGHTNING, Flags | CEF_NO_DRAW, Origin, NULL, LIGHTNING_INTERVAL);
		VectorSubtract(target, Origin, diffpos);
		lightning->radius = VectorLength(diffpos)*0.5;
		VectorCopy(Origin, lightning->r.startpos);
		VectorCopy(target, lightning->r.endpos);
		lightning->lastThinkTime = fxi.cl->time;
		lightning->LifeTime = ((int)duration)*100+250;
		lightning->Update = FXLightningThink;
		lightning->xscale = (float)width;
		if (Flags & CEF_FLAG6)
			lightning->SpawnInfo = LIGHTNING_TYPE_RED;
		else
			lightning->SpawnInfo = LIGHTNING_TYPE_BLUE;
		AddEffect(NULL, lightning); 
	}
	
	// If flagged, do red lightning.
	if (Flags & CEF_FLAG6)
		LightningBolt(LIGHTNING_TYPE_RED, (float)width, Origin, target);
	else if (Flags & CEF_FLAG7)
		LightningBolt(LIGHTNING_TYPE_GREEN, (float)width, Origin, target);	// powered up rain lightning
	else 
		LightningBolt(LIGHTNING_TYPE_BLUE, (float)width, Origin, target);	// Normal, blue lightning

}


// This is from creating the effect FX_POWER_LIGHTNING
void FXPowerLightning(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	vec3_t				target, diffpos;
	byte				width;
	client_entity_t		*lightning;
	client_particle_t	*spark;
	int					i;
	float				length;
	float				curang, degreeinc;	
	vec3_t				lastvel, upvel;
		
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_POWER_LIGHTNING].formatString, target, &width);

	VectorSubtract(target, Origin, diffpos);
	length = VectorLength(diffpos);

	// Big ol' monster zapper
	lightning = ClientEntity_new(FX_POWER_LIGHTNING, CEF_AUTO_ORIGIN, Origin, NULL, 750);
	lightning->r.model = lightning_models + LIGHTNING_TYPE_GREEN;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = width;
	lightning->d_scale = -0.5*width;
	lightning->radius = length;
	lightning->alpha = 0.95;
	lightning->d_alpha = -1.5;
	VectorCopy(Origin, lightning->r.startpos);
	VectorCopy(target, lightning->r.endpos);
 	lightning->r.spriteType = SPRITE_LINE;
	AddEffect(NULL, lightning); 

	// Halo around the lightning
	lightning = ClientEntity_new(FX_POWER_LIGHTNING, CEF_AUTO_ORIGIN, Origin, NULL, 1000);
	lightning->r.model = lightning_models + LIGHTNING_TYPE_GREEN;
	lightning->r.frame = 1;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = width * LIGHTNING_POWER_WIDTH_MULT;
	lightning->d_scale = -0.5*width;
	lightning->radius = length;
	lightning->alpha = 0.5;
	lightning->d_alpha = -0.5;
	VectorCopy(Origin, lightning->r.startpos);
	VectorCopy(target, lightning->r.endpos);
	lightning->r.spriteType = SPRITE_LINE;
	AddEffect(NULL, lightning); 

	// Big ol' flash at source to cover up the flatness of the line's end.
	lightning = ClientEntity_new(FX_POWER_LIGHTNING, CEF_ADDITIVE_PARTS, Origin, NULL, 750);
	lightning->r.model = lightning_models + 6;		// The bright halo model
	lightning->r.frame = 1;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = 0.75;
	lightning->d_scale = 2.0;
	lightning->radius = 128.0;
	lightning->alpha = 0.95;
	lightning->d_alpha = -1.333;
	lightning->color.c = 0xffffffff;
	AddEffect(NULL, lightning);

	// Now add a bunch of sparks to the source too to add interest.
	for(i=0; i<8; i++)
	{	// Half green, half yellow particles
		if (i&0x01)
			spark = ClientParticle_new(PART_16x16_SPARK_Y, lightning->color, 1000);
		else
			spark = ClientParticle_new(PART_16x16_SPARK_G, lightning->color, 1000);
		VectorSet(spark->velocity, flrand(-80,80), flrand(-80,80), flrand(-80,80));
		VectorScale(spark->velocity, -1.0, spark->acceleration);
		spark->scale = flrand(20.0, 32.0);
		spark->d_scale = -spark->scale;
		spark->d_alpha = flrand(-384.0, -256);
		AddParticleToList(lightning, spark);
	}

	lightning = ClientEntity_new(FX_POWER_LIGHTNING, CEF_ADDITIVE_PARTS, target, NULL, 1000);
	lightning->r.model = lightning_models + 6;		// The bright halo model
	lightning->r.origin[2] += 8.0;
	lightning->r.frame = 1;
	lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.scale = 2.0;
	lightning->d_scale = -2.0;
	lightning->radius = 128.0;
	lightning->alpha = 0.95;
	lightning->d_alpha = -1.333;
	lightning->color.c = 0xffffffff;
	AddEffect(NULL, lightning);

	// And yet more sparks to the hit point too to add interest.
	for(i=0; i<12; i++)
	{	// Half green, half yellow particles
		if (i&0x01)
			spark = ClientParticle_new(PART_16x16_SPARK_Y, lightning->color, 1000);
		else
			spark = ClientParticle_new(PART_16x16_SPARK_G, lightning->color, 1000);
		VectorSet(spark->velocity, 
					flrand(-LIGHTNING_RING_VEL,LIGHTNING_RING_VEL), 
					flrand(-LIGHTNING_RING_VEL,LIGHTNING_RING_VEL), 
					flrand(0,32));
		VectorScale(spark->velocity, -1.0, spark->acceleration);
		spark->scale = flrand(20.0, 32.0);
		spark->d_scale = -spark->scale;
		spark->d_alpha = flrand(-384.0, -256);
		AddParticleToList(lightning, spark);
	}

	// Draw a circle of expanding lines.
	curang = 0;
	degreeinc = (360.0*ANGLE_TO_RAD)/(float)NUM_LIGHTNING_RINGBITS;
	VectorSet(lastvel, LIGHTNING_RING_VEL, 0.0, 0.0);
	VectorSet(upvel, 0, 0, 32.0);
	for(i = 0; i < NUM_LIGHTNING_RINGBITS; i++)
	{
		curang+=degreeinc;

		lightning = ClientEntity_new(FX_LIGHTNING, 
						CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS, 
						target, NULL, 750);
		lightning->r.model = lightning_models + LIGHTNING_TYPE_GREEN;
		lightning->r.frame = 1;		// Just use the halo
		lightning->r.spriteType = SPRITE_LINE;
		lightning->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		lightning->radius = 64.0;
		
		// The startpos and startvel comes from the last velocity.
		VectorCopy(lastvel, lightning->velocity);
		VectorScale(lightning->velocity, -1.0, lightning->acceleration);
		VectorMA(target, .01, lightning->velocity, lightning->r.startpos);	// Move the line out a bit to avoid a zero-length line.

		// The endpos is calculated from the current angle.
		VectorSet(lightning->velocity2, LIGHTNING_RING_VEL*cos(curang), LIGHTNING_RING_VEL*sin(curang), 0.0);
		VectorScale(lightning->velocity2, -1.0, lightning->acceleration2);
		VectorMA(target, .01, lightning->velocity2, lightning->r.endpos);	// Move the line out a bit to avoid a zero-length line.

		// Finally, copy the last velocity we used.
		VectorCopy(lightning->velocity2, lastvel);

		// Now, add the additional velocity upwards
		VectorAdd(lightning->velocity, upvel, lightning->velocity);
		VectorAdd(lightning->velocity2, upvel, lightning->velocity2);

		lightning->r.scale = .5;
		lightning->d_scale = 32.0;
		lightning->alpha = 0.1;
		lightning->d_alpha = 3.0;
		lightning->color.c = 0xffffffff;

		AddEffect(NULL, lightning);

		// Now spawn a particle quick to save against the nasty joints (ugh).
		// Half green, half yellow particles
		if (i&0x01)
		{	
			lightning->r.tile = 0.5;		// Alternate tiles
			lightning->r.tileoffset = 0.5;
			spark = ClientParticle_new(PART_16x16_SPARK_Y, lightning->color, 750);
		}
		else
		{
			lightning->r.tile = 0.5;		// Alternate tiles
			lightning->r.tileoffset = 0.0;
			spark = ClientParticle_new(PART_16x16_SPARK_G, lightning->color, 750);
		}
		VectorCopy(lightning->r.startpos, spark->origin);
		VectorCopy(lightning->velocity, spark->velocity);
		VectorCopy(lightning->acceleration, spark->acceleration);
		spark->scale = 0.5;
		spark->d_scale = 32.0;
		spark->color.a = 1;
		spark->d_alpha = 768.0;

		AddParticleToList(lightning, spark);
	}

	// Now finally flash the screen
	fxi.Activate_Screen_Flash(0x8080ffc0);
	// make our screen shake a bit
	// values are : a, b, c, d
	// a = amount of maximum screen shake, in pixels
	// b = duration of screen shake in milli seconds
	// c = current time - in milli seconds - if this routine is called from the server, remember this
	// d = dir of shake - see game_stats.h for definitions
	fxi.Activate_Screen_Shake(4, 500, fxi.cl->time, SHAKE_ALL_DIR);

	if (Flags & CEF_FLAG8)	// Play sound flag
		fxi.S_StartSound(target, -1, CHAN_WEAPON, fxi.S_RegisterSound("weapons/LightningPower.wav"), 1, ATTN_NORM, 0);
}



// end
