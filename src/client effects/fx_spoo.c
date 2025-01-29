//
// fx_spoo.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"

static struct model_s* spoo_models[2];

void PreCacheSpoo(void)
{
	spoo_models[0] = fxi.RegisterModel("sprites/fx/spoo.sp2");
	spoo_models[1] = fxi.RegisterModel("sprites/fx/spoo2.sp2");
}

static qboolean FXSpooTrailThink(struct client_entity_s* self, centity_t* owner)
{
	const int count = GetScaledCount(8, 0.85f);

	for (int i = 0; i < count; i++)
	{
		client_entity_t* trail = ClientEntity_new(FX_SPOO, (int)(self->flags & ~(CEF_OWNERS_ORIGIN | CEF_NO_DRAW)), owner->origin, NULL, 1000);

		trail->radius = 20.0f;
		trail->r.model = &spoo_models[irand(0, 1)];
		trail->r.flags = RF_TRANSLUCENT | RF_FULLBRIGHT;
		trail->r.scale = 0.65f;
		trail->d_scale = flrand(-4.0f, -3.5f);
		trail->d_alpha = -2.0f;
		trail->color = color_white;

		for (int c = 0; c < 3; c++)
			trail->r.origin[c] += flrand(-3.0f, 3.0f);

		VectorSet(trail->velocity, flrand(-64.0f, 64.0f), flrand(-64.0f, 64.0f), -64.0f);

		AddEffect(NULL, trail);
	}

	VectorCopy(owner->current.origin, self->startpos);

	return true;
}

void FXSpoo(centity_t *owner,int type,int Flags,vec3_t origin)
{
	client_entity_t	*Trail;
	paletteRGBA_t	LightColor={255,153,77,255};

	Trail=ClientEntity_new(type,Flags,origin,NULL,20);

	Trail->Update=FXSpooTrailThink;
	Trail->flags|=CEF_NO_DRAW;
	VectorCopy(origin, Trail->startpos);

	AddEffect(owner,Trail);

	FXSpooTrailThink(Trail,owner);
}

void FXSpooSplat(centity_t *owner,int type,int Flags,vec3_t origin)
{
	client_entity_t	*TrailEnt;
	vec3_t			dir;
	int				count;

	fxi.GetEffect(owner, Flags, clientEffectSpawners[FX_SPOO_SPLAT].formatString, &dir);

	count = GetScaledCount(16, 0.85);

	while (count--)
	{
		TrailEnt=ClientEntity_new(FX_SPOO,
								  0,
								  origin,
								  NULL,
								  1000);

		TrailEnt->r.model = spoo_models + irand(0,1);
		
		VectorRandomCopy(dir, TrailEnt->velocity, 16.0f);
		VectorNormalize(TrailEnt->velocity);
		VectorScale(TrailEnt->velocity, flrand(100.0f, 200.0f), TrailEnt->velocity);

		VectorSet(TrailEnt->acceleration, 0, 0, -128);
		
		TrailEnt->r.scale = flrand(0.75, 1.0);
		TrailEnt->alpha=1.0;
		
		TrailEnt->r.flags |= RF_TRANSLUCENT;
		
		TrailEnt->r.frame=0;
		TrailEnt->d_scale=flrand( -1.25, -1.0);
		TrailEnt->d_alpha=flrand(-1, -0.5);
		TrailEnt->color.c = 0xA0FFFFFF;
		TrailEnt->radius=20.0;
	
		AddEffect(NULL,TrailEnt);
	}
}
