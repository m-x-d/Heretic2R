//
// Random.c
//
// Copyright 1998 Raven Software
//

#include "Random.h"
#include <stdlib.h>

//mxd. Not a part of original H2 logic. Borrowed from Q2 source for simplicity's sake... Returns a value in [0.0 .. 1.0] range
static float frand(void)
{
	return (rand() & 32767) * (1.0f / 32767);
}

H2COMMON_API float flrand(const float min, const float max)
{
	return (max - min) * frand() + min;
}

H2COMMON_API int irand(const int min, const int max)
{
	return (int)((float)(max - min + 1) * frand()) + min;
}