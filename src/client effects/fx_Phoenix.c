//
// fx_Phoenix.c
//
// Copyright 1998 Raven Software
//

#include "fx_Phoenix.h" //mxd
#include "fx_PhoenixLocal.h" //mxd
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define EXPLODE_SPEED			160.0f
#define EXPLODE_GRAVITY			(-320.0f)
#define EXPLODE_SCALE			14.0f
#define EXPLODE_NUM_BITS		32
#define EXPLODE_BALL_SPEED		64.0f
#define EXPLODE_NUM_SMALLBALLS	3
#define EXPLODE_LIFETIME		50
#define EXPLODE_TIME_MAX		750

#define FIRETRAIL_PARTS			4
#define FIRETRAIL_RADIUS		6.0f
#define FIRETRAIL_SPEED			16.0f
#define FIRETRAIL_SCALE			12.0f
#define FIRETRAIL_ACCEL			32.0f

#define SMOKETRAIL_RADIUS		2.0f
#define SMOKETRAIL_SCALE		0.25f
#define SMOKETRAIL_ALPHA		0.5f

static struct model_s* phoenix_models[6];

void PreCachePhoenix(void)
{
	phoenix_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	phoenix_models[1] = fxi.RegisterModel("models/spells/phoenixarrow/tris.fm");
	phoenix_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
	phoenix_models[3] = fxi.RegisterModel("sprites/spells/phoenix.sp2");
	phoenix_models[4] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm");
	phoenix_models[5] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
}

#pragma region ========================== PHOENIX EXPLOSION ==========================

static qboolean FXPhoenixMissileThink(client_entity_t* missile, centity_t* owner)
{
	int duration;

	if ((int)r_detail->value == DETAIL_LOW)
		duration = 1400;
	else if ((int)r_detail->value == DETAIL_NORMAL)
		duration = 1700;
	else
		duration = 2000;

	// Here we want to shoot out flame to either side.
	vec3_t angles;
	VectorScale(missile->r.angles, RAD_TO_ANGLE, angles);

	vec3_t fwd;
	vec3_t right;
	AngleVectors(angles, fwd, right, NULL);
	VectorScale(fwd, -4.0f * FIRETRAIL_SPEED, fwd);
	VectorScale(right, FIRETRAIL_SPEED, right);

	// Throw smoke to each side, alternating.
	const float side = ((missile->LifeTime-- & 1) ? 1.0f : -1.0f); //mxd. 1.0 - right, -1.0 - left.

	vec3_t smoke_origin;
	VectorSet(smoke_origin,
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS / 2.0f, SMOKETRAIL_RADIUS / 2.0f));

	VectorAdd(smoke_origin, missile->origin, smoke_origin);

	client_entity_t* smoke = ClientEntity_new(-1, CEF_DONT_LINK, smoke_origin, NULL, duration);

	smoke->radius = 64.0f; //BUGFIX: 128.0 for the left side in original version. Why?..
	smoke->r.model = &phoenix_models[0]; // steam_add sprite.
	smoke->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	smoke->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->r.scale = SMOKETRAIL_SCALE;
	VectorScale(right, 2.0f * side, smoke->velocity);
	smoke->d_scale = 2.0f; // Rate of change in scale.
	smoke->d_alpha = -1.0f;

	AddEffect(NULL, smoke);	// Add the smoke as independent world smoke.

	// Add fire to the tail. Attach it to the smoke because it doesn't get out of the fx radius so quickly.
	for (int i = 0; i < FIRETRAIL_PARTS; i++)
	{
		const paletteRGBA_t light_color = { .r = 0xff, .g = 0x7f, .b = 0x00, .a = 0xe5 };
		client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, duration);

		VectorSet(flame->origin,
			flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS),
			flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS),
			flrand(-FIRETRAIL_RADIUS / 3.0f, FIRETRAIL_RADIUS / 3.0f));

		VectorAdd(missile->origin, flame->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity,
			flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED),
			flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED),
			flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side
		VectorAdd(flame->velocity, fwd, flame->velocity);

		// Alternate left and right side of phoenix.
		if (i & 1)
			VectorAdd(flame->velocity, right, flame->velocity);
		else
			VectorSubtract(flame->velocity, right, flame->velocity);

		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0f, -10.0f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(smoke, flame);
	}

	// Update animation frame.

	// Check if the time is up.
	if (fxi.cl->time >= missile->lastThinkTime)
	{
		// Set up animations to go the other direction.
		if (missile->NoOfAnimFrames == 7)
		{
			// Set to go backwards to 3.
			missile->NoOfAnimFrames = 3;
			missile->r.frame = 7;
		}
		else
		{
			// Set to go forward to 7
			missile->NoOfAnimFrames = 7;
			missile->r.frame = 3;
		}

		missile->Scale = -1;
		missile->lastThinkTime = fxi.cl->time + (4 * 50);
	}
	else
	{
		missile->r.frame = missile->NoOfAnimFrames - (int)(missile->Scale * ((float)(missile->lastThinkTime - fxi.cl->time) / 50.0f)) - 1;
	}

	// Remember for even spread of particles.
	VectorCopy(missile->r.origin, missile->origin);

	return true;
}

void FXPhoenixMissile(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 25);
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_PHOENIXMISSILE].formatString, missile->velocity);

	const float velocity = PHOENIX_ARROW_SPEED * ((flags & CEF_FLAG8) ? 0.5f : 1.0f);
	Vec3ScaleAssign(velocity, missile->velocity);

	vec3_t dir;
	VectorCopy(missile->velocity, dir);
	VectorNormalize(dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 256.0f;
	missile->r.model = &phoenix_models[1]; // Phoenix arrow model.
	missile->flags |= CEF_ADDITIVE_PARTS;
	missile->lastThinkTime = fxi.cl->time + (50 * 7); // Time to play last frame.
	missile->NoOfAnimFrames = 7; // End on frame number 7.
	missile->Scale = 1.0f; // Positive frame count.
	missile->r.scale = 0.8f;
	missile->color.c = 0xff00ffff;
	missile->LifeTime = 1000;

	if ((int)r_detail->value != DETAIL_LOW)
		missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);

	if (flags & CEF_FLAG6)
		missile->Update = FXPhoenixMissilePowerThink;
	else
		missile->Update = FXPhoenixMissileThink;

	AddEffect(owner, missile);
}

static qboolean FXPhoenixExplosionSmallBallThink(client_entity_t* ball, centity_t* owner) //mxd. Moved above FXPhoenixExplosionBallThink.
{
	if (fxi.cl->time - ball->startTime > EXPLODE_TIME_MAX)
		return false;

	const float vel_factor = (float)(fxi.cl->time - ball->lastThinkTime) / 1000.0f;
	ball->lastThinkTime = fxi.cl->time;

	// Spin the ball of fire while it expands and fades.
	ball->r.angles[YAW] += ball->velocity2[YAW] * vel_factor;
	ball->r.angles[PITCH] += ball->velocity2[PITCH] * vel_factor;

	return true;
}

// This is also exported for use in FXBarrelExplode.
qboolean FXPhoenixExplosionBallThink(client_entity_t* ball, centity_t* owner)
{
	if (!FXPhoenixExplosionSmallBallThink(ball, owner))
		return false;

	if (ball->dlight->intensity > 0.0f)
		ball->dlight->intensity -= 5.0f;

	return true;
}

static qboolean FXPhoenixExplosionBirdThink(client_entity_t* bird, centity_t* owner)
{
	bird->LifeTime--;

	if (bird->LifeTime <= 0)
		return false;

	int duration;
	if ((int)r_detail->value == DETAIL_LOW)
		duration = 175;
	else if ((int)r_detail->value == DETAIL_NORMAL)
		duration = 210;
	else
		duration = 250;

	// Spawn another trail bird.
	vec3_t pos;
	VectorRandomSet(pos, 8.0f);
	VectorAdd(pos, bird->r.origin, pos);

	client_entity_t* new_bird = ClientEntity_new(-1, bird->r.flags, pos, NULL, duration);

	new_bird->radius = 128.0f;
	new_bird->r.model = &phoenix_models[3]; // Phoenix sprite.
	new_bird->r.frame = 1;
	new_bird->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	new_bird->r.scale = bird->r.scale;
	new_bird->alpha = bird->alpha;
	new_bird->d_alpha = -4.0f * bird->alpha;
	new_bird->d_scale = 2.0f;
	VectorCopy(bird->velocity, new_bird->velocity);

	AddEffect(NULL, new_bird);

	return true;
}

// This is also exported for use in FXBarrelExplode.
client_entity_t* CreatePhoenixSmallExplosion(const vec3_t ball_origin) //TODO: rename to FXPhoenixCreateSmallExplosion?
{
	client_entity_t* sub_explosion = ClientEntity_new(FX_WEAPON_PHOENIXEXPLODE, CEF_BROADCAST, ball_origin, NULL, 17);

	sub_explosion->radius = 128.0f;
	sub_explosion->r.model = &phoenix_models[4]; // Inner explosion model.
	sub_explosion->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	sub_explosion->startTime = fxi.cl->time;
	sub_explosion->lastThinkTime = fxi.cl->time;
	sub_explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	sub_explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);
	sub_explosion->Update = FXPhoenixExplosionSmallBallThink;

	return sub_explosion;
}

//////////////////////////
// From CreateEffect FX_WEAPON_PHOENIXEXPLODE
//////////////////////////
void FXPhoenixExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*explosion, *subexplosion;
	paletteRGBA_t		color;
	vec3_t				dir, sdir;
	client_particle_t	*spark;
	int					i;
	float				ballnum;
	int					count, dur;

	fxi.GetEffect(owner,flags,clientEffectSpawners[FX_WEAPON_PHOENIXEXPLODE].formatString,dir,sdir);

	// make the scorch mark if we should
	if(flags & CEF_FLAG8)
		FXClientScorchmark(origin, sdir);

	if(flags & CEF_FLAG6)
	{	// powered up version
		FXPhoenixExplodePower(owner, type, flags, origin, dir);
		return;
	}

	flags |= CEF_OWNERS_ORIGIN;

	if(r_detail->value != DETAIL_LOW)
	{
		count =	EXPLODE_NUM_SMALLBALLS;
		if(r_detail->value == DETAIL_NORMAL)
			count =	EXPLODE_NUM_SMALLBALLS - 1;

		// Create three smaller explosion spheres.
		for (i=0; i < count; i++)
		{
			ballnum = i;
			subexplosion = CreatePhoenixSmallExplosion(origin);
			VectorSet(subexplosion->velocity, 
							flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[0]*EXPLODE_BALL_SPEED),
							flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[1]*EXPLODE_BALL_SPEED),
							flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[2]*EXPLODE_BALL_SPEED));
			subexplosion->r.scale = 0.1;
			subexplosion->d_scale = 3.0 + ballnum;
			subexplosion->d_alpha = -1.5 - 0.5*ballnum;

			AddEffect(NULL, subexplosion);
		}
	}

	// Create the main big explosion sphere.
	explosion = ClientEntity_new(type, flags, origin, NULL, 17);
	explosion->r.model = phoenix_models + 5;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->flags |= CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA;
	explosion->alpha = 0.1;
	explosion->r.scale= 0.1;
	explosion->d_alpha = 3.0;
	explosion->d_scale=5.0;
	explosion->radius=128;
	explosion->startTime = fxi.cl->time;
	explosion->lastThinkTime = fxi.cl->time;
	explosion->velocity2[YAW] = flrand(-M_PI, M_PI);
	explosion->velocity2[PITCH] = flrand(-M_PI, M_PI);

	color.c = 0xff00ffff;
	explosion->dlight = CE_DLight_new(color, 150.0F, 0.0F);
	explosion->Update = FXPhoenixExplosionBallThink;
	AddEffect(NULL, explosion);
	
	// Add some glowing blast particles.
	VectorScale(dir,EXPLODE_SPEED,dir);
	count = GetScaledCount(EXPLODE_NUM_BITS, 0.3);
	for(i = 0; i < count; i++)
	{
		spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 2000);
		VectorSet(spark->velocity,	flrand(-EXPLODE_SPEED, EXPLODE_SPEED), 
									flrand(-EXPLODE_SPEED, EXPLODE_SPEED), 
									flrand(-EXPLODE_SPEED, EXPLODE_SPEED));
		VectorAdd(spark->velocity, dir, spark->velocity);
		spark->acceleration[2] = EXPLODE_GRAVITY;
		spark->scale = EXPLODE_SCALE;
		spark->d_scale = flrand(-20.0, -10.0);
		spark->d_alpha = flrand(-400.0, -320.0);
		spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(explosion, spark);
	}

	// ...and a big-ass flash
	explosion = ClientEntity_new(-1, flags, origin, NULL, 250);
	explosion->r.model = phoenix_models + 2;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 1;
	explosion->radius=128;
	explosion->r.scale=1.5;
	explosion->d_alpha=-4.0;
	explosion->d_scale=-4.0;
	AddEffect(NULL, explosion);

	if(r_detail->value == DETAIL_LOW)
		dur = 150;
	else
	if(r_detail->value == DETAIL_NORMAL)
		dur = 125;
	else
		dur = 100;

	// ...and draw the phoenix rising from the explosion
	explosion = ClientEntity_new(type, flags, origin, NULL, dur);
	explosion->r.model = phoenix_models + 3;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 0;
	explosion->radius=128;
	explosion->r.scale=0.1;
	VectorScale(dir, 0.25, explosion->velocity);
	explosion->acceleration[2] = 64;
	explosion->alpha = 1.0;
	explosion->d_alpha=-1.0;
	explosion->d_scale=1.25;
	explosion->LifeTime = 10;
	explosion->Update = FXPhoenixExplosionBirdThink;
	AddEffect(NULL, explosion);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/PhoenixHit.wav"), 1, ATTN_NORM, 0);
}

#pragma endregion

#pragma region ========================== POWERED PHOENIX EXPLOSION ==========================

#define PHOENIXPOWER_NUMTRAILS 11
#define PHOENIXPOWER_PARTS_PER_TRAIL 8
#define PHOENIXPOWER_RADIUS 72.0

static qboolean FXPhoenixExplosionBirdThinkPower(client_entity_t *bird, centity_t *owner)
{
	bird->LifeTime--;
	if (bird->LifeTime <= 0)
	{
		return(false);
	}
	return (true);
}

void FXPhoenixExplodePower(centity_t *owner, int type, int flags, vec3_t origin, vec3_t dir)
{
	client_entity_t		*explosion, *subexplosion;
	paletteRGBA_t		color;
	client_particle_t	*spark;
	int					i, j;
	vec3_t				phOrg;
	float				cosVal, sinVal;
	trace_t				trace;
	vec3_t				endPos;
	vec3_t				minmax = {0, 0, 0};
	int					numTrails;
	int					numParts;
	float				partHeight;
	float				detail_scale;

	flags |= CEF_OWNERS_ORIGIN;

	// This isn't actually used but we need something to anchor the particles to
	// Create the main big explosion sphere.
	explosion = ClientEntity_new(type, flags, origin, NULL, 17);
	explosion->r.model = phoenix_models + 5;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->flags |= CEF_ADDITIVE_PARTS;
	explosion->alpha = 1.0;
	explosion->r.scale= .1;
	explosion->d_alpha=-2.0/1.5;
	explosion->radius=128;
	explosion->LifeTime=EXPLODE_LIFETIME;

	color.c = 0xff00ffff;
	explosion->dlight = CE_DLight_new(color, 150.0F, 0.0F);
	explosion->Update = FXPhoenixExplosionBallThink;
	AddEffect(NULL, explosion);
	color.c = 0xffffffff;

	numTrails = GetScaledCount(PHOENIXPOWER_NUMTRAILS-4, 0.8) + 4;//4 is the minimum
	numParts = GetScaledCount(PHOENIXPOWER_PARTS_PER_TRAIL - 4, .8) + 4;//ditto

	for(i = 0; i < numTrails; i++)
	{
		cosVal = cos((float)i / (float)numTrails * (M_PI*2));
		sinVal = sin((float)i / (float)numTrails * (M_PI*2));

		VectorCopy(origin, phOrg);
		phOrg[0] += cosVal * PHOENIXPOWER_RADIUS;
		phOrg[1] += sinVal * PHOENIXPOWER_RADIUS;

		VectorCopy(phOrg, endPos);
		endPos[2] -= 64;

		fxi.Trace(	phOrg, minmax, minmax, endPos, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

		if(trace.fraction > .99)
		{	// Burst in the air, no ground found.
			subexplosion = ClientEntity_new(-1, flags, phOrg, NULL, 1000);
			subexplosion->r.model = phoenix_models + 2;
			subexplosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
			subexplosion->r.frame = 1;
			subexplosion->radius=128;
			subexplosion->r.scale=1.5;
			subexplosion->d_alpha=-1.0;
			AddEffect(NULL, subexplosion);

			for(j = 0; j < numParts; j++)
			{
				partHeight = (j * PHOENIXPOWER_PARTS_PER_TRAIL)/numParts;

				spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 1000);
				spark->origin[0] = cosVal * 72;
				spark->origin[1] = sinVal * 72;
				// alternate up and down.
				if (j&0x01)
					spark->velocity[2] = partHeight * 15.0* (18.0/8.0) + flrand(0, 15);
				else
					spark->velocity[2] = -partHeight * 15.0* (18.0/8.0) + flrand(0, 15);
				spark->scale = 32.0 - (partHeight * (18.0/8.0)/3);
				spark->d_scale = -partHeight * (18.0/8.0);
				spark->d_alpha = flrand(-400.0, -320.0)/1.3;
				spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

				AddParticleToList(explosion, spark);
			}
		}
		else
		{
			subexplosion = ClientEntity_new(-1, flags, trace.endpos, NULL, 1000);
			subexplosion->r.model = phoenix_models + 2;
			subexplosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
			subexplosion->r.frame = 1;
			subexplosion->radius=128;
			subexplosion->r.scale=1.5;
			subexplosion->d_alpha=-1.0;
			AddEffect(NULL, subexplosion);

			for(j = 0; j < numParts; j++)
			{
				partHeight = (j * PHOENIXPOWER_PARTS_PER_TRAIL)/numParts;

				spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color, 1000);
				spark->origin[0] = cosVal * 72;
				spark->origin[1] = sinVal * 72;
				spark->origin[2] = trace.endpos[2] - origin[2];
				spark->velocity[2] = partHeight * 15.0* (18.0/8.0) + flrand(0, 15);
				spark->scale = 32.0 - (partHeight * (18.0/8.0)/3);
				spark->d_scale = -partHeight * (18.0/8.0);
				spark->d_alpha = flrand(-400.0, -320.0)/1.3;
				spark->duration = (255.0 * 2000.0) / -spark->d_alpha;		// time taken to reach zero alpha

				AddParticleToList(explosion, spark);
			}
		}
	}

	if (r_detail->value == DETAIL_LOW)
		detail_scale = 1.5;
	else
		detail_scale = 2.0;

	// ...and draw the phoenix rising from the explosion
	explosion = ClientEntity_new(type, flags, origin, NULL, 100);
	explosion->r.model = phoenix_models + 3;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 0;
	explosion->radius=128;
	explosion->r.scale=1.0;
	VectorScale(dir, 192.0, explosion->velocity);
	explosion->acceleration[2] = 256.0;
	explosion->alpha = 1.0;
	explosion->d_alpha=-1.5;
	explosion->d_scale=detail_scale;
	explosion->LifeTime = 6;
	explosion->Update = FXPhoenixExplosionBirdThinkPower;
	AddEffect(NULL, explosion);

	// inner phoenix
	explosion = ClientEntity_new(type, flags, origin, NULL, 100);
	explosion->r.model = phoenix_models + 3;
	explosion->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;// | RF_FULLBRIGHT;
	explosion->r.frame = 0;
	explosion->radius=128;
	explosion->r.scale=1.0;
	VectorScale(dir, 192.0, explosion->velocity);
	explosion->acceleration[2] = 256.0;
	explosion->alpha = 1.0;
	explosion->d_alpha= 0.0;
	explosion->d_scale=-1.0/.6;
	explosion->LifeTime = 6;
	explosion->Update = FXPhoenixExplosionBirdThinkPower;
	AddEffect(NULL, explosion);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/PhoenixPowerHit.wav"), 1, ATTN_NORM, 0);
}

static qboolean FXPhoenixMissilePowerThink(client_entity_t *missile, centity_t *owner)
{
	int					i, dur;
	client_particle_t	*flame;
	client_entity_t		*smoke;
	vec3_t				angles, fwd, right, smokeorigin;
	paletteRGBA_t		LightColor={0xff, 0x7f, 0x00, 0xe5};
	int					sideVal;

	if (r_detail->value == DETAIL_LOW)
		dur = 1400;
	else
	if (r_detail->value == DETAIL_NORMAL)
		dur = 1700;
	else
		dur = 2000;
	
	// Here we want to shoot out flame to either side
	VectorScale(missile->r.angles, 180.0/M_PI, angles);
	AngleVectors(angles, fwd, right, NULL);
	VectorScale(fwd, -4.0*FIRETRAIL_SPEED, fwd);
	VectorScale(right, FIRETRAIL_SPEED, right);

	sideVal = (missile->LifeTime&0x1) ? -1:1;
	missile->LifeTime--;
	// Throw smoke to each side, alternating.  
	VectorSet(	smokeorigin, 
				flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
				flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS), 
				flrand(-SMOKETRAIL_RADIUS/2.0, SMOKETRAIL_RADIUS/2.0));
	VectorAdd(	smokeorigin, missile->origin, smokeorigin);
	smoke = ClientEntity_new(-1, CEF_DONT_LINK, smokeorigin, NULL, dur);
	smoke->r.model = phoenix_models;
	smoke->r.flags |= RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	smoke->radius = 64.0F;
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->r.scale = SMOKETRAIL_SCALE * 2.5;
	smoke->velocity[0] = sideVal * right[0]*2.0;
	smoke->velocity[1] = sideVal * right[1]*2.0;
	smoke->velocity[2] = sideVal * right[2]*2.0;
	smoke->d_scale = 2.0;		// Rate of change in scale
	smoke->d_alpha = -1.0;
	AddEffect(NULL, smoke);	// add the smoke as independent world smoke
	smoke->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;

	// Burn baby burn add fire to the tail.  Attach it to the smoke because it doesn't get out of the fx radius so quickly
	for(i = 0; i < FIRETRAIL_PARTS; i++)
	{
		flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), LightColor, dur);
		VectorSet(	flame->origin, 
					flrand(-FIRETRAIL_RADIUS/3, FIRETRAIL_RADIUS/3), 
					flrand(-FIRETRAIL_RADIUS/3, FIRETRAIL_RADIUS/3), 
					flrand(-FIRETRAIL_RADIUS/3.0, FIRETRAIL_RADIUS/3.0));
		VectorAdd(	missile->origin, flame->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity, 
				flrand(-FIRETRAIL_SPEED/3, FIRETRAIL_SPEED/3), flrand(-FIRETRAIL_SPEED/3, FIRETRAIL_SPEED/3), flrand(-1.0, 1.0));
		// Make the fire shoot out the back and to the side
		VectorAdd(flame->velocity, fwd, flame->velocity);
		// Alternate left and right side of phoenix
		if (i&0x01)
			VectorAdd(flame->velocity, right, flame->velocity);
		else
			VectorSubtract(flame->velocity, right, flame->velocity);
		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0, -10.0);
		flame->d_alpha = flrand(-200.0, -160.0);
		flame->duration = (255.0 * 1000.0) / -flame->d_alpha;		// time taken to reach zero alpha

		AddParticleToList(smoke, flame);
	}


	// Remember for even spread of particles
	VectorCopy(missile->r.origin, missile->origin);
	return(true);
}

#pragma endregion