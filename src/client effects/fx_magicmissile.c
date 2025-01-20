//
// fx_magicmissile.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "Random.h"
#include "q_Sprite.h"
#include "Utilities.h"
#include "g_playstats.h"

#define MISSILE_TRAIL_SCALE			0.3f
#define MISSILE_TRAIL_SPEED			32.0f

#define NUM_MISSILE_EXPLODE_PARTS	12
#define MISSILE_SCALE				1.0f
#define MISSILE_EXPLODE_SPEED		200.0f

#define BLAST_DIFF					40.0f
#define BLAST_SCALE					0.3f
#define BLAST_BACKSPEED				(-1.5f)
#define BLAST_GRAVITY				(-32.0f)

static struct model_s* missile_models[3];

void PreCacheArray(void)
{
	missile_models[0] = fxi.RegisterModel("sprites/spells/halo_ind.sp2");
	missile_models[1] = fxi.RegisterModel("Sprites/spells/spark_ind.sp2");
	missile_models[2] = fxi.RegisterModel("Sprites/spells/indigostreak.sp2");
}

// These need to be converted to particles.
static qboolean FXMagicMissileTrailThink(const struct client_entity_s* self, centity_t* owner)
{
	vec3_t vel_normal;
	VectorCopy(self->velocity, vel_normal);
	VectorNormalize(vel_normal);

	for (int i = 0; i < 2; i++)	// Each cardinal direction.
	{
		client_entity_t* halo = ClientEntity_new(FX_WEAPON_MAGICMISSILE, 0, self->r.origin, NULL, 500);

		// self->up holds the direction to move in.
		VectorMA(halo->r.origin, (float)(-i) * 3.0f - flrand(0.0f, 3.0f), vel_normal, halo->r.origin);

		const float scaler = (i == 0 ? 1.0f : -1.0f); //mxd. 0: Up/right, 1: Down/left.
		VectorScale(self->up, scaler, halo->velocity);

		halo->r.model = &missile_models[0]; // Indigo halo sprite.
		halo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		halo->r.scale = flrand(MISSILE_TRAIL_SCALE, MISSILE_TRAIL_SCALE + 0.1f);
		halo->d_scale = -1.0f;
		halo->d_alpha = -2.0f;
		halo->radius = 20.0f;

		AddEffect(NULL, halo);
	}

	client_entity_t* trail = ClientEntity_new(FX_WEAPON_MAGICMISSILE, CEF_AUTO_ORIGIN | CEF_USE_VELOCITY2, self->r.origin, NULL, 500);

	trail->r.model = &missile_models[2]; // Indigo streak sprite.
	trail->r.spriteType = SPRITE_LINE;
	trail->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	VectorCopy(trail->r.origin, trail->r.startpos);
	VectorMA(trail->r.origin, -0.1f, self->velocity, trail->r.endpos);
	VectorScale(self->velocity, 0.9f, trail->velocity);
	VectorScale(self->velocity, 0.5f, trail->velocity2);
	trail->r.scale = 12.0f;
	trail->d_scale = -24.0f;
	trail->d_alpha = -2.0f;

	AddEffect(NULL, trail);

	return true;
}

static qboolean FXMagicMissileModelThink(struct client_entity_s* self, centity_t* owner)
{
	self->d_scale = 0.0f;
	self->r.scale = 0.8f;

	self->Update = FXMagicMissileTrailThink;
	FXMagicMissileTrailThink(self, owner);

	return true;
}

void FXMagicMissile(centity_t* owner, const int type, const int flags, const vec3_t origin)
{
	const paletteRGBA_t	light_color = { .r = 128, .g = 64, .b = 96, .a = 255 };

	short shortyaw;
	short shortpitch;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MAGICMISSILE].formatString, &shortyaw, &shortpitch);

	vec3_t angles;
	angles[YAW] = (float)shortyaw * SHORT_TO_ANGLE;
	angles[PITCH] = (float)shortpitch * SHORT_TO_ANGLE;
	angles[ROLL] = 0.0f;

	vec3_t fwd;
	vec3_t right;
	vec3_t up;
	AngleVectors(angles, fwd, right, up);

	// Add the magic-missile model.
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);

	missile->r.model = &missile_models[0]; // Star-like indigo halo sprite.
	missile->r.frame = 1;

	const float speed = MAGICMISSILE_SPEED * ((flags & CEF_FLAG6) ? 0.5f : 1.0f); //mxd
	VectorScale(fwd, speed, missile->velocity);

	VectorCopy(angles, missile->r.angles);

	// Set up the direction we want the trail to fly from the missile.
	VectorMA(up, 2.0f, right, missile->up);
	VectorScale(missile->up, MISSILE_TRAIL_SPEED, missile->up);

	missile->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD;
	missile->r.scale = 0.4f;
	missile->d_scale = 4.0f;
	missile->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	missile->Update = FXMagicMissileModelThink;

	AddEffect(owner, missile);
}

// ************************************************************************************************
// FXMagicMissileExplode
// ************************************************************************************************

void FXMagicMissileExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t			dir;
	client_entity_t	*smokepuff;
	int				i;
	paletteRGBA_t	lightcolor = {0, 128, 128, 255};

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MAGICMISSILEEXPLODE].formatString, dir);
	if(flags & CEF_FLAG6)
	{
		FXClientScorchmark(origin, dir);
	}
	Vec3ScaleAssign(32.0, dir);

	for(i = 0; i < NUM_MISSILE_EXPLODE_PARTS; i++)
	{
		smokepuff = ClientEntity_new(type, flags, origin, 0, 500);

		smokepuff->r.model = missile_models + 1;
		smokepuff->r.scale = flrand(MISSILE_SCALE * 0.75, MISSILE_SCALE * 1.5);
		smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, smokepuff->velocity, MISSILE_EXPLODE_SPEED);
		smokepuff->acceleration[2] = GetGravity()*0.3;

		smokepuff->d_scale = -1.0;
		smokepuff->d_alpha = -2.0;
		smokepuff->radius = 20.0;

		AddEffect(NULL, smokepuff);
	}

	// Big flash
	smokepuff = ClientEntity_new(type, flags, origin, 0, 500);

	smokepuff->r.model = missile_models;
	smokepuff->r.frame = 0;

	smokepuff->r.scale = 2.0;
	smokepuff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

	VectorScale(dir, 8.0, smokepuff->velocity);

	smokepuff->d_scale = -6.0;
	smokepuff->d_alpha = -2.0;
	smokepuff->radius = 20.0;

	smokepuff->dlight = CE_DLight_new(lightcolor, 150.0, -50.0);

	AddEffect(NULL, smokepuff);
}

// Create Effect FX_WEAPON_BLAST
void FXBlast(centity_t *owner, int type, int flags, vec3_t origin)
{
	vec3_t endpos, curpos;
	vec3_t unit, back;
	int i, numpuffs;
	client_entity_t *puff;
	client_particle_t *spark;
	paletteRGBA_t pal;
	float length, scale;
	short slength[BLAST_NUM_SHOTS], syaw, spitch;
	int		shot;
	vec3_t	angles;

	assert(BLAST_NUM_SHOTS==5);

	// Sends over the network 7 shorts and an origin.
	// Note that this is a vast improvement over five seperate effects with a vector each (60 bytes+origins)
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_BLAST].formatString, &syaw, &spitch, &slength[0], &slength[1], &slength[2], &slength[3], &slength[4]);

	// Compress the angles into two shorts.
	angles[YAW] = (float)syaw*(360.0/65536.0);
	angles[PITCH] = (float)spitch*(360.0/65536.0);
	angles[ROLL] = 0.0;

	// Set up for array.
	angles[YAW] -= BLAST_ANGLE_INC * (BLAST_NUM_SHOTS-1) * 0.5;
	for (shot=0; shot<BLAST_NUM_SHOTS; shot++)
	{
		AngleVectors(angles, unit, NULL, NULL);
		length = (float)slength[shot];
		VectorMA(origin, length, unit, endpos);

		numpuffs = (int)(length/BLAST_DIFF);
		VectorCopy(origin, curpos);
		VectorScale(unit, BLAST_DIFF, unit);
		VectorScale(unit, BLAST_BACKSPEED, back);
		scale = BLAST_SCALE;
		if (numpuffs>40)
			numpuffs=40;
		for(i=0; i<=numpuffs; i++)
		{
			puff = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, curpos, NULL, 750);
			puff->r.model = missile_models + 1;
			puff->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			puff->r.scale = scale;
			puff->radius = 14.0;
			puff->alpha = 0.95;
//			puff->d_alpha = -1.0;
			puff->d_scale = -1.4*scale;
			VectorSet(puff->velocity, flrand(-8.0,8.0), flrand(-8.0,8.0), flrand(-8.0,8.0));
			VectorAdd(puff->velocity, back, puff->velocity);
			puff->acceleration[2] = BLAST_GRAVITY;
			AddEffect(NULL, puff);
			VectorAdd(curpos, unit, curpos);
			scale+=0.1;
		}

		// We added the line, now throw out the impact
		// Big flash first...
		puff = ClientEntity_new(type, CEF_ADDITIVE_PARTS, endpos, NULL, 1000);
		puff->r.model = missile_models;
		puff->r.frame = 0;
		puff->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		puff->r.scale = 1.0 + length * 0.001;		// Bigger when further out.
		puff->alpha = 0.95;
		puff->d_scale = -5.0;
		puff->d_alpha = -1.0;
		puff->radius = 16.0;
		VectorCopy(back, puff->velocity);

		AddEffect(NULL, puff);

		pal.c = 0xffffffff;
		for (i=0; i<9; i++)
		{
			spark = ClientParticle_new(PART_16x16_SPARK_I, pal, 750);
			VectorSet(spark->velocity, flrand(-64.0, 64.0), flrand(-64.0, 64.0), flrand(64.0, 128.0));
			spark->acceleration[2] = -PARTICLE_GRAVITY*3.0;
			spark->scale = 16.0;
			spark->d_scale = -16.0*1.4;
			AddParticleToList(puff, spark);
		}

		angles[YAW] += BLAST_ANGLE_INC;
	}
}
 