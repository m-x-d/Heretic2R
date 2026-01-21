//
// fx_RedRain.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Motion.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_Dlight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define DROP_RADIUS				2.0f
#define RAIN_INITIAL_VELOCITY	(-600.0f)
#define NUM_DROPS				2
#define MAX_FALL_DISTANCE		1300
#define RAIN_HEIGHT				32.0f
#define PARTICLE_OFFSET			5.0f
#define NUM_TRAIL_PARTICLES		6
#define REDRAIN_EXPLODE_NUM		5
#define RED_RAIN_WIDTH			3.0f
#define POWER_RAIN_WIDTH		6.0f

// Mutant Ssithra arrow - uses red-rain arrow.
#define MSSITHRA_FX_ARROW_SPEED		750.0f

static struct model_s* rain_models[7];
static struct sfx_s* rain_sounds[2]; //mxd

void PreCacheRedrain(void)
{
	rain_models[0] = fxi.RegisterModel("sprites/spells/spark_red.sp2");
	rain_models[1] = fxi.RegisterModel("models/spells/redrainarrow/tris.fm");
	rain_models[2] = fxi.RegisterModel("sprites/spells/rsteam.sp2");
	rain_models[3] = fxi.RegisterModel("sprites/fx/redraindrop.sp2");
	rain_models[4] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
	rain_models[5] = fxi.RegisterModel("sprites/fx/halored.sp2"); //mxd
	rain_models[6] = fxi.RegisterModel("sprites/fx/halogreen.sp2"); //mxd
}

void PreCacheRedrainSFX(void) //mxd
{
	rain_sounds[0] = fxi.S_RegisterSound("weapons/RedRainHit.wav");
	rain_sounds[1] = fxi.S_RegisterSound("weapons/RedRainPowerHit.wav");
}

// Thinker for the explosion, just fades the light.
static qboolean RedRainDLightUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXRedRainDLightThink' in original logic.
{
	self->dlight->intensity -= 10.0f;
	return self->dlight->intensity > 0.0f;
}

static qboolean RedRainCloudUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'RedRainExplosionThink' in original logic.
{
	// The cloud sprite should be drawn to orbit the rain generation spot.
	self->LifeTime -= 100;

	if (self->LifeTime > 1000)
	{
		// Vary intensity.
		self->alpha = 1.0f - self->r.scale * 0.1f;
	}
	else if (self->LifeTime == 1000)
	{
		// Fade them out.
		self->d_alpha = -0.5f;
		self->d_scale = -2.0f;
	}
	else if (self->LifeTime < 0)
	{
		return false;
	}

	self->r.angles[YAW] += 20.0f;

	const float radius = ((self->SpawnInfo == 1) ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS); //mxd

	vec3_t forward;
	AngleVectors(self->r.angles, forward, NULL, NULL);

	vec3_t target_pos;
	VectorMA(self->direction, radius * 1.5f, forward, target_pos);

	vec3_t random_vect;
	VectorRandomSet(random_vect, radius);
	Vec3AddAssign(random_vect, target_pos);

	// This is the velocity it would need to reach the position in one second.
	vec3_t diff_pos;
	VectorSubtract(target_pos, self->r.origin, diff_pos);

	// Average this velocity with the current one.
	Vec3AddAssign(self->velocity, diff_pos);
	VectorScale(diff_pos, 0.5f, self->velocity);

	return true;
}

// This is similar to the FXRedRainMissileExplode, except that the cloud needs knowledge of the rainfall height.
static void SpawnRedRainClouds(const vec3_t impact_pos, const vec3_t rain_pos, const int duration, const qboolean powerup, centity_t* owner) //mxd. Named 'RedRainExplosion' in original logic.
{
	client_entity_t* dlight = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, impact_pos, NULL, 100);

	const paletteRGBA_t light_color = { .c = (powerup ? 0xff0080ff : 0xff0000ff) }; // Orange when powered up, red when not.
	dlight->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	dlight->Update = RedRainDLightUpdate;

	AddEffect(NULL, dlight);

	// Always have at least 3 clouds.
	int count = GetScaledCount(REDRAIN_EXPLODE_NUM, 0.3f);
	count = max(3, count);

	const float degree_inc = 360.0f / (float)count;

	for (int i = 0; i < count; i++)
	{
		vec3_t org;
		VectorRandomCopy(impact_pos, org, RED_RAIN_RADIUS / 3.0f);

		client_entity_t* cloud = ClientEntity_new(FX_WEAPON_REDRAIN, CEF_DONT_LINK, org, NULL, 100);

		cloud->radius = 16.0f;
		cloud->r.model = &rain_models[2]; // rsteam sprite (32x32).
		cloud->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		cloud->alpha = flrand(0.2f, 0.4f); //mxd. Randomize alpha a bit.
		cloud->LifeTime = duration;

		if (powerup)
		{
			cloud->SpawnInfo = 1;
			cloud->r.frame = 1; // Green sprite.
			cloud->r.scale = flrand(2.8f, 3.2f); //mxd. Randomize scale a bit.
		}
		else
		{
			cloud->r.frame = 0; // Red sprite.
			cloud->r.scale = flrand(2.3f, 2.7f); //mxd. Randomize scale a bit.
		}

		VectorSet(cloud->velocity, flrand(-128.0f, 128.0f), flrand(-128.0f, 128.0f), flrand(-128.0f, 0.0f));
		cloud->r.angles[YAW] = (float)i * degree_inc;
		VectorCopy(rain_pos, cloud->direction);

		cloud->Update = RedRainCloudUpdate;

		RE_SetupRollSprite(&cloud->r, 32.0f, flrand(0.0f, 359.0f)); //mxd
		AddEffect(owner, cloud);
	}

	// Add a big red flash at impact of course.
	client_entity_t* flash = ClientEntity_new(-1, 0, impact_pos, NULL, 500);

	flash->r.model = &rain_models[powerup ? 6 : 5]; //mxd. Use halogreen/halored sprites (original logic uses spark_green/spark_red -- too low res).
	flash->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	flash->radius = 10.0f;
	flash->r.scale = 5.0f;
	flash->d_scale = -4.0f;
	flash->alpha = 0.3f; //mxd
	flash->d_alpha = -1.0f;

	//mxd. If visible, add RF_NODEPTHTEST flag.
	trace_t trace;
	fxi.Trace(flash->r.origin, vec3_origin, vec3_origin, fxi.cl->refdef.vieworg, (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER), CTF_CLIP_TO_ALL, &trace);

	if (trace.fraction == 1.0f)
		flash->r.flags |= RF_NODEPTHTEST;

	RE_SetupFlipSprite(&flash->r, 32.0f, 32.0f, 64.0f, 64.0f, irand(0, 1), irand(0, 1)); //mxd
	AddEffect(NULL, flash);

	fxi.S_StartSound(impact_pos, -1, CHAN_AUTO, rain_sounds[powerup ? 1 : 0], 1.0f, ATTN_NORM, 0.0f);
}

// This is a delayed effect which creates a splash out of red sparks.
static qboolean RedRainSplashUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXRedRainSplashThink' in original logic.
{
	client_entity_t* mist = ClientEntity_new(-1, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, self->r.origin, NULL, 500);
	AddEffect(NULL, mist);

	const int spark_type = ((self->SpawnInfo == 1) ? PART_16x16_SPARK_G : PART_16x16_SPARK_R); // Green spark when powered.

	for (int i = 0; i < 4; i++)
	{
		client_particle_t* spark = ClientParticle_new(spark_type, color_white, 500);

		VectorSet(spark->velocity, flrand(-48.0f, 48.0f), flrand(-48.0f, 48.0f), flrand(48.0f, 96.0f));
		spark->acceleration[2] = -PARTICLE_GRAVITY * 3.0f;
		spark->scale = 16.0f;
		spark->d_scale = -24.0f;

		AddParticleToList(mist, spark);
	}

	return false;
}

// The drops need to update as they're added to the view, because velocity doesn't update the sprite line's start and endpoint.
static qboolean RedRainDropAddToView(client_entity_t* self, centity_t* owner) //mxd. Named 'FXRedRainDropUpdate' in original logic.
{
	// Make sure that the top of the drop doesn't go higher that the spawn height.
	self->r.startpos[2] = min(self->SpawnData, self->r.origin[2] + RAIN_HEIGHT);
	self->r.endpos[2] = self->r.origin[2] - RAIN_HEIGHT;

	return true; //mxd. Returns 'false' in original logic.
}

// This constantly starts new drops up at the top. It also spawns a splash, which is set to go off at the appropriate fall time.
static qboolean RedRainDropSpawnerUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXRedRainThink' in original logic.
{
	if (self->nextEventTime <= fx_time)
		return false; // In case we lose the packet that tells us to remove.

	if (self->SpawnData < 0.0f)
		self->SpawnData = min(0.0f, self->SpawnData + 8.0f);

	if (owner->current.effects & EF_DISABLE_EXTRA_FX)
		return true;

	const float radius = ((self->SpawnInfo == 1) ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS);
	const float width = ((self->SpawnInfo == 1) ? POWER_RAIN_WIDTH : RED_RAIN_WIDTH);

	for (int i = 0; i < NUM_DROPS; i++)
	{
		vec3_t origin = VEC3_INITA(self->origin, flrand(-radius, radius), flrand(-radius, radius), self->SpawnData + flrand(-8.0f, 8.0f));

		trace_t trace;
		const int duration = GetFallTime(origin, RAIN_INITIAL_VELOCITY, -PARTICLE_GRAVITY, DROP_RADIUS, 3.0f, &trace);

		client_entity_t* drop = ClientEntity_new(-1, CEF_DONT_LINK, origin, NULL, duration);

		drop->radius = RAIN_HEIGHT;
		drop->r.model = &rain_models[3]; // redraindrop sprite.
		drop->r.frame = self->SpawnInfo;
		drop->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		drop->r.spriteType = SPRITE_LINE;
		drop->r.scale = width;
		drop->alpha = 0.75f;

		VectorCopy(origin, drop->r.startpos);
		VectorCopy(origin, drop->r.endpos);
		drop->r.endpos[2] -= RAIN_HEIGHT;
		drop->velocity[2] = RAIN_INITIAL_VELOCITY;

		drop->SpawnInfo = self->SpawnInfo;
		drop->SpawnData = origin[2]; // This allows the drop to remember its top position, so the top doesn't go higher than it.
		drop->AddToView = RedRainDropAddToView;

		AddEffect(NULL, drop);

		if (duration > 20 && R_DETAIL > DETAIL_LOW)
		{
			origin[2] += GetDistanceOverTime(RAIN_INITIAL_VELOCITY, -PARTICLE_GRAVITY, (float)duration * 0.001f);

			client_entity_t* splash = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, origin, NULL, duration);

			splash->SpawnInfo = self->SpawnInfo;
			splash->Update = RedRainSplashUpdate;

			AddEffect(NULL, splash);
		}
	}

	return true;
}

// Create FX_RED_RAIN effect.
void FXRedRain(centity_t* owner, const int type, int flags, vec3_t origin)
{
	const qboolean powerup = (flags & CEF_FLAG6); //mxd
	const float radius = (powerup ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS); //mxd

	//mxd. Original logic uses 'radius * 0.5f' as GetSolidDist() radius arg. Changed to avoid clouds getting stuck near impact point
	//when arrow direction was nearly parallel to a wall (so applying ARROW_BACKUP in RedRainMissileTouch() doesn't move origin enough away from it).
	const float ceiling = GetSolidDist(origin, 1.0f, MAX_REDRAINHEIGHT, false) - radius * 0.5f;
	const vec3_t ceil_origin = VEC3_INITA(origin, 0.0f, 0.0f, ceiling);

	const float floor = GetSolidDist(origin, 1.0f, -MAX_FALL_DISTANCE, false);

	const int duration = ((int)RED_RAIN_DURATION + 1) * 1000; //mxd
	flags = (int)(flags | CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED) & ~CEF_OWNERS_ORIGIN;
	client_entity_t* spawner = ClientEntity_new(type, flags, ceil_origin, NULL, 200);

	spawner->radius = radius + MAX_REDRAINHEIGHT + (ceiling - floor);
	spawner->color = color_white;
	spawner->nextEventTime = fx_time + duration; // Waits for EF_DISABLE from owner, but in case we miss the message, time out.

	// The rain should start at the impact height, then move up to the target height.
	spawner->SpawnData = -ceiling;
	spawner->SpawnInfo = (powerup ? 1 : 0);

	spawner->Update = RedRainDropSpawnerUpdate;

	AddEffect(owner, spawner);

	// Pass the explosion point as well as the rain generation point.
	SpawnRedRainClouds(origin, ceil_origin, duration, powerup, owner);
}

// The red rain projectile's trail of red sparks.
static qboolean RedRainMissileUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXRedRainMissileThink' in original logic.
{
	vec3_t diff;
	VectorSubtract(self->r.origin, self->origin, diff);
	Vec3ScaleAssign(1.0f / NUM_TRAIL_PARTICLES, diff);

	vec3_t cur_pos = { 0 };
	const qboolean powerup = (self->SpawnInfo == 1); //mxd

	for (int i = 0; i < NUM_TRAIL_PARTICLES; i++)
	{
		vec3_t origin;
		VectorRandomCopy(self->origin, origin, PARTICLE_OFFSET);
		Vec3AddAssign(cur_pos, origin);

		client_entity_t* ce = ClientEntity_new(-1, 0, origin, NULL, 500);

		ce->radius = 16.0f;
		ce->r.model = &rain_models[powerup ? 4 : 0]; // spark_green sprite when powered, spark_red when not.
		ce->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
		ce->d_scale = 2.0f;
		ce->d_alpha = -2.2f;

		AddEffect(NULL, ce);

		Vec3AddAssign(diff, cur_pos);
	}

	// Remember for even spread of particles.
	VectorCopy(self->r.origin, self->origin);

	return true;
}

// Create FX_WEAPON_REDRAINMISSILE effect.
void FXRedRainMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	int duration;

	if (R_DETAIL == DETAIL_LOW)
		duration = 150;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 125;
	else
		duration = 100; //TODO: separate case for DETAIL_UBERHIGH.

	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, duration);
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_REDRAINMISSILE].formatString, missile->velocity);

	float arrow_speed = ((flags & CEF_FLAG7) ? MSSITHRA_FX_ARROW_SPEED : RED_ARROW_SPEED); //mxd
	if (flags & CEF_FLAG8)
		arrow_speed *= 0.5f;

	Vec3ScaleAssign(arrow_speed, missile->velocity);

	vec3_t temp;
	VectorNormalize2(missile->velocity, temp);
	AnglesFromDir(temp, missile->r.angles);

	missile->radius = 32.0f;
	missile->r.model = &rain_models[1]; // Red rain arrow model.

	if (flags & CEF_FLAG6)
	{
		// Powered-up rain.
		missile->SpawnInfo = 1;
		missile->color.c = 0xff00ff80; // Green.
	}
	else
	{
		missile->color.c = 0xff0000ff; // Red.
	}

	missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);
	missile->Update = RedRainMissileUpdate;

	AddEffect(owner, missile);
}