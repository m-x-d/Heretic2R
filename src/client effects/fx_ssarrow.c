//
// fx_SsithraArrow.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Reference.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_Dlight.h"

#define SSARROW_PARTICLE_OFFSET			5.0f
#define SSARROW_NUM_TRAIL_PARICLES		6

static struct model_s* ssarrow_models[2];

void PreCacheSsithraArrow(void)
{
	ssarrow_models[0] = fxi.RegisterModel("sprites/fx/steampuff.sp2");
	ssarrow_models[1] = fxi.RegisterModel("models/objects/projectiles/sitharrow/tris.fm");
}

static qboolean FXSsithraArrowGlowThink(struct client_entity_s* self, centity_t* owner)
{
	// Reset update time to regular after game has been given enough time to gen lerp info.
	self->updateTime = 100;
	self->dlight->intensity = 150.0f + (cosf((float)fxi.cl->time * 0.01f) * 20.0f);

	return true;
}

void FXSsithraArrowGlow(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*glow;
	vec3_t				org;

	VectorClear(org);
	glow = ClientEntity_new(type, flags | CEF_NO_DRAW, org, 0, Q_ftol(fxi.cls->frametime * 2000.0));

	glow->color.c = 0xff00ffff;
	glow->dlight = CE_DLight_new(glow->color, 150.0F, 0.0F);
	glow->Update = FXSsithraArrowGlowThink;
	glow->AddToView = OffsetLinkedEntityUpdatePlacement;			
	glow->refMask = 1 << CORVUS_LEFTHAND;

	AddEffect(owner, glow);
}

// -----------------------------------------------------------------------------------------

static qboolean FXSsithraArrowMissileThink(client_entity_t *missile, centity_t *owner)
{
	int					i;
	client_entity_t		*ce;
	vec3_t				diff, curpos, org;

	VectorSubtract(missile->r.origin, missile->origin, diff);
	Vec3ScaleAssign((1.0 / SSARROW_NUM_TRAIL_PARICLES), diff);
	VectorClear(curpos);

	for(i = 0; i < SSARROW_NUM_TRAIL_PARICLES; i++)
	{
		VectorRandomCopy(missile->origin, org, SSARROW_PARTICLE_OFFSET);
		Vec3AddAssign(curpos, org);
		ce = ClientEntity_new(-1, 0, org, NULL, 500);
		ce->r.model = ssarrow_models;		// Can be a particle now
		ce->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		ce->r.color = missile->color;
		ce->radius = 16.0F;
		ce->r.scale = 0.1F;
		ce->d_scale = 2.0F;
		ce->d_alpha = -2.2F;
		AddEffect(NULL, ce);

		Vec3AddAssign(diff, curpos);
	}
	// Remember for even spread of particles
	VectorCopy(missile->r.origin, missile->origin);
	return(true);
}

void FXSsithraArrowMissile(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*missile;
	vec3_t				temp;

	missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);
	fxi.GetEffect(owner, flags, "v", missile->velocity);

	VectorCopy(missile->velocity, temp);
	VectorNormalize(temp);
	AnglesFromDir(temp, missile->r.angles);
	missile->r.angles[PITCH] -= ANGLE_90;
	missile->r.angles[YAW] += ANGLE_90;

	missile->r.model = ssarrow_models + 1;
	missile->Update = FXSsithraArrowMissileThink;
	missile->radius = 32.0F;
	missile->color.c = 0xff00ffff;
	missile->dlight = CE_DLight_new(missile->color, 150.0F, 00.0F);
	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/pssithra/arrow1.wav"), 1, ATTN_NORM, 0);
}

// -----------------------------------------------------------------------------------------

static qboolean FXSsithraArrowDLightThink(client_entity_t *dlight, centity_t *owner)
{
	dlight->dlight->intensity -= 10.0F;
	if(dlight->dlight->intensity < 0.0F)
		return(false);

	return(true);
}

void FXSsithraArrowExplode(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*dlight;
	paletteRGBA_t		color;

	dlight = ClientEntity_new(-1, CEF_NO_DRAW | CEF_NOMOVE, origin, NULL, 100);
	color.c = 0xff00ffff;
	dlight->dlight = CE_DLight_new(color, 150.0F, 0.0F);
	dlight->Update = FXSsithraArrowDLightThink;
	AddEffect(NULL, dlight);

//NOTE: depends on impacted surface & material and if exploding arrow
	fxi.S_StartSound(origin, -1, CHAN_AUTO, fxi.S_RegisterSound("weapons/ramphit1.wav"), 1, ATTN_NORM, 0);
}

// end