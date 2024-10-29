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
#include "Matrix.h"
#include "Random.h"
#include "Utilities.h"
#include "motion.h"
#include "Reference.h"
#include "ce_Dlight.h"
#include "q_Sprite.h"
#include "g_playstats.h"


#define	NUM_WALL_MODELS	3

static struct model_s *wall_models[NUM_WALL_MODELS];

void PreCacheWall()
{
	wall_models[0] = fxi.RegisterModel("sprites/spells/wflame.sp2");
	wall_models[1] = fxi.RegisterModel("sprites/spells/wflame2.sp2");
	wall_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
}




// *****************************************************************
// The fire wall
// Powered up
// *****************************************************************


#define FIREWORM_LIFETIME		1.0
#define FIREWORM_BLASTLIFE		0.25
#define FIREWORM_LIFETIME_MS	(FIREWORM_LIFETIME * 1000)
#define FIREWORM_ACCEL			(-1000.0)
#define FIREWORM_INITVEL		(-0.5 * FIREWORM_LIFETIME * FIREWORM_ACCEL)
#define FIREWORM_TRAILVEL		128
#define FIREWORM_BLASTVEL		128
#define FIREWORM_BLASTNUM		12
#define FIREWORM_BLASTRAD		32

static qboolean FXFireWormThink(client_entity_t *worm, centity_t *owner)
{
	client_entity_t		*blast;
	client_particle_t	*spark, *spark2;
	vec3_t				diffpos;
	float				dtime, dtime2;
	float				anginc, ang;
	int					i;
	paletteRGBA_t		color;

	dtime = (fxi.cl->time - worm->startTime)/FIREWORM_LIFETIME_MS;
	dtime2 = dtime * dtime * 0.5;

	color.c = 0xffffffff;

	if (dtime > FIREWORM_LIFETIME)
	{	// Impact at the centerpoint.
		if (worm->SpawnInfo == 0 || dtime > FIREWORM_LIFETIME+FIREWORM_BLASTLIFE)
		{	// Do nothing, wait for blast to expire.
			worm->nextThinkTime = fxi.cl->time + 500;
			worm->Update = RemoveSelfAI;
			return true;
		}
		else
		{
			blast = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_ADDITIVE_PARTS, worm->endpos, NULL, 500);
			blast->r.model = wall_models + 2;
			blast->r.frame = 2;		// Circular halo for blast.
			blast->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			blast->alpha = 0.95;
			blast->d_alpha = -0.5;
			blast->r.scale = 0.25*worm->r.scale;
			blast->d_scale = -3.0*worm->r.scale;
			VectorSet(blast->velocity,
						flrand(-0.25*FIREWORM_BLASTVEL*worm->r.scale, 0.25*FIREWORM_BLASTVEL*worm->r.scale), 
						flrand(-0.25*FIREWORM_BLASTVEL*worm->r.scale, 0.25*FIREWORM_BLASTVEL*worm->r.scale), 
						flrand(0, 0.25*FIREWORM_BLASTVEL*worm->r.scale));
			AddEffect(NULL, blast);

			// Spray out in a big ring
			ang = flrand(0, M_PI*2.0);
			anginc = (M_PI * 2.0) / FIREWORM_BLASTNUM;
			for (i=0; i<8; i++)
			{
				diffpos[0] = cos(ang);
				diffpos[1] = sin(ang);
				diffpos[2] = 0;
				ang += anginc;

				// Higher particle
				spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 500);

				VectorScale(diffpos, FIREWORM_BLASTRAD*worm->r.scale, spark->origin);
				VectorScale(diffpos, flrand(0.45, 0.5)*FIREWORM_BLASTVEL*worm->r.scale, spark->velocity);
				spark->velocity[2] += flrand(0.80, 1.0)*FIREWORM_BLASTVEL*worm->r.scale;

				spark->color.a = 254;
				spark->d_alpha = flrand(-512.0, -768.0);
				spark->scale = 16.0*worm->r.scale;
				spark->d_scale = flrand(8.0, 16.0)*worm->r.scale;

				AddParticleToList(blast, spark);

				// Lower to ground particle
				spark2 = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color, 500);

				VectorCopy(spark->origin, spark2->origin);
				VectorCopy(spark->velocity, spark2->velocity);
				spark2->velocity[2] *= 0.33;

				spark2->color.a = 254;
				spark2->d_alpha = flrand(-512.0, -768.0);
				spark2->scale = 16.0*worm->r.scale;
				spark2->d_scale = flrand(8.0, 16.0)*worm->r.scale;

				AddParticleToList(blast, spark2);
			}

			// Spray up in a little fountain too.
			for (i=0; i<4; i++)
			{
				spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 500);

				VectorSet(spark->velocity,
							flrand(-0.1*FIREWORM_BLASTVEL*worm->r.scale, 0.1*FIREWORM_BLASTVEL*worm->r.scale), 
							flrand(-0.1*FIREWORM_BLASTVEL*worm->r.scale, 0.1*FIREWORM_BLASTVEL*worm->r.scale), 
							flrand(-0.2*FIREWORM_BLASTVEL*worm->r.scale, 0.2*FIREWORM_BLASTVEL*worm->r.scale));
				spark->velocity[2] += FIREWORM_BLASTVEL;

				spark->color.a = 254;
				spark->d_alpha = flrand(-512.0, -768.0);
				spark->scale = 16.0*worm->r.scale;
				spark->d_scale = flrand(-8.0, -16.0);

				AddParticleToList(blast, spark);
			}
		}

		return true;
	}

	// Continue snaking to target.

//	VectorSubtract(worm->endpos, worm->startpos, diffpos);

	// Move linearly to the target.
//	VectorMA(worm->startpos, dtime, diffpos, worm->r.origin);
	// Now add in an additional arc.
//	worm->r.origin[2] += FIREWORM_INITVEL*dtime + FIREWORM_ACCEL*dtime2;
//	worm->velocity[2] = (diffpos[2]/FIREWORM_LIFETIME) + FIREWORM_INITVEL + FIREWORM_ACCEL*dtime;

	// Add a trail entity and particle trail segment.
	blast = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, worm->r.origin, NULL, 500);
	VectorClear(blast->velocity);
	AddEffect(NULL, blast);

	for (i=0; i<4; i++)
	{
//		spark = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color, 500);
		spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 500);

		VectorSet(spark->velocity,
					flrand(-0.25*FIREWORM_TRAILVEL, 0.25*FIREWORM_TRAILVEL), 
					flrand(-0.25*FIREWORM_TRAILVEL, 0.25*FIREWORM_TRAILVEL), 
					flrand(-0.25*FIREWORM_TRAILVEL, 0.25*FIREWORM_TRAILVEL));
		VectorMA(spark->velocity, 0.5, worm->velocity, spark->velocity);
		VectorScale(spark->velocity, -2.0, spark->acceleration);

		spark->color.a = 254;
		spark->d_alpha = flrand(-512.0, -768.0);
		spark->scale = 20.0*worm->r.scale;
		spark->d_scale = flrand(8.0, 16.0)*worm->r.scale;
		
		AddParticleToList(blast, spark);
	}

	return true;
}


#define FIREWAVE_TRACEDOWN		(128)
#define FIREWAVE_WORM_TIME		(0.5*1000)
#define FIREWAVE_BLAST_NUM		4
#define FIREWAVE_IMPACT_NUM		2

static FXFireWaveImpact(client_entity_t *wall)
{
	client_entity_t *blast;
	vec3_t			blastpt, spawnvel;
	int				i;

	VectorScale(wall->direction, -48.0, blastpt);
	VectorAdd(blastpt, wall->r.origin, blastpt);
	VectorScale(wall->direction, -64.0, spawnvel);

	// Add some blasty bits along a line
	for (i=0; i<FIREWAVE_BLAST_NUM; i++)
	{	// Spawn along the top line of the wall
		if (i&0x01)
		{	// Throw blast up
			spawnvel[2] = 128.0;
		}
		else
		{
			spawnvel[2] = -128.0;
		}

		blast = ClientEntity_new(FX_WEAPON_FIREWAVE, 0, blastpt, NULL, 500);
		blast->r.model = wall_models+2;
		blast->r.frame = 2;
		blast->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		blast->radius = 64.0;

		blast->alpha = 0.95;
		blast->r.scale = 1.6;
		blast->d_scale = -2.0;
		blast->d_alpha = -2.0;

		VectorMA(spawnvel, flrand(-0.2, -0.1), wall->velocity, blast->velocity);
		
		AddEffect(NULL, blast);
	}
}


static qboolean FXFireWaveThink(client_entity_t *wall, centity_t *owner)
{
	client_entity_t *blast, *worm;
	vec3_t			destpt,	spawnpt, spawnvel, bottom, minmax={0,0,0};
	trace_t			trace;
	qboolean		hitground=false;
	int				i;
	float			value, scale, detailscale;

	switch((int)(r_detail->value))
	{
	case DETAIL_LOW:
		detailscale = 0.5;
		break;
	case DETAIL_HIGH:
		detailscale = 0.9;
		break;
	case DETAIL_UBERHIGH:
		detailscale = 1.0;
		break;
	case DETAIL_NORMAL:
	default:
		detailscale = 0.7;
		break;
	}

	if (owner->current.effects & EF_ALTCLIENTFX)
	{	// Time for this wall to die.
		if (wall->SpawnInfo != 1)
		{
			// Wait one second before disappearing.
			VectorClear(wall->velocity);
			wall->lastThinkTime = fxi.cl->time + 1000;
			wall->SpawnInfo = 1;
			FXFireWaveImpact(wall);
			return true;
		}
		else if (wall->lastThinkTime > fxi.cl->time)
		{	// Still some time left to live...
			wall->dlight->intensity -= 20.0;
			return true;
		}
		else
		{	// Time's up
			return false;
		}
	}

	// Update radius
	wall->radius = FIREWAVE_RADIUS + (fxi.cl->time - wall->startTime)*(FIREWAVE_DRADIUS/1000.0);
	if (wall->dlight->intensity < 250.0)
		wall->dlight->intensity += 15.0;

	// Add some blasty bits along a line
	for (i=0; i<FIREWAVE_BLAST_NUM; i++)
	{	// Spawn along the top line of the wall
		VectorSet(spawnpt, flrand(-6.0,6.0), flrand(-6.0,6.0), flrand(-6.0,6.0));
		switch(i)
		{
		case 0:		// Throw blast left
			value = flrand(0.2, 0.8);
			VectorMA(spawnpt, -value*wall->radius, wall->right, spawnpt);
			VectorSet(spawnvel, flrand(-16.0,16.0), flrand(-16.0,16.0), 0.0);
			scale = 1-value;
			break;
		case 1:		// Throw blast right
			value = flrand(0.2, 0.8);
			VectorMA(spawnpt, value*wall->radius, wall->right, spawnpt);
			VectorSet(spawnvel, flrand(-16.0,16.0), flrand(-16.0,16.0), 0.0);
			scale = 1-value;
			break;
		case 2:		// Blast about at the center
			spawnpt[2] -= flrand(0, 0.2)*FIREWAVE_DOWN;
			scale = 0.8;
			break;
		case 3:
		default:	// Throw blast down
			VectorMA(spawnpt, flrand(-0.4,0.4)*wall->radius, wall->right, spawnpt);
			spawnpt[2] -= flrand(0.3,0.6)*FIREWAVE_DOWN;
			scale = 0.8;
			break;
		}

		VectorAdd(spawnpt, wall->r.origin, spawnpt);
		// vary a bit above or below the wall as well...
		spawnpt[2] += FIREWAVE_UP;

		blast = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_PULSE_ALPHA, spawnpt, NULL, 500);
		blast->r.model = wall_models;
		blast->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		blast->radius = 64.0;

		VectorMA(spawnvel, flrand(0.1, 0.2), wall->velocity, blast->velocity);
		blast->acceleration[2] += -300.0;
		blast->alpha = 0.01;
		blast->d_alpha = flrand(4.0, 5.0);
		blast->r.scale = scale*detailscale;
		blast->d_scale = scale*detailscale * flrand(-1.0, -1.5);
		
		AddEffect(NULL, blast);

		// Spawn along the bottom line of the wall
		VectorSet(spawnpt, flrand(-6.0,6.0), flrand(-6.0,6.0), flrand(-6.0,6.0));
		switch(i)
		{
		case 0:		// Throw blast left
			value = flrand(0.2, 0.8);
			VectorMA(spawnpt, -value*wall->radius, wall->right, spawnpt);
			VectorSet(spawnvel, flrand(-16.0,16.0), flrand(-16.0,16.0), 0.0);
			scale = 1-value;
			break;
		case 1:		// Throw blast right
			value = flrand(0.2, 0.8);
			VectorMA(spawnpt, value*wall->radius, wall->right, spawnpt);
			VectorSet(spawnvel, flrand(-16.0,16.0), flrand(-16.0,16.0), 0.0);
			scale = 1-value;
			break;
		case 2:		// Blast about at the center
			spawnpt[2] += flrand(0, 0.2)*FIREWAVE_DOWN;
			scale = 0.8;
			break;
		case 3:
		default:	// Throw blast down
			VectorMA(spawnpt, flrand(-0.4,0.4)*wall->radius, wall->right, spawnpt);
			spawnpt[2] += flrand(0.3,0.6)*FIREWAVE_DOWN;
			scale = 0.8;
			break;
		}

		VectorAdd(spawnpt, wall->r.origin, spawnpt);
		// vary a bit above or below the wall as well...
		spawnpt[2] -= FIREWAVE_DOWN;

		blast = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_PULSE_ALPHA, spawnpt, NULL, 500);
		blast->r.model = wall_models;
		blast->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		blast->radius = 64.0;

		VectorMA(spawnvel, flrand(0.1, 0.2), wall->velocity, blast->velocity);
		blast->acceleration[2] += 300.0;
		blast->alpha = 0.01;
		blast->d_alpha = flrand(4.0, 5.0);
		blast->r.scale = scale*detailscale;
		blast->d_scale = scale*detailscale * flrand(-1.0, -1.5);
		
		AddEffect(NULL, blast);
	}

	if (fxi.cl->time >= wall->nextEventTime)
	{	// Spawn a worm.

		// Find a random spot somewhere to have a fire worm hit.
		VectorMA(wall->r.origin, flrand(-1.0,1.0) * wall->radius, wall->right, destpt);

		// Trace back a little bit and spawn the fire worm there.
		VectorMA(destpt, -0.5*FIREWORM_LIFETIME, wall->velocity, spawnpt);

		// Trace down a bit to look for a nice place to spawn stuff.
		VectorCopy(destpt, bottom);
		bottom[2] -= FIREWAVE_TRACEDOWN;
		fxi.Trace(destpt, minmax, minmax, bottom, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

		if(trace.fraction < .99)
		{	// Hit the ground, smack it!!!
			VectorCopy(trace.endpos, destpt);
			hitground = true;
		}

		worm = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_ADDITIVE_PARTS, spawnpt, NULL, 75);
		worm->r.model = wall_models+2;
		worm->r.frame = 2;
		worm->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		worm->radius = 64.0;

		VectorCopy(spawnpt, worm->startpos);
		VectorCopy(destpt, worm->endpos);
		worm->alpha = 0.95;

		// New worm, but a small one.
		worm->r.scale = 0.5*detailscale;

		worm->Update = FXFireWormThink;
		worm->color.c = 0xff0080ff;
		worm->dlight = CE_DLight_new(worm->color, 128.0, 0.0);
		VectorCopy(wall->velocity, worm->velocity);
		worm->velocity[2] += FIREWORM_INITVEL;
		worm->acceleration[2] = FIREWORM_ACCEL;

		if (hitground)
			worm->SpawnInfo = 1;

		AddEffect(NULL, worm);

		FXFireWormThink(worm, NULL);

		wall->nextEventTime = fxi.cl->time + (flrand(0.8,1.2)*FIREWAVE_WORM_TIME);
	}

	return true;
}


// Create Effect FX_WEAPON_FIREWAVE
void FXFireWave(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*wall;
	short				shortyaw,shortpitch;

	wall = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_DONT_LINK, origin, NULL, 150);

	fxi.GetEffect(owner,flags,clientEffectSpawners[FX_WEAPON_FIREWAVE].formatString, &shortyaw, &shortpitch);

	wall->r.angles[YAW]=(float)shortyaw * (360.0/65536.0);
	wall->r.angles[PITCH]=(float)shortpitch * (360.0/65536.0);
	wall->r.angles[ROLL]=0.0;
	AngleVectors(wall->r.angles, wall->direction, wall->right, NULL);	

	if (flags & CEF_FLAG8)	// Since we're in deathmatch, throw it faster.
		VectorScale(wall->direction, FIREWAVE_DM_SPEED, wall->velocity);
	else
		VectorScale(wall->direction, FIREWAVE_SPEED, wall->velocity);

	if (flags&CEF_FLAG6)
	{	// Add a tad of left velocity
		VectorMA(wall->velocity, -FIREWAVE_DRADIUS, wall->right, wall->velocity);
	}
	else if (flags&CEF_FLAG7)
	{	// Add a tad of right velocity
		VectorMA(wall->velocity, FIREWAVE_DRADIUS, wall->right, wall->velocity);
	}

	wall->Update = FXFireWaveThink;
	wall->radius = FIREWAVE_RADIUS;
	wall->color.c = 0xff00afff;
	wall->dlight = CE_DLight_new(wall->color, 120.0F, 0.0F);
	wall->nextEventTime = fxi.cl->time + (flrand(0,1.0)*FIREWAVE_WORM_TIME);
	wall->SpawnInfo = 0;

	AddEffect(owner, wall);
}



// Create Effect FX_WEAPON_FIREWAVEWORM
void FXFireWaveWorm(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t *worm;
	vec3_t			fwd, spawnpt;
	vec3_t			bottom, minmax={0,0,0};
	trace_t			trace;
	float			detailscale;

	switch((int)(r_detail->value))
	{
	case DETAIL_LOW:
		detailscale = 0.5;
		break;
	case DETAIL_HIGH:
		detailscale = 0.9;
		break;
	case DETAIL_UBERHIGH:
		detailscale = 1.0;
		break;
	case DETAIL_NORMAL:
	default:
		detailscale = 0.7;
		break;
	}

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FIREWAVEWORM].formatString, fwd);		// Gets the movedir of the wall.

	// Trace back half a second and get the proper spawn location.
	VectorMA(origin, -0.5*FIREWORM_LIFETIME*FIREWAVE_SPEED, fwd, spawnpt);

	// Trace down a bit to look for a nice place to spawn stuff.
	VectorCopy(spawnpt, bottom);
	bottom[2] -= FIREWAVE_TRACEDOWN;
	fxi.Trace(spawnpt, minmax, minmax, bottom, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

	if(trace.fraction < .99)
	{	// Hit the ground, smack it!!!
		VectorCopy(trace.endpos, spawnpt);
	}

	worm = ClientEntity_new(FX_WEAPON_FIREWAVE, flags | CEF_ADDITIVE_PARTS, spawnpt, NULL, 75);
	worm->r.model = wall_models+2;
	worm->r.frame = 2;
	worm->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	worm->radius = 64.0;

	VectorCopy(spawnpt, worm->startpos);
	VectorCopy(origin, worm->endpos);
	worm->alpha = 0.95;
	worm->r.scale = 1.5*detailscale;
	worm->Update = FXFireWormThink;
	worm->color.c = 0xff0080ff;
	worm->dlight = CE_DLight_new(worm->color, 128.0, 0.0);
	VectorScale(fwd, FIREWAVE_SPEED, worm->velocity);
	worm->velocity[2] += FIREWORM_INITVEL;
	worm->acceleration[2] = FIREWORM_ACCEL;

	worm->SpawnInfo = 1;

	AddEffect(NULL, worm);

	FXFireWormThink(worm, NULL);
}





// *****************************************************************
// The fire blast
// Unpowered
// *****************************************************************

static FXFireBurstImpact(client_entity_t *wall)
{
	client_entity_t *blast;
	vec3_t			blastpt, spawnvel, blastvel;
	int				i;

	VectorScale(wall->direction, -48.0, blastpt);
	VectorAdd(blastpt, wall->r.origin, blastpt);
	VectorScale(wall->direction, -64.0, spawnvel);

	// Add some blasty bits along a line
	for (i=0; i<FIREWAVE_BLAST_NUM; i++)
	{	// Spawn along the top line of the wall
		if (i&0x01)
		{	// Throw blast to the right
			VectorMA(spawnvel, 128.0, wall->right, blastvel);
		}
		else
		{	// Throw blast to the left
			VectorMA(spawnvel, -128.0, wall->right, blastvel);
		}

		blast = ClientEntity_new(FX_WEAPON_FIREBURST, 0, blastpt, NULL, 500);
		blast->r.model = wall_models+2;
		blast->r.frame = 2;
		blast->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		blast->radius = 64.0;

		blast->alpha = 0.95;
		blast->r.scale = 1.2;
		blast->d_scale = -2.0;
		blast->d_alpha = -2.0;

		VectorMA(blastvel, flrand(-0.2, -0.1), wall->velocity, blast->velocity);
		
		AddEffect(NULL, blast);
	}
}



#define FIREBURST_PART_SPEED		160

static qboolean FXFireBurstThink(client_entity_t *wall, centity_t *owner)
{
	client_entity_t		*burst;

	vec3_t				newDir;
	vec3_t				origin, org;
	int					i, j;
	float				ang2;
	float				numFlameColumns;
	int					edgeVal;
	paletteRGBA_t		color;
	float				dtime, detailscale;

	if (owner->current.effects & EF_ALTCLIENTFX)
	{	// Time for this wall to die.
		if (wall->SpawnInfo != 1)
		{
			// Wait one second before disappearing.
			VectorClear(wall->velocity);
			wall->lastThinkTime = fxi.cl->time + 1000;
			wall->SpawnInfo = 1;
			FXFireBurstImpact(wall);
			return true;
		}
		else if (wall->lastThinkTime > fxi.cl->time)
		{	// Still some time left to live...
			wall->dlight->intensity -= 20.0;
			return true;
		}
		else
		{	// Time's up
			return false;
		}
	}

	switch((int)(r_detail->value))
	{
	case DETAIL_LOW:
		detailscale = 0.6;
		break;
	case DETAIL_HIGH:
		detailscale = 0.9;
		break;
	case DETAIL_UBERHIGH:
		detailscale = 1.0;
		break;
	case DETAIL_NORMAL:
	default:
		detailscale = 0.75;
		break;
	}

	color.c = 0xe5007fff;
	if (wall->dlight->intensity < 250.0)
		wall->dlight->intensity += 15.0;

	dtime = 1.0 + ((fxi.cl->time - wall->lastThinkTime) * (FIREBLAST_DRADIUS/1000.0));
	wall->radius = FIREBLAST_RADIUS*dtime;

	VectorMA(wall->r.origin, -24, wall->direction, origin);

	numFlameColumns = GetScaledCount(8, 0.8) + 4;

	for(i = 0; i < numFlameColumns; i++)
	{
		ang2 = (M_PI) * (float)i/((float)numFlameColumns-1);

		VectorScale(wall->right, cos(ang2)*dtime, newDir);
		VectorMA(newDir, sin(ang2), wall->direction, newDir);

		VectorCopy(origin, org);
		org[2] -= 16;
		VectorMA(org, 16.0, newDir, org);

		edgeVal = abs((numFlameColumns/2)-i)*(12.0/numFlameColumns);

		j=1;
//		for(j = 0; j < 2; j++)
		{
			burst = ClientEntity_new(FX_WEAPON_FIREBURST, 0, org, NULL, 1000);

			burst->r.model = wall_models + 1;
			
			burst->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			burst->r.frame = 0;

			VectorScale(newDir, FIREBURST_PART_SPEED + irand(0, 40) - (edgeVal*10) + (j*20), burst->velocity);
			burst->velocity[2] += j * (90 - (edgeVal*9)) + flrand(0, 10);
			
			if(i&1)
			{
				burst->velocity[2] *= .5;
			}

			burst->r.scale = flrand(0.5, 0.75)*detailscale;
			burst->radius = 20.0;
			burst->d_scale = 1.0;
			burst->d_alpha = -2.5;
			
			burst->origin[0] += irand(-32, 32);
			burst->origin[1] += irand(-32, 32);
			burst->origin[2] += irand(-16, 16);
			
			burst->acceleration[2] = flrand(16, 64);
			burst->velocity[2] += flrand(16, 64);

			AddEffect(NULL,burst);

			if(((i == 0)||(i == numFlameColumns-1))&&(j == 1))
			{
				burst->dlight = CE_DLight_new(color, 150.0F, -250.0F);
			}
		}
	}

	return true;
}


// Create effect FX_WEAPON_FIREBURST
void FXFireBurst(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*wall;
	client_particle_t	*spark;
	vec3_t				fwd;
	int					i;
	short				shortyaw,shortpitch;

	wall = ClientEntity_new(type, flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_DONT_LINK, origin, NULL, 150);
	fxi.GetEffect(owner,flags,clientEffectSpawners[FX_WEAPON_FIREBURST].formatString, &shortyaw, &shortpitch);

	wall->r.angles[YAW]=(float)shortyaw * (360.0/65536.0);
	wall->r.angles[PITCH]=(float)shortpitch * (360.0/65536.0);
	wall->r.angles[ROLL]=0.0;

	// The Build the velocity out of the fwd vector constructed from the two angles given.
	AngleVectors(wall->r.angles, fwd, wall->right, NULL);
	VectorScale(fwd, FIREBLAST_SPEED, wall->velocity);

	// Zero out the direction Z velocity because it isn't used during the think.
	VectorCopy(fwd, wall->direction);
	wall->direction[2] = 0.0;
	wall->right[2] = 0.0;

//	wall->r.model = wall_models + 1;
//	wall->alpha = 0.01;

	wall->Update = FXFireBurstThink;
	wall->radius = FIREBLAST_RADIUS;
	wall->color.c = 0xff00afff;
//	wall->r.scale = 8.0;
//	wall->d_scale = 56.0;
	wall->dlight = CE_DLight_new(wall->color, 150.0F, 0.0F);
	wall->lastThinkTime = fxi.cl->time;

	AddEffect(owner, wall);

	// Okay, this weapon feels REALLY weak at launch, so I'm going to add a little punch to it.

	// Add a big ol' flash.
	wall = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 1000);
	wall->r.model = wall_models + 2;		// The starry halo.
	wall->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	wall->radius = 64.0;

	wall->r.scale = 0.1;
	wall->d_scale = 4.0;
	wall->alpha = 0.95;
	wall->d_alpha = -2.0;
	wall->color.c = 0xffffffff;
	VectorScale(fwd, FIREBLAST_SPEED*0.15, wall->velocity);
	
	AddEffect(NULL, wall);

	// And add a bunch o' particle blasty bits to it
	for (i=0; i<25; i++)
	{
		spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), wall->color, 1000);
		VectorSet(spark->velocity, 
				flrand(-0.1*FIREBLAST_SPEED, 0.1*FIREBLAST_SPEED),
				flrand(-0.1*FIREBLAST_SPEED, 0.1*FIREBLAST_SPEED),
				flrand(-0.1*FIREBLAST_SPEED, 0.1*FIREBLAST_SPEED));
		VectorAdd(wall->velocity, spark->velocity, spark->velocity);
		spark->d_alpha = flrand(-256.0, -512.0);
		spark->scale = 4.0;
		spark->d_scale = flrand(8.0, 16.0);

		AddParticleToList(wall, spark);
	}
}

