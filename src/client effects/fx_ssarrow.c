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

void FXSsithraArrowGlow(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW), vec3_origin, NULL, Q_ftol(fxi.cls->frametime * 2000.0f));

	glow->color.c = 0xff00ffff;
	glow->dlight = CE_DLight_new(glow->color, 150.0f, 0.0f);
	glow->refMask = 1 << CORVUS_LEFTHAND;
	glow->AddToView = OffsetLinkedEntityUpdatePlacement;
	glow->Update = FXSsithraArrowGlowThink;

	AddEffect(owner, glow);
}

static qboolean FXSsithraArrowMissileThink(client_entity_t* missile, centity_t* owner)
{
	vec3_t diff;
	VectorSubtract(missile->r.origin, missile->origin, diff);
	Vec3ScaleAssign(1.0f / SSARROW_NUM_TRAIL_PARICLES, diff);

	vec3_t cur_pos = { 0 };

	for (int i = 0; i < SSARROW_NUM_TRAIL_PARICLES; i++)
	{
		vec3_t org;
		VectorRandomCopy(missile->origin, org, SSARROW_PARTICLE_OFFSET);
		Vec3AddAssign(cur_pos, org);

		client_entity_t* ce = ClientEntity_new(-1, 0, org, NULL, 500);

		ce->radius = 16.0f;
		ce->r.model = &ssarrow_models[0]; // steampuff sprite.
		ce->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
		ce->r.color = missile->color;
		ce->r.scale = 0.1f;
		ce->d_scale = 2.0f;
		ce->d_alpha = -2.2f;

		AddEffect(NULL, ce);
		Vec3AddAssign(diff, cur_pos);
	}

	// Remember for even spread of particles.
	VectorCopy(missile->r.origin, missile->origin);

	return true;
}

void FXSsithraArrowMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 100);
	fxi.GetEffect(owner, flags, "v", missile->velocity);

	missile->radius = 32.0f;
	missile->r.model = &ssarrow_models[1]; // sitharrow model.
	missile->color.c = 0xff00ffff;
	missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);

	vec3_t dir;
	VectorCopy(missile->velocity, dir);
	VectorNormalize(dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->r.angles[PITCH] -= ANGLE_90;
	missile->r.angles[YAW] += ANGLE_90;
	missile->Update = FXSsithraArrowMissileThink;

	AddEffect(owner, missile);

	fxi.S_StartSound(missile->r.origin, -1, CHAN_WEAPON, fxi.S_RegisterSound("monsters/pssithra/arrow1.wav"), 1.0f, ATTN_NORM, 0.0f);
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