//
// fx_lensflare.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Client Entities.h"
#include "Particle.h"
#include "ResourceManager.h"
#include "FX.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "random.h"
#include "Utilities.h"
#include "g_playstats.h"

#define	NUM_LENS_MODELS	7

static struct model_s *flare_models[NUM_LENS_MODELS];

void PreCacheFlare()
{
	int		i;
	char	model[128];

	flare_models[0] = fxi.RegisterModel("sprites/lens/blind1.sp2");
	for(i = 1; i < NUM_LENS_MODELS; i++)
	{
		sprintf(model, "sprites/lens/flare%d.sp2", i);
		flare_models[i] = fxi.RegisterModel(model);
	}
}

// FIXME: These need to interpolate their movement so as to not do snap position changes

// ************************************************************************************************
// FXFlareThink
// ************************************************************************************************

static qboolean FXFlareThink(struct client_entity_s *self,centity_t *owner)
{
	float			dot, near_clip, dist;

	vec3_t			from, at, center, view_dir, light_dir, light, axis, dx, dy, tmp, t_axis, pos1, pos2;

	vec3_t			mins = { 0, 0, 0}, 
					maxs = { 0, 0, 0};
	
	trace_t			trace;

	if ( ( self->LifeTime > 0 ) && ( self->LifeTime < fxi.cl->time ) )
		return false;

	//Determine Visibility
	fxi.Trace(	fxi.cl->refdef.vieworg, 
				mins, 
				maxs, 
				self->direction, 
				(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER), 
				CEF_CLIP_TO_WORLD, 
				&trace);
	
	if (trace.fraction < 1 && !(trace.surface->flags & SURF_SKY))
	{	
		self->flags |= CEF_NO_DRAW;
		return true;
	}
	
	if (self->up[1])
	{
		if (self->flags & CEF_NO_DRAW)
			self->flags &= ~CEF_NO_DRAW;

		AngleVectors (fxi.cl->refdef.viewangles, pos1, NULL, NULL);
		VectorSubtract (self->direction, fxi.cl->refdef.vieworg, pos2);
		VectorNormalize (pos2);
		
		dot = DotProduct (pos1, pos2);

		if (dot < 0.75)
		{
			self->flags |= CEF_NO_DRAW;
			return true;
		}

		self->alpha = self->up[2] - ((1 - dot) * 2);

		if (self->alpha > 1)
			self->alpha = 1;

		if (self->alpha < 0.1)
			self->alpha = 0.1;

		VectorMA(fxi.cl->refdef.vieworg, 16, pos2, pos1);
		VectorCopy(pos1, self->r.origin);
		return true;
	}
	
	AngleVectors (fxi.cl->refdef.viewangles, pos1, NULL, NULL);
	VectorSubtract (self->direction, fxi.cl->refdef.vieworg, pos2);
	VectorNormalize (pos2);
	
	dot = DotProduct (pos1, pos2);

	if (dot < 0.75)
	{
		self->flags |= CEF_NO_DRAW;
		return true;
	}
		
	if (self->flags & CEF_NO_DRAW)
		self->flags &= ~CEF_NO_DRAW;

	VectorCopy(self->direction, from);
	VectorCopy(fxi.cl->refdef.vieworg, at);

	VectorSubtract(from, at, view_dir);
	dist = VectorNormalize(view_dir);
	
	VectorMA(at, 250, view_dir, from);

	VectorSubtract(from, at, view_dir);
	dist = VectorNormalize(view_dir);

	AngleVectors(fxi.cl->refdef.viewangles, light, NULL, NULL);
	
	VectorScale(light, dist, light);
	VectorAdd(from, light, light);
	
	near_clip = 1.01f;
	
	VectorScale(view_dir, near_clip, view_dir);
	VectorAdd(from, view_dir, center);

	VectorSubtract(light, from, light_dir);
	VectorNormalize(light_dir);

	dot = (DotProduct(view_dir, light_dir));
	VectorScale(light_dir, near_clip, tmp);
	VectorScale(tmp, 1.0f / dot, tmp);
	VectorAdd(tmp, from, light);

	VectorSubtract(light, center, axis);

	VectorCopy(axis, dx);
	VectorNormalize(dx);
	CrossProduct(dx, view_dir, dy);

	VectorScale(axis, self->up[0]*1000, t_axis);
	VectorAdd(center, t_axis, self->r.origin);
	return (true);
}

// ************************************************************************************************
// FXFlareThinkAttached
// ************************************************************************************************

static qboolean FXFlareThinkAttached(struct client_entity_s *self,centity_t *owner)
{
	float			dot, near_clip, dist;
	float			lerp, oldtime, newtime;

	vec3_t			from, at, center, view_dir, light_dir, light, axis, dx, dy, tmp, t_axis, pos1, pos2, vec_diff;

	vec3_t			mins = { 0, 0, 0}, 
					maxs = { 0, 0, 0};
	
	trace_t			trace;
	centity_t		*fake_owner = (centity_t *) self->extra;


	if ( ( self->LifeTime > 0 ) && ( self->LifeTime < fxi.cl->time ) )
		return (false);

	if(fake_owner->current.effects & EF_DISABLE_EXTRA_FX)
		return (false);

//Interpolate- why am I only getting 2 frames of interpolation?
	oldtime = self->lastThinkTime/100.0;
	newtime = fxi.cl->time/100.0;

	if ((int)oldtime < (int)newtime)
	{
		VectorCopy(self->endpos2, self->startpos2);
		VectorCopy(fake_owner->current.origin, self->endpos2);//where I need to be
		self->lastThinkTime = fxi.cl->time;
	}

	lerp = newtime - (int)newtime;

	VectorSubtract(self->endpos2, self->startpos2, vec_diff);//diff between last updated spot and where to be
	VectorMA(self->startpos2, lerp, vec_diff, self->direction);

	//Determine Visibility
	fxi.Trace(	fxi.cl->refdef.vieworg, 
				mins, 
				maxs, 
				self->direction,
				(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER), 
				CEF_CLIP_TO_WORLD, 
				&trace);
	
	if (trace.fraction < 1 && !(trace.surface->flags & SURF_SKY))
	{	
		self->flags |= CEF_NO_DRAW;
		return true;
	}
	
 	if (self->up[1])
	{
		if (self->flags & CEF_NO_DRAW)
			self->flags &= ~CEF_NO_DRAW;

		AngleVectors (fxi.cl->refdef.viewangles, pos1, NULL, NULL);
		VectorSubtract (self->direction, fxi.cl->refdef.vieworg, pos2);
		VectorNormalize (pos2);
		
		dot = DotProduct (pos1, pos2);

		if (dot < 0.75)
		{
			self->flags |= CEF_NO_DRAW;
			return true;
		}

		self->alpha = self->up[2] - ((1 - dot) * 2);

		if (self->alpha > 1)
			self->alpha = 1;

		if (self->alpha < 0.1)
			self->alpha = 0.1;

		VectorMA(fxi.cl->refdef.vieworg, 16, pos2, pos1);
		VectorCopy(pos1, self->r.origin);
		return true;
	}
	
	AngleVectors (fxi.cl->refdef.viewangles, pos1, NULL, NULL);
	VectorSubtract (self->direction, fxi.cl->refdef.vieworg, pos2);
	VectorNormalize (pos2);
	
	dot = DotProduct (pos1, pos2);

	if (dot < 0.75)
	{
		self->flags |= CEF_NO_DRAW;
		return true;
	}
		
	if (self->flags & CEF_NO_DRAW)
		self->flags &= ~CEF_NO_DRAW;

	VectorCopy(self->direction, from);
	VectorCopy(fxi.cl->refdef.vieworg, at);

	VectorSubtract(from, at, view_dir);
	dist = VectorNormalize(view_dir);
	
	VectorMA(at, 250, view_dir, from);

	VectorSubtract(from, at, view_dir);
	dist = VectorNormalize(view_dir);

	AngleVectors(fxi.cl->refdef.viewangles, light, NULL, NULL);
	
	VectorScale(light, dist, light);
	VectorAdd(from, light, light);
	
	near_clip = 1.01f;
	
	VectorScale(view_dir, near_clip, view_dir);
	VectorAdd(from, view_dir, center);

	VectorSubtract(light, from, light_dir);
	VectorNormalize(light_dir);

	dot = (DotProduct(view_dir, light_dir));
	VectorScale(light_dir, near_clip, tmp);
	VectorScale(tmp, 1.0f / dot, tmp);
	VectorAdd(tmp, from, light);

	VectorSubtract(light, center, axis);

	VectorCopy(axis, dx);
	VectorNormalize(dx);
	CrossProduct(dx, view_dir, dy);

	VectorScale(axis, self->up[0]*1000, t_axis);
	VectorAdd(center, t_axis, self->r.origin);
	
	return true;
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