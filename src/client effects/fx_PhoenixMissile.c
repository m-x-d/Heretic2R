//
// fx_PhoenixMissile.c -- Part of 'fx_Phoenix.c' in original logic --mxd.
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

#define FIRETRAIL_PARTS		4
#define FIRETRAIL_RADIUS	6.0f
#define FIRETRAIL_SPEED		16.0f
#define FIRETRAIL_SCALE		12.0f
#define FIRETRAIL_ACCEL		32.0f

#define SMOKETRAIL_RADIUS	2.0f
#define SMOKETRAIL_SCALE	0.25f
#define SMOKETRAIL_ALPHA	0.5f

enum //mxd
{
	PAMDL_ARROW,
	PAMDL_STEAM,

	PAMDL_NUM_MODELS
};

static struct model_s* phoenix_missile_models[PAMDL_NUM_MODELS];
static const paletteRGBA_t pm_dlight_color = { .r = 255, .g = 127, .b = 0, .a = 229 };

void PreCachePhoenixMissile(void)
{
	phoenix_missile_models[PAMDL_ARROW] = fxi.RegisterModel("models/spells/phoenixarrow/tris.fm");
	phoenix_missile_models[PAMDL_STEAM] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
}

static qboolean PhoenixMissileUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPhoenixMissileThink' in original logic.
{
	int duration;

	if (R_DETAIL == DETAIL_LOW)
		duration = 1400;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 1700;
	else
		duration = 2000; //TODO: even longer duration for DETAIL_UBERHIGH?

	// Here we want to shoot out flame to either side.
	const vec3_t angles = VEC3_INITS(self->r.angles, RAD_TO_ANGLE);

	vec3_t forward;
	vec3_t right;
	AngleVectors(angles, forward, right, NULL);
	Vec3ScaleAssign(-4.0f * FIRETRAIL_SPEED, forward);
	Vec3ScaleAssign(FIRETRAIL_SPEED, right);

	// Throw smoke to each side, alternating.
	const float side = ((self->LifeTime-- & 1) ? 1.0f : -1.0f); //mxd. 1.0 - right, -1.0 - left.
	const vec3_t smoke_origin = VEC3_INITA(self->origin,
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS / 2.0f, SMOKETRAIL_RADIUS / 2.0f));

	client_entity_t* smoke = ClientEntity_new(-1, CEF_DONT_LINK | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS, smoke_origin, NULL, duration);

	smoke->radius = 64.0f; //BUGFIX: 128.0 for the left side in original version. Why?..
	smoke->r.model = &phoenix_missile_models[PAMDL_STEAM]; // steam_add sprite.
	smoke->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->d_alpha = -1.0f;
	smoke->r.scale = SMOKETRAIL_SCALE;
	smoke->d_scale = 2.0f; // Rate of change in scale.
	VectorScale(right, 2.0f * side, smoke->velocity);

	RE_SetupRollSprite(&smoke->r, 32.0f, flrand(0.0f, 360.0f)); //mxd
	AddEffect(NULL, smoke);	// Add the smoke as independent world smoke.

	// Add fire to the tail. Attach it to the smoke because it doesn't get out of the fx radius so quickly.
	for (int i = 0; i < FIRETRAIL_PARTS; i++)
	{
		client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), pm_dlight_color, duration);

		VectorSet(flame->origin,
			flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS),
			flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS),
			flrand(-FIRETRAIL_RADIUS / 3.0f, FIRETRAIL_RADIUS / 3.0f));

		Vec3AddAssign(self->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity,
			flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED),
			flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED),
			flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side.
		Vec3AddAssign(forward, flame->velocity);

		// Alternate left and right side of phoenix.
		if (i & 1)
			Vec3AddAssign(right, flame->velocity);
		else
			Vec3SubtractAssign(right, flame->velocity);

		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0f, -10.0f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(smoke, flame);
	}

	// Update animation frame.

	// Check if the time is up.
	if (fx_time >= self->lastThinkTime)
	{
		// Set up animations to go the other direction.
		if (self->NoOfAnimFrames == 7)
		{
			// Set to go backwards to 3.
			self->NoOfAnimFrames = 3;
			self->r.frame = 7;
		}
		else
		{
			// Set to go forward to 7
			self->NoOfAnimFrames = 7;
			self->r.frame = 3;
		}

		self->Scale = -1.0f;
		self->lastThinkTime = fx_time + (4 * 50);
	}
	else
	{
		self->r.frame = self->NoOfAnimFrames - (int)(self->Scale * ((float)(self->lastThinkTime - fx_time) / 50.0f)) - 1;
	}

	// Remember for even spread of particles.
	VectorCopy(self->r.origin, self->origin);

	return true;
}

static qboolean PhoenixMissilePowerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPhoenixMissilePowerThink' in original logic.
{
	int duration;

	if (R_DETAIL == DETAIL_LOW)
		duration = 1400;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 1700;
	else
		duration = 2000; //TODO: even longer duration for DETAIL_UBERHIGH?

	// Here we want to shoot out flame to either side.
	const vec3_t angles = VEC3_INITS(self->r.angles, RAD_TO_ANGLE);

	vec3_t forward;
	vec3_t right;
	AngleVectors(angles, forward, right, NULL);
	Vec3ScaleAssign(-4.0f * FIRETRAIL_SPEED, forward);
	Vec3ScaleAssign(FIRETRAIL_SPEED, right);

	// Throw smoke to each side, alternating.
	const float side = ((self->LifeTime-- & 1) ? 1.0f : -1.0f); //mxd. 1.0 - right, -1.0 - left.
	const vec3_t smoke_origin = VEC3_INITA(self->origin,
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS / 2.0f, SMOKETRAIL_RADIUS / 2.0f));

	client_entity_t* smoke = ClientEntity_new(-1, CEF_DONT_LINK | CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS, smoke_origin, NULL, duration);

	smoke->radius = 64.0f;
	smoke->r.model = &phoenix_missile_models[PAMDL_STEAM]; // steam_add sprite.
	smoke->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->d_alpha = -1.0f;
	smoke->r.scale = SMOKETRAIL_SCALE * 2.5f;
	smoke->d_scale = 2.0f; // Rate of change in scale.
	VectorScale(right, 2.0f * side, smoke->velocity);

	RE_SetupRollSprite(&smoke->r, 32.0f, flrand(0.0f, 360.0f)); //mxd
	AddEffect(NULL, smoke);	// Add the smoke as independent world smoke.

	// Add fire to the tail. Attach it to the smoke because it doesn't get out of the fx radius so quickly.
	const float trail_offset = FIRETRAIL_RADIUS / 3.0f; //mxd
	const float trail_speed = FIRETRAIL_SPEED / 3.0f; //mxd

	for (int i = 0; i < FIRETRAIL_PARTS; i++)
	{
		client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), pm_dlight_color, duration);

		VectorRandomSet(flame->origin, trail_offset);
		Vec3AddAssign(self->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity, flrand(-trail_speed, trail_speed), flrand(-trail_speed, trail_speed), flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side.
		Vec3AddAssign(forward, flame->velocity);

		// Alternate left and right side of phoenix.
		if (i & 1)
			Vec3AddAssign(right, flame->velocity);
		else
			Vec3SubtractAssign(right, flame->velocity);

		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0f, -10.0f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(smoke, flame);
	}

	// Remember for even spread of particles.
	VectorCopy(self->r.origin, self->origin);

	return true;
}

void FXPhoenixMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK | CEF_ADDITIVE_PARTS, origin, NULL, 25);
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_PHOENIXMISSILE].formatString, missile->velocity);

	const float velocity = PHOENIX_ARROW_SPEED * ((flags & CEF_FLAG8) ? 0.5f : 1.0f);
	Vec3ScaleAssign(velocity, missile->velocity);

	vec3_t dir;
	VectorNormalize2(missile->velocity, dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 256.0f;
	missile->r.model = &phoenix_missile_models[PAMDL_ARROW]; // Phoenix arrow model.
	missile->lastThinkTime = fx_time + (50 * 7); // Time to play last frame.
	missile->NoOfAnimFrames = 7; // End on frame number 7.
	missile->Scale = 1.0f; // Positive frame count.
	missile->r.scale = 0.8f;
	missile->color.c = 0xff00ffff;
	missile->LifeTime = 1000;

	if (R_DETAIL > DETAIL_LOW)
		missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);

	if (flags & CEF_FLAG6)
		missile->Update = PhoenixMissilePowerUpdate;
	else
		missile->Update = PhoenixMissileUpdate;

	AddEffect(owner, missile);
}