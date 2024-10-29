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

#define EXPLODE_SPEED		160.0
#define EXPLODE_GRAVITY		(-320.0)
#define EXPLODE_SCALE		14.0
#define EXPLODE_NUM_BITS	32
#define EXPLODE_BALL_SPEED	64.0
#define EXPLODE_BALL_DELAY	5
#define EXPLODE_NUM_SMALLBALLS	3
#define EXPLODE_LIFETIME	50
#define EXPLODE_TIME_MAX	750

#define FIRETRAIL_PARTS		4
#define FIRETRAIL_RADIUS	6.0
#define FIRETRAIL_SPEED		16.0
#define FIRETRAIL_SCALE		12.0
#define FIRETRAIL_ACCEL		32.0

#define SMOKETRAIL_RADIUS	2.0
#define SMOKETRAIL_SCALE	0.25
#define SMOKETRAIL_ALPHA	0.5

static qboolean FXPhoenixMissilePowerThink(client_entity_t *missile, centity_t *owner);
void FXPhoenixExplodePower(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir);


#define	NUM_PHOEN_MODELS	6
static struct model_s *phoen_models[NUM_PHOEN_MODELS];
void PreCachePhoenix()
{
	phoen_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	phoen_models[1] = fxi.RegisterModel("models/spells/phoenixarrow/tris.fm");
	phoen_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
	phoen_models[3] = fxi.RegisterModel("sprites/spells/phoenix.sp2");
	phoen_models[4] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm");
	phoen_models[5] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
}


// -----------------------------------------------------------------------------------------


static qboolean FXPhoenixMissileThink(client_entity_t *missile, centity_t *owner)
{
	int					i;
	int					dur;
	client_particle_t	*flame;
	client_entity_t		*smoke;
	vec3_t				angles, fwd, right, smokeorigin;
	paletteRGBA_t		LightColor={0xff, 0x7f, 0x00, 0xe5};

	if (r_detail->value == DETAIL_LOW)
		dur = 1400;
	else
	if (r_detail->value == DETAIL_NORMAL)
		dur = 1700;
	else
		dur = 2000;

	// Here we want to shoot out flame to either side
	VectorScale(missile->r.angles, 180.0/M_PI, angles);
	AngleVectors(angles, fwd, right, NULL);
	VectorScale(fwd, -4.0*FIRETRAIL_SPEED, fwd);
	VectorScale(right, FIRETRAIL_SPEED, right);

	// Throw smoke to each side, alternating.  
	if ((missile->LifeTime--)&0x01)
	{	// to the right
		VectorSet(	smokeorigin, 
					flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
					flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
					flrand(-SMOKETRAIL_RADIUS/2.0, SMOKETRAIL_RADIUS/2.0));
		VectorAdd(	smokeorigin, missile->origin, smokeorigin);
		smoke = ClientEntity_new(-1, CEF_DONT_LINK, smokeorigin, NULL, dur);
		smoke->r.model = phoen_models;
		smoke->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		smoke->radius = 64.0F;
		smoke->alpha = SMOKETRAIL_ALPHA;
		smoke->r.scale = SMOKETRAIL_SCALE;
		smoke->velocity[0] = right[0]*2.0;
		smoke->velocity[1] = right[1]*2.0;
		smoke->velocity[2] = right[2]*2.0;
		smoke->d_scale = 2.0;		// Rate of change in scale
		smoke->d_alpha = -1.0;
		AddEffect(NULL, smoke);	// add the smoke as independent world smoke
	}
	else
	{	// and to the left
		VectorSet(	smokeorigin, 
					flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
					flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
					flrand(-SMOKETRAIL_RADIUS/2.0, SMOKETRAIL_RADIUS/2.0));
		VectorAdd(	smokeorigin, missile->origin, smokeorigin);
		smoke = ClientEntity_new(-1, CEF_DONT_LINK, smokeorigin, NULL, dur);
		smoke->r.model = phoen_models;
		smoke->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		smoke->radius = 128.0F;
		smoke->alpha = SMOKETRAIL_ALPHA;
		smoke->r.scale = SMOKETRAIL_SCALE;
		smoke->velocity[0] = -right[0]*2.0;
		smoke->velocity[1] = -right[1]*2.0;
		smoke->velocity[2] = -right[2]*2.0;
		smoke->d_scale = 2.0;		// Rate of change in scale
		smoke->d_alpha = -1.0;
		AddEffect(NULL, smoke);	// add the smoke as independent world smoke
	}

	smoke->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	// Burn baby burn add fire to the tail.  Attach it to the smoke because it doesn't get out of the fx radius so quickly
	for(i = 0; i < FIRETRAIL_PARTS; i++)
	{
		flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), LightColor, dur);
		VectorSet(	flame->origin, 
					flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS), 
					flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS), 
					flrand(-FIRETRAIL_RADIUS/3.0, FIRETRAIL_RADIUS/3.0));
		VectorAdd(	missile->origin, flame->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity, 
				flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED), flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED), flrand(-1.0, 1.0));
		// Make the fire shoot out the back and to the side
		VectorAdd(flame->velocity, fwd, flame->velocity);
		// Alternate left and right side of phoenix
		if (i&0x01)
			VectorAdd(flame->velocity, right, flame->velocity);
		else
			VectorSubtract(flame->velocity, right, flame->velocity);
		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0, -10.0);
		flame->d_alpha = flrand(-200.0, -160.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(smoke, flame);
	}

	// Update animation frame.

	// Check if the time is up.
	if (fxi.cl->time >= missile->lastThinkTime)
	{	// Set up animations to go the other direction.
		if (missile->NoOfAnimFrames == 7)
		{	// Set to go backwards to 3.
			missile->NoOfAnimFrames = 3;
			missile->Scale = -1;
			missile->lastThinkTime = fxi.cl->time + (4*50);
			missile->r.frame = 7;
		}
		else
		{	// Set to go forward to 7
			missile->NoOfAnimFrames = 7;
			missile->Scale = -1;
			missile->lastThinkTime = fxi.cl->time + (4*50);
			missile->r.frame = 3;
		}
	}
	else
	{
		missile->r.frame = missile->NoOfAnimFrames - (missile->Scale * (int)((missile->lastThinkTime-fxi.cl->time)/50)) - 1;
	}

	// Remember for even spread of particles
	VectorCopy(missile->r.origin, missile->origin);
	return(true);
}


///////////////////////////
// From CreateEffect FX_WEAPON_PHOENIXMISSILE
///////////////////////////
void FXPhoenixMissile(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*missile;
	vec3_t				temp;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 25);
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_PHOENIXMISSILE].formatString, missile->velocity);

	if (flags & CEF_FLAG8)
		Vec3ScaleAssign(PHOENIX_ARROW_SPEED/2,missile->velocity);
	else
		Vec3ScaleAssign(PHOENIX_ARROW_SPEED,missile->velocity);


	VectorCopy(missile->velocity, temp);
	VectorNormalize(temp);
	AnglesFromDir(temp, missile->r.angles);

	missile->r.model = phoen_models + 1;
	missile->flags |= CEF_ADDITIVE_PARTS;
	missile->r.frame = 0;
	missile->lastThinkTime = fxi.cl->time + (50*7);	// Time to play last frame.
	missile->NoOfAnimFrames = 7;					// End on frame number 7.
	missile->Scale = 1;								// Positive frame count
	missile->r.scale= .8;							
	if(flags & CEF_FLAG6)
	{
		missile->Update = FXPhoenixMissilePowerThink;
	}
	else
	{
		missile->Update = FXPhoenixMissileThink;
	}
	missile->radius = 256;
	missile->color.c = 0xff00ffff;
	if (r_detail->value != DETAIL_LOW)
		missile->dlight = CE_DLight_new(missile->color, 150.0F, 00.0F);
	missile->LifeTime = 1000;
	AddEffect(owner, missile);

}

// -----------------------------------------------------------------------------------------
// This is also exported for use in FXBarrelExplode/
qboolean FXPhoenixExplosionBallThink(client_entity_t *explosion, centity_t *owner)
{
	float velfactor;

	if (fxi.cl->time-explosion->startTime > EXPLODE_TIME_MAX)
	{
		return(false);
	}

	velfactor = (fxi.cl->time - explosion->lastThinkTime) / 1000.0;
	explosion->lastThinkTime = fxi.cl->time;

	// Spin the ball of fire while it expands and fades.
	explosion->r.angles[YAW] += explosion->velocity2[YAW] * velfactor;
	explosion->r.angles[PITCH] += explosion->velocity2[PITCH] * velfactor;

	if(explosion->dlight->intensity > 0.0F)
		explosion->dlight->intensity -= 5.0F;
	
	return(true);
}

// This is also exported for use in FXBarrelExplode/
qboolean FXPhoenixExplosionSmallBallThink(client_entity_t *explosion, centity_t *owner)
{
	float velfactor;

	if (fxi.cl->time-explosion->startTime > EXPLODE_TIME_MAX)
	{
		return(false);
	}

	velfactor = (fxi.cl->time - explosion->lastThinkTime) / 1000.0;
	explosion->lastThinkTime = fxi.cl->time;

	// Spin the ball of fire while it expands and fades.
	explosion->r.angles[YAW] += explosion->velocity2[YAW] * velfactor;
	explosion->r.angles[PITCH] += explosion->velocity2[PITCH] * velfactor;

	return(true);
}

static qboolean FXPhoenixExplosionBirdThink(client_entity_t *bird, centity_t *owner)
{
	client_entity_t *newbird;
	vec3_t	pos;
	int		dur;

	bird->LifeTime--;
	if (bird->LifeTime <= 0)
	{
		return(false);
	}

	if(r_detail->value == DETAIL_LOW)
		dur = 175;
	else
	if(r_detail->value == DETAIL_NORMAL)
		dur = 210;
	else
		dur = 250;

	// Spawn another trail bird
	VectorSet(pos, flrand(-8, 8), flrand(-8, 8), flrand(-8, 8));
	VectorAdd(pos, bird->r.origin, pos);
	newbird = ClientEntity_new(-1, bird->r.flags, pos, NULL, dur);
	newbird->r.model = phoen_models + 3;
	newbird->r.frame = 1;
	newbird->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	newbird->radius	= 128;
	newbird->r.scale= bird->r.scale;
	newbird->alpha	= bird->alpha;
	newbird->d_alpha= -(newbird->alpha*4.0);
	newbird->d_scale= 2.0;
	VectorCopy(bird->velocity, newbird->velocity);
	AddEffect(NULL, newbird);

	return (true);
}

// This is also exported for use in FXBarrelExplode
client_entity_t *CreatePhoenixSmallExplosion(vec3_t ballorigin)
{
	client_entity_t *subexplosion;

	subexplosion = ClientEntity_new(FX_WEAPON_PHOENIXEXPLODE, CEF_BROADCAST, ballorigin, NULL, 17);
	subexplosion->r.model = phoen_models + 4;
	subexplosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	subexplosion->alpha = 1.0;
	subexplosion->r.scale = 1.0;
	subexplosion->radius=128;
	subexplosion->startTime = fxi.cl->time;
	subexplosion->lastThinkTime = fxi.cl->time;
	subexplosion->velocity2[YAW] = flrand(-M_PI, M_PI);
	subexplosion->velocity2[PITCH] = flrand(-M_PI, M_PI);
	subexplosion->Update = FXPhoenixExplosionSmallBallThink;

	return(subexplosion);
}



//////////////////////////
// From CreateEffect FX_WEAPON_PHOENIXEXPLODE
//////////////////////////
void FXPhoenixExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*explosion, *subexplosion;
	paletteRGBA_t		color;
	vec3_t				dir, sdir;
	client_particle_t	*spark;
	int					i;
	float				ballnum;
	int					count, dur;

	fxi.GetEffect(owner,flags,clientEffectSpawners[FX_WEAPON_PHOENIXEXPLODE].formatString,dir,sdir);

	// make the scorch mark if we should
	if(flags & CEF_FLAG8)
		FXClientScorchmark(origin, sdir);

	if(flags & CEF_FLAG6)
	{	// powered up version
		FXPhoenixExplodePower(owner, type, flags, origin, dir);
		return;
	}

	flags |= CEF_OWNERS_ORIGIN;

	if(r_detail->value != DETAIL_LOW)
	{
		count =	EXPLODE_NUM_SMALLBALLS;
		if(r_detail->value == DETAIL_NORMAL)
			count =	EXPLODE_NUM_SMALLBALLS - 1;

		// Create three smaller explosion spheres.
		for (i=0; i < count; i++)
		{
			ballnum = i;
			subexplosion = CreatePhoenixSmallExplosion(origin);
			VectorSet(subexplosion->velocity, 
							flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[0]*EXPLODE_BALL_SPEED),
							flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[1]*EXPLODE_BALL_SPEED),
							flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[2]*EXPLODE_BALL_SPEED));
			subexplosion->r.scale = 0.1;
			subexplosion->d_scale = 3.0 + ballnum;
			subexplosion->d_alpha = -1.5 - 0.5*ballnum;

			AddEffect(NULL, subexplosion);
		}
	}

	// Create the main big explosion sphere.
	explosion = ClientEntity_new(type, flags, origin, NULL, 17);
	explosion->r.model = phoen_models + 5;
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
	VectorScale(dir,EXPLODE_SPEED,dir);
	count = GetScaledCount(EXPLODE_NUM_BITS, 0.3);
	for(i = 0; i < count; i++)
	{
		spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 2000);
		VectorSet(spark->velocity,	flrand(-EXPLODE_SPEED, EXPLODE_SPEED), 
									flrand(-EXPLODE_SPEED, EXPLODE_SPEED), 
									flrand(-EXPLODE_SPEED, EXPLODE_SPEED));
		VectorAdd(spark->velocity, dir, spark->velocity);
		spark->acceleration[2] = EXPLODE_GRAVITY;
		spark->scale = EXPLODE_SCALE;
		spark->d_scale = flrand(-20.0, -10.0);
		spark->d_alpha = flrand(-400.0, -320.0);
		spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(explosion, spark);
	}

	// ...and a big-ass flash
	explosion = ClientEntity_new(-1, flags, origin, NULL, 250);
	explosion->r.model = phoen_models + 2;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 1;
	explosion->radius=128;
	explosion->r.scale=1.5;
	explosion->d_alpha=-4.0;
	explosion->d_scale=-4.0;
	AddEffect(NULL, explosion);

	if(r_detail->value == DETAIL_LOW)
		dur = 150;
	else
	if(r_detail->value == DETAIL_NORMAL)
		dur = 125;
	else
		dur = 100;

	// ...and draw the phoenix rising from the explosion
	explosion = ClientEntity_new(type, flags, origin, NULL, dur);
	explosion->r.model = phoen_models + 3;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 0;
	explosion->radius=128;
	explosion->r.scale=0.1;
	VectorScale(dir, 0.25, explosion->velocity);
	explosion->acceleration[2] = 64;
	explosion->alpha = 1.0;
	explosion->d_alpha=-1.0;
	explosion->d_scale=1.25;
	explosion->LifeTime = 10;
	explosion->Update = FXPhoenixExplosionBirdThink;
	AddEffect(NULL, explosion);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/PhoenixHit.wav"), 1, ATTN_NORM, 0);
}



// end


#define PHOENIXPOWER_NUMTRAILS 11
#define PHOENIXPOWER_PARTS_PER_TRAIL 8
#define PHOENIXPOWER_RADIUS 72.0

static qboolean FXPhoenixExplosionBirdThinkPower(client_entity_t *bird, centity_t *owner)
{
	bird->LifeTime--;
	if (bird->LifeTime <= 0)
	{
		return(false);
	}
	return (true);
}

void FXPhoenixExplodePower(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir)
{
	client_entity_t		*explosion, *subexplosion;
	paletteRGBA_t		color;
	client_particle_t	*spark;
	int					i, j;
	vec3_t				phOrg;
	float				cosVal, sinVal;
	trace_t				trace;
	vec3_t				endPos;
	vec3_t				minmax = {0, 0, 0};
	int					numTrails;
	int					numParts;
	float				partHeight;
	float				detail_scale;

	flags |= CEF_OWNERS_ORIGIN;

	// This isn't actually used but we need something to anchor the particles to
	// Create the main big explosion sphere.
	explosion = ClientEntity_new(type, flags, origin, NULL, 17);
	explosion->r.model = phoen_models + 5;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->flags |= CEF_ADDITIVE_PARTS;
	explosion->alpha = 1.0;
	explosion->r.scale= .1;
	explosion->d_alpha=-2.0/1.5;
	explosion->radius=128;
	explosion->LifeTime=EXPLODE_LIFETIME;

	color.c = 0xff00ffff;
	explosion->dlight = CE_DLight_new(color, 150.0F, 0.0F);
	explosion->Update = FXPhoenixExplosionBallThink;
	AddEffect(NULL, explosion);
	color.c = 0xffffffff;

	numTrails = GetScaledCount(PHOENIXPOWER_NUMTRAILS-4, 0.8) + 4;//4 is the minimum
	numParts = GetScaledCount(PHOENIXPOWER_PARTS_PER_TRAIL - 4, .8) + 4;//ditto

	for(i = 0; i < numTrails; i++)
	{
		cosVal = cos((float)i / (float)numTrails * (M_PI*2));
		sinVal = sin((float)i / (float)numTrails * (M_PI*2));

		VectorCopy(origin, phOrg);
		phOrg[0] += cosVal * PHOENIXPOWER_RADIUS;
		phOrg[1] += sinVal * PHOENIXPOWER_RADIUS;

		VectorCopy(phOrg, endPos);
		endPos[2] -= 64;

		fxi.Trace(	phOrg, minmax, minmax, endPos, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

		if(trace.fraction > .99)
		{	// Burst in the air, no ground found.
			subexplosion = ClientEntity_new(-1, flags, phOrg, NULL, 1000);
			subexplosion->r.model = phoen_models + 2;
			subexplosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
			subexplosion->r.frame = 1;
			subexplosion->radius=128;
			subexplosion->r.scale=1.5;
			subexplosion->d_alpha=-1.0;
			AddEffect(NULL, subexplosion);

			for(j = 0; j < numParts; j++)
			{
				partHeight = (j * PHOENIXPOWER_PARTS_PER_TRAIL)/numParts;

				spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 1000);
				spark->origin[0] = cosVal * 72;
				spark->origin[1] = sinVal * 72;
				// alternate up and down.
				if (j&0x01)
					spark->velocity[2] = partHeight * 15.0* (18.0/8.0) + flrand(0, 15);
				else
					spark->velocity[2] = -partHeight * 15.0* (18.0/8.0) + flrand(0, 15);
				spark->scale = 32.0 - (partHeight * (18.0/8.0)/3);
				spark->d_scale = -partHeight * (18.0/8.0);
				spark->d_alpha = flrand(-400.0, -320.0)/1.3;
				spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

				AddParticleToList(explosion, spark);
			}
		}
		else
		{
			subexplosion = ClientEntity_new(-1, flags, trace.endpos, NULL, 1000);
			subexplosion->r.model = phoen_models + 2;
			subexplosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
			subexplosion->r.frame = 1;
			subexplosion->radius=128;
			subexplosion->r.scale=1.5;
			subexplosion->d_alpha=-1.0;
			AddEffect(NULL, subexplosion);

			for(j = 0; j < numParts; j++)
			{
				partHeight = (j * PHOENIXPOWER_PARTS_PER_TRAIL)/numParts;

				spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 1000);
				spark->origin[0] = cosVal * 72;
				spark->origin[1] = sinVal * 72;
				spark->origin[2] = trace.endpos[2] - origin[2];
				spark->velocity[2] = partHeight * 15.0* (18.0/8.0) + flrand(0, 15);
				spark->scale = 32.0 - (partHeight * (18.0/8.0)/3);
				spark->d_scale = -partHeight * (18.0/8.0);
				spark->d_alpha = flrand(-400.0, -320.0)/1.3;
				spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

				AddParticleToList(explosion, spark);
			}
		}
	}

	if (r_detail->value == DETAIL_LOW)
		detail_scale = 1.5;
	else
		detail_scale = 2.0;

	// ...and draw the phoenix rising from the explosion
	explosion = ClientEntity_new(type, flags, origin, NULL, 100);
	explosion->r.model = phoen_models + 3;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 0;
	explosion->radius=128;
	explosion->r.scale=1.0;
	VectorScale(dir, 192.0, explosion->velocity);
	explosion->acceleration[2] = 256.0;
	explosion->alpha = 1.0;
	explosion->d_alpha=-1.5;
	explosion->d_scale=detail_scale;
	explosion->LifeTime = 6;
	explosion->Update = FXPhoenixExplosionBirdThinkPower;
	AddEffect(NULL, explosion);

	// inner phoenix
	explosion = ClientEntity_new(type, flags, origin, NULL, 100);
	explosion->r.model = phoen_models + 3;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 0;
	explosion->radius=128;
	explosion->r.scale=1.0;
	VectorScale(dir, 192.0, explosion->velocity);
	explosion->acceleration[2] = 256.0;
	explosion->alpha = 1.0;
	explosion->d_alpha= 0.0;
	explosion->d_scale=-1.0/.6;
	explosion->LifeTime = 6;
	explosion->Update = FXPhoenixExplosionBirdThinkPower;
	AddEffect(NULL, explosion);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/PhoenixPowerHit.wav"), 1, ATTN_NORM, 0);
}

static qboolean FXPhoenixMissilePowerThink(client_entity_t *missile, centity_t *owner)
{
	int					i, dur;
	client_particle_t	*flame;
	client_entity_t		*smoke;
	vec3_t				angles, fwd, right, smokeorigin;
	paletteRGBA_t		LightColor={0xff, 0x7f, 0x00, 0xe5};
	int					sideVal;

	if (r_detail->value == DETAIL_LOW)
		dur = 1400;
	else
	if (r_detail->value == DETAIL_NORMAL)
		dur = 1700;
	else
		dur = 2000;
	
	// Here we want to shoot out flame to either side
	VectorScale(missile->r.angles, 180.0/M_PI, angles);
	AngleVectors(angles, fwd, right, NULL);
	VectorScale(fwd, -4.0*FIRETRAIL_SPEED, fwd);
	VectorScale(right, FIRETRAIL_SPEED, right);

	sideVal = (missile->LifeTime&0x1) ? -1:1;
	missile->LifeTime--;
	// Throw smoke to each side, alternating.  
	VectorSet(	smokeorigin, 
				flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
				flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
				flrand(-SMOKETRAIL_RADIUS/2.0, SMOKETRAIL_RADIUS/2.0));
	VectorAdd(	smokeorigin, missile->origin, smokeorigin);
	smoke = ClientEntity_new(-1, CEF_DONT_LINK, smokeorigin, NULL, dur);
	smoke->r.model = phoen_models;
	smoke->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	smoke->radius = 64.0F;
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->r.scale = SMOKETRAIL_SCALE * 2.5;
	smoke->velocity[0] = sideVal * right[0]*2.0;
	smoke->velocity[1] = sideVal * right[1]*2.0;
	smoke->velocity[2] = sideVal * right[2]*2.0;
	smoke->d_scale = 2.0;		// Rate of change in scale
	smoke->d_alpha = -1.0;
	AddEffect(NULL, smoke);	// add the smoke as independent world smoke
	smoke->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;

	// Burn baby burn add fire to the tail.  Attach it to the smoke because it doesn't get out of the fx radius so quickly
	for(i = 0; i < FIRETRAIL_PARTS; i++)
	{
		flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), LightColor, dur);
		VectorSet(	flame->origin, 
					flrand(-FIRETRAIL_RADIUS/3, FIRETRAIL_RADIUS/3), 
					flrand(-FIRETRAIL_RADIUS/3, FIRETRAIL_RADIUS/3), 
					flrand(-FIRETRAIL_RADIUS/3.0, FIRETRAIL_RADIUS/3.0));
		VectorAdd(	missile->origin, flame->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity, 
				flrand(-FIRETRAIL_SPEED/3, FIRETRAIL_SPEED/3), flrand(-FIRETRAIL_SPEED/3, FIRETRAIL_SPEED/3), flrand(-1.0, 1.0));
		// Make the fire shoot out the back and to the side
		VectorAdd(flame->velocity, fwd, flame->velocity);
		// Alternate left and right side of phoenix
		if (i&0x01)
			VectorAdd(flame->velocity, right, flame->velocity);
		else
			VectorSubtract(flame->velocity, right, flame->velocity);
		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0, -10.0);
		flame->d_alpha = flrand(-200.0, -160.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(smoke, flame);
	}


	// Remember for even spread of particles
	VectorCopy(missile->r.origin, missile->origin);
	return(true);
}

