//
// fx_PhoenixExplode.c -- Part of 'fx_Phoenix.c' in original logic --mxd.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define EXPLODE_SPEED				220.0f // H2: 160
#define EXPLODE_GRAVITY				(-320.0f)
#define EXPLODE_SCALE				18.0f // H2: 14
#define EXPLODE_NUM_BITS			64 // H2: 32
#define EXPLODE_NUM_SHOCKWAVE_BITS	24 //mxd
#define EXPLODE_TIME_MAX			750
#define EXPLODE_SMOKE_LIFETIME		2000 //mxd

#define PHOENIXPOWER_NUM_RINGS		16 //mxd
#define PHOENIXPOWER_PARTS_PER_RING	32 //mxd

enum //mxd
{
	PEMDL_PHOENIX,
	PEMDL_EXPLOSION,

	PEMDL_NUM_MODELS
};

static struct model_s* phoenix_explode_models[PEMDL_NUM_MODELS];
static struct sfx_s* phoenix_explode_sounds[2]; //mxd

void PreCachePhoenixExplode(void)
{
	phoenix_explode_models[PEMDL_PHOENIX] =		fxi.RegisterModel("sprites/spells/phoenix.sp2");
	phoenix_explode_models[PEMDL_EXPLOSION] =	fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
}

void PreCachePhoenixExplodeSFX(void) //mxd
{
	phoenix_explode_sounds[0] = fxi.S_RegisterSound("weapons/PhoenixHit.wav");
	phoenix_explode_sounds[1] = fxi.S_RegisterSound("weapons/PhoenixPowerHit.wav");
}

#pragma region ========================== PHOENIX EXPLOSION ==========================

static qboolean PhoenixExplosionBallUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPhoenixExplosionBallThink' in original logic.
{
	if (fx_time - self->startTime > EXPLODE_TIME_MAX)
	{
		//mxd. Allow lingering smoke to expire... 
		self->Update = RemoveSelfAI;
		self->updateTime = EXPLODE_SMOKE_LIFETIME;
		self->flags |= CEF_NO_DRAW;

		return true;
	}

	const float vel_factor = (float)(fx_time - self->lastThinkTime) / 1000.0f;
	self->lastThinkTime = fx_time;

	// Spin the ball of fire while it expands and fades.
	self->r.angles[YAW] += self->velocity2[YAW] * vel_factor;
	self->r.angles[PITCH] += self->velocity2[PITCH] * vel_factor;

	if (self->dlight->intensity > 0.0f)
		self->dlight->intensity -= 5.0f;

	return true;
}

void PhoenixExplode(const int type, int flags, const vec3_t origin, const vec3_t dir)
{
	static const paletteRGBA_t light_color = { .c = 0xff00ffff };

	// Create the main big explosion sphere.
	client_entity_t* explosion = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. Update each frame.

	explosion->radius = 128.0f;
	explosion->r.model = &phoenix_explode_models[PEMDL_EXPLOSION]; // Outer explosion model.
	explosion->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT); //mxd. +RF_FULLBRIGHT flag.
	explosion->flags |= (CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA);
	explosion->alpha = 0.1f;
	explosion->d_alpha = 5.0f;
	explosion->d_scale = 7.0f;
	explosion->lastThinkTime = fx_time;
	explosion->r.angles[YAW] = flrand(-ANGLE_180, ANGLE_180); //mxd
	explosion->r.angles[PITCH] = flrand(-ANGLE_180, ANGLE_180); //mxd
	explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);
	explosion->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	explosion->Update = PhoenixExplosionBallUpdate;

	AddEffect(NULL, explosion);

	// Add some glowing blast particles.
	vec3_t vel;
	VectorScale(dir, flrand(EXPLODE_SPEED * 0.8f, EXPLODE_SPEED), vel); //mxd. Randomize velocity.
	const int count = GetScaledCount(EXPLODE_NUM_BITS, 0.3f);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, 1000);

		VectorRandomSet(spark->velocity, EXPLODE_SPEED);
		Vec3AddAssign(vel, spark->velocity);
		spark->acceleration[2] = flrand(EXPLODE_GRAVITY * 0.8f, EXPLODE_GRAVITY * 1.2f); //mxd. Randomize acceleration.
		spark->scale = flrand(EXPLODE_SCALE, EXPLODE_SCALE * 1.2f); //mxd. Randomize scale.
		spark->d_scale = flrand(-40.0f, -30.0f);
		spark->d_alpha = flrand(-500.0f, -400.0f);

		AddParticleToList(explosion, spark);
	}

	//mxd. Add shockwave ring particles (but only if hit surface is horizontal enough).
	float angle = flrand(0.0f, ANGLE_360);
	const float angle_increment = ANGLE_360 / EXPLODE_NUM_SHOCKWAVE_BITS;

	vec3_t up;
	PerpendicularVector(up, dir);

	vec3_t right;
	CrossProduct(up, dir, right);

	if (dir[2] > 0.7f || dir[2] < -0.7f)
	{
		for (int i = 0; i < EXPLODE_NUM_SHOCKWAVE_BITS; i++)
		{
			if (irand(0, EXPLODE_NUM_SHOCKWAVE_BITS / 2) == 0) // Randomly skip some bits.
				continue;

			const float part_angle = angle + flrand(-angle_increment * 0.2f, angle_increment * 0.2f);
			angle += angle_increment;

			vec3_t diff_pos;
			VectorScale(up, sinf(part_angle), diff_pos);
			VectorMA(diff_pos, cosf(part_angle), right, diff_pos);

			// Steam particle.
			client_particle_t* steam = ClientParticle_new(PART_32x32_STEAM, color_white, 500);

			VectorScale(diff_pos, PHOENIX_EXPLODE_RADIUS * 0.25f, steam->origin);
			VectorMA(steam->origin, flrand(8.0f, 16.0f), dir, steam->origin);

			VectorScale(diff_pos, flrand(300.0f, 360.0f), steam->velocity);
			steam->velocity[2] += flrand(16.0f, 24.0f);

			steam->color.a = (byte)irand(164, 192);
			steam->d_alpha = flrand(-500.0f, -350.0f);
			steam->scale = flrand(8.0f, 12.0f);
			steam->d_scale = flrand(32.0f, 48.0f);

			AddParticleToList(explosion, steam);
		}
	}

	//mxd. Add lingering smoke.
	angle = flrand(0.0f, ANGLE_360);

	for (int i = 0; i < EXPLODE_NUM_SHOCKWAVE_BITS; i++)
	{
		const float part_angle = angle + flrand(-angle_increment * 0.3f, angle_increment * 0.3f);
		angle += angle_increment;

		vec3_t diff_pos;
		VectorScale(up, sinf(part_angle), diff_pos);
		VectorMA(diff_pos, cosf(part_angle), right, diff_pos);

		client_particle_t* smoke = ClientParticle_new(PART_32x32_STEAM, color_white, EXPLODE_SMOKE_LIFETIME);

		const float scaler = flrand(0.1f, 1.0f);

		VectorRandomSet(smoke->origin, 16.0f);
		VectorMA(smoke->origin, PHOENIX_EXPLODE_RADIUS * (1.0f - scaler) * 0.7f, diff_pos, smoke->origin);
		VectorMA(smoke->origin, flrand(8.0f, 16.0f), dir, smoke->origin);

		VectorRandomSet(smoke->acceleration, 8.0f);
		VectorMA(smoke->acceleration, flrand(8.0f, 16.0f), dir, smoke->acceleration);

		smoke->color.a = (byte)(flrand(32.0f, 48.0f) * (0.5f + scaler * 0.5f));
		smoke->scale = flrand(24.0f, 32.0f) * (0.5f + scaler * 0.5f);
		smoke->d_scale = smoke->scale * flrand(0.2f, 0.3f);
		smoke->d_alpha = flrand(-30.0f, -20.0f);

		AddParticleToList(explosion, smoke);
	}

	fxi.S_StartSound(origin, -1, CHAN_AUTO, phoenix_explode_sounds[0], 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

#pragma region ========================== POWERED PHOENIX EXPLOSION ==========================

static qboolean PhoenixExplosionPowerBirdUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPhoenixExplosionBirdThink' in original logic.
{
	vec3_t bird_pos;
	VectorMA(self->r.origin, 16.0f, self->direction, bird_pos);

	//mxd. Extra bird for godrays-like effect.
	client_entity_t* new_bird = ClientEntity_new(-1, self->r.flags, bird_pos, NULL, 500);

	new_bird->radius = 128.0f;
	new_bird->r.model = &phoenix_explode_models[PEMDL_PHOENIX]; // Phoenix sprite.
	new_bird->r.flags = self->r.flags;
	new_bird->r.scale = self->r.scale;
	new_bird->alpha = self->alpha * 0.5f;
	new_bird->d_alpha = self->d_alpha * 1.5f;
	new_bird->d_scale = self->d_scale * 3.0f;

	//mxd. When going down, flip the sprite.
	if (self->direction[2] < 0.0f)
		RE_SetupRollSprite(&new_bird->r, 128.0f, 180.0f);

	AddEffect(NULL, new_bird);

	return true;
}

static qboolean PhoenixExplosionPowerUpdate(client_entity_t* self, centity_t* owner) //mxd
{
	self->LifeTime--;

	if (self->LifeTime == 0)
	{
		self->updateTime = 4000; // Wait 4 seconds to allow attached particles to expire.
		self->Update = RemoveSelfAI;

		return true;
	}

	if (self->dlight->intensity > 0.0f)
		self->dlight->intensity -= 37.5f;

	// Spawn current particles ring.
	vec3_t up;
	PerpendicularVector(up, self->direction);

	vec3_t right;
	CrossProduct(up, self->direction, right);

	const float lerp = ((float)self->LifeTime / (float)PHOENIXPOWER_NUM_RINGS); // [~0.9 .. ~0.08]
	const float scaler = 0.25f - sinf(-ANGLE_90 * lerp) * 0.75f; // [~0.99 .. ~0.34]
	int count = (int)((float)PHOENIXPOWER_PARTS_PER_RING * (1.0f - lerp * 0.5f));
	count = GetScaledCount(count, 0.3f);

	float angle = flrand(0.0f, ANGLE_360);
	const float angle_increment = ANGLE_360 / (float)count;

	//mxd. Create current step of fire ring particle effect.
	for (int i = 0; i < count; i++)
	{
		vec3_t diff_pos;
		VectorScale(up, sinf(angle), diff_pos);
		VectorMA(diff_pos, cosf(angle), right, diff_pos);

		vec3_t trail_end;
		Vec3ScaleAssign(PHOENIX_EXPLODE_RADIUS_POWER, diff_pos); //mxd. Use actual explosion radius from the game logic.
		VectorAdd(self->r.origin, diff_pos, trail_end);

		angle += angle_increment;

		vec3_t trail_check_start;
		VectorMA(trail_end, 64.0f, self->direction, trail_check_start);

		vec3_t trail_check_end;
		VectorMA(trail_end, -64.0f, self->direction, trail_check_end);

		trace_t trace;
		fxi.Trace(trail_check_start, vec3_origin, vec3_origin, trail_check_end, CONTENTS_SOLID, CTF_CLIP_TO_WORLD | CTF_CLIP_TO_BMODELS, &trace);

		const qboolean hit_ground = (trace.fraction < 1.0f); //mxd

		if (hit_ground)
			VectorCopy(trace.endpos, trail_end);

		// Current particle ring...
		const paletteRGBA_t spark_color = { .r = 255, .g = (byte)(flrand(192.0f, 255.0f) * scaler), .b = (byte)(flrand(64, 255) * scaler), .a = 255};
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), spark_color, 1000);

		VectorLerp(trail_end, lerp, self->r.origin, spark->origin);
		VectorMA(spark->origin, flrand(8.0f, 16.0f), self->direction, spark->origin);

		// When burst in the air, alternate up and down.
		const float side = (!hit_ground && !(self->LifeTime & 1) ? -1.0f : 1.0f); //mxd
		VectorScale(self->direction, scaler * side * 128.0f + flrand(0.0f, 32.0f), spark->velocity);
		spark->scale = 32.0f + (1.0f - scaler) * flrand(16.0f, 24.0f);
		spark->d_scale = -spark->scale * 0.5f * scaler;
		spark->d_alpha = flrand(-300.0f, -220.0f) / (1.0f - lerp);

		AddParticleToList(self, spark);

		// Current smoke ring...
		if (i % 4 == 0 && irand(0, 1))
		{
			client_particle_t* smoke = ClientParticle_new(PART_32x32_STEAM, color_white, 4000);

			VectorRandomSet(smoke->origin, 16.0f);
			Vec3AddAssign(spark->origin, smoke->origin);

			smoke->color.a = (byte)(flrand(48.0f, 64.0f) * scaler);
			smoke->scale = flrand(32.0f, 48.0f);
			smoke->d_scale = smoke->scale * flrand(0.2f, 0.3f);
			smoke->d_alpha = flrand(-30.0f, -20.0f);

			vec3_t dir_out;
			VectorSubtract(spark->origin, self->origin, dir_out);
			VectorNormalize(dir_out);

			VectorRandomSet(smoke->acceleration, 16.0f);
			VectorMA(smoke->acceleration, flrand(32.0f, 48.0f), dir_out, smoke->acceleration);

			AddParticleToList(self, smoke);
		}
	}

	return true;
}

static void PhoenixExplodePower(const int type, int flags, const vec3_t origin, const vec3_t dir) //mxd. Named 'FXPhoenixExplodePower' in original logic.
{
	static const paletteRGBA_t light_color = { .c = 0xff00ffff };

	// Create the explosion core (so we can attach dlight and particles to it).
	client_entity_t* core = ClientEntity_new(type, (int)(flags | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS | CEF_NO_DRAW), origin, NULL, 25);

	core->radius = 128.0f;
	core->LifeTime = PHOENIXPOWER_NUM_RINGS;
	core->dlight = CE_DLight_new(light_color, 300.0f, 0.0f);
	VectorCopy(dir, core->direction);
	core->Update = PhoenixExplosionPowerUpdate;

	AddEffect(NULL, core);
	PhoenixExplosionPowerUpdate(core, NULL);

	// Draw the phoenix rising from the explosion.
	vec3_t phoenix_pos;
	VectorMA(origin, 76.0f, dir, phoenix_pos); //mxd. Add vertical offset.
	client_entity_t* phoenix = ClientEntity_new(type, flags, phoenix_pos, NULL, 0); //mxd. Update each frame.

	phoenix->radius = 128.0f;
	phoenix->r.model = &phoenix_explode_models[PEMDL_PHOENIX]; // Phoenix sprite.
	phoenix->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT); //mxd. +RF_FULLBRIGHT flag.
	phoenix->r.scale = 0.05f;
	VectorScale(dir, 128.0f, phoenix->velocity);
	phoenix->acceleration[2] = 64.0f; // H2: 256.0f.
	phoenix->d_alpha = -0.75f;
	phoenix->d_scale = 1.75f;
	VectorCopy(dir, phoenix->direction);
	phoenix->Update = PhoenixExplosionPowerBirdUpdate;

	//mxd. If visible, add RF_NODEPTHTEST flag.
	trace_t trace;
	fxi.Trace(phoenix->r.origin, vec3_origin, vec3_origin, fxi.cl->refdef.vieworg, (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER), CTF_CLIP_TO_WORLD | CTF_CLIP_TO_BMODELS, &trace);

	if (trace.fraction == 1.0f)
		phoenix->r.flags |= RF_NODEPTHTEST;

	//mxd. When going down, flip the sprite.
	if (dir[2] < 0.0f)
		RE_SetupRollSprite(&phoenix->r, 128.0f, 180.0f);

	AddEffect(NULL, phoenix);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, phoenix_explode_sounds[1], 1.0f, ATTN_NORM, 0.0f);
}

#pragma endregion

void FXPhoenixExplode(centity_t* owner, const int type, int flags, vec3_t origin)
{
	vec3_t dir;
	vec3_t scorch_dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_PHOENIXEXPLODE].formatString, dir, scorch_dir);

	// Make the scorch mark if we should.
	if (flags & CEF_FLAG8)
		FXClientScorchmark(origin, scorch_dir);

	// Powered-up version.
	if (flags & CEF_FLAG6)
		PhoenixExplodePower(type, flags, origin, dir);
	else
		PhoenixExplode(type, flags, origin, dir);
}