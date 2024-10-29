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
#include "Random.h"
#include "Utilities.h"
#include "motion.h"
#include "Reference.h"
#include "ce_Dlight.h"
#include "q_Sprite.h"
#include "g_playstats.h"

#define MAX_REDRAINHEIGHT	200.0F
#define RED_RAIN_RADIUS		60.0F
#define POWER_RAIN_RADIUS	80.0F
#define	DROP_RADIUS			2.0F
#define RAIN_INIT_VEL		-600.0F
#define NUM_SPLASH_PARTS	4
#define NUM_TAIL_PARTS		4
#define	NUM_DROPS			2
#define MAX_FALL_DIST		1300		
#define RAIN_HEIGHT			32.0	
#define	PART_OFF			5.0
#define NUM_TRAIL_PARTS		6
#define	REDRAIN_EXPLODE_NUM	5
#define RED_RAIN_DAMPFACTOR 0.5
#define RED_RAIN_WIDTH		3.0
#define POWER_RAIN_WIDTH	6.0

#define	NUM_REDRAIN_MODELS	5

// Mutant Ssithra arrow- uses red-rain arrow
#define MSSITHRA_FX_ARROW_SPEED			750.0

static struct model_s *rain_models[NUM_REDRAIN_MODELS];
void PreCacheRedrain()
{
	rain_models[0] = fxi.RegisterModel("sprites/spells/spark_red.sp2");
	rain_models[1] = fxi.RegisterModel("models/spells/redrainarrow/tris.fm");
	rain_models[2] = fxi.RegisterModel("sprites/spells/rsteam.sp2");
	rain_models[3] = fxi.RegisterModel("sprites/fx/redraindrop.sp2");
	rain_models[4] = fxi.RegisterModel("sprites/spells/spark_green.sp2");
}

void RedRainExplosion(vec3_t impactpos, vec3_t rainpos, int duration, qboolean powerup,centity_t *owner);
void DoLightning(vec3_t groundpos, vec3_t airpos);




// Things dropped by the red rain.

// --------------------------------------------------------------

// This is a delayed effect which creates a splash out of red sparks.
//static
qboolean FXRedRainSplashThink(client_entity_t *splash, centity_t *owner)
{
	client_entity_t		*mist;
	client_particle_t	*spark;
	int i;
	paletteRGBA_t		pal;
	int					sparktype;
	float				grav;

	mist = ClientEntity_new(-1, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, splash->r.origin, NULL, 500);
	AddEffect(NULL, mist);

	pal.c = 0xffffffff;
	if (splash->SpawnInfo)
	{	// Powered up rain
		sparktype = PART_16x16_SPARK_G;
		grav = PARTICLE_GRAVITY * 4.0;
	}
	else
	{
		sparktype = PART_16x16_SPARK_R;
		grav = -PARTICLE_GRAVITY * 3.0;
	}

	for (i=0; i<4; i++)
	{
		spark = ClientParticle_new(sparktype, pal, 500);
		VectorSet(spark->velocity, flrand(-48.0, 48.0), flrand(-48.0, 48.0), flrand(48.0, 96.0));
		spark->acceleration[2] = -PARTICLE_GRAVITY*3.0;
		spark->scale = 16.0;
		spark->d_scale = -24.0;
		AddParticleToList(mist, spark);
	}
	return(false);
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
		duration = GetFallTime(origin, RAIN_INIT_VEL, -PARTICLE_GRAVITY, DROP_RADIUS, 3.0F, &trace);
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
		drop->velocity[2] = RAIN_INIT_VEL;
		drop->r.spriteType = SPRITE_LINE;
		drop->SpawnData = origin[2]; // This allows the drop to remember its top position, so the top doesn't go higher than it.
		drop->AddToView = FXRedRainDropUpdate;
		AddEffect(NULL, drop); 

		if((duration > 20) && (r_detail->value > DETAIL_LOW))
		{
			origin[2] += GetDistanceOverTime(RAIN_INIT_VEL, -PARTICLE_GRAVITY, (float)duration * 0.001F);
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
	GetSolidDist(Origin, 1, -MAX_FALL_DIST, &floor);

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
	Vec3ScaleAssign((1.0 / NUM_TRAIL_PARTS), diff);
	VectorClear(curpos);

	for(i = 0; i < NUM_TRAIL_PARTS; i++)
	{
		VectorRandomCopy(missile->origin, org, PART_OFF);
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

// Red Rain Explosion

// ---------------------------------------------------------------------------


// Thinker for the explosion, just fades the light
static qboolean FXRedRainDLightThink(client_entity_t *dlight, centity_t *owner)
{
	dlight->dlight->intensity -= 10.0F;
	if(dlight->dlight->intensity < 0.0F)
		return(false);

	return(true);
}

static qboolean RedRainExplosionThink(client_entity_t *explosion, centity_t *owner)
{	// The explosion bit should be drawn to orbit the rain generation spot.
	vec3_t targetpos, diffpos, dir, randomvect;
	float	radius;
	
	explosion->updateTime = 100;
	explosion->LifeTime -= 100;

	if (explosion->LifeTime > 1000)
	{	// Vary intesity
		explosion->alpha = 1.0 - explosion->r.scale*0.1;
	}
	else if(explosion->LifeTime == 1000)
	{	// Fade them out
		explosion->d_alpha = -0.5;
		explosion->d_scale = -2.0;
	}
	else if (explosion->LifeTime < 0)
		return(false);

	explosion->r.angles[YAW] += 20.0;

	if (explosion->SpawnInfo)
	{	// Powered up rain
		radius = POWER_RAIN_RADIUS;
	}
	else
	{	// Non-powered
		radius = RED_RAIN_RADIUS;
	}

	AngleVectors(explosion->r.angles, dir, NULL, NULL);
	VectorMA(explosion->direction, radius*1.5, dir, targetpos);
	VectorSet(randomvect, 
				flrand(-radius, radius), 
				flrand(-radius, radius), 
				flrand(-radius, radius));
	VectorAdd(targetpos, randomvect, targetpos);

	// This is the velocity it would need to reach the position in one second.
	VectorSubtract(targetpos, explosion->r.origin, diffpos);

	// Average this velocity with the current one.
	VectorAdd(explosion->velocity, diffpos, diffpos);
	VectorScale(diffpos, 0.5, explosion->velocity);

	return(true);
}

// This is similar to the FXRedRainMissileExplode, except that the explosion needs knowledge of the rainfall height.
void RedRainExplosion(vec3_t impactpos, vec3_t rainpos, int duration, qboolean powerup,centity_t *owner)
{
	client_entity_t		*explo;
	client_entity_t		*dlight;
	vec3_t				org;
	paletteRGBA_t		color;
	int					i;
	float				degreeinc;
	int					count;

	dlight = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, impactpos, NULL, 100);
	if (powerup)
		color.c = 0xff0080ff;	// Orange when powered up
	else	
		color.c = 0xff0000ff;	// Red when not.
	dlight->dlight = CE_DLight_new(color, 150.0F, 0.0F);
	dlight->Update = FXRedRainDLightThink;
	AddEffect(NULL, dlight);

	// always have at least 3 clouds
	count = GetScaledCount(REDRAIN_EXPLODE_NUM, 0.3);
	if (count < 3)
		count = 3;

	degreeinc = (360.0)/(float)count;
	for(i = 0; i < count; i++)
	{
		VectorRandomCopy(impactpos, org, RED_RAIN_RADIUS/3.0);
		explo = ClientEntity_new(FX_WEAPON_REDRAIN, CEF_DONT_LINK, org, NULL, 100);

		explo->r.model = rain_models + 2;
		explo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		explo->radius = 16.0F;
		VectorSet(explo->velocity, flrand(-128.0, 128.0), flrand(-128.0, 128.0), flrand(-128.0, 0.0));
		explo->r.angles[YAW] = (float)i*degreeinc;
		VectorCopy(rainpos, explo->direction);
		explo->alpha = 0.3;
		explo->lastThinkTime = fxi.cl->time;
		explo->LifeTime = duration;
		explo->Update = RedRainExplosionThink;
		if (powerup)
		{
			explo->SpawnInfo = 1;
			explo->r.frame = 1;
			explo->r.scale = 3;
		}
		else
		{
			explo->r.frame = 0;
			explo->r.scale = 2.5;
		}

		AddEffect(owner, explo);
	}

	// Add a big red flash at impact of course.
	explo = ClientEntity_new(-1, 0, impactpos, NULL, 500);
	if (powerup)
	{	// Green flash
		explo->r.model = rain_models+4;
	}
	else
	{	// Red flash
		explo->r.model = rain_models;
	}
	explo->r.frame = 0;
	explo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	explo->radius = 16.0F;
	explo->r.scale = 8.0F;
	explo->d_scale = -16.0F;
	explo->d_alpha = -2.0F;
	AddEffect(NULL, explo);

	if (powerup)
		fxi.S_StartSound(impactpos, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/RedRainPowerHit.wav"), 1, ATTN_NORM, 0);
	else
		fxi.S_StartSound(impactpos, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/RedRainHit.wav"), 1, ATTN_NORM, 0);
}



// end
