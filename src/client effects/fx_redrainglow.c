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
#include "Vector.h"
#include "Matrix.h"
#include "random.h"
#include "Utilities.h"
#include "Ambient effects.h"
#include "ce_Dlight.h"
#include "reference.h"
#include "g_playstats.h"

#define CLOUD_GEN_RAD	30.0


// -------------------------------------------------------
static qboolean FXRedRainGlowThink(struct client_entity_s *self, centity_t *owner)
{
	client_particle_t	*spark;
	vec3_t				vel, orgleft, orgright, atkvector;
	int					i;
	paletteRGBA_t		color;
	matrix3_t			rotation;
	int					sparktype;

	// If we've timed out, stop the effect (allow for fading)
	if (self->LifeTime > 0)
	{
		if (self->LifeTime < fxi.cl->time)
		{	// Lifetime is up
			self->Update=RemoveSelfAI;
			self->updateTime = fxi.cl->time + 500;
			return true;
		}
	}
	else 
	{	// If we're not on a time limit, check the EF flag
		if (!(owner->current.effects & EF_TRAILS_ENABLED))
		{
			self->Update=RemoveSelfAI;
			self->updateTime = fxi.cl->time + 500;
			return true;
		}
	}

	// This tells if we are wasting our time, because the reference points are culled.
	if (!RefPointsValid(owner))
		return true;

	// Reset update time to regular after game has been given enough time to gen lerp info
	if (r_detail->value == DETAIL_LOW)
		self->updateTime = 200;
	else
	if (r_detail->value == DETAIL_NORMAL)
		self->updateTime = 150;
	else
		self->updateTime = 100;

	VectorCopy(owner->origin, self->r.origin);

	// Create a rotation matrix
	Matrix3FromAngles(owner->lerp_angles, rotation);
	// Let's take the origin and transform it to the proper coordinate offset from the owner's origin.
	Matrix3MultByVec3(rotation, owner->referenceInfo->references[CORVUS_LEFTHAND].placement.origin, orgleft);
	Matrix3MultByVec3(rotation, owner->referenceInfo->references[CORVUS_RIGHTHAND].placement.origin, orgright);
	VectorSubtract(orgleft, orgright, atkvector);
	if (self->dlight)
		self->dlight->intensity = flrand(140.0, 160.0);

	if (self->SpawnInfo == 1)
	{	// Powered up, fire sparks
		sparktype = PART_16x16_SPARK_G;
	}
	else
	{
		sparktype = PART_16x16_SPARK_R;
	}

	color.c = 0xffffffff;
	for(i = 0; i < 2; i++)
	{
		// Calc spherical offset around left hand ref point
		VectorSet(vel, flrand(-1.0, 1.0), flrand(-1.0, 1.0), flrand(-1.0, 1.0));
		if(Vec3IsZero(vel))
			vel[2] = 1.0;			// Safety in case flrand gens all zeros (VERY unlikely)
		VectorNormalize(vel);
		VectorScale(vel, CLOUD_GEN_RAD, vel);
		VectorSubtract(vel, atkvector, vel);

		spark = ClientParticle_new(sparktype, color, 500);
		VectorAdd(vel, orgleft, spark->origin);

		VectorScale(vel, -0.125, spark->velocity);
		VectorScale(vel, -6.0, spark->acceleration);

		spark->scale = 12.0F;
		spark->d_scale = -16.0F;
		spark->color.a = 4;
		spark->d_alpha = 500.0F;
		spark->duration = 500;

		AddParticleToList(self, spark);
	}

	// Add the core to the effect
	spark = ClientParticle_new(sparktype, color, 500);
	VectorCopy(orgleft, spark->origin);
	VectorSet(spark->velocity, flrand(-16.0, 16.0), flrand(-16.0, 16.0), flrand(-8.0, 24.0));
	VectorMA(spark->velocity, -1.75, atkvector, spark->velocity);
	spark->scale = 10.0F;
	spark->d_scale = 10.0F;
	spark->d_alpha = -500.0F;

	AddParticleToList(self, spark);

	return(true);
}


void FXRedRainGlow(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
	vec3_t				org;
	char				lifetime;

	VectorClear(org);

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_REDRAINGLOW].formatString, &lifetime);

	glow = ClientEntity_new(type, flags | CEF_NO_DRAW, org, 0, Q_ftol(fxi.cls->frametime * 2000.0));
	glow->flags|=CEF_NO_DRAW | CEF_OWNERS_ORIGIN | CEF_ADDITIVE_PARTS;

	glow->Update = FXRedRainGlowThink;
	glow->AddToView = LinkedEntityUpdatePlacement;			
	glow->radius = 128;
	if (flags & CEF_FLAG6)
	{	// powered up, green-yellow glow.
		glow->SpawnInfo = 1;
		glow->color.c = 0xff00ff80;
	}
	else
	{
		glow->color.c = 0xff0000ff;
	}
	if (r_detail->value != DETAIL_LOW)
		glow->dlight = CE_DLight_new(glow->color, 150.0F, 0.0F);
	
  	if (lifetime > 0)
  		glow->LifeTime = fxi.cl->time + (lifetime * 100);
  	else
  		glow->LifeTime = -1;


	AddEffect(owner, glow);
}

// end
