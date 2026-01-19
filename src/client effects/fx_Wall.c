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
	wall_models[0] = fxi.RegisterModel("sprites/spells/wflame.sp2");
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

static qboolean FireWormUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFireWormThink' in original logic.
{
	const float delta_time = (float)(fx_time - self->startTime) / FIREWORM_LIFETIME_MS;
	const float worm_scale = FIREWORM_BLAST_VELOCITY * self->r.scale; //mxd

	if (delta_time > FIREWORM_LIFETIME)
	{
		// Impact at the centerpoint.
		if (self->SpawnInfo == 0 || delta_time > FIREWORM_LIFETIME + FIREWORM_BLASTLIFE)
		{
			// Do nothing, wait for blast to expire.
			self->nextThinkTime = fx_time + 500;
			self->Update = RemoveSelfAI;

			return true;
		}

		client_entity_t* blast = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_ADDITIVE_PARTS, self->endpos, NULL, 500);

		blast->r.model = &wall_models[2]; // Halo sprite.
		blast->r.frame = 2; // Circular halo for blast.
		blast->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		blast->alpha = 0.95f;
		blast->d_alpha = -0.5f;
		blast->r.scale = 0.25f * self->r.scale;
		blast->d_scale = -3.0f * self->r.scale;

		const float vel_max = worm_scale * 0.25f; //mxd
		VectorSet(blast->velocity, flrand(-vel_max, vel_max), flrand(-vel_max, vel_max), flrand(0.0f, vel_max));

		AddEffect(NULL, blast);

		// Spray out in a big ring.
		float angle = flrand(0.0f, ANGLE_360);
		const float angle_increment = ANGLE_360 / FIREWORM_BLAST_NUM;

		for (int i = 0; i < 8; i++) //TODO: shouldn't this count to FIREWORM_BLAST_NUM?..
		{
			const vec3_t diff_pos = VEC3_SET(cosf(angle), sinf(angle), 0.0f);
			angle += angle_increment;

			// Higher particle.
			client_particle_t* spark_hi = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color_white, 500);

			VectorScale(diff_pos, FIREWORM_BLAST_RADIUS * self->r.scale, spark_hi->origin);
			VectorScale(diff_pos, flrand(0.45f, 0.5f) * worm_scale, spark_hi->velocity);
			spark_hi->velocity[2] += flrand(0.8f, 1.0f) * worm_scale;

			spark_hi->color.a = 254;
			spark_hi->d_alpha = flrand(-768.0f, -512.0f);
			spark_hi->scale = 16.0f * self->r.scale;
			spark_hi->d_scale = flrand(8.0f, 16.0f) * self->r.scale;

			AddParticleToList(blast, spark_hi);

			// Lower to ground particle.
			client_particle_t* spark_low = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color_white, 500);

			VectorCopy(spark_hi->origin, spark_low->origin);
			VectorCopy(spark_hi->velocity, spark_low->velocity);
			spark_low->velocity[2] *= 0.33f;

			spark_low->color.a = 254;
			spark_low->d_alpha = flrand(-768.0f, -512.0f);
			spark_low->scale = 16.0f * self->r.scale;
			spark_low->d_scale = flrand(8.0f, 16.0f) * self->r.scale;

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
			spark->scale = 16.0f * self->r.scale;
			spark->d_scale = flrand(-16.0f, -8.0f);

			AddParticleToList(blast, spark);
		}

		return true;
	}

	// Add a trail entity and particle trail segment.
	client_entity_t* trail = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, self->r.origin, NULL, 500);
	VectorClear(trail->velocity);
	AddEffect(NULL, trail);

	for (int i = 0; i < 4; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color_white, 500);

		VectorRandomSet(spark->velocity, 0.25f * FIREWORM_TRAIL_VELOCITY);
		VectorMA(spark->velocity, 0.5f, self->velocity, spark->velocity);
		VectorScale(spark->velocity, -2.0f, spark->acceleration);

		spark->color.a = 254;
		spark->d_alpha = flrand(-768.0f, -512.0f);
		spark->scale = 20.0f * self->r.scale;
		spark->d_scale = flrand(8.0f, 16.0f) * self->r.scale;

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
	Vec3AddAssign(wall->r.origin, blast_pt);

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
		blast->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
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

static qboolean FireWaveUpdate(client_entity_t* wall, centity_t* owner) //mxd. Named 'FXFireWaveThink' in original logic.
{
	float detail_scale;

	switch (R_DETAIL)
	{
		default:
		case DETAIL_LOW:		detail_scale = 0.5f; break;
		case DETAIL_NORMAL:		detail_scale = 0.7f; break;
		case DETAIL_HIGH:		detail_scale = 0.9f; break;
		case DETAIL_UBERHIGH:	detail_scale = 1.0f; break;
	}

	if (owner->current.effects & EF_ALTCLIENTFX)
	{
		// Time for this wall to die.
		if (!wall->firewavewall_expired)
		{
			// Wait one second before disappearing.
			VectorClear(wall->velocity);
			wall->lastThinkTime = fx_time + 1000;
			wall->firewavewall_expired = true;
			FireWaveImpact(wall);

			return true;
		}

		if (wall->lastThinkTime > fx_time)
		{
			// Still some time left to live...
			wall->dlight->intensity -= 20.0f;
			return true;
		}

		// Time's up
		return false;
	}

	// Update radius.
	wall->radius = FIREWAVE_RADIUS + (float)(fx_time - wall->startTime) * (FIREWAVE_DRADIUS / 1000.0f);

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
				spawn_pt[2] -= flrand(0.0f, 0.2f) * FIREWAVE_DOWN;
				scale = 0.8f;
				break;

			case 3:
			default: // Throw blast down.
				VectorMA(spawn_pt, flrand(-0.4f, 0.4f) * wall->radius, wall->right, spawn_pt);
				spawn_pt[2] -= flrand(0.3f, 0.6f) * FIREWAVE_DOWN;
				scale = 0.8f;
				break;
		}

		Vec3AddAssign(wall->r.origin, spawn_pt);
		spawn_pt[2] += FIREWAVE_UP; // Vary a bit above or below the wall as well...

		client_entity_t* blast_top = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_PULSE_ALPHA, spawn_pt, NULL, 500);

		blast_top->radius = 64.0f;
		blast_top->r.model = &wall_models[0]; // wflame sprite.
		blast_top->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		VectorMA(spawn_vel, flrand(0.1f, 0.2f), wall->velocity, blast_top->velocity);
		blast_top->acceleration[2] += -300.0f;
		blast_top->alpha = 0.01f;
		blast_top->d_alpha = flrand(4.0f, 5.0f);
		blast_top->r.scale = scale * detail_scale;
		blast_top->d_scale = scale * detail_scale * flrand(-1.5f, -1.0f);

		RE_SetupRollSprite(&blast_top->r, 128.0f, flrand(0.0f, 359.0f)); //mxd
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

		Vec3AddAssign(wall->r.origin, spawn_pt);
		spawn_pt[2] -= FIREWAVE_DOWN; // Vary a bit above or below the wall as well...

		client_entity_t* blast_bottom = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_PULSE_ALPHA, spawn_pt, NULL, 500);

		blast_bottom->radius = 64.0f;
		blast_bottom->r.model = &wall_models[0]; // wflame sprite.
		blast_bottom->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

		VectorMA(spawn_vel, flrand(0.1f, 0.2f), wall->velocity, blast_bottom->velocity);
		blast_bottom->acceleration[2] += 300.0f;
		blast_bottom->alpha = 0.01f;
		blast_bottom->d_alpha = flrand(4.0f, 5.0f);
		blast_bottom->r.scale = scale * detail_scale;
		blast_bottom->d_scale = scale * detail_scale * flrand(-1.5f, -1.0f);

		RE_SetupRollSprite(&blast_bottom->r, 128.0f, flrand(0.0f, 359.0f)); //mxd
		AddEffect(NULL, blast_bottom);
	}

	if (fx_time >= wall->nextEventTime)
	{
		// Spawn a worm.

		// Find a random spot somewhere to have a fire worm hit.
		vec3_t dest_pt;
		VectorMA(wall->r.origin, flrand(-1.0f, 1.0f) * wall->radius, wall->right, dest_pt);

		// Trace back a little bit and spawn the fire worm there.
		vec3_t spawn_pt;
		VectorMA(dest_pt, -0.5f * FIREWORM_LIFETIME, wall->velocity, spawn_pt);

		// Trace down a bit to look for a nice place to spawn stuff.
		trace_t trace;
		const vec3_t bottom = VEC3_INITA(dest_pt, 0.0f, 0.0f, -FIREWAVE_TRACEDOWN);
		fxi.Trace(dest_pt, vec3_origin, vec3_origin, bottom, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

		const qboolean hitground = (trace.fraction < 0.99f);
		if (hitground)
			VectorCopy(trace.endpos, dest_pt); // Hit the ground, smack it!!!

		client_entity_t* worm = ClientEntity_new(FX_WEAPON_FIREWAVE, CEF_ADDITIVE_PARTS, spawn_pt, NULL, 75);

		worm->radius = 64.0f;
		worm->r.model = &wall_models[2]; // halo sprite.
		worm->r.frame = 2;
		worm->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);

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
		worm->Update = FireWormUpdate;

		RE_SetupRollSprite(&worm->r, 64.0f, flrand(0.0f, 359.0f)); //mxd
		AddEffect(NULL, worm);
		FireWormUpdate(worm, NULL);

		wall->nextEventTime = fx_time + (int)(flrand(0.8f, 1.2f) * FIREWAVE_WORM_TIME);
	}

	return true;
}

static client_entity_t* FireWaveFlashInit(const int type, const int flags, const vec3_t origin) //mxd
{
	client_entity_t* flash = ClientEntity_new(type, flags, origin, NULL, 1000);

	flash->radius = 64.0f;
	flash->r.model = &wall_models[2]; // halo sprite.
	flash->r.frame = 2; //mxd. Use red halo.
	flash->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	flash->r.scale = 1.2f;
	flash->d_scale = -2.5f;
	flash->alpha = 0.9f;
	flash->d_alpha = -1.5f;

	RE_SetupRollSprite(&flash->r, 64.0f, flrand(0.0f, 359.0f));

	return flash;
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
	wall->nextEventTime = fx_time + (int)(flrand(0.0f, 1.0f) * FIREWAVE_WORM_TIME);
	wall->Update = FireWaveUpdate;

	AddEffect(owner, wall);

	//mxd. Add flash.
	vec3_t flash_org;
	VectorMA(origin, 18.0f, vec3_up, flash_org); // Offset a bit (so its spawned at chest height instead of pelvis height).

	client_entity_t* flash = FireWaveFlashInit(type, flags, flash_org);
	VectorScale(wall->direction, FIREBLAST_SPEED * 0.15f, flash->velocity);

	AddEffect(NULL, flash);
}

void FXFireWaveWorm(centity_t* owner, int type, const int flags, vec3_t origin)
{
	float detail_scale;

	switch (R_DETAIL)
	{
		default:
		case DETAIL_LOW:		detail_scale = 0.5f; break;
		case DETAIL_NORMAL:		detail_scale = 0.7f; break;
		case DETAIL_HIGH:		detail_scale = 0.9f; break;
		case DETAIL_UBERHIGH:	detail_scale = 1.0f; break;
	}

	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FIREWAVEWORM].formatString, dir); // Get the movedir of the wall.

	// Trace back half a second and get the proper spawn location.
	vec3_t spawn_pt;
	VectorMA(origin, -0.5f * FIREWORM_LIFETIME * FIREWAVE_SPEED, dir, spawn_pt);

	// Trace down a bit to look for a nice place to spawn stuff.
	trace_t trace;
	const vec3_t bottom = VEC3_INITA(spawn_pt, 0.0f, 0.0f, -FIREWAVE_TRACEDOWN);
	fxi.Trace(spawn_pt, vec3_origin, vec3_origin, bottom, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

	if (trace.fraction < 0.99f)
		VectorCopy(trace.endpos, spawn_pt); // Hit the ground.

	client_entity_t* worm = ClientEntity_new(FX_WEAPON_FIREWAVE, flags | CEF_ADDITIVE_PARTS, spawn_pt, NULL, 75);

	worm->radius = 64.0f;
	worm->r.model = &wall_models[2]; // Halo sprite.
	worm->r.frame = 2;
	worm->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	worm->r.scale = 1.5f * detail_scale;

	VectorCopy(spawn_pt, worm->startpos);
	VectorCopy(origin, worm->endpos);
	worm->alpha = 0.95f;
	worm->color.c = 0xff0080ff;
	worm->SpawnInfo = 1;

	VectorScale(dir, FIREWAVE_SPEED, worm->velocity);
	worm->velocity[2] += FIREWORM_INITIAL_VELOCITY;
	worm->acceleration[2] = FIREWORM_ACCELERATION;
	worm->dlight = CE_DLight_new(worm->color, 128.0f, 0.0f);
	worm->Update = FireWormUpdate;

	AddEffect(NULL, worm);
	FireWormUpdate(worm, NULL);
}

#pragma endregion

#pragma region ========================== FIRE BLAST (UNPOWERED) ==========================

static void FireBurstImpact(const client_entity_t* wall)
{
	vec3_t blast_pt;
	VectorScale(wall->direction, -48.0f, blast_pt);
	Vec3AddAssign(wall->r.origin, blast_pt);

	vec3_t spawn_vel;
	VectorScale(wall->direction, -64.0f, spawn_vel);

	// Add some blast bits along a line.
	for (int i = 0; i < FIREWAVE_BLAST_NUM; i++)
	{
		// Spawn along the top line of the wall.
		client_entity_t* blast = ClientEntity_new(FX_WEAPON_FIREBURST, 0, blast_pt, NULL, 500);

		blast->r.model = &wall_models[2]; // Halo sprite.
		blast->r.frame = 2;
		blast->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		blast->radius = 64.0f;

		blast->alpha = 0.95f;
		blast->r.scale = 1.2f;
		blast->d_scale = -2.0f;
		blast->d_alpha = -2.0f;

		vec3_t blast_vel;
		const float scale = ((i & 1) ? 1.0f : -1.0f); //mxd
		VectorMA(spawn_vel, 128.0f * scale, wall->right, blast_vel); // Throw blast to the right or right.
		VectorMA(blast_vel, flrand(-0.2f, -0.1f), wall->velocity, blast->velocity);

		AddEffect(NULL, blast);
	}
}

static qboolean FireBurstUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXFireBurstThink' in original logic.
{
#define FIREBURST_PART_SPEED	160.0f //mxd. int in original logic.

	static const paletteRGBA_t burst_dlight_color = { .c = 0xe5007fff };

	if (owner->current.effects & EF_ALTCLIENTFX)
	{
		// Time for this burst to die.
		if (!self->firewaveburst_expired)
		{
			// Wait one second before disappearing.
			VectorClear(self->velocity);
			self->lastThinkTime = fx_time + 1000;
			self->firewaveburst_expired = true;
			FireBurstImpact(self);

			return true;
		}

		if (self->lastThinkTime > fx_time)
		{
			// Still some time left to live...
			self->dlight->intensity -= 20.0f;
			return true;
		}

		// Time's up
		return false;
	}

	float detail_scale;

	switch (R_DETAIL)
	{
		default:
		case DETAIL_LOW:		detail_scale = 0.6f;  break;
		case DETAIL_NORMAL:		detail_scale = 0.75f; break;
		case DETAIL_HIGH:		detail_scale = 0.9f;  break;
		case DETAIL_UBERHIGH:	detail_scale = 1.0f;  break;
	}

	if (self->dlight->intensity < 250.0f)
		self->dlight->intensity += 15.0f;

	const float delta_time = 1.0f + (float)(fx_time - self->lastThinkTime) * (FIREBLAST_DRADIUS / 1000.0f);
	self->radius = FIREBLAST_RADIUS * delta_time;

	vec3_t origin;
	VectorMA(self->r.origin, -24.0f, self->direction, origin);

	const int num_flame_columns = GetScaledCount(8, 0.8f) + 4;

	for (int i = 0; i < num_flame_columns; i++)
	{
		const float angle = ANGLE_180 * (float)i / ((float)num_flame_columns - 1);

		vec3_t dir;
		VectorScale(self->right, cosf(angle) * delta_time, dir);
		VectorMA(dir, sinf(angle), self->direction, dir);

		vec3_t cur_origin = VEC3_INITA(origin, 0.0f, 0.0f, -16.0f);
		VectorMA(cur_origin, 16.0f, dir, cur_origin);

		client_entity_t* burst = ClientEntity_new(FX_WEAPON_FIREBURST, 0, cur_origin, NULL, 1000);

		burst->radius = 20.0f;
		burst->r.model = &wall_models[1]; // wflame2 sprite.
		burst->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		burst->r.scale = flrand(0.5f, 0.75f) * detail_scale;
		burst->d_scale = 1.0f;
		burst->d_alpha = -2.5f;

		const float edge_val = fabsf(((float)num_flame_columns / 2.0f) - (float)i) * (12.0f / (float)num_flame_columns);
		VectorScale(dir, FIREBURST_PART_SPEED + flrand(0.0f, 40.0f) - (edge_val * 10.0f) + 20.0f, burst->velocity); //mxd. irand() in original logic.
		burst->velocity[2] += 90.0f - (edge_val * 9.0f) + flrand(0.0f, 10.0f);

		if (i & 1)
			burst->velocity[2] *= 0.5f;

		//mxd. Original logic sets burst->origin instead (didn't affect burst position).
		burst->r.origin[0] += flrand(-32.0f, 32.0f); //mxd. irand() in original logic.
		burst->r.origin[1] += flrand(-32.0f, 32.0f); //mxd. irand() in original logic.
		burst->r.origin[2] += flrand(-16.0f, 16.0f); //mxd. irand() in original logic.

		burst->acceleration[2] = flrand(16.0f, 64.0f);
		burst->velocity[2] += flrand(16.0f, 64.0f);

		RE_SetupRollSprite(&burst->r, 64.0f, flrand(0.0f, 359.0f)); //mxd
		AddEffect(NULL, burst);

		if (i == 0 || i == num_flame_columns - 1)
			burst->dlight = CE_DLight_new(burst_dlight_color, 150.0f, -250.0f);
	}

	return true;
}

void FXFireBurst(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	short short_yaw;
	short short_pitch;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_FIREBURST].formatString, &short_yaw, &short_pitch);

	client_entity_t* wall = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_DONT_LINK), origin, NULL, 150);

	wall->radius = FIREBLAST_RADIUS;
	wall->color.c = 0xff00afff;
	wall->lastThinkTime = fx_time;

	wall->r.angles[YAW] = SHORT2ANGLE(short_yaw);
	wall->r.angles[PITCH] = SHORT2ANGLE(short_pitch);
	wall->r.angles[ROLL] = 0.0f;

	// The Build the velocity out of the forward vector constructed from the two angles given.
	vec3_t forward;
	AngleVectors(wall->r.angles, forward, wall->right, NULL);
	VectorScale(forward, FIREBLAST_SPEED, wall->velocity);

	// Zero out the direction Z velocity because it isn't used during the think.
	VectorCopy(forward, wall->direction);
	wall->direction[2] = 0.0f;
	wall->right[2] = 0.0f;

	wall->dlight = CE_DLight_new(wall->color, 150.0f, 0.0f);
	wall->Update = FireBurstUpdate;

	AddEffect(owner, wall);

	// Okay, this weapon feels REALLY weak at launch, so I'm going to add a little punch to it.
	vec3_t flash_org;
	VectorMA(origin, 18.0f, vec3_up, flash_org); //mxd. Offset a bit (so its spawned at chest height instead of pelvis height).

	client_entity_t* flash = FireWaveFlashInit(type, flags | CEF_ADDITIVE_PARTS, flash_org);
	VectorScale(forward, FIREBLAST_SPEED * 0.15f, flash->velocity);

	AddEffect(NULL, flash);

	// Add a bunch o' particle blast bits to it.
	for (int i = 0; i < 25; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), flash->color, 1000);

		VectorRandomSet(spark->velocity, FIREBLAST_SPEED * 0.1f);
		Vec3AddAssign(flash->velocity, spark->velocity);
		spark->d_alpha = flrand(-512.0f, -256.0f);
		spark->scale = 4.0f;
		spark->d_scale = flrand(8.0f, 16.0f);

		AddParticleToList(flash, spark);
	}
}

#pragma endregion