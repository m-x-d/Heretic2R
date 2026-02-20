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
static struct sfx_s* ripper_impact_sound; //mxd

void PreCacheMaceball(void)
{
	mace_models[0] = fxi.RegisterModel("sprites/spells/maceball.sp2");
	mace_models[1] = fxi.RegisterModel("sprites/fx/halogreen.sp2"); //mxd. 'halo.sp2' in original logic.
	mace_models[2] = fxi.RegisterModel("sprites/fx/neon.sp2");
	mace_models[3] = fxi.RegisterModel("models/spells/maceball/tris.fm");
	mace_models[4] = fxi.RegisterModel("sprites/fx/ballstreak.sp2");
	mace_models[5] = fxi.RegisterModel("sprites/spells/patball.sp2");
	mace_models[6] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
}

void PreCacheRipperSFX(void) //mxd
{
	ripper_impact_sound = fxi.S_RegisterSound("weapons/RipperImpact.wav");
}

#pragma region ========================== MACE BALL ==========================

static qboolean MaceballUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXMaceballThink' in original logic.
{
	self->dlight->intensity = 150.0f + cosf((float)fx_time * 0.01f) * 20.0f;
	self->r.angles[2] += ANGLE_30;

	if (self->r.scale >= BALL_MAX_RADIUS)
		self->d_scale = 0.0f;

	return true;
}

void FXMaceball(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* ball = ClientEntity_new(type, flags, origin, NULL, 100);

	ball->r.model = &mace_models[0]; // Maceball sprite.
	ball->r.scale = BALL_RADIUS;
	ball->d_scale = BALL_GROWTH;
	ball->color.c = 0xff00ffff;
	ball->dlight = CE_DLight_new(ball->color, 150.0f, 0.0f);
	ball->Update = MaceballUpdate;

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

	client_entity_t* hit_fx = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, BALL_BOUNCE_LIFETIME);

	hit_fx->radius = BALL_RADIUS;
	VectorScale(normal, MACEBALL_SPARK_VEL, hit_fx->velocity); // This velocity is used by the sparks (to be more precise, hit_fx->r.origin is used to update particles position --mxd).
	AddEffect(NULL, hit_fx);

	Vec3ScaleAssign(8.0f, normal);

	// Draw a circle of expanding lines.
	vec3_t last_vel;
	VectorScale(right, MACEBALL_RING_VEL, last_vel);
	const int ring_flags = (CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS); //mxd
	float cur_yaw = 0.0f;

	for (int i = 0; i < NUM_RIPPER_PUFFS; i++)
	{
		cur_yaw += RIPPER_PUFF_ANGLE;

		client_entity_t* ring = ClientEntity_new(type, ring_flags, origin, NULL, 500);

		ring->r.model = &mace_models[2]; // Neon-green sprite.
		ring->r.frame = 1;
		ring->r.spriteType = SPRITE_LINE;
		ring->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
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
		Vec3AddAssign(normal, ring->velocity);
		Vec3AddAssign(normal, ring->velocity2);

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
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_G, color_white, BALL_BOUNCE_LIFETIME); //mxd. Use BALL_BOUNCE_LIFETIME define.

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

static qboolean RipperExplodeBallUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXRipperExplodeBallThink' in original logic.
{
	vec3_t diff;
	VectorScale(self->direction, -6.0f, diff);

	vec3_t cur_pos = VEC3_INIT(self->r.origin);
	float scale = 0.8f;

	for (int i = 0; i < 4; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_WEAPON_RIPPEREXPLODE, 0, cur_pos, NULL, 500);

		trail->r.model = &mace_models[6]; // Green spark sprite.
		trail->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		VectorCopy(self->velocity, trail->velocity);
		VectorScale(trail->velocity, -1.0f, trail->acceleration);
		trail->r.scale = scale;
		trail->d_scale = -scale;
		trail->alpha = 0.3f;
		trail->d_alpha = -0.6f;
		trail->radius = 10.0f;

		AddEffect(NULL, trail);

		Vec3AddAssign(diff, cur_pos);
		scale -= 0.12f;
	}

	return true;
}

void FXRipperExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	static const paletteRGBA_t halo_dlight_color = { .c = 0xff00e400 }; //mxd

	vec3_t caster_pos;
	byte byte_yaw;
	short ball_array[8];
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_RIPPEREXPLODE].formatString, caster_pos, &byte_yaw,
		&ball_array[0], &ball_array[1], &ball_array[2], &ball_array[3], &ball_array[4], &ball_array[5], &ball_array[6], &ball_array[7]);

	// Convert from a byte back to radians.
	float cur_yaw = (float)byte_yaw * BYTEANGLE_TO_RAD; //mxd. Use define.

	// Throw out a bunch o' balls.
	for (int i = 0; i < RIPPER_BALLS; i++)
	{
		// Create the ball.
		client_entity_t* ripper = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

		ripper->r.model = &mace_models[0]; // Maceball sprite.
		ripper->r.flags = RF_TRANSLUCENT; // Use the alpha channel.

		// Set up the velocities.
		VectorSet(ripper->velocity, cosf(cur_yaw), sinf(cur_yaw), 0.0f);
		VectorCopy(ripper->velocity, ripper->direction);
		vectoangles(ripper->velocity, ripper->r.angles);
		Vec3ScaleAssign(RIPPER_EXPLODE_SPEED, ripper->velocity);

		// Set up the basic attributes
		ripper->r.scale = 0.25f;
		ripper->r.color = color_white;
		ripper->radius = 10.0f;
		ripper->Update = RipperExplodeBallUpdate;

		// Add to the entity passed in, not the "owner".
		assert(ball_array[i]);
		AddEffect(&fxi.server_entities[ball_array[i]], ripper);

		cur_yaw += RIPPER_BALL_ANGLE;
	}

	// Draw the impact graphic.
	client_entity_t* halo = ClientEntity_new(type, 0, origin, NULL, 750); //mxd. next_think_time:50 in original logic. Increased to allow proper halo/dlight fading.

	halo->r.model = &mace_models[1]; // Halogreen sprite.
	halo->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	halo->radius = 32.0f; // H2: 20
	halo->r.scale = 0.5f; //H2: 0.75
	halo->d_scale = 1.25f; //H2: -1
	halo->d_alpha = -1.4f;

	halo->dlight = CE_DLight_new(halo_dlight_color, 96.0f, 110.0f); //mxd. color:color_white, intensity:150, d_intensity:-100 in original logic.
	CE_DLight_SetColorFade(halo->dlight, 0.0f, 0.0f, 0.0f, halo->updateTime); //mxd

	fxi.S_StartSound(halo->r.origin, -1, CHAN_WEAPON, ripper_impact_sound, 1.0f, ATTN_NORM, 0.0f);

	AddEffect(NULL, halo);

	// Draw a circle of expanding lines.
	vec3_t last_vel = VEC3_SET(RIPPER_RING_VEL, 0.0f, 0.0f);
	const int ring_flags = (CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS); //mxd
	cur_yaw = 0.0f;

	for (int i = 0; i < NUM_RIPPER_PUFFS; i++)
	{
		cur_yaw += RIPPER_PUFF_ANGLE;

		client_entity_t* ring = ClientEntity_new(type, ring_flags, origin, NULL, 750);

		ring->r.model = &mace_models[2]; // Neon-green sprite.
		ring->r.frame = 1;
		ring->r.spriteType = SPRITE_LINE;
		ring->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		ring->radius = 64.0f;
		ring->r.scale = 0.5f;
		ring->d_scale = 32.0f;
		ring->alpha = 0.1f;
		ring->d_alpha = 3.0f;

		// The startpos and startvel comes from the last velocity.
		VectorCopy(last_vel, ring->velocity);
		VectorScale(ring->velocity, -1.0f, ring->acceleration);
		VectorMA(origin, 0.01f, ring->velocity, ring->r.startpos); // Move the line out a bit to avoid a zero-length line.

		// The endpos is calculated from the current angle.
		VectorSet(ring->velocity2, RIPPER_RING_VEL * cosf(cur_yaw), RIPPER_RING_VEL * sinf(cur_yaw), 0.0f);
		VectorScale(ring->velocity2, -1.0f, ring->acceleration2);
		VectorMA(origin, 0.01f, ring->velocity2, ring->r.endpos); // Move the line out a bit to avoid a zero-length line.

		// Finally, copy the last velocity we used.
		VectorCopy(ring->velocity2, last_vel);

		// NOW apply the extra directional velocity.
		Vec3AddAssign(halo->velocity, ring->velocity);
		Vec3AddAssign(halo->velocity, ring->velocity2);

		AddEffect(NULL, ring);

		// Now spawn a particle quick to save against the nasty joints (ugh).
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_G, color_white, 750);

		VectorCopy(ring->r.startpos, spark->origin);
		VectorCopy(ring->velocity, spark->velocity);
		VectorCopy(ring->acceleration, spark->acceleration);
		spark->scale = 0.5f;
		spark->d_scale = 32.0f;
		spark->color.a = 1;
		spark->d_alpha = 768.0f;

		AddParticleToList(ring, spark);
	}

	// Get the length for the firing streak.
	vec3_t diff;
	VectorSubtract(origin, caster_pos, diff);
	const float length = VectorLength(diff);

	if (length > 8.0f)
	{
		// Draw the streak from the caster to the impact point.
		client_entity_t* flash = ClientEntity_new(FX_WEAPON_RIPPEREXPLODE, CEF_AUTO_ORIGIN, caster_pos, NULL, 500);

		flash->r.model = &mace_models[4]; // Ballstreak sprite.
		flash->r.spriteType = SPRITE_LINE;
		flash->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		flash->r.scale = 8.0f;
		flash->d_scale = -8.0f;
		flash->alpha = 0.5f;
		flash->d_alpha = -1.0f;

		VectorCopy(caster_pos, flash->r.endpos);
		VectorCopy(origin, flash->r.startpos);
		flash->radius = length * 0.5f;

		AddEffect(NULL, flash);

		// Draw some flashy bits along the line for thickness.
		int num = (int)(length / 32.0f);

		vec3_t cur_pos = VEC3_INIT(caster_pos);
		Vec3ScaleAssign(1.0f / (float)num, diff);

		num = min(40, num);

		for (int i = 0; i < num; i++)
		{
			flash = ClientEntity_new(FX_WEAPON_RIPPEREXPLODE, 0, cur_pos, NULL, 500);

			flash->r.model = &mace_models[5]; // Patball sprite.
			flash->r.frame = 1;
			flash->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
			flash->r.scale = 0.16f;
			flash->d_scale = -0.16f;
			flash->alpha = 0.5f;
			flash->d_alpha = -1.0f;

			AddEffect(NULL, flash);

			Vec3AddAssign(diff, cur_pos);
		}
	}
}

#pragma endregion