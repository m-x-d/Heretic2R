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

static struct model_s* rain_models[5];

void PreCacheRedrain(void)
{
	rain_models[0] = fxi.RegisterModel("sprites/spells/spark_red.sp2");
	rain_models[1] = fxi.RegisterModel("models/spells/redrainarrow/tris.fm");
	rain_models[2] = fxi.RegisterModel("sprites/spells/rsteam.sp2");
	rain_models[3] = fxi.RegisterModel("sprites/fx/redraindrop.sp2");
	rain_models[4] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
}

// Thinker for the explosion, just fades the light.
static qboolean FXRedRainDLightThink(const client_entity_t* dlight, centity_t* owner)
{
	dlight->dlight->intensity -= 10.0f;
	return dlight->dlight->intensity > 0.0f;
}

static qboolean RedRainExplosionThink(client_entity_t* explosion, centity_t* owner)
{
	// The explosion bit should be drawn to orbit the rain generation spot.
	explosion->updateTime = 100;
	explosion->LifeTime -= 100;

	if (explosion->LifeTime > 1000)
	{
		// Vary intensity.
		explosion->alpha = 1.0f - explosion->r.scale * 0.1f;
	}
	else if (explosion->LifeTime == 1000)
	{
		// Fade them out.
		explosion->d_alpha = -0.5f;
		explosion->d_scale = -2.0f;
	}
	else if (explosion->LifeTime < 0)
	{
		return false;
	}

	explosion->r.angles[YAW] += 20.0f;

	const float radius = ((explosion->SpawnInfo == 1) ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS); //mxd

	vec3_t dir;
	AngleVectors(explosion->r.angles, dir, NULL, NULL);

	vec3_t target_pos;
	VectorMA(explosion->direction, radius * 1.5f, dir, target_pos);

	vec3_t random_vect;
	VectorRandomSet(random_vect, radius);
	VectorAdd(target_pos, random_vect, target_pos);

	// This is the velocity it would need to reach the position in one second.
	vec3_t diff_pos;
	VectorSubtract(target_pos, explosion->r.origin, diff_pos);

	// Average this velocity with the current one.
	VectorAdd(explosion->velocity, diff_pos, diff_pos);
	VectorScale(diff_pos, 0.5f, explosion->velocity);

	return true;
}

// This is similar to the FXRedRainMissileExplode, except that the explosion needs knowledge of the rainfall height.
static void RedRainExplosion(vec3_t impact_pos, vec3_t rain_pos, const int duration, const qboolean powerup, centity_t* owner)
{
	client_entity_t* dlight = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, impact_pos, NULL, 100);

	const paletteRGBA_t light_color = { .c = (powerup ? 0xff0080ff : 0xff0000ff) }; // Orange when powered up, red when not.
	dlight->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	dlight->Update = FXRedRainDLightThink;

	AddEffect(NULL, dlight);

	// Always have at least 3 clouds.
	int count = GetScaledCount(REDRAIN_EXPLODE_NUM, 0.3f);
	count = max(3, count);

	const float degree_inc = 360.0f / (float)count;

	for (int i = 0; i < count; i++)
	{
		vec3_t org;
		VectorRandomCopy(impact_pos, org, RED_RAIN_RADIUS / 3.0f);

		client_entity_t* explosion = ClientEntity_new(FX_WEAPON_REDRAIN, CEF_DONT_LINK, org, NULL, 100);

		explosion->radius = 16.0f;
		explosion->r.model = &rain_models[2]; // rsteam sprite.
		explosion->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		explosion->alpha = 0.3f;
		explosion->lastThinkTime = fxi.cl->time;
		explosion->LifeTime = duration;

		if (powerup)
		{
			explosion->SpawnInfo = 1;
			explosion->r.frame = 1; // Green sprite.
			explosion->r.scale = 3.0f;
		}
		else
		{
			explosion->r.frame = 0; // Red sprite.
			explosion->r.scale = 2.5f;
		}

		VectorSet(explosion->velocity, flrand(-128.0f, 128.0f), flrand(-128.0f, 128.0f), flrand(-128.0f, 0.0f));
		explosion->r.angles[YAW] = (float)i * degree_inc;
		VectorCopy(rain_pos, explosion->direction);

		explosion->Update = RedRainExplosionThink;

		AddEffect(owner, explosion);
	}

	// Add a big red flash at impact of course.
	client_entity_t* flash = ClientEntity_new(-1, 0, impact_pos, NULL, 500);

	flash->r.model = &rain_models[powerup ? 4 : 0]; // spark_green sprite when powered, spark_red when not.
	flash->r.frame = 0;
	flash->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	flash->radius = 16.0f;
	flash->r.scale = 8.0f;
	flash->d_scale = -16.0f;
	flash->d_alpha = -2.0f;

	AddEffect(NULL, flash);

	const char* snd_name = (powerup ? "weapons/RedRainPowerHit.wav" : "weapons/RedRainHit.wav"); //mxd
	fxi.S_StartSound(impact_pos, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_NORM, 0);
}

// This is a delayed effect which creates a splash out of red sparks.
static qboolean FXRedRainSplashThink(const client_entity_t* splash, centity_t* owner)
{
	client_entity_t* mist = ClientEntity_new(-1, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, splash->r.origin, NULL, 500);
	AddEffect(NULL, mist);

	const int sparktype = ((splash->SpawnInfo == 1) ? PART_16x16_SPARK_G : PART_16x16_SPARK_R); // Green spark when powered.

	for (int i = 0; i < 4; i++)
	{
		client_particle_t* spark = ClientParticle_new(sparktype, color_white, 500);

		VectorSet(spark->velocity, flrand(-48.0f, 48.0f), flrand(-48.0f, 48.0f), flrand(48.0f, 96.0f));
		spark->acceleration[2] = -PARTICLE_GRAVITY * 3.0f;
		spark->scale = 16.0f;
		spark->d_scale = -24.0f;

		AddParticleToList(mist, spark);
	}

	return false;
}


// The drops need to update as they're added to the view, because velocity doesn't update the sprite line's start and endpoint.
//static
qboolean FXRedRainDropUpdate(client_entity_t *drop, centity_t *owner)
{
	drop->r.startpos[2] = drop->r.origin[2] + RAIN_HEIGHT;
	if (drop->r.startpos[2] > drop->SpawnData)	// Make sure that the top of the drop doesn't go higher that the spawn height
		drop->r.startpos[2] = drop->SpawnData;
	drop->r.endpos[2] = drop->r.origin[2] - RAIN_HEIGHT;

	return(false);
}


// Red Rain area

// This constantly starts new drops up at the top.  It also spawns a splash, which is set to go off at the appropriate fall time
static qboolean FXRedRainThink(client_entity_t *rain, centity_t *owner)
{
	client_entity_t		*splash;
	client_entity_t		*drop;
	vec3_t				origin;		//, top;
	int					j;
	float				duration, radius, width;
	trace_t				trace;

	if(rain->nextEventTime <= fxi.cl->time)
		return (false);//in case we lose the packet that tells us to remove

	if (rain->SpawnInfo)
	{	// Powered up rain
		radius = POWER_RAIN_RADIUS;
		width = POWER_RAIN_WIDTH;
	}
	else
	{	// Unpowered
		radius = RED_RAIN_RADIUS;
		width = RED_RAIN_WIDTH;
	}

	if (rain->SpawnData < 0.0)
	{
		rain->SpawnData += 8.0;
		if (rain->SpawnData > 0.0)
			rain->SpawnData = 0.0;
	}

	if(owner->current.effects&EF_DISABLE_EXTRA_FX)//rain->LifeTime < 1000)
		return(true);

	for(j = 0; j < NUM_DROPS; j++)
	{
		VectorSet(origin, 
				flrand(-radius, radius), 
				flrand(-radius, radius), 
				rain->SpawnData + flrand(-8.0F, 8.0F));
		VectorAdd(rain->origin, origin, origin);
		duration = GetFallTime(origin, RAIN_INITIAL_VELOCITY, -PARTICLE_GRAVITY, DROP_RADIUS, 3.0F, &trace);
		drop = ClientEntity_new(-1, CEF_DONT_LINK, origin, NULL, duration);
		drop->r.model = rain_models + 3;
		drop->r.frame = rain->SpawnInfo;
		drop->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		drop->alpha = 0.75;
		drop->SpawnInfo = rain->SpawnInfo;
		drop->r.scale = width;
		drop->radius = RAIN_HEIGHT;
		VectorCopy(origin, drop->r.startpos);
		VectorCopy(origin, drop->r.endpos);
		drop->r.endpos[2] -= RAIN_HEIGHT;
		drop->velocity[2] = RAIN_INITIAL_VELOCITY;
		drop->r.spriteType = SPRITE_LINE;
		drop->SpawnData = origin[2]; // This allows the drop to remember its top position, so the top doesn't go higher than it.
		drop->AddToView = FXRedRainDropUpdate;
		AddEffect(NULL, drop); 

		if((duration > 20) && (r_detail->value > DETAIL_LOW))
		{
			origin[2] += GetDistanceOverTime(RAIN_INITIAL_VELOCITY, -PARTICLE_GRAVITY, (float)duration * 0.001F);
			splash = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, origin, NULL, duration);
			splash->Update = FXRedRainSplashThink;
			splash->SpawnInfo = rain->SpawnInfo;
			AddEffect(NULL, splash); 
		}
	}
	return(true);
}

// This is from creating the effect FX_RED_RAIN.
void FXRedRain(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*spawner;
	vec_t				ceiling;
	vec_t				floor;
	vec3_t				ceil_org;
	float				radius;
	qboolean			powerup;

	VectorCopy(Origin, ceil_org);
	if (Flags & CEF_FLAG6)
	{	// powered up
		radius = POWER_RAIN_RADIUS;
		powerup = true;
	}
	else
	{	// unpowered
		radius = RED_RAIN_RADIUS;
		powerup = false;
	}
	GetSolidDist(Origin, radius * 0.5, MAX_REDRAINHEIGHT, &ceiling);
	ceil_org[2] += ceiling;
	GetSolidDist(Origin, 1, -MAX_FALL_DISTANCE, &floor);

	Flags = (Flags | CEF_NO_DRAW | CEF_NOMOVE | CEF_CULLED | CEF_VIEWSTATUSCHANGED) & ~CEF_OWNERS_ORIGIN;
	spawner = ClientEntity_new(Type, Flags, ceil_org, NULL, 200);
	spawner->Update = FXRedRainThink;			// FXRedRainThink;
	spawner->radius = radius + MAX_REDRAINHEIGHT + (ceiling - floor);

	spawner->color.c = 0xffffffff;
	spawner->nextEventTime = fxi.cl->time + (RED_RAIN_DURATION+1.0)*1000;//waits for EF_DISABLE from owner, but in case we miss the message, time out

	// The rain should start at the impact height, then move up to the target height.
	spawner->SpawnData = -ceiling;
	if (powerup)
		spawner->SpawnInfo = 1;

	AddEffect(Owner, spawner); 

	// Pass the explosion point as well as the rain generation point.
	RedRainExplosion(Origin, ceil_org, (RED_RAIN_DURATION+1.0)*1000, powerup, Owner);
}

// Red Rain Missile



// ---------------------------------------------------------------------------


// The red rain projectile's trail of red sparks.
static qboolean FXRedRainMissileThink(client_entity_t *missile, centity_t *owner)
{
	int					i;
	client_entity_t		*ce;
	vec3_t				diff, curpos, org;

	VectorSubtract(missile->r.origin, missile->origin, diff);
	Vec3ScaleAssign((1.0 / NUM_TRAIL_PARTICLES), diff);
	VectorClear(curpos);

	for(i = 0; i < NUM_TRAIL_PARTICLES; i++)
	{
		VectorRandomCopy(missile->origin, org, PARTICLE_OFFSET);
		Vec3AddAssign(curpos, org);
		ce = ClientEntity_new(-1, 0, org, NULL, 500);
		if (missile->SpawnInfo)	// Powered up
		{
			ce->r.model = rain_models+4;
		}
		else
		{
			ce->r.model = rain_models;
		}
		ce->r.scale = 1.0F;
		ce->d_scale = 2.0F;
		ce->r.frame = 0;
		ce->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		ce->radius = 16.0F;
		ce->d_alpha = -2.2F;
		AddEffect(NULL, ce);

		Vec3AddAssign(diff, curpos);
	}
	// Remember for even spread of particles
	VectorCopy(missile->r.origin, missile->origin);
	return(true);
}						 

// From creation of the effect FX_RED_RAIN_MISSILE
void FXRedRainMissile(centity_t *Owner, int Type, int Flags, vec3_t Origin)
{
	client_entity_t		*missile;
	vec3_t				temp;
	int					dur;

	if (r_detail->value == DETAIL_LOW)
		dur = 150;
	else
	if (r_detail->value == DETAIL_NORMAL)
		dur = 125;
	else
		dur = 100;

	missile = ClientEntity_new(Type, Flags | CEF_DONT_LINK, Origin, NULL, dur);
	fxi.GetEffect(Owner, Flags, clientEffectSpawners[FX_WEAPON_REDRAINMISSILE].formatString, missile->velocity);

	if (Flags & CEF_FLAG7)
	{
		if (Flags & CEF_FLAG8)
			Vec3ScaleAssign(MSSITHRA_FX_ARROW_SPEED/2,missile->velocity);
		else
			Vec3ScaleAssign(MSSITHRA_FX_ARROW_SPEED,missile->velocity);
	}

	else
	{
		if (Flags & CEF_FLAG8)
			Vec3ScaleAssign(RED_ARROW_SPEED/2,missile->velocity);
		else
			Vec3ScaleAssign(RED_ARROW_SPEED,missile->velocity);
	}

	VectorCopy(missile->velocity, temp);
	VectorNormalize(temp);
	AnglesFromDir(temp, missile->r.angles);

	missile->r.model = rain_models + 1;
	missile->Update = FXRedRainMissileThink;
	missile->radius = 32.0F;
	if (Flags & CEF_FLAG6)
	{	// Powered up rain
		missile->SpawnInfo = 1;
		missile->color.c = 0xff00ff80;	// green
	}
	else
	{
		missile->color.c = 0xff0000ff;	// Red
	}
	missile->dlight = CE_DLight_new(missile->color, 150.0F, 00.0F);
	AddEffect(Owner, missile);
}