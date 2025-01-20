//
// fx_lensflare.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "g_playstats.h"

#define NUM_LENS_MODELS 7
static struct model_s* flare_models[NUM_LENS_MODELS];

void PreCacheFlare(void)
{
	flare_models[0] = fxi.RegisterModel("sprites/lens/blind1.sp2");

	for (int i = 1; i < NUM_LENS_MODELS; i++)
	{
		char model[128];
		sprintf_s(model, sizeof(model), "sprites/lens/flare%i.sp2", i); //mxd. sprintf -> sprintf_s.
		flare_models[i] = fxi.RegisterModel(model);
	}
}

//mxd. Added to reduce code duplication. //TODO: the logic is kinda broken, at least for env_sun1 entity...
static qboolean UpdateFlareOrigin(struct client_entity_s* self)
{
	// Determine visibility.
	trace_t tr;
	const int brushmask = CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER;
	fxi.Trace(fxi.cl->refdef.vieworg, vec3_origin, vec3_origin, self->direction, brushmask, CEF_CLIP_TO_WORLD, &tr);

	if (tr.fraction < 1.0f && !(tr.surface->flags & SURF_SKY))
	{
		self->flags |= CEF_NO_DRAW;
		return true;
	}

	vec3_t view_fwd;
	AngleVectors(fxi.cl->refdef.viewangles, view_fwd, NULL, NULL);

	vec3_t halo_dir;
	VectorSubtract(self->direction, fxi.cl->refdef.vieworg, halo_dir);
	VectorNormalize(halo_dir);

	const float view_dot = DotProduct(view_fwd, halo_dir);

	if (view_dot < 0.75f)
	{
		self->flags |= CEF_NO_DRAW;
		return true;
	}

	if (self->flags & CEF_NO_DRAW)
		self->flags &= ~CEF_NO_DRAW;

	if (self->up[1] != 0.0f) // Halo sprite.
	{
		self->alpha = self->up[2] - ((1.0f - view_dot) * 2.0f);
		Clamp(self->alpha, 0.1f, 1.0f);

		VectorMA(fxi.cl->refdef.vieworg, 16.0f, halo_dir, self->r.origin);

		return true;
	}

	// Lens flare sprite.
	vec3_t view_dir;
	VectorSubtract(self->direction, fxi.cl->refdef.vieworg, view_dir);
	VectorNormalize(view_dir);

	vec3_t view_offset;
	VectorMA(fxi.cl->refdef.vieworg, 250.0f, view_dir, view_offset);

	VectorSubtract(view_offset, fxi.cl->refdef.vieworg, view_dir);
	const float view_dist = VectorNormalize(view_dir);

	vec3_t fwd_offset;
	VectorScale(view_fwd, view_dist, fwd_offset);

	vec3_t light_offset;
	VectorAdd(view_offset, fwd_offset, light_offset);

	const float near_clip = 1.01f;
	VectorScale(view_dir, near_clip, view_dir);

	vec3_t center;
	VectorAdd(view_offset, view_dir, center);

	vec3_t light_dir;
	VectorSubtract(light_offset, view_offset, light_dir);
	VectorNormalize(light_dir);

	const float light_dot = DotProduct(view_dir, light_dir);

	vec3_t tmp;
	VectorScale(light_dir, near_clip, tmp);
	VectorScale(tmp, 1.0f / light_dot, tmp);

	vec3_t light_pos;
	VectorAdd(tmp, view_offset, light_pos);

	vec3_t axis;
	VectorSubtract(light_pos, center, axis);

	vec3_t dx;
	VectorCopy(axis, dx);
	VectorNormalize(dx);

	vec3_t dy;
	CrossProduct(dx, view_dir, dy);

	vec3_t t_axis;
	VectorScale(axis, self->up[0] * 1000.0f, t_axis);
	VectorAdd(center, t_axis, self->r.origin);

	return true;
}

// FIXME: These need to interpolate their movement so as to not do snap position changes.
static qboolean FXFlareThink(struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime > 0 && self->LifeTime < fxi.cl->time)
		return false;

	return UpdateFlareOrigin(self); //mxd
}

static qboolean FXFlareThinkAttached(struct client_entity_s* self, centity_t* owner)
{
	if (self->LifeTime > 0 && self->LifeTime < fxi.cl->time)
		return false;

	const centity_t* fake_owner = (centity_t*)self->extra;
	if (fake_owner->current.effects & EF_DISABLE_EXTRA_FX)
		return false;

	// Interpolate. Why am I only getting 2 frames of interpolation?
	const float oldtime = (float)self->lastThinkTime / 100.0f;
	const float newtime = (float)fxi.cl->time / 100.0f;

	if ((int)oldtime < (int)newtime)
	{
		VectorCopy(self->endpos2, self->startpos2);
		VectorCopy(fake_owner->current.origin, self->endpos2); // Where I need to be.
		self->lastThinkTime = fxi.cl->time;
	}

	const float lerp = newtime - (int)newtime;

	vec3_t diff;
	VectorSubtract(self->endpos2, self->startpos2, diff); // Diff between last updated spot and where to be.
	VectorMA(self->startpos2, lerp, diff, self->direction);

	return UpdateFlareOrigin(self); //mxd
}

// ************************************************************************************************
// FXLensFlare
// ************************************************************************************************

float flare_loc[] = { 1.0, 0.7, 0.3, 0.1, 0.0, -0.2,
					  1.0, 0.8, 0.6, 0.2, 0.0, -0.4 };

float flare_scale[] = {2, 1.75, 1.5, 1.25, 1.5, 1.75, 2, 2 };

void FXLensFlare(centity_t *owner,int Type,int Flags,vec3_t Origin)
{
	int					Count, I;
	client_entity_t		*Explosion;
	int					useOther;
	float				alpha;
	paletteRGBA_t		tint;
	
	Count = 7;
	
	fxi.GetEffect(owner, Flags, clientEffectSpawners[FX_LENSFLARE].formatString, &tint.r, &tint.g, &tint.b, &alpha);

	// no lens flares in low detail
	if(r_detail->value <= DETAIL_NORMAL)
		return;

	if (Flags & CEF_FLAG6)
		useOther = 6;
	else
		useOther = 0;

	for(I=0;I<Count;I++)
	{	
		Explosion=ClientEntity_new(FX_LENSFLARE,
								   0,
								   Origin,
								   NULL,
								   17);
		
		VectorSet(Explosion->up, 0.0f, 0.0f, 0.0f);

	  	Explosion->r.color.r = tint.r;
	  	Explosion->r.color.g = tint.g;
	  	Explosion->r.color.b = tint.b;
		Explosion->alpha=alpha;

		switch (I)
		{
			case 0:
				Explosion->r.model = flare_models + 1;
			break;
			case 1:
				Explosion->r.model = flare_models + 2;
			break;
			case 2:
				Explosion->r.model = flare_models + 4;
			break;
			case 3:
				Explosion->r.model = flare_models + 3;
			break;
			case 4:
				Explosion->r.model = flare_models + 6;
			break;
			case 5:
				Explosion->r.model = flare_models + 3;
			break;
			case 6:
				Explosion->r.model = flare_models;
				Explosion->up[1] = 1;
				Explosion->up[2] = alpha;
				if (Flags & CEF_FLAG8)
				{
					Explosion->r.color.r=255;
					Explosion->r.color.g=255;
					Explosion->r.color.b=255;
				}
			break;
		}

		if (Flags & CEF_FLAG8)
			Explosion->LifeTime = 4000 + fxi.cl->time;


		Explosion->r.flags|=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA|RF_NODEPTHTEST;
		Explosion->Scale=flare_scale[I];
		Explosion->r.frame=0;
		Explosion->d_scale=0.0;
		Explosion->d_alpha=0.0;
		Explosion->NoOfAnimFrames=1;
		Explosion->Update=FXFlareThink;
		VectorCopy(Origin, Explosion->direction);
		
		if (Flags & CEF_FLAG7)
		{
			Explosion->direction[0] *= 4;
			Explosion->direction[1] *= 4;
			Explosion->direction[2] *= 4;
		}

		VectorCopy(Origin, Explosion->origin);
		Explosion->up[0] = flare_loc[I + useOther];

		if (owner)
		{
/*			client_entity_t		*nullfx;
			nullfx=ClientEntity_new(FX_LENSFLARE,
									   CEF_NO_DRAW|CEF_OWNERS_ORIGIN,
									   Origin,
									   NULL,
									   1000000);
			nullfx->flags |= CEF_NO_DRAW;

			AddEffect(owner, nullfx);*/

			AddEffect(owner,Explosion);
//			Explosion->AddToView = LinkedEntityUpdatePlacement;

			Explosion->extra = (centity_t *) owner;
			Explosion->Update=FXFlareThinkAttached;
			Explosion->updateTime = 17;
			FXFlareThinkAttached(Explosion,owner);

			VectorCopy(Explosion->direction, Explosion->startpos2);
			VectorCopy(Explosion->direction, Explosion->endpos2);
			Explosion->lastThinkTime = fxi.cl->time;
		}
		else
		{
			AddEffect(NULL,Explosion);
			FXFlareThink(Explosion,NULL);
		}
	}
}


void FXClientLensFlare(centity_t *owner,int Type,int Flags,vec3_t Origin, int lifeTime, paletteRGBA_t *tint)
{
	int					Count,I;
	client_entity_t		*Explosion;
	int					useOther;
	
	Count = 7;
	
	// no lens flares in low detail
	if(r_detail->value <= DETAIL_NORMAL)
		return;

	if (Flags & CEF_FLAG6)
		useOther = 6;
	else
		useOther = 0;

	for(I=0;I<Count;I++)
	{	
		Explosion=ClientEntity_new(FX_LENSFLARE,
								   0,
								   Origin,
								   NULL,
								   20);
		
		VectorSet(Explosion->up, 0.0f, 0.0f, 0.0f);

	  	Explosion->r.color.r = tint->r;
	  	Explosion->r.color.g = tint->g;
	  	Explosion->r.color.b = tint->b;
		Explosion->alpha = (float)(tint->a)/255.0;

		switch (I)
		{
			case 0:
				Explosion->r.model = flare_models + 1;
			break;
			case 1:
				Explosion->r.model = flare_models + 2;
			break;
			case 2:
				Explosion->r.model = flare_models + 4;
			break;
			case 3:
				Explosion->r.model = flare_models + 3;
			break;
			case 4:
				Explosion->r.model = flare_models + 6;
			break;
			case 5:
				Explosion->r.model = flare_models + 3;
			break;
			case 6:
				Explosion->r.model = flare_models;
				Explosion->up[1] = 1;
				Explosion->up[2] = Explosion->alpha;
				if (Flags & CEF_FLAG8)
				{
					Explosion->r.color.r=255;
					Explosion->r.color.g=255;
					Explosion->r.color.b=255;
				}
			break;
		}

		if (lifeTime > 0)
			Explosion->LifeTime = (lifeTime*1000)+fxi.cl->time;

		Explosion->r.flags|=RF_FULLBRIGHT|RF_TRANSLUCENT|RF_TRANS_ADD|RF_TRANS_ADD_ALPHA|RF_NODEPTHTEST;
		Explosion->Scale=flare_scale[I];
		Explosion->r.frame=0;
		Explosion->d_scale=0.0;
		Explosion->d_alpha=0.0;
		Explosion->NoOfAnimFrames=1;
		Explosion->Update=FXFlareThink;
		VectorCopy(Origin, Explosion->direction);
		
		if (Flags & CEF_FLAG7)
		{
			Explosion->direction[0] *= 4;
			Explosion->direction[1] *= 4;
			Explosion->direction[2] *= 4;
		}

		VectorCopy(Origin, Explosion->origin);
		Explosion->up[0] = flare_loc[I + useOther];

		if (owner)
		{
/*			client_entity_t		*nullfx;
			nullfx=ClientEntity_new(FX_LENSFLARE,
									   CEF_NO_DRAW|CEF_OWNERS_ORIGIN,
									   Origin,
									   NULL,
									   1000000);
			nullfx->flags |= CEF_NO_DRAW;

			AddEffect(owner, nullfx);*/

			AddEffect(owner,Explosion);
//			Explosion->AddToView = LinkedEntityUpdatePlacement;

			Explosion->extra = (centity_t *) owner;
			Explosion->Update=FXFlareThinkAttached;
			Explosion->updateTime = 17;
			FXFlareThinkAttached(Explosion,owner);

			VectorCopy(Explosion->direction, Explosion->startpos2);
			VectorCopy(Explosion->direction, Explosion->endpos2);
			Explosion->lastThinkTime = fxi.cl->time;
		}
		else
		{
			AddEffect(NULL,Explosion);
			FXFlareThink(Explosion,NULL);
		}
	}
}