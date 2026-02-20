//
// fx_HellStaff.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "Random.h"
#include "Utilities.h"
#include "q_sprite.h"
#include "g_playstats.h"

#define HELLBOLT_IMPACT_SPARKS_MIN		8 //mxd
#define HELLBOLT_IMPACT_SPARKS_MAX		12 //mxd
#define HELLBOLT_IMPACT_SPARK_VEL_MIN	64.0f //mxd
#define HELLBOLT_IMPACT_SPARK_VEL_MAX	96.0f //mxd

#define HELLLASER_PARTS			9
#define HELLLASER_SPEED			32.0f

static struct model_s* hell_models[2];
static struct sfx_s* hell_hit_sound; //mxd

const paletteRGBA_t hellbolt_dlight_color = { .r = 255, .g = 96, .b = 48, .a = 255 };

void PreCacheHellstaff(void)
{
	hell_models[0] = fxi.RegisterModel("sprites/spells/hellstafproj.sp2");
	hell_models[1] = fxi.RegisterModel("sprites/fx/helllaser.sp2");
}

void PreCacheHellstaffSFX(void) //mxd
{
	hell_hit_sound = fxi.S_RegisterSound("weapons/HellHit.wav");
}

void FXHellbolt(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	vec3_t vel;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_HELLBOLT].formatString, vel);

	const float speed = HELLBOLT_SPEED * ((flags & CEF_FLAG6) ? 0.5f : 1.0f); //mxd
	Vec3ScaleAssign(speed, vel);

	client_entity_t* hellbolt = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 10000);

	hellbolt->r.model = &hell_models[0];
	hellbolt->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	hellbolt->r.frame = irand(0, 1);
	vectoangles(vel, hellbolt->r.angles);
	VectorCopy(vel, hellbolt->velocity);

	hellbolt->r.scale = 0.5f;
	hellbolt->r.color = hellbolt_dlight_color;
	hellbolt->d_alpha = 0.0f;
	hellbolt->radius = 10.0f;
	hellbolt->dlight = CE_DLight_new(hellbolt_dlight_color, 120.0f, 0.0f);

	AddEffect(owner, hellbolt);
}

static void HellboltExplode(const vec3_t loc, const vec3_t vel)
{
	client_entity_t* blast = ClientEntity_new(-1, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, loc, NULL, 500);

	blast->radius = 32.0f;
	blast->dlight = CE_DLight_new(hellbolt_dlight_color, 120.0f, -20.0f); //mxd. intensity:150, d_intensity:-200 in original logic.
	CE_DLight_SetColorFade(blast->dlight, 0.0f, 0.0f, 0.0f, blast->updateTime); //mxd

	fxi.S_StartSound(blast->r.origin, -1, CHAN_WEAPON, hell_hit_sound, 1.0f, ATTN_NORM, 0.0f);
	AddEffect(NULL, blast);

	const int num_particles = irand(HELLBOLT_IMPACT_SPARKS_MIN, HELLBOLT_IMPACT_SPARKS_MAX); //mxd
	for (int i = 0; i < num_particles; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_R, hellbolt_dlight_color, 500);

		//mxd. Scale here instead of FXHellboltExplode() (was done before calling FXClientScorchmark(), which could re-normalize 'dir'...).
		VectorRandomCopy(vel, spark->velocity, 0.65f); // Original logic scales vel by 32, then randomizes it by 64 --mxd.
		VectorNormalize(spark->velocity);
		Vec3ScaleAssign(flrand(HELLBOLT_IMPACT_SPARK_VEL_MIN, HELLBOLT_IMPACT_SPARK_VEL_MAX), spark->velocity);

		VectorSet(spark->acceleration, 0.0f, 0.0f, GetGravity() * flrand(0.15f, 0.3f)); //mxd. Randomize gravity scaler a bit.
		spark->scale = flrand(12.0f, 16.0f);
		spark->d_scale = -24.0f;
		spark->d_alpha = flrand(-640.0f, -512.0f);

		AddParticleToList(blast, spark);
	}
}

void FXHellboltExplode(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_HELLBOLTEXPLODE].formatString, dir);

	if (flags & CEF_FLAG6)
		FXClientScorchmark(origin, dir);

	HellboltExplode(origin, dir);
}

static void HellLaserBurn(vec3_t loc, vec3_t fwd, vec3_t right, vec3_t up)
{
	client_entity_t* blast = ClientEntity_new(-1, CEF_NO_DRAW | CEF_ADDITIVE_PARTS, loc, NULL, 1000);

	blast->radius = 32.0f;
	blast->dlight = CE_DLight_new(hellbolt_dlight_color, 120.0f, -30.0f); //mxd. intensity:150, d_intensity:-300 in original logic.
	CE_DLight_SetColorFade(blast->dlight, 0.0f, 0.0f, 0.0f, blast->updateTime); //mxd

	AddEffect(NULL, blast);

	const float delta_angle = ANGLE_360 / (float)HELLLASER_PARTS;
	float cur_angle = flrand(0.0f, delta_angle);
	Vec3ScaleAssign(-0.25f * HELLLASER_SPEED, fwd);
	Vec3ScaleAssign(HELLLASER_SPEED, right);
	Vec3ScaleAssign(HELLLASER_SPEED, up);

	for (int i = 0; i < HELLLASER_PARTS; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_R, color_white, 1000);

		VectorMA(fwd, cosf(cur_angle), right, spark->velocity);
		VectorMA(spark->velocity, sinf(cur_angle), up, spark->velocity);
		spark->acceleration[2] = 64.0f;
		spark->scale = flrand(8.0f, 24.0f);
		spark->d_scale = -12.0f;
		spark->d_alpha = flrand(-512.0f, -256.0f);

		AddParticleToList(blast, spark);

		cur_angle += delta_angle;
	}
}

void FXHellstaffPowerBurn(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_HELLSTAFF_POWER_BURN].formatString, &dir);

	vec3_t angles;
	vectoangles(dir, angles);
	angles[PITCH] *= -1.0f; // Something's broken with angle signs somewhere ;(

	vec3_t fwd;
	vec3_t right;
	vec3_t up;
	AngleVectors(angles, fwd, right, up);

	HellLaserBurn(origin, fwd, right, up);
}

void FXHellstaffPower(centity_t* owner, int type, const int flags, vec3_t origin)
{
	vec3_t dir;
	byte beam_length;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_HELLSTAFF_POWER].formatString, &dir, &beam_length);

	vec3_t angles;
	vectoangles(dir, angles);
	angles[PITCH] *= -1.0f; // Something's broken with angle signs somewhere ;(

	vec3_t fwd;
	vec3_t right;
	vec3_t up;
	AngleVectors(angles, fwd, right, up);

	vec3_t endpos;
	const float len = (float)beam_length * 8.0f;
	VectorMA(origin, len, fwd, endpos);

	// Make the line beam.
	client_entity_t* beam = ClientEntity_new(-1, CEF_DONT_LINK | CEF_ABSOLUTE_PARTS | CEF_ADDITIVE_PARTS, origin, NULL, 333);

	beam->r.model = &hell_models[1];
	beam->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	beam->r.scale = flrand(6.0f, 10.0f);
	beam->r.tile = len / flrand(40.0f, 48.0f);
	beam->r.tileoffset = flrand(0.0f, 1.0f);
	beam->radius = 40.0f;
	beam->alpha = 0.95f;
	beam->d_alpha = -3.0f;
	VectorCopy(origin, beam->r.startpos);
	VectorCopy(endpos, beam->r.endpos);
	beam->r.spriteType = SPRITE_LINE;

	AddEffect(NULL, beam);

	// Make the line beam halo.
	client_entity_t* beam2 = ClientEntity_new(-1, CEF_DONT_LINK, origin, NULL, 500);

	beam2->r.model = &hell_models[1];
	beam2->r.frame = 1;
	beam2->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	beam2->r.scale = beam->r.scale * 1.4f;
	beam2->r.tile = beam->r.tile;
	beam2->r.tileoffset = beam->r.tileoffset;
	beam2->radius = 40.0f;
	beam2->alpha = 0.95f;
	beam2->d_alpha = -4.0f;
	VectorCopy(origin, beam2->r.startpos);
	VectorCopy(endpos, beam2->r.endpos);
	beam2->r.spriteType = SPRITE_LINE;

	AddEffect(NULL, beam2);

	vec3_t delta_pos;
	const int count = GetScaledCount((int)(len / 16.0f), 0.3f);
	VectorScale(fwd, len / (float)count, delta_pos);

	vec3_t cur_pos = VEC3_INIT(origin);

	// Make the particles along the beam.
	for (int i = 0; i < count; i++)
	{
		client_particle_t* spark = ClientParticle_new(PART_16x16_SPARK_R, color_white, 500);

		spark->scale = flrand(8.0f, 12.0f);
		spark->d_scale = -2.0f * spark->scale;
		spark->acceleration[2] = 80.0f;
		VectorCopy(cur_pos, spark->origin);
		VectorRandomSet(spark->velocity, HELLLASER_SPEED); //mxd

		AddParticleToList(beam, spark);
		Vec3AddAssign(delta_pos, cur_pos);
	}

	VectorSubtract(beam->r.endpos, beam->r.startpos, dir);
	VectorNormalize(dir);

	if (flags & CEF_FLAG7)
		FXClientScorchmark(beam->r.endpos, dir);

	if (flags & CEF_FLAG6)
		HellLaserBurn(endpos, fwd, right, up);
}