//
// decals.c
//
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "Vector.h"

// 'plane_dir' parameter will become redundant when all scorchmarks are spawned on the client.
qboolean IsDecalApplicable(const edict_t* target, const vec3_t origin, const csurface_t* surface, const cplane_t* plane, vec3_t plane_dir)
{
	if (plane == NULL || Vec3IsZero(plane->normal) || surface == NULL || (surface->flags & SURF_SKY) || target == NULL)
		return false;

	// Target is not a brush or is damageable.
	if (target->s.number > 0 || target->takedamage != DAMAGE_NO)
		return false;

	const int contents = gi.pointcontents(origin);
	if ((contents & MASK_WATER) || (target->solid == SOLID_BSP && (contents & CONTENTS_TRANSLUCENT)))
		return false;

	if (plane_dir != NULL)
		VectorCopy(plane->normal, plane_dir);

	return true;
}