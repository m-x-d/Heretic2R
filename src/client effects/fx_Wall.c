//
// fx_Wall.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

static struct model_s* wall_models[3];

void PreCacheWall(void)
{
	wall_models[0] = fxi.RegisterModel("sprites/spells/wflame.sp2"); //TODO: wflame.m8 is missing from Htic2-0.pak!
	wall_models[1] = fxi.RegisterModel("sprites/spells/wflame2.sp2");
	wall_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

#pragma region =========================== FIRE WALL (POWERED) ===========================

#define FIREWORM_LIFETIME			1.0f
#define FIREWORM_BLASTLIFE			0.25f
#define FIREWORM_LIFETIME_MS		(FIREWORM_LIFETIME * 1000.0f)
#define FIREWORM_ACCELERATION		(-1000.0f)
#define FIREWORM_INITIAL_VELOCITY	(-0.5f * FIREWORM_LIFETIME * FIREWORM_ACCELERATION)
#define FIREWORM_TRAIL_VELOCITY		128
#define FIREWORM_BLAST_VELOCITY		128
#define FIREWORM_BLAST_NUM			12
#define FIREWORM_BLAST_RADIUS		32

static qboolean FireWormThink(client_entity_t* worm, centity_t* owner)
{
	const float delta_time = (float)(fxi.cl->time - worm->startTime) / FIREWORM_LIFETIME_MS;
	const float worm_scale = FIREWORM_BLAST_VELOCITY * worm->r.scale; //mxd

	if (delta_time > FIREWORM_LIFETIME)
	{
		// Impact at the centerpoint.
		if (worm->SpawnInfo == 0 || delta_time > FIREWORM_LIFETIME + FIREWORM_BLASTLIFE)
		{
			// Do nothing, wait for blast to expire.
			worm->nextThinkTime = fxi.cl->time + 500;
			worm->Update = RemoveSelfAI;

			return true;
		}

		client_entity_t* blast = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_ADDITIVE_PARTS, worm->endpos, NULL, 500);

		blast->r.model = &wall_models[2]; // Halo sprite.
		blast->r.frame = 2; // Circular halo for blast.
		blast->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		blast->alpha = 0.95f;
		blast->d_alpha = -0.5f;
		blast->r.scale = 0.25f * worm->r.scale;
		blast->d_scale = -3.0f * worm->r.scale;

		const float vel_max = worm_scale * 0.25f; //mxd
		VectorSet(blast->velocity, flrand(-vel_max, vel_max), flrand(-vel_max, vel_max), flrand(0.0f, vel_max));

		AddEffect(NULL, blast);

		// Spray out in a big ring.
		float angle = flrand(0.0f, ANGLE_360);
		const float angle_increment = ANGLE_360 / FIREWORM_BLAST_NUM;

		for (int i = 0; i < 8; i++) //TODO: shouldn't this count to FIREWORM_BLAST_NUM?..
		{
			const vec3_t diff_pos = { cosf(angle), sinf(angle), 0.0f };
			angle += angle_increment;

			// Higher particle.
			client_particle_t* spark_hi = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color_white, 500);

			VectorScale(diff_pos, FIREWORM_BLAST_RADIUS * worm->r.scale, spark_hi->origin);
			VectorScale(diff_pos, flrand(0.45f, 0.5f) * worm_scale, spark_hi->velocity);
			spark_hi->velocity[2] += flrand(0.8f, 1.0f) * worm_scale;

			spark_hi->color.a = 254;
			spark_hi->d_alpha = flrand(-768.0f, -512.0f);
			spark_hi->scale = 16.0f * worm->r.scale;
			spark_hi->d_scale = flrand(8.0f, 16.0f) * worm->r.scale;

			AddParticleToList(blast, spark_hi);

			// Lower to ground particle.
			client_particle_t* spark_low = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color_white, 500);

			VectorCopy(spark_hi->origin, spark_low->origin);
			VectorCopy(spark_hi->velocity, spark_low->velocity);
			spark_low->velocity[2] *= 0.33f;

			spark_low->color.a = 254;
			spark_low->d_alpha = flrand(-768.0f, -512.0f);
			spark_low->scale = 16.0f * worm->r.scale;
			spark_low->d_scale = flrand(8.0f, 16.0f) * worm->r.scale;

			AddParticleToList(blast, spark_low);
		}

		// Spray up in a little fountain too.
		for (int i = 0; i < 4; i++)
		{
			client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color_white, 500);

			VectorSet(spark->velocity,
				flrand(-0.1f * worm_scale, 0.1f * worm_scale),
				flrand(-0.1f * worm_scale, 0.1f * worm_scale),
				flrand(-0.2f * worm_scale, 0.2f * worm_scale));
			spark->velocity[2] += FIREWORM_BLAST_VELOCITY;

			spark->color.a = 254;
			spark->d_alpha = flrand(-768.0f, -512.0f);
			spark->scale = 16.0f * worm->r.scale;
			spark->d_scale = flrand(-16.0f, -8.0f);

			AddParticleToList(blast, spark);
		}

		return true;
	}

	// Add a trail entity and particle trail segment.
	client_entity_t* trail = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, worm->r.origin, NULL, 500);
	VectorClear(trail->velocity);
	AddEffect(NULL, trail);

	for (int i = 0; i < 4; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color_white, 500);

		VectorRandomSet(spark->velocity, 0.25f * FIREWORM_TRAIL_VELOCITY);
		VectorMA(spark->velocity, 0.5f, worm->velocity, spark->velocity);
		VectorScale(spark->velocity, -2.0f, spark->acceleration);

		spark->color.a = 254;
		spark->d_alpha = flrand(-768.0f, -512.0f);
		spark->scale = 20.0f * worm->r.scale;
		spark->d_scale = flrand(8.0f, 16.0f) * worm->r.scale;

		AddParticleToList(trail, spark);
	}

	return true;
}

#define FIREWAVE_TRACEDOWN		128.0f //mxd. Was int.
#define FIREWAVE_WORM_TIME		500.0f //mxd. Was int.
#define FIREWAVE_BLAST_NUM		4

static void FireWaveImpact(const client_entity_t* wall)
{
	vec3_t blast_pt;
	VectorScale(wall->direction, -48.0f, blast_pt);
	VectorAdd(blast_pt, wall->r.origin, blast_pt);

	vec3_t spawn_vel;
	VectorScale(wall->direction, -64.0f, spawn_vel);

	// Add some blast bits along a line.
	for (int i = 0; i < FIREWAVE_BLAST_NUM; i++)
	{
		// Spawn along the top line of the wall.
		client_entity_t* blast = ClientEntity_new(FX_WEAPON_FIREWAVE, 0, blast_pt, NULL, 500);

		blast->radius = 64.0f;
		blast->r.model = &wall_models[2]; // Halo sprite.
		blast->r.frame = 2;
		blast->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		blast->alpha = 0.95f;
		blast->r.scale = 1.6f;
		blast->d_scale = -2.0f;
		blast->d_alpha = -2.0f;

		const float dir = ((i & 1) ? 1.0f : -1.0f);
		spawn_vel[2] = 128.0f * dir; // Throw blast up or down.
		VectorMA(spawn_vel, flrand(-0.2f, -0.1f), wall->velocity, blast->velocity);

		AddEffect(NULL, blast);
	}
}

static qboolean FireWaveThink(client_entity_t* wall, centity_t* owner)
{
	float detail_scale;

	switch ((int)r_detail->value)
	{
		case DETAIL_LOW:
			detail_scale = 0.5f;
			break;

		case DETAIL_NORMAL:
		default:
			detail_scale = 0.7f;
			break;

		case DETAIL_HIGH:
			detail_scale = 0.9f;
			break;

		case DETAIL_UBERHIGH:
			detail_scale = 1.0f;
			break;
	}

	if (owner->current.effects & EF_ALTCLIENTFX)
	{
		// Time for this wall to die.
		if (wall->SpawnInfo != 1)
		{
			// Wait one second before disappearing.
			VectorClear(wall->velocity);
			wall->lastThinkTime = fxi.cl->time + 1000;
			wall->SpawnInfo = 1;
			FireWaveImpact(wall);

			return true;
		}

		if (wall->lastThinkTime > fxi.cl->time)
		{
			// Still some time left to live...
			wall->dlight->intensity -= 20.0f;
			return true;
		}

		// Time's up
		return false;
	}

	// Update radius.
	wall->radius = FIREWAVE_RADIUS + (float)(fxi.cl->time - wall->startTime) * (FIREWAVE_DRADIUS / 1000.0f);

	if (wall->dlight->intensity < 250.0f)
		wall->dlight->intensity += 15.0f;

	// Add some blast bits along a line.
	for (int i = 0; i < FIREWAVE_BLAST_NUM; i++)
	{
		float value;
		float scale;
		vec3_t spawn_vel;

		// Spawn along the top line of the wall.
		vec3_t spawn_pt;
		VectorRandomSet(spawn_pt, 6.0f);

		switch (i)
		{
			case 0: // Throw blast left.
				value = flrand(0.2f, 0.8f);
				VectorMA(spawn_pt, -value * wall->radius, wall->right, spawn_pt);
				VectorSet(spawn_vel, flrand(-16.0f, 16.0f), flrand(-16.0f, 16.0f), 0.0f);
				scale = 1.0f - value;
				break;

			case 1: // Throw blast right.
				value = flrand(0.2f, 0.8f);
				VectorMA(spawn_pt, value * wall->radius, wall->right, spawn_pt);
				VectorSet(spawn_vel, flrand(-16.0f, 16.0f), flrand(-16.0f, 16.0f), 0.0f);
				scale = 1.0f - value;
				break;

			case 2: // Blast about at the center.
				spawn_pt[2] -= flrand(0.0f, 0.2f) * FIREWAVE_DOWN; //TODO: shouldn't this use FIREWAVE_UP?..
				scale = 0.8f;
				break;

			case 3:
			default: // Throw blast down.
				VectorMA(spawn_pt, flrand(-0.4f, 0.4f) * wall->radius, wall->right, spawn_pt);
				spawn_pt[2] -= flrand(0.3f, 0.6f) * FIREWAVE_DOWN; //TODO: shouldn't this use FIREWAVE_UP?..
				scale = 0.8f;
				break;
		}

		VectorAdd(spawn_pt, wall->r.origin, spawn_pt);
		spawn_pt[2] += FIREWAVE_UP; // Vary a bit above or below the wall as well...

		client_entity_t* blast_top = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_PULSE_ALPHA, spawn_pt, NULL, 500);

		blast_top->radius = 64.0f;
		blast_top->r.model = &wall_models[0]; // wflame sprite.
		blast_top->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorMA(spawn_vel, flrand(0.1f, 0.2f), wall->velocity, blast_top->velocity);
		blast_top->acceleration[2] += -300.0f;
		blast_top->alpha = 0.01f;
		blast_top->d_alpha = flrand(4.0f, 5.0f);
		blast_top->r.scale = scale * detail_scale;
		blast_top->d_scale = scale * detail_scale * flrand(-1.5f, -1.0f);

		AddEffect(NULL, blast_top);

		// Spawn along the bottom line of the wall
		VectorRandomSet(spawn_pt, 6.0f);

		switch (i)
		{
			case 0: // Throw blast left.
				value = flrand(0.2f, 0.8f);
				VectorMA(spawn_pt, -value * wall->radius, wall->right, spawn_pt);
				VectorSet(spawn_vel, flrand(-16.0f, 16.0f), flrand(-16.0f, 16.0f), 0.0f);
				scale = 1.0f - value;
				break;

			case 1: // Throw blast right.
				value = flrand(0.2f, 0.8f);
				VectorMA(spawn_pt, value * wall->radius, wall->right, spawn_pt);
				VectorSet(spawn_vel, flrand(-16.0f, 16.0f), flrand(-16.0f, 16.0f), 0.0f);
				scale = 1.0f - value;
				break;

			case 2: // Blast about at the center.
				spawn_pt[2] += flrand(0.0f, 0.2f) * FIREWAVE_DOWN;
				scale = 0.8f;
				break;

			case 3:
			default: // Throw blast down.
				VectorMA(spawn_pt, flrand(-0.4f, 0.4f) * wall->radius, wall->right, spawn_pt);
				spawn_pt[2] += flrand(0.3f, 0.6f) * FIREWAVE_DOWN;
				scale = 0.8f;
				break;
		}

		VectorAdd(spawn_pt, wall->r.origin, spawn_pt);
		spawn_pt[2] -= FIREWAVE_DOWN; // Vary a bit above or below the wall as well...

		client_entity_t* blast_bottom = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_PULSE_ALPHA, spawn_pt, NULL, 500);

		blast_bottom->radius = 64.0f;
		blast_bottom->r.model = &wall_models[0]; // wflame sprite.
		blast_bottom->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorMA(spawn_vel, flrand(0.1f, 0.2f), wall->velocity, blast_bottom->velocity);
		blast_bottom->acceleration[2] += 300.0f;
		blast_bottom->alpha = 0.01f;
		blast_bottom->d_alpha = flrand(4.0f, 5.0f);
		blast_bottom->r.scale = scale * detail_scale;
		blast_bottom->d_scale = scale * detail_scale * flrand(-1.5f, -1.0f);

		AddEffect(NULL, blast_bottom);
	}

	if (fxi.cl->time >= wall->nextEventTime)
	{
		// Spawn a worm.

		// Find a random spot somewhere to have a fire worm hit.
		vec3_t dest_pt;
		VectorMA(wall->r.origin, flrand(-1.0f, 1.0f) * wall->radius, wall->right, dest_pt);

		// Trace back a little bit and spawn the fire worm there.
		vec3_t spawn_pt;
		VectorMA(dest_pt, -0.5f * FIREWORM_LIFETIME, wall->velocity, spawn_pt);

		// Trace down a bit to look for a nice place to spawn stuff.
		vec3_t bottom;
		VectorCopy(dest_pt, bottom);
		bottom[2] -= FIREWAVE_TRACEDOWN;

		trace_t trace;
		fxi.Trace(dest_pt, vec3_origin, vec3_origin, bottom, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

		const qboolean hitground = (trace.fraction < 0.99f);
		if (hitground)
			VectorCopy(trace.endpos, dest_pt); // Hit the ground, smack it!!!

		client_entity_t* worm = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_ADDITIVE_PARTS, spawn_pt, NULL, 75);

		worm->radius = 64.0f;
		worm->r.model = &wall_models[2]; // halo sprite.
		worm->r.frame = 2;
		worm->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorCopy(spawn_pt, worm->startpos);
		VectorCopy(dest_pt, worm->endpos);
		worm->alpha = 0.95f;

		// New worm, but a small one.
		worm->r.scale = 0.5f * detail_scale;
		worm->color.c = 0xff0080ff;
		worm->dlight = CE_DLight_new(worm->color, 128.0f, 0.0f);
		VectorCopy(wall->velocity, worm->velocity);
		worm->velocity[2] += FIREWORM_INITIAL_VELOCITY;
		worm->acceleration[2] = FIREWORM_ACCELERATION;
		worm->SpawnInfo = (hitground ? 1 : 0);
		worm->Update = FireWormThink;

		AddEffect(NULL, worm);
		FireWormThink(worm, NULL);

		wall->nextEventTime = fxi.cl->time + (int)(flrand(0.8f, 1.2f) * FIREWAVE_WORM_TIME);
	}

	return true;
}

void FXFireWave(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	short short_yaw;
	short short_pitch;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FIREWAVE].formatString, &short_yaw, &short_pitch);

	client_entity_t* wall = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_DONT_LINK), origin, NULL, 150);

	wall->radius = FIREWAVE_RADIUS;
	wall->r.angles[YAW] = SHORT2ANGLE(short_yaw);
	wall->r.angles[PITCH] = SHORT2ANGLE(short_pitch);
	wall->r.angles[ROLL] = 0.0f;

	AngleVectors(wall->r.angles, wall->direction, wall->right, NULL);

	const float speed = ((flags & CEF_FLAG8) ? FIREWAVE_DM_SPEED : FIREWAVE_SPEED); // Throw it faster in deathmatch.
	VectorScale(wall->direction, speed, wall->velocity);

	if (flags & CEF_FLAG6) // Add a tad of left velocity.
		VectorMA(wall->velocity, -FIREWAVE_DRADIUS, wall->right, wall->velocity);
	else if (flags & CEF_FLAG7) // Add a tad of right velocity.
		VectorMA(wall->velocity, FIREWAVE_DRADIUS, wall->right, wall->velocity);

	wall->color.c = 0xff00afff;
	wall->dlight = CE_DLight_new(wall->color, 120.0f, 0.0f);
	wall->nextEventTime = fxi.cl->time + (int)(flrand(0.0f, 1.0f) * FIREWAVE_WORM_TIME);
	wall->Update = FireWaveThink;

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
	worm->Update = FireWormThink;
	worm->color.c = 0xff0080ff;
	worm->dlight = CE_DLight_new(worm->color, 128.0, 0.0);
	VectorScale(fwd, FIREWAVE_SPEED, worm->velocity);
	worm->velocity[2] += FIREWORM_INITIAL_VELOCITY;
	worm->acceleration[2] = FIREWORM_ACCELERATION;

	worm->SpawnInfo = 1;

	AddEffect(NULL, worm);

	FireWormThink(worm, NULL);
}

#pragma endregion

#pragma region ========================== FIRE BLAST (UNPOWERED) ==========================

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

#pragma endregion