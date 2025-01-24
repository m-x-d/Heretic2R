//
// fx_lightning.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define LIGHTNING_WIDTH				6.0f
#define LIGHTNING_WIDTH_MULT		1.4f
#define LIGHTNING_POWER_WIDTH_MULT	2.0f
#define LIGHTNING_JOINT_SCALE		(1.0f / 12.0f)

#define LIGHTNING_TYPE_BLUE			0
#define LIGHTNING_TYPE_RED			1
#define LIGHTNING_TYPE_GREEN		2
#define LIGHTNING_JOINT_OFFSET		3

#define MIN_LIGHTNING_PARTS			5
#define MAX_LIGHTNING_SEGMENT		64.0f

#define LIGHTNING_INTERVAL			100
#define NUM_LIGHTNING_RINGBITS		12
#define LIGHTNING_RING_VELOCITY		320.0f

static struct model_s* lightning_models[7];

void PreCacheLightning(void)
{
	lightning_models[LIGHTNING_TYPE_BLUE] = fxi.RegisterModel("sprites/fx/lightning.sp2");
	lightning_models[LIGHTNING_TYPE_RED] = fxi.RegisterModel("sprites/fx/rlightning.sp2");
	lightning_models[LIGHTNING_TYPE_GREEN] = fxi.RegisterModel("sprites/fx/plightning.sp2");
	lightning_models[LIGHTNING_TYPE_BLUE + LIGHTNING_JOINT_OFFSET] = fxi.RegisterModel("sprites/spells/spark_blue.sp2");
	lightning_models[LIGHTNING_TYPE_RED + LIGHTNING_JOINT_OFFSET] = fxi.RegisterModel("sprites/spells/spark_red.sp2");
	lightning_models[LIGHTNING_TYPE_GREEN + LIGHTNING_JOINT_OFFSET] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
	lightning_models[6] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

static client_entity_t* MakeLightningPiece(const int type, const float width, const vec3_t start, const vec3_t end, const float radius)
{
	// Lightning.
	client_entity_t* lightning = ClientEntity_new(FX_LIGHTNING, CEF_DONT_LINK, start, NULL, 250);

	lightning->radius = radius;
	lightning->r.model = &lightning_models[type];
	lightning->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.spriteType = SPRITE_LINE;
	lightning->r.scale = width;
	lightning->alpha = 0.95f;
	lightning->d_alpha = -4.0f;
	VectorCopy(start, lightning->r.startpos);
	VectorCopy(end, lightning->r.endpos);

	AddEffect(NULL, lightning);

	// Halo around the lightning.
	client_entity_t* halo = ClientEntity_new(FX_LIGHTNING, CEF_DONT_LINK, start, NULL, 400);

	halo->radius = radius;
	halo->r.model = &lightning_models[type];
	halo->r.frame = 1;
	halo->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	halo->r.spriteType = SPRITE_LINE;
	halo->r.scale = width * LIGHTNING_WIDTH_MULT;
	halo->alpha = 0.5f;
	halo->d_alpha = -1.25f;
	VectorCopy(start, halo->r.startpos);
	VectorCopy(end, halo->r.endpos);

	AddEffect(NULL, halo);

	// Add a little ball at the joint (end)
	client_entity_t* spark = ClientEntity_new(FX_LIGHTNING, CEF_DONT_LINK, start, NULL, 250);

	spark->radius = radius;
	spark->r.model = &lightning_models[type + LIGHTNING_JOINT_OFFSET];
	spark->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	spark->r.scale = width * LIGHTNING_JOINT_SCALE;
	spark->alpha = 0.95f;
	spark->d_alpha = -2.0f;
	spark->d_scale = -2.0f;
	VectorCopy(start, spark->r.startpos);
	VectorCopy(end, spark->r.endpos);

	AddEffect(NULL, spark);

	return spark;
}

// Directed lightning bolt.
static void LightningBolt(const int model, const float width, const vec3_t start_pos, const vec3_t end_pos)
{
	vec3_t diff_pos;
	VectorSubtract(end_pos, start_pos, diff_pos);
	const float length = VectorLength(diff_pos);

	int segments;
	float seg_mult;

	if (length < MIN_LIGHTNING_PARTS * MAX_LIGHTNING_SEGMENT)
	{
		segments = MIN_LIGHTNING_PARTS;
		seg_mult = 1.0f / (float)MIN_LIGHTNING_PARTS;
	}
	else
	{
		segments = (int)(length / MAX_LIGHTNING_SEGMENT);
		seg_mult = 1.0f / (float)segments;
	}

	vec3_t fwd;
	VectorNormalize2(diff_pos, fwd);

	vec3_t up;
	PerpendicularVector(up, fwd);

	vec3_t right;
	CrossProduct(up, fwd, right); //TODO: can't fwd/up/right be initialized using AngleVectors() instead?

	VectorScale(diff_pos, seg_mult, diff_pos);
	const float variance = length * seg_mult * 0.4f;

	vec3_t last_pos;
	VectorCopy(start_pos, last_pos);

	vec3_t ref_point;
	VectorCopy(start_pos, ref_point);

	for (int i = 0; i < segments - 1; i++)
	{
		VectorAdd(ref_point, diff_pos, ref_point);

		vec3_t rand;
		VectorScale(up, flrand(-variance, variance), rand);
		VectorMA(rand, flrand(-variance, variance), right, rand);

		vec3_t cur_pos;
		VectorAdd(ref_point, rand, cur_pos);
		MakeLightningPiece(model, width, cur_pos, last_pos, variance);

		VectorCopy(cur_pos, last_pos);
	}

	// Draw the last point to the destination, no variance.
	MakeLightningPiece(model, width, end_pos, last_pos, variance);
}

static qboolean FXLightningThink(const client_entity_t* thinker, centity_t* owner)
{
	if (fxi.cl->time - thinker->lastThinkTime < thinker->LifeTime)
	{
		LightningBolt(thinker->SpawnInfo, thinker->xscale, thinker->r.startpos, thinker->r.endpos);
		return true;
	}

	return false;
}

void FXLightning(centity_t* owner, int type, const int flags, const vec3_t origin)
{
	byte width;
	byte duration;
	vec3_t target;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_LIGHTNING].formatString, target, &width, &duration);

	if (duration > 1) // Duration is in 1/10 of a second.
	{
		// Create a client effect to zap over time.
		client_entity_t* lightning = ClientEntity_new(FX_LIGHTNING, (int)(flags | CEF_NO_DRAW), origin, NULL, LIGHTNING_INTERVAL);

		vec3_t diff_pos;
		VectorSubtract(target, origin, diff_pos);
		lightning->radius = VectorLength(diff_pos) * 0.5f;
		VectorCopy(origin, lightning->r.startpos);
		VectorCopy(target, lightning->r.endpos);
		lightning->lastThinkTime = fxi.cl->time;
		lightning->LifeTime = duration * 100 + 250;
		lightning->SpawnInfo = ((flags & CEF_FLAG6) ? LIGHTNING_TYPE_RED : LIGHTNING_TYPE_BLUE);
		lightning->xscale = (float)width;
		lightning->Update = FXLightningThink;

		AddEffect(NULL, lightning);
	}

	int model = LIGHTNING_TYPE_BLUE;	// Normal, blue lightning

	if (flags & CEF_FLAG6)
		model = LIGHTNING_TYPE_RED;		// If flagged, do red lightning.
	else if (flags & CEF_FLAG7)
		model = LIGHTNING_TYPE_GREEN;	// Powered-up rain lightning.

	LightningBolt(model, width, origin, target);
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
					flrand(-LIGHTNING_RING_VELOCITY, LIGHTNING_RING_VELOCITY),
					flrand(-LIGHTNING_RING_VELOCITY, LIGHTNING_RING_VELOCITY),
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
	VectorSet(lastvel, LIGHTNING_RING_VELOCITY, 0.0, 0.0);
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
		VectorSet(lightning->velocity2, LIGHTNING_RING_VELOCITY *cos(curang), LIGHTNING_RING_VELOCITY *sin(curang), 0.0);
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
