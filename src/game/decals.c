//
// decals.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "Vector.h"

// ************************************************************************************************
// IsDecalApplicable
// ************************************************************************************************

// planeDir parameter will become redundant when all scorchmarks are spawned on the client

qboolean IsDecalApplicable(edict_t *owner, edict_t *target, vec3_t origin, csurface_t *surface,cplane_t *plane, vec3_t planeDir)
{
	int		contents;

#if DEMO_CODE
	if(deathmatch->value)
		return(false);
#endif


	if(!plane)
	{
		return(false);
	}
	if(!surface)
	{
		return(false);
	}
	if(!Vec3NotZero(plane->normal))
	{
		return(false);
	}
	// this is a brush
	if (target->s.number)
	{	
		return(false);
	}
	// target is damageable
	if(target->takedamage)
	{
		return(false);
	}
	if(surface->flags & SURF_SKY)
	{
		return(false);
	}
	if(!target)
	{
		return(false);
	}
	contents = gi.pointcontents(origin);
	if(contents & MASK_WATER)
	{
		return(false);
	}
	if(target->solid == SOLID_BSP)
	{
		if(contents & CONTENTS_TRANSLUCENT)
		{
			return(false);
		}
	}
	if(planeDir)
	{
		VectorCopy(plane->normal, planeDir);
	}
	return(true);
}