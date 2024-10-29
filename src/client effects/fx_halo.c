//
// fx_halo.c
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

#define NUM_HALO_MODELS		3
static struct model_s *halo_models[NUM_HALO_MODELS];

void PreCacheHalos()
{
	halo_models[0] = fxi.RegisterModel("sprites/lens/halo1.sp2");
	halo_models[1] = fxi.RegisterModel("sprites/lens/halo2.sp2");
	halo_models[2] = fxi.RegisterModel("sprites/lens/halo3.sp2");
}

// ************************************************************************************************
// FXHaloThink
// ************************************************************************************************

static qboolean FXHaloThink(struct client_entity_s *self,centity_t *Owner)
{
	float			dot, dist, camdist;

	vec3_t			mins = { 0, 0, 0}, 
					maxs = { 0, 0, 0},
					pos1, pos2,
					ent_vec, light_vec, res_vec, org;
	
	trace_t			trace;
	entity_state_t	*es;
	
	// Effect will be deleted if CEF_DISAPPEARED flag set
	self->flags &= ~CEF_DISAPPEARED;
	// Default to nodraw
	self->flags |= CEF_NO_DRAW;

	AngleVectors (fxi.cl->refdef.viewangles, pos1, NULL, NULL);
	VectorSubtract (self->r.origin, fxi.cl->refdef.vieworg, pos2);
	VectorNormalize (pos2);
	
	dot = DotProduct (pos1, pos2);

	if (dot < 0.75)
	{
		return true;
	}

	VectorSubtract(self->r.origin, fxi.cl->refdef.vieworg, pos1);
	dist = VectorNormalize(pos1);

	if (dist > 1024)
	{
		return true;		
	}

	//Determine Visibility
	fxi.Trace(	self->r.origin, 
				mins, 
				maxs, 
				fxi.cl->refdef.vieworg, 
				(CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER), 
				CEF_CLIP_TO_ALL, 
				&trace);

	if (trace.fraction < 1.0)	//Hit something
	{	
		if ((trace.ent != ((struct edict_s *)-1)))	//Model
		{
			es = (entity_state_t *) trace.ent;

			if ((fxi.cl->playernum + 1) == es->number)
			{
				VectorCopy(es->origin, org);
				org[2] += 8;

				VectorSubtract(fxi.cl->refdef.vieworg, self->r.origin, light_vec);
				VectorSubtract(self->r.origin, org, ent_vec);
				VectorSubtract(fxi.cl->refdef.vieworg, org, pos1);

				camdist = VectorNormalize(pos1);

				dist = VectorNormalize(ent_vec);
								
				VectorNormalize(light_vec);
				VectorMA(self->r.origin, dist, light_vec, res_vec);
				VectorSubtract(org, res_vec, pos1);

				dist = VectorNormalize(pos1);

				if (dist < (10 + (camdist/100)))
				{
					if (self->alpha > 0.25)
					{
						self->alpha -= 0.25;
						return true;
					}
					else
					{
						return true;
					}
				}
				else
				{
				  	self->flags &= ~CEF_NO_DRAW;

					if (self->alpha < 0.5)
						self->alpha += 0.25;
					return true;
				}
			}
			else
			{
				return true;
			}			
		}
		else	//Hit a brush
		{
			return true;
		}
	}
			
	self->flags &= ~CEF_NO_DRAW;

	if (self->alpha < 0.5)
		self->alpha += 0.25;

	return (true);
}

// ************************************************************************************************
// FXHalo
// ************************************************************************************************

void FXHalo(centity_t *Owner,int Type,int Flags,vec3_t Origin)
{
	client_entity_t	*halo;


	// no halo's for normal or low details.. they are really expensive in traces.
	if (r_detail->value <= DETAIL_NORMAL)
		return;

	halo = ClientEntity_new(FX_HALO, Flags | CEF_NO_DRAW | CEF_VIEWSTATUSCHANGED, Origin, NULL, 100);

	// decide which halo image to use
	if (Flags & CEF_FLAG6)
		halo->r.model = halo_models + 1;
	else
		halo->r.model = halo_models;

	halo->r.flags |= RF_FULLBRIGHT | RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_NODEPTHTEST;

	// to figure out tint, we only want the top two bits of flags
	Flags &= CEF_FLAG7 | CEF_FLAG8;

	switch (Flags)
	{
	case CEF_FLAG7:
		halo->r.color.r = 90;
		halo->r.color.g = 90;		
		halo->r.color.b = 175;
		break;
	case CEF_FLAG8:
		halo->r.color.r = 190;
		halo->r.color.g = 180;		
		halo->r.color.b = 16;
		break;
	case CEF_FLAG7|CEF_FLAG8:
		halo->r.color.r = 255;
		halo->r.color.g = 255;		
		halo->r.color.b = 255;
		break;
	case 0:
	default:
		halo->r.color.r = 148;
		halo->r.color.g = 132;		
		halo->r.color.b = 82;
		break;
	}
	halo->alpha = 0.6;
	halo->Update = FXHaloThink;

	AddEffect(Owner, halo);
}

// end
