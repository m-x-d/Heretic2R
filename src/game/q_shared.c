//
// q_shared.c
//
// Copyright 1998 Raven Software
//

#include "q_shared.h"

// Returns 1, 2, or 1 + 2.
int BoxOnPlaneSide(const vec3_t emins, const vec3_t emaxs, const cplane_t* plane)
{
	// Fast axial cases.
	if (plane->type < 3)
	{
		if (plane->dist <= emins[plane->type])
			return 1;

		if (plane->dist >= emaxs[plane->type])
			return 2;

		return 3;
	}

	// General case.
	//mxd. Replaced with optimized Quakespasm version (https://github.com/sezero/quakespasm/commit/c0cbbcacb4281fa89384652224ffd357278134c7#diff-1163c63a27fca3d203c17855d43060e4d365c72e2b0b5f8a416cf67763d03b82) 
	const int xneg = plane->signbits & 1;
	const int yneg = (plane->signbits >> 1) & 1;
	const int zneg = (plane->signbits >> 2) & 1;

	const float dist1 = plane->normal[0] * (xneg ? emins : emaxs)[0] +
						plane->normal[1] * (yneg ? emins : emaxs)[1] +
						plane->normal[2] * (zneg ? emins : emaxs)[2];

	const float dist2 = plane->normal[0] * (xneg ? emaxs : emins)[0] +
						plane->normal[1] * (yneg ? emaxs : emins)[1] +
						plane->normal[2] * (zneg ? emaxs : emins)[2];

	assert(!(plane->signbits & ~7));
	
	int sides = 0;

	if (dist1 >= plane->dist)
		sides = 1;

	if (dist2 < plane->dist)
		sides |= 2;

	assert(sides != 0);

	return sides;
}