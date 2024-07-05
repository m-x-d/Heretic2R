//
// q_shared.c
//
// Copyright 1998 Raven Software
//

#include "q_shared.h"

// Transforms vector to screen space?
void TransformVector(vec3_t v, vec3_t out)
{
	NOT_IMPLEMENTED
}

// Returns 1, 2, or 1 + 2
int BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, const struct cplane_s *p)
{
	// Fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;

		if (p->dist >= emaxs[p->type])
			return 2;

		return 3;
	}

	// General case
	//mxd. Replaced with optimized Quakespasm version (https://github.com/sezero/quakespasm/commit/c0cbbcacb4281fa89384652224ffd357278134c7#diff-1163c63a27fca3d203c17855d43060e4d365c72e2b0b5f8a416cf67763d03b82) 
	const int xneg = p->signbits & 1;
	const int yneg = (p->signbits >> 1) & 1;
	const int zneg = (p->signbits >> 2) & 1;

	const float dist1 = p->normal[0] * (xneg ? emins : emaxs)[0] +
						p->normal[1] * (yneg ? emins : emaxs)[1] +
						p->normal[2] * (zneg ? emins : emaxs)[2];

	const float dist2 = p->normal[0] * (xneg ? emaxs : emins)[0] +
						p->normal[1] * (yneg ? emaxs : emins)[1] +
						p->normal[2] * (zneg ? emaxs : emins)[2];

	assert(!(p->signbits & ~7));
	
	int sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	assert(sides != 0);

	return sides;
}