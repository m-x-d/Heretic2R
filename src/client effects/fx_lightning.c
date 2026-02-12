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
static struct sfx_s* lightning_sound; //mxd

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

void PreCachePowerLightningSFX(void) //mxd
{
	lightning_sound = fxi.S_RegisterSound("weapons/LightningPower.wav");
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

	Vec3ScaleAssign(seg_mult, diff_pos);
	const float variance = length * seg_mult * 0.4f;

	vec3_t last_pos = VEC3_INIT(start_pos);
	vec3_t ref_point = VEC3_INIT(start_pos);

	for (int i = 0; i < segments - 1; i++)
	{
		Vec3AddAssign(diff_pos, ref_point);

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

static qboolean LightningUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXLightningThink' in original logic.
{
	if (fx_time - self->lastThinkTime < self->LifeTime)
	{
		LightningBolt(self->SpawnInfo, self->xscale, self->r.startpos, self->r.endpos);
		return true;
	}

	return false;
}

void FXLightning(centity_t* owner, int type, const int flags, vec3_t origin)
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
		lightning->lastThinkTime = fx_time;
		lightning->LifeTime = duration * 100 + 250;
		lightning->SpawnInfo = ((flags & CEF_FLAG6) ? LIGHTNING_TYPE_RED : LIGHTNING_TYPE_BLUE);
		lightning->xscale = (float)width;
		lightning->Update = LightningUpdate;

		AddEffect(NULL, lightning);
	}

	int model = LIGHTNING_TYPE_BLUE;	// Normal, blue lightning

	if (flags & CEF_FLAG6)
		model = LIGHTNING_TYPE_RED;		// If flagged, do red lightning.
	else if (flags & CEF_FLAG7)
		model = LIGHTNING_TYPE_GREEN;	// Powered-up rain lightning.

	LightningBolt(model, width, origin, target);
}

void FXPowerLightning(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t target;
	byte b_width;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_POWER_LIGHTNING].formatString, target, &b_width);

	const float width = b_width; //mxd

	vec3_t diff_pos;
	VectorSubtract(target, origin, diff_pos);
	const float length = VectorLength(diff_pos);

	// Big ol' monster zapper.
	client_entity_t* lightning = ClientEntity_new(FX_POWER_LIGHTNING, CEF_AUTO_ORIGIN, origin, NULL, 750);

	lightning->radius = length;
	lightning->r.model = &lightning_models[LIGHTNING_TYPE_GREEN];
	lightning->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	lightning->r.spriteType = SPRITE_LINE;
	lightning->r.scale = width;
	lightning->d_scale = -0.5f * width;
	lightning->alpha = 0.95f;
	lightning->d_alpha = -1.5f;
	VectorCopy(origin, lightning->r.startpos);
	VectorCopy(target, lightning->r.endpos);

	AddEffect(NULL, lightning);

	// Halo around the lightning.
	client_entity_t* halo = ClientEntity_new(FX_POWER_LIGHTNING, CEF_AUTO_ORIGIN, origin, NULL, 1000);

	halo->r.model = &lightning_models[LIGHTNING_TYPE_GREEN];
	halo->r.frame = 1;
	halo->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	halo->r.spriteType = SPRITE_LINE;
	halo->r.scale = width * LIGHTNING_POWER_WIDTH_MULT;
	halo->d_scale = -0.5f * width;
	halo->radius = length;
	halo->alpha = 0.5f;
	halo->d_alpha = -0.5f;
	VectorCopy(origin, halo->r.startpos);
	VectorCopy(target, halo->r.endpos);

	AddEffect(NULL, halo);

	// Big ol' flash at source to cover up the flatness of the line's end.
	client_entity_t* start_flash = ClientEntity_new(FX_POWER_LIGHTNING, CEF_ADDITIVE_PARTS, origin, NULL, 750);

	start_flash->r.model = &lightning_models[6]; // The bright halo sprite.
	start_flash->r.frame = 1;
	start_flash->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	start_flash->r.scale = 0.75f;
	start_flash->d_scale = 2.0f;
	start_flash->radius = 128.0f;
	start_flash->alpha = 0.95f;
	start_flash->d_alpha = -1.333f;

	AddEffect(NULL, start_flash);

	// Now add a bunch of sparks to the source too to add interest.
	for (int i = 0; i < 8; i++)
	{
		// Half green, half yellow particles.
		const int particle_type = ((i & 1) ? PART_16x16_SPARK_Y : PART_16x16_SPARK_G); //mxd
		client_particle_t* spark = ClientParticle_new(particle_type, start_flash->color, 1000);

		VectorRandomSet(spark->velocity, 80.0f);
		VectorScale(spark->velocity, -1.0f, spark->acceleration);
		spark->scale = flrand(20.0f, 32.0f);
		spark->d_scale = -spark->scale;
		spark->d_alpha = flrand(-384.0f, -256.0f);

		AddParticleToList(start_flash, spark);
	}

	client_entity_t* end_flash = ClientEntity_new(FX_POWER_LIGHTNING, CEF_ADDITIVE_PARTS, target, NULL, 1000);

	end_flash->radius = 128.0f;
	end_flash->r.model = &lightning_models[6]; // The bright halo sprite.
	end_flash->r.frame = 1;
	end_flash->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	end_flash->r.scale = 2.0f;
	end_flash->d_scale = -2.0f;
	end_flash->alpha = 0.95f;
	end_flash->d_alpha = -1.333f;
	end_flash->r.origin[2] += 8.0f;

	AddEffect(NULL, end_flash);

	// And yet more sparks to the hit point too to add interest.
	for (int i = 0; i < 12; i++)
	{
		// Half green, half yellow particles
		const int particle_type = ((i & 1) ? PART_16x16_SPARK_Y : PART_16x16_SPARK_G); //mxd
		client_particle_t* spark = ClientParticle_new(particle_type, end_flash->color, 1000);

		VectorSet(spark->velocity, 
			flrand(-LIGHTNING_RING_VELOCITY, LIGHTNING_RING_VELOCITY), 
			flrand(-LIGHTNING_RING_VELOCITY, LIGHTNING_RING_VELOCITY), 
			flrand(0.0f, 32.0f));

		VectorScale(spark->velocity, -1.0f, spark->acceleration);
		spark->scale = flrand(20.0f, 32.0f);
		spark->d_scale = -spark->scale;
		spark->d_alpha = flrand(-384.0f, -256.0f);

		AddParticleToList(end_flash, spark);
	}

	// Draw a circle of expanding lines.
	float cur_angle = 0.0f;
	const float degree_inc = ANGLE_360 / (float)NUM_LIGHTNING_RINGBITS;

	vec3_t last_vel = { LIGHTNING_RING_VELOCITY, 0.0f, 0.0f };
	const vec3_t up_vel = { 0.0f, 0.0f, 32.0f };
	const int glow_flags = CEF_PULSE_ALPHA | CEF_USE_VELOCITY2 | CEF_AUTO_ORIGIN | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS; //mxd

	for (int i = 0; i < NUM_LIGHTNING_RINGBITS; i++)
	{
		cur_angle += degree_inc;

		client_entity_t* glow = ClientEntity_new(FX_LIGHTNING, glow_flags, target, NULL, 750);

		glow->radius = 64.0f;
		glow->r.model = &lightning_models[LIGHTNING_TYPE_GREEN];
		glow->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		glow->r.frame = 1; // Just use the halo.
		glow->r.spriteType = SPRITE_LINE;
		glow->r.scale = 0.5f;
		glow->d_scale = 32.0f;
		glow->alpha = 0.1f;
		glow->d_alpha = 3.0f;

		// The start_pos and start_vel comes from the last velocity.
		VectorCopy(last_vel, glow->velocity);
		VectorScale(glow->velocity, -1.0f, glow->acceleration);
		VectorMA(target, 0.01f, glow->velocity, glow->r.startpos); // Move the line out a bit to avoid a zero-length line.

		// The end_pos is calculated from the current angle.
		VectorSet(glow->velocity2, LIGHTNING_RING_VELOCITY * cosf(cur_angle), LIGHTNING_RING_VELOCITY * sinf(cur_angle), 0.0f);
		VectorScale(glow->velocity2, -1.0f, glow->acceleration2);
		VectorMA(target, 0.01f, glow->velocity2, glow->r.endpos); // Move the line out a bit to avoid a zero-length line.

		// Finally, copy the last velocity we used.
		VectorCopy(glow->velocity2, last_vel);

		// Now, add the additional velocity upwards.
		Vec3AddAssign(up_vel, glow->velocity);
		Vec3AddAssign(up_vel, glow->velocity2);

		AddEffect(NULL, glow);

		// Now spawn a particle quick to save against the nasty joints (ugh).
		glow->r.tile = 0.5f;
		glow->r.tileoffset = ((i & 1) ? 0.5f : 0.0f); // Alternate tiles.

		// Half green, half yellow particles.
		const int particle_type = ((i & 1) ? PART_16x16_SPARK_Y : PART_16x16_SPARK_G); //mxd
		client_particle_t* spark = ClientParticle_new(particle_type, glow->color, 750);

		spark->scale = 0.5f;
		spark->d_scale = 32.0f;
		spark->color.a = 1;
		spark->d_alpha = 768.0f;

		VectorCopy(glow->r.startpos, spark->origin);
		VectorCopy(glow->velocity, spark->velocity);
		VectorCopy(glow->acceleration, spark->acceleration);

		AddParticleToList(glow, spark);
	}

	// Now finally flash and shake the screen.
	fxi.Activate_Screen_Flash((int)0x8080ffc0);
	fxi.Activate_Screen_Shake(4.0f, 500.0f, (float)fxi.cl->time, SHAKE_ALL_DIR); // 'current_time' MUST be cl.time, because that's what used by Perform_Screen_Shake() to calculate effect intensity/timing... --mxd.

	if (flags & CEF_FLAG8) // "Play sound" flag.
		fxi.S_StartSound(target, -1, CHAN_WEAPON, lightning_sound, 1.0f, ATTN_NORM, 0.0f);
}