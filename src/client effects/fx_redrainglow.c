//
// fx_RedRainGlow.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Matrix.h"
#include "Particle.h"
#include "Random.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define CLOUD_GEN_RAD	30.0f

static qboolean RedRainGlowThink(struct client_entity_s* self, centity_t* owner)
{
	// If we've timed out, stop the effect (allow for fading). If we're not on a time limit, check the EF flag.
	if ((self->LifeTime > 0 && self->LifeTime < fxi.cl->time) || (self->LifeTime <= 0 && !(owner->current.effects & EF_TRAILS_ENABLED)))
	{
		self->Update = RemoveSelfAI;
		self->nextThinkTime = fxi.cl->time + 500; //BUGFIX: mxd. sets updateTime in original logic (makes no sense: updateTime is ADDED to fxi.cl->time in UpdateEffects()).

		return true;
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true;

	// Reset update time to regular after game has been given enough time to generate lerp info.
	if ((int)r_detail->value == DETAIL_LOW)
		self->updateTime = 200;
	else if ((int)r_detail->value == DETAIL_NORMAL)
		self->updateTime = 150;
	else
		self->updateTime = 100;

	VectorCopy(owner->origin, self->r.origin);

	// Create a rotation matrix.
	matrix3_t rotation;
	Matrix3FromAngles(owner->lerp_angles, rotation);

	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	vec3_t org_left;
	Matrix3MultByVec3(rotation, owner->referenceInfo->references[CORVUS_LEFTHAND].placement.origin, org_left);

	vec3_t org_right;
	Matrix3MultByVec3(rotation, owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, org_right);

	vec3_t attack_dir;
	VectorSubtract(org_left, org_right, attack_dir);

	if (self->dlight != NULL)
		self->dlight->intensity = flrand(140.0f, 160.0f);

	const int spark_type = ((self->SpawnInfo == 1) ? PART_16x16_SPARK_G : PART_16x16_SPARK_R); // Powered up, fire sparks when SpawnInfo = 1.

	for (int i = 0; i < 2; i++)
	{
		// Calculate spherical offset around left hand ref point.
		vec3_t vel;
		VectorRandomSet(vel, 1.0f);

		if (Vec3IsZero(vel))
			vel[2] = 1.0f; // Safety in case flrand gens all zeros (VERY unlikely).

		VectorNormalize(vel);
		VectorScale(vel, CLOUD_GEN_RAD, vel);
		VectorSubtract(vel, attack_dir, vel);

		client_particle_t* spark = ClientParticle_new(spark_type, color_white, 500);
		VectorAdd(vel, org_left, spark->origin);

		VectorScale(vel, -0.125f, spark->velocity);
		VectorScale(vel, -6.0f, spark->acceleration);

		spark->scale = 12.0f;
		spark->d_scale = -16.0f;
		spark->color.a = 4;
		spark->d_alpha = 500.0f;
		spark->duration = 500;

		AddParticleToList(self, spark);
	}

	// Add the core to the effect.
	client_particle_t* core = ClientParticle_new(spark_type, color_white, 500);
	VectorCopy(org_left, core->origin);
	VectorSet(core->velocity, flrand(-16.0f, 16.0f), flrand(-16.0f, 16.0f), flrand(-8.0f, 24.0f));
	VectorMA(core->velocity, -1.75f, attack_dir, core->velocity);
	core->scale = 10.0f;
	core->d_scale = 10.0f;
	core->d_alpha = -500.0f;

	AddParticleToList(self, core);

	return true;
}

void FXRedRainGlow(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	char lifetime;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_REDRAINGLOW].formatString, &lifetime);

	client_entity_t* glow = ClientEntity_new(type, flags, vec3_origin, NULL, Q_ftol(fxi.cls->rframetime * 2000.0f));

	glow->radius = 128.0f;
	glow->flags |= CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ADDITIVE_PARTS;
	glow->LifeTime = ((lifetime > 0) ? fxi.cl->time + lifetime * 100 : -1);

	if (flags & CEF_FLAG6)
	{
		// Powered-up, green-yellow glow.
		glow->SpawnInfo = 1;
		glow->color.c = 0xff00ff80;
	}
	else
	{
		glow->color.c = 0xff0000ff;
	}

	if ((int)r_detail->value > DETAIL_LOW)
		glow->dlight = CE_DLight_new(glow->color, 150.0f, 0.0f);

	glow->AddToView = LinkedEntityUpdatePlacement;
	glow->Update = RedRainGlowThink;

	AddEffect(owner, glow);
}