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
static qboolean MagicMissileTrailThink(struct client_entity_s* self, centity_t* owner)
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

static qboolean MagicMissileModelThink(struct client_entity_s* self, centity_t* owner)
{
	self->d_scale = 0.0f;
	self->r.scale = 0.8f;

	self->Update = MagicMissileTrailThink;
	MagicMissileTrailThink(self, owner);

	return true;
}

void FXMagicMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
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
	missile->Update = MagicMissileModelThink;

	AddEffect(owner, missile);
}

void FXMagicMissileExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	const paletteRGBA_t light_color = { .r = 160, .g = 71, .b = 237, .a = 255 }; //mxd. {0, 128, 128, 255} in original logic. Changed to better match sprite. 

	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_MAGICMISSILEEXPLODE].formatString, dir);

	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	Vec3ScaleAssign(32.0f, dir);

	for (int i = 0; i < NUM_MISSILE_EXPLODE_PARTS; i++)
	{
		client_entity_t* puff = ClientEntity_new(type, flags, origin, NULL, 500);

		puff->r.model = &missile_models[1]; // Indigo streak sprite.
		puff->r.scale = flrand(MISSILE_SCALE * 0.75f, MISSILE_SCALE * 1.5f);
		puff->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;

		VectorRandomCopy(dir, puff->velocity, MISSILE_EXPLODE_SPEED);
		puff->acceleration[2] = GetGravity() * 0.3f;

		puff->d_scale = -1.0f;
		puff->d_alpha = -2.0f;
		puff->radius = 20.0f;

		AddEffect(NULL, puff);
	}

	// Big flash.
	client_entity_t* halo = ClientEntity_new(type, flags, origin, 0, 500);

	halo->r.model = &missile_models[0]; // Indigo halo sprite.
	halo->r.scale = 2.0f;
	halo->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	halo->d_scale = -6.0f;
	halo->d_alpha = -2.0f;
	halo->radius = 20.0f;
	halo->dlight = CE_DLight_new(light_color, 150.0f, -50.0f);

	VectorScale(dir, 8.0f, halo->velocity);

	AddEffect(NULL, halo);
}

void FXBlast(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	assert(BLAST_NUM_SHOTS == 5);

	// Sends over the network 7 shorts and an origin.
	// Note that this is a vast improvement over five separate effects with a vector each (60 bytes+origins).
	short s_yaw;
	short s_pitch;
	short s_length[BLAST_NUM_SHOTS];
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_BLAST].formatString, &s_yaw, &s_pitch, 
		&s_length[0], &s_length[1], &s_length[2], &s_length[3], &s_length[4]);

	// Compress the angles into two shorts.
	vec3_t angles;
	angles[YAW] = (float)s_yaw * SHORT_TO_ANGLE;
	angles[PITCH] = (float)s_pitch * SHORT_TO_ANGLE;
	angles[ROLL] = 0.0f;

	// Set up for array.
	angles[YAW] -= BLAST_ANGLE_INC * (BLAST_NUM_SHOTS - 1) * 0.5f;

	for (int shot = 0; shot < BLAST_NUM_SHOTS; shot++)
	{
		vec3_t unit;
		AngleVectors(angles, unit, NULL, NULL);
		const float length = s_length[shot];

		vec3_t end_pos;
		VectorMA(origin, length, unit, end_pos);

		int numpuffs = (int)(length / BLAST_DIFF);
		numpuffs = min(40, numpuffs);

		vec3_t cur_pos;
		VectorCopy(origin, cur_pos);
		VectorScale(unit, BLAST_DIFF, unit);

		vec3_t back;
		VectorScale(unit, BLAST_BACKSPEED, back);

		float scale = BLAST_SCALE;

		for (int i = 0; i <= numpuffs; i++)
		{
			client_entity_t* puff = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, cur_pos, NULL, 750);

			puff->r.model = &missile_models[1]; // Indigo spark sprite.
			puff->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
			puff->r.scale = scale;
			puff->radius = 14.0f;
			puff->alpha = 0.95f;
			puff->d_scale = -1.4f * scale;
			VectorRandomSet(puff->velocity, 8.0f);
			VectorAdd(puff->velocity, back, puff->velocity);
			puff->acceleration[2] = BLAST_GRAVITY;

			AddEffect(NULL, puff);
			VectorAdd(cur_pos, unit, cur_pos);

			scale += 0.1f;
		}

		// We added the line, now throw out the impact.
		// Big flash first...
		client_entity_t* halo = ClientEntity_new(type, CEF_ADDITIVE_PARTS, end_pos, NULL, 1000);
		halo->r.model = &missile_models[0]; // Indigo halo sprite.
		halo->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		halo->r.scale = 1.0f + length * 0.001f; // Bigger when further out.
		halo->alpha = 0.95f;
		halo->d_scale = -5.0f;
		halo->d_alpha = -1.0f;
		halo->radius = 16.0f;
		VectorCopy(back, halo->velocity);

		AddEffect(NULL, halo);

		for (int i = 0; i < 9; i++)
		{
			client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_I, color_white, 750);

			VectorSet(spark->velocity, flrand(-64.0f, 64.0f), flrand(-64.0f, 64.0f), flrand(64.0f, 128.0f));
			spark->acceleration[2] = -PARTICLE_GRAVITY * 3.0f;
			spark->scale = 16.0f;
			spark->d_scale = -16.0f * 1.4f;

			AddParticleToList(halo, spark);
		}

		angles[YAW] += BLAST_ANGLE_INC;
	}
}