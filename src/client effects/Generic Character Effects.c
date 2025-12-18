//
// Generic Character Effects.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "Vector.h"
#include "Random.h"
#include "Utilities.h"
#include "g_playstats.h"
#include "Matrix.h"
#include "Reference.h"
#include "turbsin.h"

static struct model_s* genfx_models[6];

void PreCacheOgleHitPuff(void) //mxd. Named 'PrecacheOgleHitPuff' in original logic.
{
	genfx_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	genfx_models[1] = fxi.RegisterModel("models/debris/stone/schunk1/tris.fm");
	genfx_models[2] = fxi.RegisterModel("models/debris/stone/schunk2/tris.fm");
	genfx_models[3] = fxi.RegisterModel("models/debris/stone/schunk3/tris.fm");
	genfx_models[4] = fxi.RegisterModel("models/debris/stone/schunk4/tris.fm");
	genfx_models[5] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

static qboolean ParticleTrailUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'ParticleTrailAI' in original logic.
{
#define PARTICLE_TRAIL_PUFF_TIME 1000 // Puffs last for 1 sec.

	assert(owner);

	client_entity_t* effect = ClientEntity_new(FX_PUFF, CEF_NO_DRAW, owner->current.old_origin, NULL, PARTICLE_TRAIL_PUFF_TIME);

	for (int i = 0; i < 40; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE, self->color, PARTICLE_TRAIL_PUFF_TIME);
		VectorSet(p->velocity, flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(30.0f, 80.0f));
		AddParticleToList(effect, p);
	}

	AddEffect(NULL, effect); // Add the puff to the world.

	return true; // Actual puff spawner only goes away when it owner has a FX_REMOVE_EFFECTS sent on it.
}

void GenericGibTrail(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	assert(owner);

	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);
	effect->flags |= CEF_NO_DRAW;
	effect->color.c = 0xFF2020FF;
	effect->Update = ParticleTrailUpdate;

	AddEffect(owner, effect);
	ParticleTrailUpdate(effect, owner); // Think once right away, to spawn the first puff.
}

static qboolean PebbleUpdate(struct client_entity_s* self, centity_t* owner)
{
	const int cur_time = fx_time;
	const float d_time = (float)(cur_time - self->lastThinkTime) / 1000.0f;

	self->acceleration[2] -= 75.0f;
	self->r.angles[0] += ANGLE_360 * d_time;
	self->r.angles[1] += ANGLE_360 * d_time;

	self->lastThinkTime = cur_time;

	return cur_time <= self->LifeTime;
}

// Slight variation on the normal puff.
void FXOgleHitPuff(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_OGLE_HITPUFF].formatString, dir); // Normalized direction vector.

	const float speed = VectorNormalize(dir);
	const int count = (speed > 1.0f ? irand(10, 15) : irand(1, 4));

	for (int i = 0; i < count; i++)
	{
		// Puff!
		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, 500);

		puff->r.model = &genfx_models[0];
		puff->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);

		if (speed > 1.0f)
			VectorScale(work, speed, puff->velocity);
		else if (flags & CEF_FLAG6)
			VectorScale(work, flrand(8.0f, 12.0f), puff->velocity);
		else
			VectorScale(work, 4.0f, puff->velocity);

		puff->acceleration[2] = flrand(10.0f, 50.0f);
		puff->alpha = 0.35f;
		puff->r.scale = (speed > 1.0f ? flrand(0.3f, 0.75f) : 0.1f);
		puff->d_scale = 2.0f;
		puff->d_alpha = -2.0f;
		puff->color = color_white; //mxd

		AddEffect(NULL, puff); // Add the effect as independent world effect.
	}

	for (int i = 0; i < count; i++)
	{
		// Rock!
		client_entity_t* rock = ClientEntity_new(type, flags, origin, NULL, 50);

		rock->r.model = &genfx_models[irand(1, 4)];

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, (speed > 1.0f ? speed : flrand(8.0f, 16.0f)), rock->velocity);

		if (flags & CEF_FLAG6 || speed > 1.0f)
			VectorSet(rock->acceleration, flrand(-75.0f, 75.0f), flrand(-75.0f, 75.0f), flrand(125.0f, 250.0f));
		else
			rock->acceleration[2] = flrand(-50.0f, 50.0f);

		rock->Update = PebbleUpdate;

		if (speed > 1.0f)
			rock->r.scale = flrand(0.8f, 1.5f) * speed / 100;
		else
			rock->r.scale = flrand(0.1f, 0.25f);

		rock->d_scale = 0.0f;
		rock->d_alpha = 0.0f;
		rock->color = color_white; //mxd
		rock->LifeTime = fx_time + 5000;

		AddEffect(NULL, rock); // Add the effect as independent world effect.
	}
}

void FXGenericHitPuff(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte count;
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_HITPUFF].formatString, dir, &count); // Normalized direction vector.
	count = min(40, count);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* fx = ClientEntity_new(type, flags, origin, NULL, 500);

		fx->r.model = &genfx_models[0];
		fx->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		vec3_t work;
		VectorRandomCopy(dir, work, 0.5f);
		VectorScale(work, 50.0f, fx->velocity);
		fx->acceleration[2] = -100.0f;
		fx->alpha = 0.5f;
		fx->r.scale = 0.1f;
		fx->d_scale = 1.0f;
		fx->d_alpha = -1.0f;
		fx->color = color_white; //mxd

		AddEffect(NULL, fx); // Add the effect as independent world effect.
	}

	if (flags & CEF_FLAG6)
	{
		// High-intensity impact point.
		client_entity_t* impact = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 250);

		impact->r.model = &genfx_models[5];
		impact->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		impact->r.scale = 0.4f;
		impact->d_scale = -1.6f;
		impact->d_alpha = -3.0f; // Alpha goes up to 1, then down to zero.
		impact->color.c = 0xc0ffffff;
		impact->radius = 10.0f;
		impact->alpha = 0.8f;
		impact->r.origin[2] += 8.0f;

		AddEffect(NULL, impact); // Add the effect as independent world effect.
	}
}

#define SINEAMT					1
#define SINESCALE				(256.0f / (2 * M_PI))
#define WATER_DENSITY			150.0f
#define WATER_DIST				100.0f
#define WATERPARTICLE_CLIPDIST	(WATER_DIST * WATER_DIST)

static void SetupWaterParticle(client_particle_t* p, const qboolean recycle)
{
	const byte ishade = (byte)irand(50, 150);
	p->color.r = ishade;
	p->color.g = ishade;
	p->color.b = ishade;
	p->color.a = (byte)irand(75, 150);

	const float min_vel_z = (((float)ishade * 0.04f) - 3.1f) * 0.35f;
	VectorSet(p->velocity, flrand(-1.0f, 1.0f), flrand(-1.0f, 1.0f), flrand(min_vel_z, 0.0f));

	p->acceleration[2] = 0.0f;
	p->scale = flrand(0.3f, 0.7f);

	vec3_t dist;
	VectorRandomSet(dist, WATER_DIST); //mxd

	// If we are recycling, we want to respawn as far away as possible.
	if (recycle)
	{
		VectorNormalize(dist);
		Vec3ScaleAssign(WATER_DIST, dist);
	}

	VectorAdd(fxi.cl->refdef.vieworg, dist, p->origin);
}

static void CreateWaterParticles(client_entity_t* self)
{
	// Scale number of particles by detail level.
	const int detail = (int)(WATER_DENSITY * r_detail->value);

	for (int i = 0; i < detail; i++)
	{
		client_particle_t* p = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, self->color, 1000000);
		SetupWaterParticle(p, false);
		AddParticleToList(self, p);
	}
}

static void UpdateWaterParticles(const client_entity_t* self)
{
	for (client_particle_t* p = self->p_root; p != NULL; p = p->next)
	{
		vec3_t part_dist;
		VectorSubtract(p->origin, fxi.cl->refdef.vieworg, part_dist);
		const float dist = VectorLengthSquared(part_dist);

		if (dist >= WATERPARTICLE_CLIPDIST)
		{
			SetupWaterParticle(p, true);
			continue;
		}

		float add_val = SINEAMT / 128.0f * turbsin[(int)(((float)fx_time * 0.001f + (self->origin[0] * 2.3f + p->origin[1]) * 0.0015f) * SINESCALE) & 255];
		add_val +=		SINEAMT / 256.0f * turbsin[(int)(((float)fx_time * 0.002f + (self->origin[1] * 2.3f + p->origin[0]) * 0.0015f) * SINESCALE) & 255];

		p->origin[2] += add_val;
		p->duration = fx_time + 10000000;
	}
}

static qboolean WaterParticleGeneratorUpdate(client_entity_t* self, centity_t* owner)
{
	static qboolean water_particles_spawned;

	if ((int)cl_camera_under_surface->value)
	{
		// Create particles when we are under water.
		if (!water_particles_spawned)
		{
			CreateWaterParticles(self);
			water_particles_spawned = true;
		}

		UpdateWaterParticles(self);
	}
	else
	{
		// Free up particles when we are not under water.
		if (water_particles_spawned)
		{
			FreeParticles(self);
			water_particles_spawned = false;
		}
	}

	return true;
}

static void DoWake(client_entity_t* self, const centity_t* owner, const int refpt)
{
	static int wake_particle[6] =
	{
		PART_4x4_WHITE,
		PART_8x8_BUBBLE,
		PART_16x16_WATERDROP,
		PART_32x32_WFALL,
		PART_32x32_STEAM,
		PART_32x32_BUBBLE
	};

	const paletteRGBA_t light_color = { .r = 200, .g = 255, .b = 255, .a = 140 };

	vec3_t diff;
	VectorSubtract(owner->referenceInfo->references[refpt].placement.origin, owner->referenceInfo->oldReferences[refpt].placement.origin, diff);

	vec3_t diff2;
	VectorSubtract(owner->origin, self->endpos, diff2);
	Vec3AddAssign(diff2, diff);

	int num_parts = (int)(VectorLength(diff));
	num_parts = min(6, num_parts);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	vec3_t org;
	VectorCopy(owner->referenceInfo->references[refpt].placement.origin, org);

	// Create a rotation matrix.
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t handpt;
	Matrix3MultByVec3(rotation, org, handpt);
	Vec3AddAssign(owner->origin, handpt);

	vec3_t right;
	AngleVectors(owner->lerp_angles, NULL, right, NULL);

	for (int i = 0; i < num_parts; i++)
	{
		int type = wake_particle[irand(0, 5)];
		if (R_DETAIL == DETAIL_LOW)
			type |= PFL_SOFT_MASK;

		client_particle_t* p = ClientParticle_new(type, light_color, irand(1000, 2000));

		VectorRandomSet(p->origin, 4.0f); //mxd
		VectorAdd(handpt, p->origin, p->origin);

		p->scale = flrand(0.75f, 1.5f);
		p->color.a = (byte)irand(100, 200);

		VectorRandomSet(p->velocity, 2.0f); //mxd

		const float sign = (irand(0, 1) ? -1.0f : 1.0f);
		VectorMA(p->velocity, flrand(10, 2) * sign, right, p->velocity);

		p->acceleration[2] = 16.0f;
		p->d_scale = flrand(-0.15f, -0.1f);

		AddParticleToList(self, p);
	}
}

static qboolean BubbleSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'BubbleSpawner' in original logic.
{
	vec3_t org;

	if (!(int)cl_camera_under_surface->value)
		return true;

	// Spawn some bubbles too.
	VectorRandomSet(org, 20.0f); //mxd
	VectorAdd(org, owner->origin, org);
	MakeBubble(org, self);

	// Create a wake of bubbles!
	if (R_DETAIL >= DETAIL_HIGH && RefPointsValid(owner))
	{
		DoWake(self, owner, CORVUS_RIGHTHAND);
		DoWake(self, owner, CORVUS_LEFTHAND);
		DoWake(self, owner, CORVUS_RIGHTFOOT);
		DoWake(self, owner, CORVUS_LEFTFOOT);

		VectorCopy(owner->origin, self->endpos);

		return true;
	}

	return false; // Remove the effect.
}

void FXWaterParticles(centity_t* owner, const int type, int flags, vec3_t origin)
{
	assert(owner);

	flags |= CEF_NO_DRAW | CEF_ABSOLUTE_PARTS | CEF_OWNERS_ORIGIN | CEF_VIEWSTATUSCHANGED; //mxd

	// Spawn static water particle handler.
	client_entity_t* water_fx = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);

	water_fx->AddToView = LinkedEntityUpdatePlacement;
	water_fx->radius = 100.0f;
	water_fx->Update = WaterParticleGeneratorUpdate;

	AddEffect(owner, water_fx);

	// Spawn bubble spawner.
	client_entity_t* bubble_fx = ClientEntity_new(type, flags, origin, NULL, PARTICLE_TRAIL_THINK_TIME);

	bubble_fx->AddToView = LinkedEntityUpdatePlacement;
	bubble_fx->radius = 100.0f;
	bubble_fx->Update = BubbleSpawnerUpdate;
	VectorCopy(owner->origin, bubble_fx->endpos);

	AddEffect(owner, bubble_fx);
}

void FXCorpseRemove(centity_t* owner, const int type, int flags, vec3_t origin)
{
#define	NUM_FLAME_ITEMS		20
#define NUM_FLAME_PARTS		40
#define FLAME_ABSVEL		120

	int count = GetScaledCount(NUM_FLAME_ITEMS, 0.95f);
	count = ClampI(count, 8, 20);

	// Create main client entity.
	flags |= CEF_NO_DRAW;
	client_entity_t* flame_fx = ClientEntity_new(type, flags, origin, NULL, 600);
	flame_fx->radius = 10.0f;
	flame_fx->color = color_white; //mxd
	AddEffect(NULL, flame_fx);

	// Are we destroying a rat?
	const float vel1 = (float)((flags & CEF_FLAG6) ? FLAME_ABSVEL / 2 : FLAME_ABSVEL);

	// Large particles.
	float angle = 0.0f;
	while (angle < ANGLE_360)
	{
		client_particle_t* bp = ClientParticle_new(PART_32x32_BLACKSMOKE, flame_fx->color, 600);

		bp->scale = 16.0f;
		bp->d_scale = -25.0f;

		VectorSet(bp->velocity, vel1 * cosf(angle), vel1 * sinf(angle), 0.0f);
		VectorScale(bp->velocity, -0.3f, bp->acceleration);

		AddParticleToList(flame_fx, bp);

		angle += ANGLE_360 / (float)count;
	}

	const paletteRGBA_t color = { .c = 0xff4f4f4f };
	count = GetScaledCount(NUM_FLAME_PARTS, 0.1f);

	// Small particles.
	for (int i = 0; i < count; i++)
	{
		client_particle_t* sp = ClientParticle_new(PART_4x4_WHITE, color, 600);

		sp->scale = 1.0f;
		sp->d_scale = -1.0f;

		angle = flrand(0, ANGLE_360);
		const float vel = flrand(vel1, vel1 * 2.5f);
		VectorSet(sp->velocity, vel * cosf(angle), vel * sinf(angle), 0);
		VectorScale(sp->velocity, -0.3f, sp->acceleration);
		sp->type |= PFL_ADDITIVE | PFL_SOFT_MASK;

		AddParticleToList(flame_fx, sp);
	}
}

// Create the two circles that ring the player.
static qboolean FXLeaderUpdate(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXLeaderThink' in original logic.
{
#define LEADER_RAD				12
#define TOTAL_LEADER_EFFECTS	30

	if (--self->LifeTime == 0)
		self->LifeTime = TOTAL_LEADER_EFFECTS;

	// If we are ghosted or dead, don't do the effect.
	if ((owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD))
		return true;

	const paletteRGBA_t color = { .c = 0x7fffffff };

	// Create the ring of particles that goes up.
	client_particle_t* ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
	ce->acceleration[2] = 0.0f;
	VectorSet(ce->origin, LEADER_RAD * cosf(self->Scale), LEADER_RAD * sinf(self->Scale), 4.0f);
	ce->scale = 8.0f;
	AddParticleToList(self, ce);

	// Create the ring of particles that goes down.
	ce = ClientParticle_new(PART_16x16_SPARK_Y, color, 800);
	ce->acceleration[2] = 0.0f;
	VectorSet(ce->origin, LEADER_RAD * cosf(self->Scale + 3.14f), LEADER_RAD * sinf(self->Scale + 3.14f), 4.0f);
	ce->scale = 8.0f;
	AddParticleToList(self, ce);

	// Move the rings up/down next frame.
	self->Scale += 0.17f;

	return true;
}

// Create the entity the flight loops are on.
void FXLeader(centity_t* owner, const int type, int flags, vec3_t origin)
{
	flags |= (int)(CEF_NO_DRAW | CEF_ADDITIVE_PARTS);
	client_entity_t* glow = ClientEntity_new(type, flags, origin, NULL, 60);

	VectorClear(glow->origin);
	glow->LifeTime = TOTAL_LEADER_EFFECTS;
	glow->Scale = 0.0f;
	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = FXLeaderUpdate;

	AddEffect(owner, glow);
}

static qboolean FXFeetTrailUpdate(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXFeetTrailThink' in original logic.
{
#define FOOTTRAIL_RADIUS	2.0f
#define FOOTTRAIL_SCALE		8.0f
#define FOOTTRAIL_ACCEL		20.0f

	// If the reference points are culled, or we are ghosted or dead, don't do the effect.
	if (!RefPointsValid(owner) || (owner->current.renderfx & RF_TRANS_GHOST) || (owner->current.effects & EF_CLIENT_DEAD))
		return true;

	if (!(owner->current.effects & EF_SPEED_ACTIVE))
	{
		self->Update = RemoveSelfAI;
		self->updateTime = 1500; //BUGFIX: mxd. 'fxi.cl->time + 1500' in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	vec3_t firestart;
	VectorCopy(owner->referenceInfo->references[self->refPoint].placement.origin, firestart);

	// Create a rotation matrix
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	vec3_t origin;
	Matrix3MultByVec3(rotation, firestart, origin);
	Vec3AddAssign(owner->origin, origin);

	if (Vec3NotZero(self->origin))
	{
		// Create small particles
		const int count = GetScaledCount(5, 0.5f);

		vec3_t diff;
		VectorSubtract(self->origin, origin, diff);
		Vec3ScaleAssign(1.0f / (float)count, diff);

		vec3_t cur_pos = { 0 };
		const int hand_flame_dur = (R_DETAIL < DETAIL_NORMAL ? 1500 : 2000);
		const paletteRGBA_t color = { .c = 0xffffff40 };

		for (int i = 0; i < count; i++)
		{
			client_particle_t* flame = ClientParticle_new(PART_32x32_STEAM, color, hand_flame_dur);

			VectorRandomSet(flame->origin, FOOTTRAIL_RADIUS); //mxd
			VectorAdd(flame->origin, self->origin, flame->origin);
			VectorAdd(flame->origin, cur_pos, flame->origin);

			flame->scale = FOOTTRAIL_SCALE;
			VectorSet(flame->velocity, flrand(-5.0f, 5.0f), flrand(-5.0f, 5.0f), flrand(5.0f, 15.0f));
			flame->acceleration[2] = FOOTTRAIL_ACCEL;
			flame->d_scale = flrand(-10.0f, -5.0f);
			flame->d_alpha = flrand(-200.0f, -160.0f);
			flame->duration = (int)((255.0f * 1000.0f) / -flame->d_alpha); // Time needed to reach zero alpha.

			AddParticleToList(self, flame);
			Vec3SubtractAssign(diff, cur_pos);
		}
	}

	VectorCopy(origin, self->origin);

	return true;
}

void FXFeetTrail(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const short refpoints = (1 << CORVUS_LEFTFOOT) | (1 << CORVUS_RIGHTFOOT);
	const int flame_update = (R_DETAIL > DETAIL_NORMAL ? 50 : 75);

	VectorClear(origin);

	// Add a fiery trail effect to the player's hands / feet etc.
	for (int i = 0; i < 16; i++)
	{
		if (!(refpoints & (1 << i)))
			continue;

		client_entity_t* trail = ClientEntity_new(type, flags, origin, NULL, flame_update);

		VectorClear(trail->origin);
		trail->flags |= (int)(CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ABSOLUTE_PARTS);
		trail->radius = 40;
		trail->refPoint = (short)i;
		trail->color.c = 0xe5007fff; //TODO: unused?
		trail->AddToView = LinkedEntityUpdatePlacement;
		trail->Update = FXFeetTrailUpdate;

		AddEffect(owner, trail);
	}
}